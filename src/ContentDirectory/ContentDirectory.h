/***************************************************************************
 *            ContentDirectory.h
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
 
#ifndef _CONTENTDIRECTORY_H
#define _CONTENTDIRECTORY_H

/*===============================================================================
 INCLUDES
===============================================================================*/

#include "../UPnPService.h"
#include "../HTTPMessage.h"
#include "UPnPObject.h"
#include "UPnPContainer.h"
#include "ContentDatabase.h"
#include "../UPnPActions/UPnPBrowse.h"

#include <map>
#include <string>
/*===============================================================================
 CLASS CContentDirectory
===============================================================================*/

class CContentDirectory: public CUPnPService
{
	
/* <PUBLIC> */

public:

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

  /** constructor
   *  @param  p_sHTTPServerURL  URL of the HTTP server
   */
  CContentDirectory(std::string p_sHTTPServerURL);

  /** destructor
   *  @todo  Delete all objects that were created with 'new'. 'm_Objectlist' has the pointers to these objects
   */
  ~CContentDirectory();

/*===============================================================================
 UPNP ACTION HANDLING
===============================================================================*/

  /** handles a UPnP action and creates the corresponding message
   *  @param  pUPnPAction  UPnP action to handle
   *  @param  pMessageOut  the message, that was created for the action
   *  @return returns true on success otherwise false
   */
  bool HandleUPnPAction(CUPnPAction* pUPnPAction, CHTTPMessage* pMessageOut);

/*===============================================================================
 GET
===============================================================================*/

  /** returns a UPnP object for a specific object id
   *  @param  p_sObjectID  object id to identify object
   *  @return  the UPnP object or NULL
   */
  CUPnPObject* GetItemFromObjectID(std::string p_sObjectID);


    
  void BuildDB();
/* <\PUBLIC> */

/* <PRIVATE> */

  private:

  /*============================================================================
   MEMBERS
  ============================================================================*/

    std::map<std::string, CUPnPObject*>           m_ObjectList;
    std::map<std::string, CUPnPObject*>::iterator m_ListIterator;
    CUPnPContainer*                               m_pBaseFolder;
    CContentDatabase*                             m_pDatabase;

  /*============================================================================
   HELPER
  ============================================================================*/
    
    /** handles a UPnP browse action
     *  @param  pBrowse  the browse action to handle
     *  @return string with the message content to send for the browse action
     */  
    std::string DbHandleUPnPBrowse(CUPnPBrowse* pBrowse);
  
    void BuildContainerDescription(xmlTextWriterPtr pWriter, CSelectResult* pSQLResult, std::string p_sParentId, std::string p_sChildCount);
    void BuildItemDescription(xmlTextWriterPtr pWriter, CSelectResult* pSQLResult, OBJECT_TYPE p_nObjectType, std::string p_sParentId);      
    void BuildAudioItemDescription(xmlTextWriterPtr pWriter, CSelectResult* pSQLResult, std::string p_sObjectID);      
    void BuildImageItemDescription(xmlTextWriterPtr pWriter, CSelectResult* pSQLResult, std::string p_sObjectID);      
    void BuildVideoItemDescription(xmlTextWriterPtr pWriter, CSelectResult* pSQLResult, std::string p_sObjectID);

    /** scans a specific directory
     *  @param  p_sDirectory  path to the directory to scan
     *  @param  p_pnCount  count of found objects
     *  @param  pParentFolder  the parent folder
    */
    void DbScanDir(std::string p_sDirectory, long long int p_nParentId);
   

/* <\PRIVATE> */

};

#endif /* _CONTENTDIRECTORY_H */
