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
#include "../DeviceSettings/DeviceSettings.h"

struct CAudioDetails
{
  int   nChannels;
  int   nSampleRate;
  int   nBitRate;
  unsigned int nPcmSize;  
};


class CTranscodeSessionInfo
{ 
  public:
    //CHTTPMessage* m_pHTTPMessage;
    bool          m_bBreakTranscoding;
    bool          m_bIsTranscoding;
    std::string   m_sInFileName;
    /*unsigned int* m_pnBinContentLength;
    char**        m_pszBinBuffer;*/
  
    unsigned int  m_nGuessContentLength;
  
    std::string   m_sArtist;
    std::string   m_sTitle;
    std::string   m_sAlbum;
    std::string   m_sGenre;
    std::string   m_sOriginalTrackNumber;
    int           m_nYear;
  
    //void SetTranscodeToTmpFile(std::string p_sTmpFileName);
        
    std::string   sACodec;
    std::string   sVCodec;
  
  private:
    std::string   m_sOutFileName;  
    bool          m_bTranscodeToFile;
};

class CAudioEncoderBase
{
  public:    
    virtual ~CAudioEncoderBase() {};
		virtual bool LoadLib() = 0;
  
    void SetAudioDetails(CAudioDetails* pAudioDetails) { m_pAudioDetails = pAudioDetails; }
    virtual void SetTranscodingSettings(CTranscodingSettings* pTranscodingSettings) = 0;
    void SetSessionInfo(CTranscodeSessionInfo* pSessionInfo) { m_pSessionInfo = pSessionInfo; }    
    
  
    virtual void  Init() = 0;      
    virtual int   EncodeInterleaved(short int p_PcmIn[], int p_nNumSamples, int p_nBytesRead) = 0;
    virtual int   Flush() = 0;
    virtual unsigned char* GetEncodedBuffer() = 0;
  
    virtual unsigned int GuessContentLength(unsigned int p_nNumPcmSamples) = 0;
  
  protected:
    CAudioDetails* m_pAudioDetails;
    CTranscodeSessionInfo* m_pSessionInfo;
};

class CAudioDecoderBase
{
  public:
	  virtual ~CAudioDecoderBase() {};
    virtual bool LoadLib() = 0;
    virtual bool OpenFile(std::string p_sFileName, CAudioDetails* pAudioDetails) = 0;      
    virtual void CloseFile() = 0;    
    virtual long DecodeInterleaved(char* p_PcmOut, int p_nBufferSize, int* p_nBytesRead) = 0;  
    virtual unsigned int NumPcmSamples() = 0;
  
  protected:
    fuppesLibHandle  m_LibHandle;
};

class CTranscoderBase
{
  public:
	  virtual ~CTranscoderBase() {};
    virtual bool Init(std::string p_sACodec, std::string p_sVCodec) {} ;
    virtual bool Transcode(CFileSettings* pFileSettings, std::string p_sInFile, std::string* p_psOutFile) = 0;
    virtual bool Threaded() = 0;
};

#endif // _WRAPPERBASE_H
