
#ifndef _LOG_H
#define _LOG_H

#include <list>
#include <string>
#include <stdarg.h>

namespace fuppes {

	class Exception;
	
  class Log {

		public:
      Log() {
        m_logSenders = 0;
      }

			enum Sender {
				unknown    = 0,
				http       = 1 << 0,
        soap       = 1 << 1,
        gena       = 1 << 2,
				ssdp       = 1 << 3,
				fam        = 1 << 4,
        contentdir = 1 << 5,
        contentdb  = 1 << 6,
        sql        = 1 << 7,
				plugin     = 1 << 8,
        config     = 1 << 9,
        hotplug    = 1 << 10,

        all        = 
          http | soap | gena | ssdp | fam | contentdir | contentdb | sql | plugin | config | hotplug
			};
		
		  enum Level {
		    none			= 0,
		    normal		= 1,
		    extended	= 2,
		    debug			= 3
		  };

			static std::string senderToString(Log::Sender sender);
			static Log::Sender stringToSender(std::string sender);
      
      static void init();
      static void uninit();
      
		  static void log(Log::Sender sender, 
		    Log::Level level, 
		    const std::string fileName, 
		    int lineNo,
		    const char* format,
		    ...);
      
		  static void log(Log::Sender sender, 
		    Log::Level level, 
		    const std::string fileName, 
		    int lineNo,
		    const char* format,
		    va_list args);
			
			static void log(Log::Sender sender, 
		    Log::Level level, 
		    const std::string fileName, 
		    int lineNo,
		    const std::string msg);

    static void error(Log::Sender sender, 
		    Log::Level level, 
		    const std::string fileName, 
		    int lineNo,
		    const char* format,
		    ...);
      
      static bool isActiveSender(Log::Sender sender) {

        return ((m_instance->m_logSenders & sender) == sender);
        
        /*std::list<Log::Sender>::iterator iter;
        for(iter = m_instance->m_logSenders.begin(); 
            iter != m_instance->m_logSenders.end(); 
            iter++) {
          if(*iter == sender)
            return true;
        }
        return false;*/
      }

      static void addActiveSender(Log::Sender sender) {
        if(sender == Log::unknown) return;
        m_instance->m_logSenders |= sender; // even if they are the active one
      }

      static void removeActiveSender(Log::Sender sender) {
        // and with the ones compliment of sender
        m_instance->m_logSenders &= ~sender;
      }

    private:
      static Log* m_instance;
      //std::list<Log::Sender> m_logSenders;
      int m_logSenders;
  };

}

#endif // _LOG_H
