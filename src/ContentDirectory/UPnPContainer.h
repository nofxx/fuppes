/***************************************************************************
 *            UPnPContainer.h
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
 
#ifndef _UPNPCONTAINER_H
#define _UPNPCONTAINER_H

/*===============================================================================
 INCLUDES
===============================================================================*/

#include "UPnPObject.h"
#include "UPnPItem.h"
#include "../UPnPActions/UPnPBrowse.h"

#include <vector>
#include <string>
#include <libxml/xmlwriter.h>

/*===============================================================================
 DEFINITIONS
===============================================================================*/

typedef enum tagSORT_CRITERIA
{
  SORT_CRITERIA_NONE,
  SORT_CRITERIA_MAX
}SORT_CRITERIA;

/*===============================================================================
 CLASS CStorageFolder
===============================================================================*/

class CUPnPContainer: public CUPnPObject
{

/* <PUBLIC> */

public:

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

  /** constructor
  *  @param  p_sHTTPServerURL  URL of the HTTP server
  */
  CUPnPContainer(std::string p_sHTTPServerURL);
  
  /** destructor
  */
  ~CUPnPContainer();

/*===============================================================================
 ADD
===============================================================================*/

  /** adds a UPnP object to the storage folder
  *  @param  pObject  the object to add
  */
  void AddUPnPObject(CUPnPObject* pObject);

/*===============================================================================
 GET
===============================================================================*/

  /** returns the content to show after a UPnP browse action
  *  @param  pBrowseAction  the action to handle
  *  @param  p_nNumberReturned  number of returned objects
  *  @param  p_nTotalMatches  number of all matching objects
  *  @return  
  */
  std::string GetContentAsString(CUPnPBrowse* pBrowseAction, unsigned int* p_nNumberReturned, unsigned int* p_nTotalMatches);

  /** returns count of all objects
  *  @return  the object count as string
  */
  std::string GetChildCountAsString();
  
  /** Writes a description to a XML container
  *  @param  pWriter  the XML container to write description to
  */
  void GetDescription(xmlTextWriterPtr pWriter);
  
/* <\PUBLIC> */

/* <PRIVATE> */

  private:

/*===============================================================================
 MEMBER
===============================================================================*/

    std::vector<CUPnPObject*> m_vObjects;

/*===============================================================================
 HELPER
===============================================================================*/

  /** sorts a object list
  *  @param  pObjList  the object list to sort
  *  @param  p_nSortCriteria  the sort criteria to sort by
  */
  void SortContent(std::vector<CUPnPObject*>* pObjList, SORT_CRITERIA p_nSortCriteria);
  
/* <\PRIVATE> */

};

#endif /* _UPNPCONTAINER_H */
