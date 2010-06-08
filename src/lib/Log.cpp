#include "Log.h"

#include <iostream>
#include <stdio.h>

using namespace std;
using namespace fuppes;

std::string Log::senderToString(Log::Sender sender) // static
{
	switch(sender) {
    case Log::unknown:
      return "unknown";
    case Log::http:
      return "http";
    case Log::soap:
      return "soap";
    case Log::gena:
      return "gena";
    case Log::ssdp:
      return "ssdp";
    case Log::fam:
      return "fam";

    case Log::contentdir:
      return "contentdir";
    case Log::contentdb:
      return "contentdb";
    case Log::sql:
      return "sql";

    case Log::plugin:
      return "plugin";

    case Log::config:
      return "config";

    case Log::hotplug:
      return "hotplug";
      
    case Log::all:
      return "all";
		default:
			return "unknown";
	};	
}

Log::Sender Log::stringToSender(std::string sender) // static
{
  if(sender == "unknown")
    return Log::unknown;
  else if(sender == "http")
    return Log::http;
  else if(sender == "soap")
    return Log::soap;
  else if(sender == "gena")
    return Log::gena;
  else if(sender == "ssdp")
    return Log::ssdp;
  else if(sender == "fam")
    return Log::fam;
  
  else if(sender == "contentdir")
    return Log::contentdir;
  else if(sender == "contentdb")
    return Log::contentdb;
  else if(sender == "sql")
    return Log::sql;
  
  else if(sender == "plugin")
    return Log::plugin;

  else if(sender == "config")
    return Log::config;

  else if(sender == "hotplug")
    return Log::hotplug;

  else if(sender == "all")
    return Log::all;
  else
    return Log::unknown;
}


Log* Log::m_instance = NULL;

void Log::init() // static
{
  if(m_instance != NULL)
    return;
  
  m_instance = new Log();
  m_instance->m_logSenders |= Log::hotplug;
}

void Log::uninit() // static
{
  if(m_instance == NULL)
    return;
  
  delete m_instance;
  m_instance = NULL;
}

void Log::log(Log::Sender sender, Log::Level level, const std::string fileName, int lineNo, const char* format, ...) // static
{
  bool active = isActiveSender(sender);

  if(!active) {
    //cout << "Log sender: " << Log::senderToString(sender) << " not active" << endl;
    return;
  }
  
	va_list args;
  va_start(args, format);
  Log::log(sender, level, fileName, lineNo, format, args);
	va_end(args);
}


void Log::log(Log::Sender sender, Log::Level level, const std::string fileName, int lineNo, const char* format, va_list args) // static
{
  bool active = isActiveSender(sender);

  if(!active) {
    //cout << "Log sender: " << Log::senderToString(sender) << " not active" << endl;
    return;
  }
  
	char buffer[8192 * 10];	
 	string out = "[" + Log::senderToString(sender) + "] ";
  vsnprintf(buffer, sizeof(buffer) - 1, format, args);
	
	cout << out << buffer << endl;
}



void Log::log(Log::Sender sender, Log::Level level, const std::string fileName, int lineNo, const std::string msg) // static
{
  bool active = isActiveSender(sender);

  if(!active) {
    //cout << "Log sender: " << Log::senderToString(sender) << " not active" << endl;
    return;
  }

	string out = "[" + Log::senderToString(sender) + "] " + msg;
	//CSharedLog::Log(0, fileName, lineNo, out);
  cout << out << endl;
}



void Log::error(Log::Sender sender, Log::Level level, const std::string fileName, int lineNo, const char* format, ...) // static
{
	va_list args;
  va_start(args, format);

  char buffer[8192 * 10];	
 	string out = "[" + Log::senderToString(sender) + "] ";
  vsnprintf(buffer, sizeof(buffer) - 1, format, args);
	va_end(args);
	
	cout << out << buffer << endl;
}
