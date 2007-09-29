/***************************************************************************
 *            TranscodingMgr.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2006, 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#include "TranscodingMgr.h"

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#ifndef DISABLE_TRANSCODING

  // encoder
  #ifdef HAVE_LAME
  #include "LameWrapper.h"
  #endif

  #ifdef HAVE_TWOLAME
  #include "TwoLameEncoder.h"
  #endif

  #include "PcmEncoder.h"
  #include "WavEncoder.h"

  // decoder
  #ifdef HAVE_VORBIS
  #include "VorbisWrapper.h"
  #endif

  #ifdef HAVE_MUSEPACK
  #include "MpcWrapper.h"
  #endif

  #ifdef HAVE_FLAC
  #include "FlacWrapper.h"
  #endif

  #ifdef HAVE_FAAD
  #include "FaadWrapper.h"
  #endif

  // transcoder
  #ifdef HAVE_LIBAVFORMAT
  #ifdef ENABLE_VIDEO_TRANSCODING
  #include "FFmpegWrapper.h"
  #endif
  #endif

  #ifdef HAVE_IMAGEMAGICK
  #include "ImageMagickWrapper.h"
  #endif

#endif

#include <iostream>

using namespace std;

CTranscodingMgr* CTranscodingMgr::m_Instance = 0;

CTranscodingMgr* CTranscodingMgr::Shared()
{
	if (m_Instance == 0)
		m_Instance = new CTranscodingMgr();
	return m_Instance;
}

CTranscodingMgr::CTranscodingMgr()
{
  m_bLameAvailable     = false;
  m_bTwoLameAvailable  = false;
  m_bVorbisAvailable   = false;
  m_bFlacAvailable     = false;
  m_bMusePackAvailable = false;

  #ifndef DISABLE_TRANSCODING

  m_bUseLame = false;

  #ifdef HAVE_LAME
  CLameWrapper* pLameEncoder = new CLameWrapper();
  m_bLameAvailable = pLameEncoder->LoadLib();
  m_bUseLame = m_bLameAvailable;
  delete pLameEncoder;
  #endif

  #ifdef HAVE_TWOLAME
  CTwoLameEncoder* pTwoLameEncoder = new CTwoLameEncoder();
  m_bTwoLameAvailable = pTwoLameEncoder->LoadLib();
  delete pTwoLameEncoder;
  #endif


  #ifdef HAVE_VORBIS
  CVorbisDecoder* pVorbisDecoder = new CVorbisDecoder();
  m_bVorbisAvailable = pVorbisDecoder->LoadLib();
  delete pVorbisDecoder;
  #endif

  #ifdef HAVE_MUSEPACK
  CMpcDecoder* pMpcDecoder = new CMpcDecoder();
  m_bMusePackAvailable = pMpcDecoder->LoadLib();
  delete pMpcDecoder;
  #endif

  #ifdef HAVE_FLAC
  CFLACDecoder* pFlacDecoder = new CFLACDecoder();
  m_bFlacAvailable = pFlacDecoder->LoadLib();
  delete pFlacDecoder;
  #endif

  #endif // DISABLE_TRANSCODING

  m_bTranscodeVorbis   = m_bVorbisAvailable;
  m_bTranscodeMusePack = m_bMusePackAvailable;
  m_bTranscodeFlac     = m_bFlacAvailable;
}

CTranscodingMgr::~CTranscodingMgr()
{
}

bool CTranscodingMgr::IsTranscodingAvailable()
{
  #ifdef DISABLE_TRANSCODING
  return false;
  #else
  return ((m_bLameAvailable && m_bUseLame) || (m_bTwoLameAvailable && !m_bUseLame)) && (m_bVorbisAvailable || m_bFlacAvailable || m_bMusePackAvailable);
  #endif
}

/*bool CTranscodingMgr::IsTranscodingExtension(std::string p_sFileExt)
{
  p_sFileExt = ToLower(p_sFileExt);

  if((p_sFileExt.compare("mp3") == 0))
    return false;
  
  else if((p_sFileExt.compare("ogg") == 0)  && IsTranscodingAvailable() && m_bVorbisAvailable   && m_bTranscodeVorbis)
    return true;
  else if((p_sFileExt.compare("mpc") == 0)  && IsTranscodingAvailable() && m_bMusePackAvailable && m_bTranscodeMusePack)
    return true;
  else if((p_sFileExt.compare("flac") == 0) && IsTranscodingAvailable() && m_bFlacAvailable     && m_bTranscodeFlac)
    return true;
  
  #ifdef ENABLE_VIDEO_TRANSCODING
  else if(p_sFileExt.compare("flv") == 0)
    return true;
  else if(p_sFileExt.compare("wmv") == 0)
    return true;
  else if(p_sFileExt.compare("vdr") == 0)
    return true;  
  else if(p_sFileExt.compare("asf") == 0)
    return true; 
  #endif
  
  else if(p_sFileExt.compare("jpg") == 0)
    return true;
  
  else
    return false;
}*/

