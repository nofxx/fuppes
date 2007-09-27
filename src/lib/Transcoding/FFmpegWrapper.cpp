/***************************************************************************
 *            FFmpegWrapper.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#include "FFmpegWrapper.h"

#ifdef HAVE_LIBAVFORMAT
#ifdef ENABLE_VIDEO_TRANSCODING

#include "../SharedConfig.h"

#include <iostream>
#include <sstream>

using namespace std;

CFFmpegWrapper::~CFFmpegWrapper()
{
  delete pFFmpeg;
}

bool CFFmpegWrapper::Init(std::string p_sACodec, std::string p_sVCodec)
{
  m_sACodec = p_sACodec;
  m_sVCodec = p_sVCodec;
  
  pFFmpeg = new CFFmpeg();
}

bool CFFmpegWrapper::Transcode(CFileSettings* pFileSettings, std::string p_sInFile, std::string* p_psOutFile)
{
  *p_psOutFile = CSharedConfig::Shared()->CreateTempFileName() + "." + pFileSettings->Extension(m_sACodec, m_sVCodec);
  
  int nArgs = 0;
  char* szArgs[20];   
  
  //cout << "acodec: " << m_sACodec << " vcodec: " << m_sVCodec << endl;
  
  szArgs[nArgs] = (char*)malloc(strlen("ffmpeg") * sizeof(char));
  strcpy(szArgs[nArgs], "ffmpeg");
  nArgs++;
  
  szArgs[nArgs] = (char*)malloc(strlen("-i") * sizeof(char));
  strcpy(szArgs[nArgs], "-i");
  nArgs++;
  
  szArgs[nArgs] = (char*)malloc((strlen(p_sInFile.c_str())  + 1) * sizeof(char));
  strcpy(szArgs[nArgs], p_sInFile.c_str());  
  nArgs++;

  // Video settings
  
  string VCodec = pFileSettings->pTranscodingSettings->VideoCodec(m_sVCodec);
  //cout << "out vcodec: " << VCodec << endl;
  
  szArgs[nArgs] = (char*)malloc(strlen("-vcodec") * sizeof(char));
  strcpy(szArgs[nArgs], "-vcodec");
  nArgs++;  
  
  szArgs[nArgs] = (char*)malloc((strlen(VCodec.c_str()) + 1) * sizeof(char));
  strcpy(szArgs[nArgs], VCodec.c_str());
  nArgs++;  
  
  if(pFileSettings->pTranscodingSettings->VideoBitRate() > 0) {
    stringstream sBitRate;
    sBitRate << pFileSettings->pTranscodingSettings->VideoBitRate();
  
    szArgs[nArgs] = (char*)malloc(strlen("-b") * sizeof(char));
    strcpy(szArgs[nArgs], "-b");
    nArgs++;  
  
    szArgs[nArgs] = (char*)malloc((strlen(sBitRate.str().c_str()) + 1) * sizeof(char));
    strcpy(szArgs[nArgs], sBitRate.str().c_str());
    nArgs++;
  }
    
  // Audio settings
    
  string ACodec = pFileSettings->pTranscodingSettings->AudioCodec(m_sACodec);
  
  //cout << "out acodec: " << ACodec << endl;  
  
  szArgs[nArgs] = (char*)malloc(strlen("-acodec") * sizeof(char));
  strcpy(szArgs[nArgs], "-acodec");
  nArgs++;  
  
  szArgs[nArgs] = (char*)malloc((strlen(ACodec.c_str()) + 1) * sizeof(char));
  strcpy(szArgs[nArgs], ACodec.c_str());
  nArgs++;
  
  
  if(pFileSettings->pTranscodingSettings->AudioSampleRate() > 0) {
    stringstream sSampleRate;
    sSampleRate << pFileSettings->pTranscodingSettings->AudioSampleRate();
    
    szArgs[nArgs] = (char*)malloc(strlen("-ar") * sizeof(char));
    strcpy(szArgs[nArgs], "-ar");
    nArgs++;  
  
    szArgs[nArgs] = (char*)malloc((strlen(sSampleRate.str().c_str()) + 1) * sizeof(char));
    strcpy(szArgs[nArgs], sSampleRate.str().c_str());
    nArgs++;    
  }
  
  
  if(pFileSettings->pTranscodingSettings->AudioBitRate() > 0) {
    stringstream sBitRate;
    sBitRate << pFileSettings->pTranscodingSettings->AudioBitRate();
    
    szArgs[nArgs] = (char*)malloc(strlen("-ab") * sizeof(char));
    strcpy(szArgs[nArgs], "-ab");
    nArgs++;  
  
    szArgs[nArgs] = (char*)malloc((strlen(sBitRate.str().c_str()) + 1) * sizeof(char));
    strcpy(szArgs[nArgs], sBitRate.str().c_str());
    nArgs++;    
  }

  
  szArgs[nArgs] = (char*)malloc(strlen("-ac") * sizeof(char));
  strcpy(szArgs[nArgs], "-ac");
  nArgs++;
  
  szArgs[nArgs] = (char*)malloc(strlen("2") * sizeof(char));
  strcpy(szArgs[nArgs], "2");
  nArgs++;  
    
  

  szArgs[nArgs] = (char*)malloc((strlen(p_psOutFile->c_str()) + 1) * sizeof(char));
  strcpy(szArgs[nArgs], p_psOutFile->c_str());  
  nArgs++;
  

  
  /*szArgs[nArgs] = (char*)malloc(sizeof(char));
  szArgs[nArgs] = '\0';*/

  pFFmpeg->ffmpeg_main(nArgs, szArgs);
  
  for(int i = 0; i < nArgs; i++) {
    free(szArgs[i]);
  }
  
  return true;  
}


#endif // ENABLE_VIDEO_TRANSCODING
#endif // HAVE_LIBAVFORMAT
