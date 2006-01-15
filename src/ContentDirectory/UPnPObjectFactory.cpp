/***************************************************************************
 *            UPnPObjectFactory.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005, 2006 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#include "UPnPObjectFactory.h"

#include "AudioItem.h"
#include "ImageItem.h"
#include "VideoItem.h"
#include "ContentDatabase.h"
#include "../Common.h"
#include "../SharedConfig.h"

#include <sstream>

using namespace std;

CUPnPObjectFactory::CUPnPObjectFactory(std::string p_sHTTPServerURL)
{
  m_sHTTPServerURL = p_sHTTPServerURL;
}

CUPnPObject* CUPnPObjectFactory::CreateObjectFromId(std::string p_sObjectID)
{
  CUPnPObject* pResult  = NULL;  
  CContentDatabase* pDB = new CContentDatabase();   
  unsigned int nObjID   = HexToInt(p_sObjectID);
  bool bIsAudio = false;
  
  stringstream sSQL;
  sSQL << "select PARENT_ID, TYPE, PATH, FILE_NAME, MD5, MIME_TYPE, DETAILS from OBJECTS where ID = " << nObjID;
  pDB->Select(sSQL.str());
  if(!pDB->Eof())
  {
    
    if(pDB->GetResult()->GetValue("TYPE").compare("100") == 0)
    {
      pResult = new CImageItem(m_sHTTPServerURL, pDB->GetResult()->GetValue("MIME_TYPE"));      
    }
    else if(pDB->GetResult()->GetValue("TYPE").compare("200") == 0)
    {
      pResult  = new CAudioItem(m_sHTTPServerURL, pDB->GetResult()->GetValue("MIME_TYPE"));
      bIsAudio = true;
    }
    else if(pDB->GetResult()->GetValue("TYPE").compare("300") == 0)
    {
      pResult = new CVideoItem(m_sHTTPServerURL, pDB->GetResult()->GetValue("MIME_TYPE"));
    }
    
    pResult->SetObjectID(p_sObjectID);    
    //pTmpItem->SetParent(pParentFolder);     
    pResult->SetFileName(pDB->GetResult()->GetValue("PATH"));

    /* set object name */
    stringstream sName;
    sName << TruncateFileExt(pDB->GetResult()->GetValue("FILE_NAME"));
    
    if(bIsAudio && (((CAudioItem*)pResult)->SetupTranscoding()))
    {      
      if(((CAudioItem*)pResult)->GetDoTranscode() && CSharedConfig::Shared()->GetDisplaySettings().bShowTranscodingTypeInItemNames)
      {
        switch(((CAudioItem*)pResult)->GetDecoderType())
        {
          case AUDIO_DECODER_UNKNOWN:
            break;
          case AUDIO_DECODER_VORBIS:
            sName << " (ogg)";
            break;
          case AUDIO_DECODER_MUSEPACK:
            sName << " (mpc)";
            break;
          case AUDIO_DECODER_FLAC:
            sName << " (flac)";
          case AUDIO_DECODER_NONE:
            break;                
        }                     
      }      
    }
    
    pResult->SetName(sName.str());    
  }
  
  delete pDB;
  return pResult;
}
