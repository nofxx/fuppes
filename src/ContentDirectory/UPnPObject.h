/***************************************************************************
 *            UPnPObject.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 *  Copyright (C) 2005 Thomas Schnitzler <tschnitzler@users.sourceforge.net>
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
 
#ifndef _UPNPOBJECT_H
#define _UPNPOBJECT_H

/*===============================================================================
 INCLUDES
===============================================================================*/

#include <string>
#include <libxml/xmlwriter.h>

/*===============================================================================
 DEFINITIONS
===============================================================================*/

typedef enum tagUPNP_OBJECT_TYPE
{
  UPNP_OBJECT_TYPE_STORAGE_FOLDER  = 0,
  UPNP_OBJECT_TYPE_ITEM            = 1,
  UPNP_OBJECT_TYPE_AUDIO_ITEM      = 2,
  UPNP_OBJECT_TYPE_IMAGE_ITEM      = 3,
  UPNP_OBJECT_TYPE_VIDEO_ITEM      = 4,
  UPNP_OBJECT_TYPE_PLAYLIST_ITEM   = 5
}UPNP_OBJECT_TYPE;

/*===============================================================================
 CLASS CUPnPObject
===============================================================================*/

class CUPnPObject
{

/* <PROTECTED> */

protected:
    
/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

  /** constructor
   *  @param  p_nUPnPObjectType  type of the UPnP object
   *  @param  p_sHTTPServerURL  URL of the HTTP server
   */
  CUPnPObject(UPNP_OBJECT_TYPE p_nUPnPObjectType, std::string p_sHTTPServerURL) 
  {
    m_nUPnPObjectType = p_nUPnPObjectType;
    m_sHTTPServerURL = p_sHTTPServerURL;
  }
    
  /** destructor
   */
  virtual ~CUPnPObject()
  {
  }

/* <\PROTECTED> */

/* <PUBLIC> */

public:

/*===============================================================================
 SET
===============================================================================*/

  /** sets the object id
   *  @param  p_sObjectID  object id to set
   */
  void SetObjectID(std::string p_sObjectID)   { m_sObjectID = p_sObjectID;        }

  /** sets the parent object
   *  @param  p_ParentObject  parent object to set
   */
  void SetParent(CUPnPObject* p_ParentObject) { m_pParentObject = p_ParentObject; }
  
  /** sets the object name
   *  @param  p_sName  object name to set
   */  
  void SetName(std::string p_sName)           { m_sName = p_sName;                }
  
  /** sets the file name for the object
   *  @param  p_sFileName  file name to set
   */  
  void SetFileName(std::string p_sFileName)   { m_sFileName = p_sFileName;        }

/*===============================================================================
 GET
===============================================================================*/

  /** returns the object type
   *  @return  object type
   */  
  UPNP_OBJECT_TYPE GetObjectType()            { return m_nUPnPObjectType;         }      

  /** returns the object id
  *  @return  object id as string
  */  
  std::string GetObjectID()                   { return m_sObjectID;               }
  
  /** returns the parent object
   *  @return  pointer to the parent object
   */  
  CUPnPObject* GetParent()                    { return m_pParentObject;           }
  
  /** returns the object name
   *  @return  object name as string
   */  
  std::string GetName()                       { return m_sName;                   }
  
  /** returns the object's filename
   *  @return  filename as string
   */  
  std::string GetFileName()                   { return m_sFileName;               }
  
  /** returns the object description
   *  @param  xmlTextWriterPtr  XML container to write to
   */  
  virtual void GetDescription(xmlTextWriterPtr pWriter) = 0;
    
/* <\PUBLIC> */

/* <PROTECTED> */

protected:

/*===============================================================================
 GET
===============================================================================*/
  
  /** returns the URL of the HTTP server
   *  @return  server URL as string
   */  
  std::string GetHTTPServerURL() { return m_sHTTPServerURL; }

/* <\PROTECTED> */

/* <PRIVATE> */

protected:

/*===============================================================================
 MEMBERS
===============================================================================*/

  UPNP_OBJECT_TYPE m_nUPnPObjectType;
  CUPnPObject*     m_pParentObject;
  std::string      m_sObjectID;
  std::string      m_sName;
  std::string      m_sFileName;
  std::string      m_sHTTPServerURL;

/* <\PRIVATE> */

};

#endif /* _UPNPOBJECT_H */