/*TRANSCODING_TYPE CTranscodingMgr::GetTranscodingType(CDeviceSettings* pDeviceSettings)
{
  */
  /*if((p_sFileExt.compare("mp3") == 0))
    return TT_NONE;
  
  else if((p_sFileExt.compare("ogg") == 0)  && IsTranscodingAvailable() && m_bVorbisAvailable   && m_bTranscodeVorbis)
    return TT_THREADED_DECODER_ENCODER;
  else if((p_sFileExt.compare("mpc") == 0)  && IsTranscodingAvailable() && m_bMusePackAvailable && m_bTranscodeMusePack)
    return TT_THREADED_DECODER_ENCODER;
  else if((p_sFileExt.compare("flac") == 0) && IsTranscodingAvailable() && m_bFlacAvailable     && m_bTranscodeFlac)
    return TT_THREADED_DECODER_ENCODER; 
  
  else if(p_sFileExt.compare("flv") == 0)
    return TT_THREADED_TRANSCODER;
  else if(p_sFileExt.compare("wmv") == 0)
    return TT_THREADED_TRANSCODER;
  else if(p_sFileExt.compare("vdr") == 0)
    return TT_NONE;  
  else if(p_sFileExt.compare("asf") == 0)
    return TT_THREADED_TRANSCODER; 
  
  else if(p_sFileExt.compare("jpg") == 0)
    return TT_TRANSCODER;*/
  
  //else
    /*return TT_NONE;  
}*/

void CTranscodingMgr::SetDoTranscodeVorbis(bool p_bDoTranscodeVorbis)
{
  m_bVorbisAvailable ? m_bTranscodeVorbis = p_bDoTranscodeVorbis : m_bTranscodeVorbis = false;
}

void CTranscodingMgr::SetDoTranscodeFlac(bool p_bDoTranscodeFlac)
{
  m_bFlacAvailable ? m_bTranscodeFlac = p_bDoTranscodeFlac : m_bTranscodeFlac = false;
}

void CTranscodingMgr::SetDoTranscodeMusePack(bool p_bDoTranscodeMusePack)
{
  m_bMusePackAvailable ? m_bTranscodeMusePack = p_bDoTranscodeMusePack : m_bTranscodeMusePack = false;
}

void CTranscodingMgr::SetDoUseLame(bool p_bDoUseLame)
{
  m_bLameAvailable ? m_bUseLame = p_bDoUseLame : m_bUseLame = false;
}


