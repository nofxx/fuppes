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
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "TranscodingMgr.h"

#ifndef DISABLE_TRANSCODING

  #ifndef DISABLE_LAME
  #include "LameWrapper.h"
  #endif

  #ifndef DISABLE_TWOLAME
  #include "TwoLameEncoder.h"
  #endif

  #ifndef DISABLE_VORBIS
  #include "VorbisWrapper.h"
  #endif

  #ifndef DISABLE_MUSEPACK
  #include "MpcWrapper.h"
  #endif

  #ifndef DISABLE_FLAC
  #include "FlacWrapper.h"
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

  #ifndef DISABLE_LAME
  CLameWrapper* pLameEncoder = new CLameWrapper();
  m_bLameAvailable = pLameEncoder->LoadLib();
  m_bUseLame = m_bLameAvailable;
  delete pLameEncoder;
  #endif

  #ifndef DISABLE_TWOLAME
  CTwoLameEncoder* pTwoLameEncoder = new CTwoLameEncoder();
  m_bTwoLameAvailable = pTwoLameEncoder->LoadLib();
  delete pTwoLameEncoder;
  #endif


  #ifndef DISABLE_VORBIS
  CVorbisDecoder* pVorbisDecoder = new CVorbisDecoder();
  m_bVorbisAvailable = pVorbisDecoder->LoadLib();
  delete pVorbisDecoder;
  #endif

  #ifndef DISABLE_MUSEPACK
  CMpcDecoder* pMpcDecoder = new CMpcDecoder();
  m_bMusePackAvailable = pMpcDecoder->LoadLib();
  delete pMpcDecoder;
  #endif

  #ifndef DISABLE_FLAC
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

bool CTranscodingMgr::IsTranscodingExtension(std::string p_sFileExt)
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
  else
    return false;
}


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
      *p_sHTMLVersion = "<p>Neither LAME nor TwoLame found. Transcoding disabled!</p>";
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

    #ifdef DISABLE_LAME
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

    #ifdef DISABLE_TWOLAME
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
    #ifdef DISABLE_VORBIS
    !p_sHTMLVersion ? cout << "compiled without Vorbis support" << endl : sHTML << "<td>compiled without Vorbis support</td></tr>";
    #else
    if(m_bVorbisAvailable && m_bTranscodeVorbis)
      !p_sHTMLVersion ? cout << "enabled" << endl : sHTML << "<td>enabled</td></tr>";
    else
      !p_sHTMLVersion ? cout << "disabled" << endl : sHTML << "<td>disabled</td></tr>";
    #endif

    // musepack
    !p_sHTMLVersion ? cout << "  musepack: " : sHTML << "<tr><td>MusePack</td>";
    #ifdef DISABLE_MUSEPACK
    !p_sHTMLVersion ? cout << "compiled without MusePack support" << endl : sHTML << "<td>compiled without MusePack support</td></tr>";
    #else
    if(m_bMusePackAvailable && m_bTranscodeMusePack)
      !p_sHTMLVersion ? cout << "enabled" << endl : sHTML << "<td>enabled</td></tr>";
    else
      !p_sHTMLVersion ? cout << "disabled" << endl : sHTML << "<td>disabled</td></tr>";
    #endif

    // flac
    !p_sHTMLVersion ? cout << "  flac    : " : sHTML << "<tr><td>FLAC</td>";
    #ifdef DISABLE_FLAC
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


CAudioEncoderBase* CTranscodingMgr::CreateAudioEncoder(std::string p_sFileExt)
{
  #ifndef DISABLE_TRANSCODING
  if(p_sFileExt.compare("mp3") == 0) {

    CAudioEncoderBase* pResult = NULL;

    if(m_bUseLame) {
      #ifndef DISABLE_LAME
      pResult = (CAudioEncoderBase*)(new CLameWrapper());
      #endif
    }
    else {
      #ifndef DISABLE_TWOLAME
      pResult = (CAudioEncoderBase*)(new CTwoLameEncoder());
      #endif
    }

    return pResult;
  }
  #endif

  return NULL;
}

CAudioDecoderBase* CTranscodingMgr::CreateAudioDecoder(std::string p_sFileExt, unsigned int* p_nBufferSize)
{
  CAudioDecoderBase* pResult = NULL;

  #ifndef DISABLE_TRANSCODING

  #ifndef DISABLE_VORBIS
  if(p_sFileExt.compare("ogg") == 0) {
    pResult = (CAudioDecoderBase*)(new CVorbisDecoder());
    *p_nBufferSize = MPC_DECODER_BUFFER_LENGTH * 4;
  }
  #endif

  #ifndef DISABLE_MUSEPACK
  if(p_sFileExt.compare("mpc") == 0) {
    pResult = (CAudioDecoderBase*)(new CMpcDecoder());
  }
  #endif

  #ifndef DISABLE_FLAC
  if(p_sFileExt.compare("flac") == 0) {
    pResult = (CAudioDecoderBase*)(new CFLACDecoder());
  }
  #endif

  #endif

  return pResult;
}
