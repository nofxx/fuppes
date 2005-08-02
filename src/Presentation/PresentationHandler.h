/***************************************************************************
 *            PresentationHandler.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 *  Copyright (C) 2005 Thomas Schnitzler <tschnitzler@users.sourceforge.net>
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
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#ifndef _PRESENTATIONHANDLER_H
#define _PRESENTATIONHANDLER_H

/*===============================================================================
 INCLUDES
===============================================================================*/

#include "../Fuppes.h"
#include "../UPnPDevice.h"
#include "../HTTPMessage.h"

#include <string>
#include <vector>

/*===============================================================================
 CLASS CPresentationHandler
===============================================================================*/

class CPresentationHandler: public IFuppes
{

/* <PUBLIC> */

public:

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

  /** constructor
  */  
  CPresentationHandler();

  /** destructor
  */
  virtual ~CPresentationHandler();

/*===============================================================================
 INSTANCE
===============================================================================*/
  
  /** adds a instance of FUPPES
  *  @param pFuppes  the instance to add
  */
  void AddFuppesInstance(CFuppes* pFuppes);
  
/* <\PUBLIC> */

/* <PRIVATE> */

private:

/*===============================================================================
 REQUESTS
===============================================================================*/
  
  /** handles HTTP messages add returns a corresponding message
  *  @param pSender  sender of the incoming message
  *  @param pMessage  the incoming message
  *  @param pResult  the outgoing message
  */
  void OnReceivePresentationRequest(CFuppes* pSender, CHTTPMessage* pMessage, CHTTPMessage* pResult);
  
  /** handles HTTP requests
  *  @param p_sRequest  the message to handle
  */
  std::string HandleRequest(std::string p_sRequest);  

/*===============================================================================
 GET
===============================================================================*/

  /** returns the HTML header
  *  @return the HTML header as string
  */
  std::string GetXHTMLHeader();  

  /** returns the main HTML page
  *  @return the content of the index.html
  */
  std::string GetIndexHTML();

/*===============================================================================
 HELPER
===============================================================================*/
  
  /** builds a stringlist for all devices connected to a FUPPES instance
  *  @param pFuppes a pointer to a FUPPES instance
  *  @return the device list as string
  */
  std::string BuildFuppesDeviceList(CFuppes* pFuppes);
    
/*===============================================================================
 MEMBERS
===============================================================================*/
  
  std::vector<CFuppes*> m_vFuppesInstances;
    
/* <\PRIVATE> */

};

#endif /* _PRESENTATIONHANDLER_H */
