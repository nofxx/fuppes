/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            libmain.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005-2010 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include "../../include/fuppes.h"

#include <iostream>
#include "Common/Common.h"
#include "Common/Exception.h"
#include "SharedConfig.h"
#include "SharedLog.h"
#include "Fuppes.h"
#include "ContentDirectory/ContentDatabase.h"
#include "ContentDirectory/VirtualContainerMgr.h"
#include "DeviceSettings/DeviceIdentificationMgr.h"
#include "Plugins/Plugin.h"
#include "Common/RegEx.h"

#ifdef WIN32
#include <shellapi.h>
#include <windows.h>
#endif

#include <string.h>
#include <string>
#include <sstream>
#include <getopt.h>
#include <cassert>

using namespace std;
using namespace fuppes;

CFuppes* pFuppes = 0;

static void printVersion(void);
static void printHelp(void);
static void printLine(const char* linePrefix, const char* line, int lineLen, bool withNewline = true);
static void printOptions(int lineLen);
static void printExamples(int lineLen);

// Warning: the two static arrays have to be aligned (long_options and fopt) so that the help printing
// function can use the same index on both.
static struct option long_options[] =
{
  /* These options set a flag. */
  /* All fuppes options take arguments  */
  {"help",               no_argument,       0, 'h'},
  {"version",            no_argument,       0, 'v'},
  {"friendly-name",      required_argument, 0, 'f'},
  {"log-level",          required_argument, 0, 'l'},
  {"log-file",           required_argument, 0, 'm'},
  {"config-file",        required_argument, 0, 'c'},
  {"database-file",      required_argument, 0, 'd'},
  {"vfolder-config-dir", required_argument, 0, 'g'},
  {"device-config-dir",  required_argument, 0, 'r'},
  {"temp-dir",           required_argument, 0, 't'},
  {"config-dir",         required_argument, 0, 'a'},
#ifndef WIN32
  {"plugin-dir",         required_argument, 0, 'p'},
  {"data-dir",           required_argument, 0, 'j'},
#endif
  {0, 0, 0, 0}
};

struct fuppes_options {
  const char* desc;
  const char* trail;
};

static struct fuppes_options fopt[] =
{
  {"print this help message.",""},
  {"print the fuppes version information.",""},
  {"set friendly name to something that you prefer.","<name>"},
  {"set log level. (0-none, 1-normal, 2-extended, 3-debug)","[0-3]"},
  {"set log file. (default: none)","<file>"},
  {"use alternate config file. This just specifies the name of the file; not the location. (default: fuppes.cfg)","<filename>"},
  {"use alternate database file. This just specifies the name of the file; not the location. (default: fuppes.db)","<filename>"},
  {"use alternate vfolder config directory. This just specifies the name of the directory; not the full path. (default: vfolders/)","<directory name>"},
  {"use alternate device config file. This just specifies the name of the directory; not the full path. (default: devices/)","<directory name>"},
#ifndef WIN32
  {"set temp directory. (default: /tmp/fuppes)","<directory>"},
  {"add additional directories to the PATH. (default: ~/.fuppes/)","<directory>"},
  {"add another directory, containing fuppes plugins, to the path. (default: $prefix/lib/fuppes)","<directory>"},
  {"directory containing fuppes images and css. (default: $prefix/share/fuppes)","<directory>"}
#else
  {"set temp directory. (default: %TEMP%/fuppes)","<directory>"},
  {"add additional directories to the PATH. (default: %APPDATA%/FUPPES/)","<directory>"}
#endif
};

