/***************************************************************************
 *            NotificationMgr.cpp
 *
 *  FUPPES - the Free UPnP Entertainment Service
 *
 *  Copyright (C) 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "NotificationMgr.h"

#ifdef HAVE_LIBNOTIFY
#include <libnotify/notify.h>
#include <libnotify/notification.h>
#include <libnotify/notify-enum-types.h>
#endif

#include <iostream>

using namespace std;

CNotificationMgr* CNotificationMgr::m_Instance = 0;

CNotificationMgr* CNotificationMgr::Shared()
{
	if (m_Instance == 0)
		m_Instance = new CNotificationMgr();
	return m_Instance;
}

CNotificationMgr::CNotificationMgr()
{
  #ifdef HAVE_LIBNOTIFY
  if(!notify_init("fuppes"))
    cout << __FILE__ << ", " << __LINE__ << " :: notify_init() failed" << endl;
  #endif
}

CNotificationMgr::~CNotificationMgr()
{
  #ifdef HAVE_LIBNOTIFY
  notify_uninit();
  #endif
}

void CNotificationMgr::Notify(std::string p_sTitle, std::string p_sBody)
{
  #ifdef HAVE_LIBNOTIFY
  NotifyNotification* pNotification; 
  pNotification = notify_notification_new(p_sTitle.c_str(), p_sBody.c_str(), NULL, NULL);
  
  //char* g_strdup_printf
  //NOTIFY_URGENCY_LOW 	 Low urgency. Used for unimportant notifications.
  //NOTIFY_URGENCY_NORMAL 	Normal urgency. Used for most standard notifications.
  //NOTIFY_URGENCY_CRITICAL 	Critical urgency. Used for very important notifications.  
  //notify_notification_set_urgency(pNotification, NOTIFY_URGENCY_LOW);
  
  GError* pError = NULL;
  if(!notify_notification_show(pNotification,  &pError)) {
    //
  }
  #endif  
}
