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

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <string>
#include "../Common/Common.h"
#include "../SharedLog.h"
#include "../DeviceSettings/DeviceSettings.h"

struct CAudioDetails
{
  CAudioDetails()
  {
    nNumChannels    = 0;
    nSampleRate     = 0;
    nBitRate        = 0;
    nNumPcmSamples  = 0;
  }
  
  CAudioDetails(CAudioDetails* pAudioDetails)
  {
    nNumChannels    = pAudioDetails->nNumChannels;
    nSampleRate     = pAudioDetails->nSampleRate;
    nBitRate        = pAudioDetails->nBitRate;
    nNumPcmSamples  = pAudioDetails->nNumPcmSamples;
  }
  
  int           nNumChannels;
  int           nSampleRate;
  int           nBitRate;
  unsigned int  nNumPcmSamples;  
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

typedef enum ENDIANESS
{
  E_LITTLE_ENDIAN = 0,
  E_BIG_ENDIAN = 1
} ENDIANESS;

class CAudioEncoderBase
{
  public:
    CAudioEncoderBase() {
      // default input endianness is machine dependent
      
      /* determine endianness (clever trick courtesy of Nicholas Devillard,
       * (http://www.eso.org/~ndevilla/endian/) */
      int testvar = 1;
      if(*(char *)&testvar)
        m_nInEndianess = E_LITTLE_ENDIAN;
      else
        m_nInEndianess = E_BIG_ENDIAN;
                               
      m_pAudioDetails = NULL;
    };    
    
    virtual ~CAudioEncoderBase() { if(m_pAudioDetails) delete m_pAudioDetails; };
		virtual bool LoadLib() = 0;
  
    virtual void SetAudioDetails(CAudioDetails* pAudioDetails) { 
      if(!m_pAudioDetails)
       m_pAudioDetails = new CAudioDetails(pAudioDetails);
    }
    virtual void SetTranscodingSettings(CTranscodingSettings* pTranscodingSettings) = 0;
    void SetSessionInfo(CTranscodeSessionInfo* pSessionInfo) { m_pSessionInfo = pSessionInfo; }    
    
  
    virtual void  Init() = 0;      
    virtual int   EncodeInterleaved(short int p_PcmIn[], int p_nNumSamples, int p_nBytesRead) = 0;
    virtual int   Flush() = 0;
    virtual unsigned char* GetEncodedBuffer() = 0;
  
    virtual unsigned int GuessContentLength(unsigned int p_nNumPcmSamples) = 0;
  
    ENDIANESS   InEndianess() { return m_nInEndianess; }
  
  protected:
    CAudioDetails* m_pAudioDetails;
    CTranscodeSessionInfo* m_pSessionInfo;
    
    ENDIANESS        m_nInEndianess;
};


class CAudioDecoderBase
{
  public:
    CAudioDecoderBase() {
      // default output endianness is machine dependent
      
      /* determine endianness (clever trick courtesy of Nicholas Devillard,
       * (http://www.eso.org/~ndevilla/endian/) */
      int testvar = 1;
      if(*(char *)&testvar)
        m_nOutEndianess = E_LITTLE_ENDIAN;
      else
        m_nOutEndianess = E_BIG_ENDIAN;
    };
    
	  virtual ~CAudioDecoderBase() {};
    virtual bool LoadLib() = 0;
    virtual bool OpenFile(std::string p_sFileName, CAudioDetails* pAudioDetails) = 0;      
    virtual void CloseFile() = 0;
    virtual void SetOutputEndianness(ENDIANESS p_nEndianess) { m_nOutEndianess = p_nEndianess; }
    virtual long DecodeInterleaved(char* p_PcmOut, int p_nBufferSize, int* p_nBytesRead) = 0;  
    virtual unsigned int NumPcmSamples() = 0;
  
    ENDIANESS   OutEndianess() { return m_nOutEndianess; }
  
  protected:
    fuppesLibHandle  m_LibHandle;
    ENDIANESS        m_nOutEndianess;
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