int fuppes_init(int argc, char* argv[], void(*p_log_cb)(const char* sz_log))
{
	// already initialized
  if(pFuppes)
    return FUPPES_FALSE;    
  
  // setup winsockets
  #ifdef WIN32
  WSADATA wsa;
  WSAStartup(MAKEWORD(2,2), &wsa);
  #endif  
  
  // setup config
  CSharedLog::Shared()->SetCallback(p_log_cb);

  // arguments  
  int    nLogLevel = 1;
  CSharedLog::Shared()->SetLogLevel(nLogLevel, false);
  
  int option_index = -1;
  int c;

  CSharedLog::Print("            FUPPES - %s", CSharedConfig::Shared()->GetAppVersion().c_str());
  CSharedLog::Print("    the Free UPnP Entertainment Service");
  CSharedLog::Print("      http://fuppes.ulrich-voelkel.de\n");

  #ifdef WIN32
  // TODO These windows ones are currently untested so please test them out
  // anything that does not have the \ / : * ? " < > | characters
  RegEx file("^[^\\\\\\/:*?\"<>|]+$");
  // same as a file but can have the slashes
  RegEx directory("^[^:*?\"<>|]+$");
  #else
  // everything that does not contain a slash
  RegEx file("^[^\\/]+$");
  // any character can be in a directory in linux
  RegEx directory("^.+$");
  #endif

  bool fileFail, dirFail;
  while(1) {
    // all followed by colons (:) bar h because it has no arguments
    c = getopt_long(argc, argv, "f:l:m:c:a:d:g:r:t:p:j:hv", long_options, &option_index);

    if(c == -1) break;

    fileFail = false;
    dirFail = false;
    switch((char)c) {
      case 'f':
        CSharedConfig::Shared()->globalSettings->SetFriendlyName(optarg);
        break;
      case 'l':
        nLogLevel = atoi(optarg);
        if(nLogLevel < 0) { nLogLevel = 0; }
        if(nLogLevel > 3) { nLogLevel = 3; }
        CSharedLog::Shared()->SetLogLevel(nLogLevel, false);
        break;
      case 'm':
        if(file.Search(optarg)) {
          CSharedLog::SetLogFileName(optarg);
        } else {
          fileFail = true;
        }
        break;
      case 'c':
        if(file.Search(optarg)) {
          CSharedConfig::Shared()->SetConfigFileName(optarg);
        } else {
          fileFail = true;
        }
        break;
      case 'a':
        if(directory.Search(optarg)) {
          PathFinder::addConfigPath(optarg);
        } else {
          dirFail = true;
        }
        break;
      case 'd':
        //if(file.Search(optarg)) {
          CSharedConfig::Shared()->databaseSettings->setDbFilename(optarg);
        //} else {
        //  fileFail = true;
        //}
        break;
      case 'g':
        // TODO not implemented yet
        Log::error(Log::unknown, Log::normal, __FILE__, __LINE__, 
          "Warning: Option -%c has not been implemented yet.", c);
        break;
      case 'r':
        // TODO not implemented yet
        Log::error(Log::unknown, Log::normal, __FILE__, __LINE__, 
          "Warning: Option -%c has not been implemented yet.", c);
        break;
      case 't':
        if(directory.Search(optarg)) {
	        CSharedConfig::Shared()->globalSettings->SetTempDir(optarg);
        } else {
          dirFail = true;
        }
        break;
      case 'p':
        if(directory.Search(optarg)) {
	        //CSharedConfig::Shared()->pluginDirectories->AddPluginPath(optarg);
	        CSharedConfig::Shared()->setPluginDirectory(optarg);
        } else {
          dirFail = true;
        }
        break;
      case 'j':
        if(directory.Search(optarg)) {
	        CSharedConfig::Shared()->dataDir(optarg);
        } else {
          dirFail = true;
        }
        break;
      case 'h':
			  printHelp();
			  return FUPPES_FALSE; 
      case 'v':
			  printVersion();
			  return FUPPES_FALSE; 
      default:
        // TODO place a 'maybe you meant' section in here'
        Log::error(Log::unknown, Log::normal, __FILE__, __LINE__, "Fuppes found an option it did not recognise (%c). Exiting...", (char)c);
			  return FUPPES_FALSE; 
        break;
    }

    // print warnings for bad options
    if(fileFail) {
      Log::error(Log::unknown, Log::normal, __FILE__, __LINE__, 
          "Warning: For option -%c, '%s' not recognised as a file name. Ignoring and attempting to continue.", c, optarg);
    } else if (dirFail) {
      Log::error(Log::unknown, Log::normal, __FILE__, __LINE__, 
          "Warning: For option -%c, '%s' not recognised as a directory name. Ignoring and attempting to continue.", c, optarg);
    }
  }

	xmlInitParser();

  // init config
	#ifdef WIN32
	string appDir = argv[0];
	TCHAR path[MAX_PATH] = {0};
	if(::GetModuleFileName(NULL, path, MAX_PATH)) {
		appDir = ExtractFilePath(path);
	}
	cout << "AppDir: " << appDir << endl;
	if(!CSharedConfig::Shared()->SetupConfig(appDir)) {
	#else
	if(!CSharedConfig::Shared()->SetupConfig()) {
	#endif
    return FUPPES_FALSE;
  }                 



  if(CSharedConfig::Shared()->databaseSettings->dbConnectionParams().type == "sqlite3") {
    // check Directory::writable(CSharedConfig::Shared()->databaseSettings->dbConnectionParams().filename)
  }
    
	if(!CDatabase::connect(CSharedConfig::Shared()->databaseSettings->dbConnectionParams())) {
    Log::error(Log::unknown, Log::normal, __FILE__, __LINE__, "error opening database %s", CSharedConfig::Shared()->databaseSettings->dbConnectionParams().type.c_str());
		return FUPPES_FALSE;
  }

  if(!CDatabase::setup()) {
    CDatabase::close();
    Log::error(Log::unknown, Log::normal, __FILE__, __LINE__, "error creating database %s", CSharedConfig::Shared()->databaseSettings->dbConnectionParams().type.c_str());
		return FUPPES_FALSE;
  }

  return FUPPES_TRUE;
}

void fuppes_set_error_callback(void(*p_err_cb)(const char* sz_err))
{
  CSharedLog::Shared()->SetErrorCallback(p_err_cb);
}

void fuppes_set_notify_callback(void(*p_notify_cb)(const char* sz_title, const char* sz_msg))
{
  CSharedLog::Shared()->SetNotifyCallback(p_notify_cb);
}

void fuppes_set_user_input_callback(void(*p_user_input_cb)(const char* sz_msg, char* sz_result, unsigned int n_buffer_size))
{
  CSharedLog::Shared()->SetUserInputCallback(p_user_input_cb);
}

int fuppes_start()
{
  if(pFuppes)
    return FUPPES_FALSE;

	#if !defined(WIN32) && !defined(MSG_NOSIGNAL) && !(SO_NOSIGPIPE)
	signal(SIGPIPE, SIG_IGN);
	#endif

  // create fuppes instance
  try {
    pFuppes = new CFuppes(CSharedConfig::Shared()->networkSettings->GetIPv4Address(), CSharedConfig::Shared()->GetUUID());
    CSharedConfig::Shared()->AddFuppesInstance(pFuppes);
    //CSharedLog::Log(L_EXT, __FILE__, __LINE__, fuppes_print_info());  // useful for bug reports
    return FUPPES_TRUE;
  }
  catch(fuppes::Exception ex) {
    CSharedLog::Shared()->UserError(ex.what());    
    CSharedLog::Print("[exiting]");
    return FUPPES_FALSE;
  }

}

int fuppes_stop()
{
  if(!pFuppes)
    return FUPPES_FALSE;
    
  delete pFuppes;
  pFuppes = 0;
  return FUPPES_TRUE;
}

int fuppes_cleanup()
{
	CDatabase::close();	

	delete CSharedConfig::Shared();
	delete CSharedLog::Shared();
  Log::uninit();
	
  xmlCleanupParser();
    
  // cleanup winsockets
  #ifdef WIN32
  WSACleanup();
  #endif

  return FUPPES_TRUE;
}

int fuppes_is_started()
{
  if(pFuppes != 0)
    return FUPPES_TRUE;
  else
    return FUPPES_FALSE;
}

const char* fuppes_get_version()
{
  return CSharedConfig::Shared()->GetAppVersion().c_str();
}

void fuppes_set_loglevel(int n_log_level)
{
  CSharedLog::Shared()->SetLogLevel(n_log_level, false);
}

void fuppes_inc_loglevel()
{
  CSharedLog::Shared()->ToggleLog();
}

void fuppes_activate_log_sender(const char* sender)
{
	Log::Sender tmp = Log::stringToSender(sender);
	Log::addActiveSender(tmp);
}

void fuppes_deactivate_log_sender(const char* sender)
{
	Log::Sender tmp = Log::stringToSender(sender);
	Log::removeActiveSender(tmp);
}

void fuppes_rebuild_db()
{
  CContentDatabase::Shared()->RebuildDB();
}

void fuppes_update_db()
{
  CContentDatabase::Shared()->UpdateDB();
}

void fuppes_update_db_add_new()
{
  CContentDatabase::Shared()->AddNew();
}

void fuppes_update_db_remove_missing()
{
  CContentDatabase::Shared()->RemoveMissing();
}

void fuppes_rebuild_vcontainers()
{
  CVirtualContainerMgr::Shared()->RebuildContainerList();
}

void fuppes_get_http_server_address(char* sz_addr, unsigned int n_buff_size)
{
  stringstream sAddr;
  sAddr << "http://" << pFuppes->GetHTTPServerURL();
	if(sAddr.str().length() < n_buff_size) {
		strcpy(sz_addr, sAddr.str().c_str());
	}
}

void fuppes_send_alive()
{
  pFuppes->GetSSDPCtrl()->send_alive();
}

void fuppes_send_byebye()
{
  pFuppes->GetSSDPCtrl()->send_byebye();
}

void fuppes_send_msearch()
{
  pFuppes->GetSSDPCtrl()->send_msearch();
}

#warning This function cannot print anything untif the HTTP server has initialised. That ocurrs in fuppes_start.
// We should consider moving this function somewhere eles for better access.
void fuppes_print_info()
{
  stringstream result;

  result << "general information:" << endl;
  result << "  version     : " << CSharedConfig::Shared()->GetAppVersion() << endl;
  result << "  hostname    : " << CSharedConfig::Shared()->networkSettings->GetHostname() << endl;
  result << "  OS          : " << CSharedConfig::Shared()->GetOSName() << " " << CSharedConfig::Shared()->GetOSVersion() << endl;
  result << "  build at    : " << __DATE__ << " " << __TIME__ << endl;
  result << "  build with  : " << __VERSION__ << endl;
  result << "  address     : " << CSharedConfig::Shared()->networkSettings->GetIPv4Address() << endl;
  //cout << "  sqlite      : " << CContentDatabase::Shared()->GetLibVersion() << endl;
  result << "  log-level   : " << CSharedLog::Shared()->GetLogLevel() << endl;
  result << "  webinterface: http://" << pFuppes->GetHTTPServerURL() << "/" << endl;
  result << endl;
	result << "configuration file:" << endl;
	result << "  " << CSharedConfig::Shared()->GetConfigFileName() << endl;
	result << endl;
	
  result << CPluginMgr::printInfo().c_str() << endl;
  

#ifdef HAVE_ICONV
	result << "iconv: enabled" << endl;
#else
	result << "iconv: disabled" << endl;
#endif
	
#ifndef WIN32
#ifdef HAVE_INOTIFY
	result << "inotify: enabled" << endl;
#else
	result << "inotify: disabled" << endl;
#endif

#ifdef HAVE_UUID
	result << "uuid: enabled" << endl;
#else
	result << "uuid: disabled" << endl;
#endif
#endif

  cout << result.str();
  //return result.str().c_str();
}

void fuppes_print_device_settings()
{
  CDeviceIdentificationMgr::Shared()->PrintSettings();
}


static void printVersion(void) {
  cout << "fuppes version " << CSharedConfig::Shared()->GetAppVersion() << endl;
}

static void printHelp(void)
{
  int lineLen = 80;
  cout << "usage: fuppes [OPTIONS]\n" << endl;
  cout << "Options:\n" << endl;

  //cout << "\t(-shortoption) --longoption <argument>" << endl;
  //cout << "\t   description\n" << endl;

  printLine("\t",
    "The items in the brackets are the short names and they take the same number of "
    "arguments as the long names that follow them.", 
    lineLen, true);
  cout << endl;
  printOptions(lineLen);

  cout << "Examples:\n" << endl;
  printExamples(lineLen);

  // TODO Is this section good? Maybe it needs to be checked over. And maybe we just have a webpage
  // of authors whereby they are listed by the number of changes (lines added + lines removed) to the
  // source code; then we just leave the primary author or some contact email here.
  cout << "Project Lead / Author:\n" << endl; 
  printLine("\t", "Ulrich Völkel <u-voelkel@users.sourceforge.net> (Copyright (C) 2005-2010)", lineLen);
  cout << endl;
  cout << "Developers:\n" << endl;
  printLine("\t", "Robert Massaioli <robertmassaioli@gmail.com>", lineLen);
}

static void printOptions(int lineLen) {
  int size = sizeof(fopt) / sizeof(fopt[0]); // array length
  for(int i = 0; i < size; ++i) {
    cout << "\t-" << (char)long_options[i].val << ", --" << 
      long_options[i].name << " " << fopt[i].trail << endl;

    printLine("\t   ", fopt[i].desc, lineLen);
    putc('\n', stdout);
  }
}

static void printExamples(int lineLen) {
  printLine("\t",
    "Using the options above are pretty simple to use. Here are some examples of some of "
    "the more common ways in which fuppes is used. You can add any combination of the options "
    "together that you like:",
    lineLen);
  cout << endl; // double newline

  printLine("\t", "Making a log file that prints everything:", lineLen);
  printLine("\t\t", "fuppes --log-file whatIsHappening.log -l 3", lineLen);
  cout << endl;

  printLine("\t", "Adding another directory to the front of the plugin search path:", lineLen);
  printLine("\t\t", "fuppes -p /path/to/plugindir", lineLen);
  cout << endl;

  printLine("\t", 
      "Adding to the file path and changing the default names for a file structure with "
      "all of the devices in the 'dev' directory and all of the vfolders in the 'vfconf' directory. "
      "Please note that the changed names must be found within the path; thus the names "
      "are just the names of the unusual directories, not their path:"
      , lineLen);
  printLine("\t\t", "fuppes -a /path/to/new/config/dir/ -g vfconf -r dev", lineLen);
  cout << endl;

  printLine("\t", 
      "Changing the database filename to use (this option is only for databases "
      "like sqlite where the whole database is in a file):"
      , lineLen);
  printLine("\t\t", "fuppes --database-file different-db.sqlite", lineLen);
  cout << endl;
}

// Warning: this function will run into problems if a word is longer that lineLen where
// a word is any stream of characters that does not contain a space.
static void printLine(const char* linePrefix, const char* line, int lineLen, bool withNewline) {
  assert(linePrefix != NULL);
  assert(line != NULL);
  assert(lineLen > 1); // bare minimum for a line methinks

  const char *wordStart, *wordEnd, *cur, *lineStart;

  cur = lineStart = line;
  fputs(linePrefix, stdout);
  while (*cur) { // while we have not reached the end of the line
    wordStart = cur;
    while(*wordStart == ' ') ++wordStart;
    wordEnd = wordStart;
    while(*wordEnd != ' ' && *wordEnd) ++wordEnd;
    if(wordEnd - lineStart > lineLen) {
      putc('\n', stdout);
      fputs(linePrefix, stdout);
      lineStart = wordStart;
      cur = wordStart;
    }

    while (cur < wordEnd) {
      putc(*cur, stdout);
      ++cur;
    }
  }
  if(withNewline) putc('\n', stdout);
}