void CTranscodingMgr::PrintTranscodingSettings(std::string* p_sHTMLVersion)
{
  #ifdef DISABLE_TRANSCODING
  if(p_sHTMLVersion)
    *p_sHTMLVersion = "<p>compiled without transcoding support.</p>";
  else
    cout << "compiled without transcoding support." << endl;
  #else

  // no encoder available
  if(!m_bLameAvailable && !m_bTwoLameAvailable)
  {
    if(p_sHTMLVersion) {
		  stringstream sTmp;
			sTmp << "<p>Neither LAME nor TwoLame found. Transcoding disabled!";
			#ifdef WIN32
      sTmp << "Get a copy of the lame_enc.dll and" << endl;
      sTmp << "put it in the application directory." << endl;
      #endif
			sTmp << "</p>" << endl;
      *p_sHTMLVersion = sTmp.str();;
    }
    else {    
      cout << endl;
      cout << "Neither LAME nor TwoLame found. Transcoding disabled!" << endl;
      #ifdef WIN32
      cout << "Get a copy of the lame_enc.dll and" << endl;
      cout << "put it in the application directory." << endl;
      #endif
      cout << endl;
    }
      
    return;
  }


  // no decoder
  if(!m_bVorbisAvailable && !m_bMusePackAvailable && !m_bFlacAvailable)
  {
    if(p_sHTMLVersion) {
      *p_sHTMLVersion = "<p>no decoding library found. Transcoding disabled!</p>";
    }
    else {
      cout << endl;
      cout << "no decoding library found. Transcoding disabled!" << endl;
      cout << endl;
    }    
  }
  else
  {
    stringstream sHTML;

    sHTML << "<table>";

    !p_sHTMLVersion ?
      cout << "transcoding settings:" << endl :
      sHTML << "<tr><th colspan=\"2\">Encoder</th></tr>";

    // lame
    !p_sHTMLVersion ? cout << "  lame    : " : sHTML << "<tr><td>LAME</td>";

    #ifndef HAVE_LAME
    !p_sHTMLVersion ? cout << "compiled without Lame support" << endl : sHTML << "<td>compiled without Lame support</td></tr>";
    #else
    if(m_bLameAvailable)
      !p_sHTMLVersion ? cout << "enabled" << endl : sHTML << "<td>enabled</td></tr>";
    else
      !p_sHTMLVersion ? cout << "disabled" << endl : sHTML << "<td>disabled</td></tr>";
    #endif
    // lame end

    // twolame
    !p_sHTMLVersion ? cout << "  twolame : " : sHTML << "<tr><td>TwoLAME</td>";

    #ifndef HAVE_TWOLAME
    !p_sHTMLVersion ? cout << "compiled without TwoLame support" << endl : sHTML << "<td>compiled without TwoLame support</td></tr>";
    #else
    if(m_bTwoLameAvailable)
      !p_sHTMLVersion ? cout << "enabled" << endl : sHTML << "<td>enabled</td></tr>";
    else
      !p_sHTMLVersion ? cout << "disabled" << endl : sHTML << "<td>disabled</td></tr>";
    #endif
    // twolame end


    // active encoder
    !p_sHTMLVersion ?
      cout << endl << "active encoder: " :
      sHTML << "<tr><td>active encoder</td><td>";
    if(m_bUseLame)
      !p_sHTMLVersion ? cout << "lame" : sHTML << "LAME</td></tr>";
    else
      !p_sHTMLVersion ? cout << "twolame" : sHTML << "TwoLAME</td></tr>";


    !p_sHTMLVersion ?
      cout << endl << endl :
      sHTML << "<tr><th colspan=\"2\">Decoder</th></tr>";


    // vorbis
    !p_sHTMLVersion ? cout << "  vorbis  : " : sHTML << "<tr><td>Vorbis</td>";
    #ifndef HAVE_VORBIS
    !p_sHTMLVersion ? cout << "compiled without Vorbis support" << endl : sHTML << "<td>compiled without Vorbis support</td></tr>";
    #else
    if(m_bVorbisAvailable && m_bTranscodeVorbis)
      !p_sHTMLVersion ? cout << "enabled" << endl : sHTML << "<td>enabled</td></tr>";
    else
      !p_sHTMLVersion ? cout << "disabled" << endl : sHTML << "<td>disabled</td></tr>";
    #endif

    // musepack
    !p_sHTMLVersion ? cout << "  musepack: " : sHTML << "<tr><td>MusePack</td>";
    #ifndef HAVE_MUSEPACK
    !p_sHTMLVersion ? cout << "compiled without MusePack support" << endl : sHTML << "<td>compiled without MusePack support</td></tr>";
    #else
    if(m_bMusePackAvailable && m_bTranscodeMusePack)
      !p_sHTMLVersion ? cout << "enabled" << endl : sHTML << "<td>enabled</td></tr>";
    else
      !p_sHTMLVersion ? cout << "disabled" << endl : sHTML << "<td>disabled</td></tr>";
    #endif

    // flac
    !p_sHTMLVersion ? cout << "  flac    : " : sHTML << "<tr><td>FLAC</td>";
    #ifndef HAVE_FLAC
    !p_sHTMLVersion ? cout << "compiled without FLAC support" << endl : sHTML << "<td>compiled without FLAC support</td></tr>";
    #else
    if(m_bFlacAvailable && m_bTranscodeFlac)
      !p_sHTMLVersion ? cout << "enabled" << endl : sHTML << "<td>enabled</td></tr>";
    else
      !p_sHTMLVersion ? cout << "disabled" << endl : sHTML << "<td>disabled</td></tr>";
    #endif

    if(!p_sHTMLVersion)
      cout << endl;
    else {
      sHTML << "</table>";
      *p_sHTMLVersion = sHTML.str();
    }
  }
  #endif
}


