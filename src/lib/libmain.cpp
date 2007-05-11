/***************************************************************************
 *            libmain.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 - 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
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

#include "../../include/fuppes.h"

#include <iostream>
#include "Common/Common.h"
#include "SharedConfig.h"
#include "SharedLog.h"
#include "Fuppes.h"
#include "ContentDirectory/ContentDatabase.h"
#include "ContentDirectory/VirtualContainerMgr.h"

#ifdef WIN32
#include <shellapi.h>
#include <windows.h>
#endif

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#ifdef HAVE_IMAGEMAGICK
#include <Magick++.h>
#endif

using namespace std;

CFuppes* pFuppes = 0;

int fuppes_init(int argc, char* argv[], void(*p_log_cb)(const char* sz_log))
{
  // already initialized
  if(pFuppes)
    return FUPPES_FALSE;    
  
  CSharedLog::Shared()->SetCallback(p_log_cb);

  cout << "            FUPPES - " << CSharedConfig::Shared()->GetAppVersion() << endl;
  cout << "    the Free UPnP Entertainment Service" << endl;
  cout << "       http://fuppes.sourceforge.net" << endl << endl;

  
  // arguments  
  string sConfigDir;
  for(int i = 0; i < argc; i++) {
    if((strcmp(argv[i], "--config-dir") == 0) && (argc > i + 1)) {
      sConfigDir = argv[i +1];
    }
  }  
  
  // setup winsockets
  #ifdef WIN32
  WSADATA wsa;
  WSAStartup(MAKEWORD(2,2), &wsa);
  #endif    
    
  // init config
  if(!CSharedConfig::Shared()->SetupConfig(sConfigDir))
    return FUPPES_FALSE;

	#ifdef HAVE_IMAGEMAGICK
  //InitializeMagick(*argv);
	//MagickWandGenesis();
  #endif    
    
  xmlInitParser();
    
  return FUPPES_OK;
}

void fuppes_set_error_callback(void(*p_err_cb)(const char* sz_err))
{
  CSharedLog::Shared()->SetErrorCallback(p_err_cb);
}

void fuppes_set_notify_callback(void(*p_notify_cb)(const char* sz_title, const char* sz_msg))
{
  CSharedLog::Shared()->SetNotifyCallback(p_notify_cb);
}


int fuppes_start()
{
  if(pFuppes)
    return FUPPES_FALSE;
    
  // create fuppes instance
  try {
    pFuppes = new CFuppes(CSharedConfig::Shared()->GetIPv4Address(), CSharedConfig::Shared()->GetUUID());
    CSharedConfig::Shared()->AddFuppesInstance(pFuppes);
    return FUPPES_OK;
  }
  catch(EException ex) {
    CSharedLog::Shared()->UserError(ex.What());    
    cout << "[exiting]" << endl;
    return FUPPES_FALSE;
  }  
}

int fuppes_stop()
{
  if(!pFuppes)
    return FUPPES_FALSE;
    
  delete pFuppes;
  pFuppes = 0;
  return FUPPES_OK;
}

int fuppes_cleanup()
{
  xmlCleanupParser();
    
  #ifdef HAVE_IMAGEMAGICK	  
	//MagickWandTerminus();
  #endif
    
  // cleanup winsockets
  #ifdef WIN32
  WSACleanup();
  #endif

  return FUPPES_OK;
}

int fuppes_is_started()
{
  if(pFuppes != 0)
    return FUPPES_OK;
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

void fuppes_rebuild_db()
{
  CContentDatabase::Shared()->BuildDB();
}

void fuppes_rebuild_vcontainers()
{
  CVirtualContainerMgr::Shared()->RebuildContainerList();
}

void fuppes_get_http_server_address(char* sz_addr, int n_buff_size)
{
  stringstream sAddr;
  sAddr << "http://" << pFuppes->GetHTTPServerURL();
  strcpy(sz_addr, sAddr.str().c_str());
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

void fuppes_print_info()
{
  cout << "general information:" << endl;
  cout << "  version     : " << CSharedConfig::Shared()->GetAppVersion() << endl;
  cout << "  hostname    : " << CSharedConfig::Shared()->GetHostname() << endl;
  cout << "  OS          : " << CSharedConfig::Shared()->GetOSName() << " " << CSharedConfig::Shared()->GetOSVersion() << endl;
  cout << "  build at    : " << __DATE__ << " " << __TIME__ << endl;
  cout << "  build with  : " << __VERSION__ << endl;
  cout << "  address     : " << CSharedConfig::Shared()->GetIPv4Address() << endl;
  cout << "  sqlite      : " << CContentDatabase::Shared()->GetLibVersion() << endl;
  cout << "  log-level   : " << CSharedLog::Shared()->GetLogLevel() << endl;
  cout << "  webinterface: http://" << pFuppes->GetHTTPServerURL() << "/" << endl;
  cout << endl;
  CSharedConfig::Shared()->PrintTranscodingSettings();
	cout << endl;
	cout << "configuration file:" << endl;
	cout << "  " << CSharedConfig::Shared()->GetConfigFileName() << endl;
	cout << endl;
	cout << "ImageMagick: ";
	#ifdef HAVE_IMAGEMAGICK
	cout << " enabled" << endl;
	#else
	cout << " compiled without ImageMagick support" << endl;	
	#endif
	
	
}
