/***************************************************************************
 *            WrapperBase.h
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
 
#ifndef _WRAPPERBASE_H
#define _WRAPPERBASE_H

#include <string>
#include "../Common/Common.h"
#include "../SharedLog.h"


struct CAudioDetails
{
  int   nChannels;
  int   nSamplerate;
  int   nBitrate;
  unsigned int nPcmSize;  
};

class CAudioEncoderBase
{
  public:    
    virtual ~CAudioEncoderBase() {};
		virtual bool LoadLib() = 0;
  
    void SetAudioDetails(CAudioDetails* pAudioDetails) { m_pAudioDetails = pAudioDetails; }
  
    virtual void  Init() = 0;      
    virtual int   EncodeInterleaved(short int p_PcmIn[], int p_nNumSamples, int p_nBytesRead) = 0;
    virtual int   Flush() = 0;
    virtual unsigned char* GetEncodedBuffer() = 0;
  
  protected:
    CAudioDetails* m_pAudioDetails;
};

class CAudioDecoderBase
{
  public:
	  virtual ~CAudioDecoderBase() {};
    virtual bool LoadLib() = 0;
    virtual bool OpenFile(std::string p_sFileName, CAudioDetails* pAudioDetails) = 0;      
    virtual void CloseFile() = 0;    
    virtual long DecodeInterleaved(char* p_PcmOut, int p_nBufferSize, int* p_nBytesRead) = 0;  
};

class CTranscoderBase
{
  public:
	  virtual ~CTranscoderBase() {};
    virtual bool Transcode(std::string p_sInFile, std::string p_sParams, std::string* p_psOutFile) = 0;
};

#endif // _WRAPPERBASE_H