CAudioEncoderBase* CTranscodingMgr::CreateAudioEncoder(ENCODER_TYPE p_nEncoderType)
{
  CAudioEncoderBase* pResult = NULL;  
  #ifndef DISABLE_TRANSCODING 
  
  switch(p_nEncoderType) {
    
    case ET_LAME:
      #ifdef HAVE_LAME
      pResult = (CAudioEncoderBase*)(new CLameWrapper());
      #endif
      break;
    
    case ET_TWOLAME:
      #ifdef HAVE_TWOLAME
      pResult = (CAudioEncoderBase*)(new CTwoLameEncoder());
      #endif
      break;
    
    case ET_WAV:
      pResult = (CAudioEncoderBase*)(new CWavEncoder());
      break;
    
    case ET_PCM:
      pResult = (CAudioEncoderBase*)(new CPcmEncoder());
      break;
    
    default:
      break;
  }
  #endif

  return pResult;
}

CAudioDecoderBase* CTranscodingMgr::CreateAudioDecoder(DECODER_TYPE p_nDecoderType, unsigned int* p_nBufferSize)
{
  CAudioDecoderBase* pResult = NULL;

  #ifndef DISABLE_TRANSCODING
  switch(p_nDecoderType) {
    
    case DT_OGG_VORBIS:
      #ifdef HAVE_VORBIS
      pResult = (CAudioDecoderBase*)(new CVorbisDecoder());
      #endif
      break;
    
    case DT_MUSEPACK:
      #ifdef HAVE_MUSEPACK
      pResult = (CAudioDecoderBase*)(new CMpcDecoder());
      *p_nBufferSize = MPC_DECODER_BUFFER_LENGTH * 4;
      #endif
      break;
  
    case DT_FLAC:
      #ifdef HAVE_FLAC
      pResult = (CAudioDecoderBase*)(new CFLACDecoder());
      #endif
      break;
    
    case DT_FAAD:
      #ifdef HAVE_FAAD
      pResult = (CAudioDecoderBase*)(new CFaadWrapper());
      *p_nBufferSize = FAAD_MIN_STREAMSIZE * FAAD_MAX_CHANNELS;
      #endif
      break;
    
    default:
      break;
  } 
  
  #endif

  return pResult;
}

CTranscoderBase* CTranscodingMgr::CreateTranscoder(TRANSCODER_TYPE p_nTranscoderType)
{
  CTranscoderBase* pResult = NULL;
  
  #ifdef HAVE_LIBAVFORMAT
  #ifdef ENABLE_VIDEO_TRANSCODING
  if(p_nTranscoderType == TTYP_FFMPEG) {     
    pResult = new CFFmpegWrapper();
  }
  #endif
  #endif
  
  #ifndef DISABLE_TRANSCODING
  #ifdef HAVE_IMAGEMAGICK
  if(p_nTranscoderType == TTYP_IMAGE_MAGICK) {
    pResult = new CImageMagickWrapper();
  }
  #endif
  #endif
  
  return pResult;
}
