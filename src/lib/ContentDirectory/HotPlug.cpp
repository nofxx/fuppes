/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            HotPlug.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2010 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#include "HotPlug.h"

#include "../Log.h"
using namespace fuppes;


HotPlugMgr* HotPlugMgr::m_instance = 0;

void HotPlugMgr::init() // static
{
  if(m_instance == 0)
    m_instance = new HotPlugMgr();
}

void HotPlugMgr::uninit() // static
{
  if(m_instance == 0)
    return;
  
  delete m_instance;
  m_instance = 0;
}

HotPlugMgr::HotPlugMgr()
{
  m_hotPlug = 0;

#ifdef HAVE_DBUS
  m_hotPlug = new HotPlugDbus();
  m_hotPlug->setup();
#endif
}

HotPlugMgr::~HotPlugMgr()
{
  if(m_hotPlug)
    delete m_hotPlug;
}




#ifdef HAVE_DBUS

bool HotPlugDbus::setup()
{
  Log::log(Log::hotplug, Log::normal, __FILE__, __LINE__, "creating new dbus connection");

  DBusError error;
  dbus_error_init(&error);
  
  m_connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
  if(m_connection == NULL) {
    Log::error(Log::hotplug, Log::normal, __FILE__, __LINE__, "error creating dbus connection");
    if(dbus_error_is_set(&error)) {
      Log::error(Log::hotplug, Log::normal, __FILE__, __LINE__, "dbus error: %s", error.message);
    }
    return false;
  }  
  
  dbus_connection_set_exit_on_disconnect(m_connection, FALSE);


  dbus_bus_add_match(m_connection, 
         "type='signal',interface='test.signal.Type'", 
         &error); // see signals from the given interface
   dbus_connection_flush(m_connection);
   if (dbus_error_is_set(&error)) { 
      Log::error(Log::hotplug, Log::normal, __FILE__, __LINE__, "dbus error: %s", error.message);
      return false; 
   }

  // start event loop
  start();
  return true;
}

HotPlugDbus::~HotPlugDbus()
{
  close();
  
  if(m_connection != NULL)
    dbus_connection_unref(m_connection);

  /*DBusError error;
  dbus_error_init(&error);
  
  dbus_error_free(&error);*/
}

void HotPlugDbus::run()
{
  while(!stopRequested()) {

    msleep(500);
  }
  
}

#endif // HAVE_DBUS
