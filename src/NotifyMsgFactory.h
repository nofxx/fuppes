/***************************************************************************
 *            NotifyMsgFactory.h
 * 
 *  Copyright  2005  Ulrich Völkel
 *  mail@ulrich-voelkel.de
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#ifndef _NOTIFYMSGFACTORY_H
#define _NOTIFYMSGFACTORY_H

#include <string>

enum msg_type
{
	mt_usn,
	mt_root_device,
	mt_connection_manager,
	mt_content_directory,
	mt_media_server
};

class CNotifyMsgFactory
{
	public:
		static CNotifyMsgFactory* shared();
		
		std::string msearch();	
	  std::string notify_alive(msg_type);	
	  std::string notify_bye_bye(msg_type);
	
		void SetHTTPServerURL(std::string);
	
	protected:
		CNotifyMsgFactory();
	
	private:
		static CNotifyMsgFactory* instance;
	  static std::string type_to_string(msg_type);
	
		std::string m_sHTTPServerURL;
};	

#endif /* _NOTIFYMSGFACTORY_H */
