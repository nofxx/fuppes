/***************************************************************************
 *            ContentDirectory.h
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
 
#ifndef _CONTENTDIRECTORY_H
#define _CONTENTDIRECTORY_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include "../UPnPService.h"
#include "../HTTP/HTTPMessage.h"
#include "ContentDatabase.h"
#include "../UPnPActions/UPnPBrowse.h"
#include "../UPnPActions/UPnPSearch.h"

#include <map>
#include <string>

class CContentDirectory: public CUPnPService
{
	
  public:

    /** constructor
     *  @param  p_sHTTPServerURL  URL of the HTTP server
     */
    CContentDirectory(std::string p_sHTTPServerURL);

    /** destructor     
     */
    ~CContentDirectory();


    std::string GetServiceDescription();

    /** handles a UPnP action and creates the corresponding message
     *  @param  pUPnPAction  UPnP action to handle
     *  @param  pMessageOut  the message, that was created for the action   
     */
    void HandleUPnPAction(CUPnPAction* pUPnPAction, CHTTPMessage* pMessageOut);



  private:
    /** scans a specific directory
     *  @param  p_sDirectory  path to the directory to scan
     *  @param  p_pnCount  count of found objects
     *  @param  pParentFolder  the parent folder
    */
    void DbScanDir(std::string p_sDirectory, long long int p_nParentId);
       
    
    
    /** handles a UPnP browse action
     *  @param  pBrowse  the browse action to handle
     *  @return string with the message content to send for the browse action
     */  
    void DbHandleUPnPBrowse(CUPnPBrowse* pBrowse, std::string* p_psResult);

    void BrowseMetadata(xmlTextWriterPtr pWriter, 
                        unsigned int* p_pnTotalMatches,
                        unsigned int* p_pnNumberReturned,
                        CUPnPBrowse*  pUPnPBrowse);
                        
    void BrowseDirectChildren(xmlTextWriterPtr pWriter, 
                              unsigned int* p_pnTotalMatches,
                              unsigned int* p_pnNumberReturned,
                              CUPnPBrowse*  pUPnPBrowse);

    void BuildDescription(xmlTextWriterPtr pWriter,
                          CSelectResult* pSQLResult,
                          CUPnPBrowseSearchBase*  pUPnPBrowse,
                          std::string p_sParentId);
  
    void BuildContainerDescription(xmlTextWriterPtr pWriter,
                                   CSelectResult* pSQLResult,
                                   CUPnPBrowseSearchBase*  pUPnPBrowse,
                                   std::string p_sParentId,
                                   OBJECT_TYPE p_nContainerType);
    void BuildItemDescription(xmlTextWriterPtr pWriter,
                              CSelectResult* pSQLResult,
                              CUPnPBrowseSearchBase*  pUPnPBrowse,
                              OBJECT_TYPE p_nObjectType,
                              std::string p_sParentId);      
    void BuildAudioItemDescription(xmlTextWriterPtr pWriter,
                                   CSelectResult* pSQLResult,
                                   CUPnPBrowseSearchBase*  pUPnPBrowse,
                                   std::string p_sObjectID);      
    void BuildAudioItemAudioBroadcastDescription(xmlTextWriterPtr pWriter,
                                   CSelectResult* pSQLResult,
                                   CUPnPBrowseSearchBase*  pUPnPBrowse,
                                   std::string p_sObjectID);                                    
    void BuildImageItemDescription(xmlTextWriterPtr pWriter,
                                   CSelectResult* pSQLResult,
                                   CUPnPBrowseSearchBase*  pUPnPBrowse,
                                   std::string p_sObjectID);
    void BuildVideoItemDescription(xmlTextWriterPtr pWriter,
                                   CSelectResult* pSQLResult,
                                   CUPnPBrowseSearchBase*  pUPnPBrowse,
                                   std::string p_sObjectID); 
    void BuildVideoItemVideoBroadcastDescription(xmlTextWriterPtr pWriter,
                                   CSelectResult* pSQLResult,
                                   CUPnPBrowseSearchBase*  pUPnPBrowse,
                                   std::string p_sObjectID);   
    void BuildPlaylistItemDescription(xmlTextWriterPtr pWriter,
                                   CSelectResult* pSQLResult,
                                   CUPnPBrowseSearchBase*  pUPnPBrowse,
                                   std::string p_sObjectID);                                    


    void HandleUPnPGetSearchCapabilities(CUPnPAction* pAction, std::string* p_psResult);

    void HandleUPnPGetSortCapabilities(CUPnPAction* pAction, std::string* p_psResult);
    
    void HandleUPnPGetSystemUpdateID(CUPnPAction* pAction, std::string* p_psResult);    
		
    void HandleUPnPSearch(CUPnPSearch* pSearch, std::string* p_psResult);
  
    std::string BuildProtocolInfo(bool p_bTranscode,
                                  std::string p_sMimeType,
                                  std::string p_sProfileId,
                                  CUPnPBrowseSearchBase*  pUPnPBrowse);
};

#endif // _CONTENTDIRECTORY_H
