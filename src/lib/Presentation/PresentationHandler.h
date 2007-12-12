/***************************************************************************
 *            PresentationHandler.h
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
 
#ifndef _PRESENTATIONHANDLER_H
#define _PRESENTATIONHANDLER_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include "../UPnPDevice.h"
#include "../HTTP/HTTPMessage.h"
#include "../Fuppes.h"

#include <string>
#include <vector>


typedef enum tagPRESENTATION_PAGE
{
  PRESENTATION_PAGE_UNKNOWN,
  PRESENTATION_BINARY_IMAGE,
  PRESENTATION_PAGE_INDEX,
  PRESENTATION_PAGE_ABOUT,
  PRESENTATION_PAGE_HELP,
  PRESENTATION_PAGE_OPTIONS,
  PRESENTATION_PAGE_STATUS
}PRESENTATION_PAGE;

class CPresentationHandler
{
  public:
    CPresentationHandler();
    virtual ~CPresentationHandler();


    /** handles HTTP messages add returns a corresponding message
     *  @param pSender  sender of the incoming message
     *  @param pMessage  the incoming message
     *  @param pResult  the outgoing message
     */
    void OnReceivePresentationRequest(CHTTPMessage* pMessage, CHTTPMessage* pResult);
  
  private:

    /** handles HTTP requests
     *  @param p_sRequest  the message to handle
     */
    std::string HandleRequest(std::string p_sRequest);  

    /** returns the HTML header
     *  @return the HTML header as string
     */
    std::string GetXHTMLHeader();  

    std::string GetPageHeader(PRESENTATION_PAGE p_nPresentationPage, std::string p_sImgPath, std::string p_sPageName);
    std::string GetPageFooter(PRESENTATION_PAGE p_nPresentationPage);

    /** returns the main HTML page
     *  @return the content of the index.html
     */
    std::string GetIndexHTML(std::string p_sImgPath);

    std::string GetAboutHTML(std::string p_sImgPath);
  
    std::string GetOptionsHTML(std::string p_sImgPath);
  
    std::string GetStatusHTML(std::string p_sImgPath);

    std::string GetConfigHTML(std::string p_sImgPath, CHTTPMessage* pRequest);


    /** builds a stringlist for all devices connected to a FUPPES instance
     *  @param pFuppes a pointer to a FUPPES instance
     *  @return the device list as string
     */
    std::string BuildFuppesDeviceList(CFuppes* pFuppes, std::string p_sImgPath);

};

#endif // _PRESENTATIONHANDLER_H
