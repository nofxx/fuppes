/***************************************************************************
 *            VorbisWrapper.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef DISABLE_TRANSCODING

#include "VorbisWrapper.h"

CVorbisDecoder::CVorbisDecoder()
{
  /* determine endianness (clever trick courtesy of Nicholas Devillard,
   * (http://www.eso.org/~ndevilla/endian/) */
  int testvar = 1;
  if(*(char *)&testvar)
    m_nEndianess = 0;  // little endian
  else
    m_nEndianess = 1;  // big endian	   
}

CVorbisDecoder::~CVorbisDecoder()
{
  //fclose(m_pVorbisFileHandle);
  ov_clear(&m_VorbisFile);
}
  
bool CVorbisDecoder::OpenFile(std::string p_sFileName)
{
  if ((m_pVorbisFileHandle = fopen(p_sFileName.c_str(), "r")) == NULL)
  {
    fprintf(stderr, "Cannot open %s\n", p_sFileName.c_str()); 
    return false;
  }
	
  if(ov_open(m_pVorbisFileHandle, &m_VorbisFile, NULL, 0) < 0) 
  {
    fprintf(stderr,"Input does not appear to be an Ogg bitstream.\n");      
    return false;
  }	 

  m_pVorbisInfo = ov_info(&m_VorbisFile, -1);
     
  char **ptr = ov_comment(&m_VorbisFile,-1)->user_comments;
  while(*ptr)
  {
    fprintf(stderr,"%s\n",*ptr);
    ++ptr;
  }
  fprintf(stderr,"\nBitstream is %d channel, %ldHz\n", m_pVorbisInfo->channels, m_pVorbisInfo->rate);
  fprintf(stderr,"\nDecoded length: %ld samples\n", (long)ov_pcm_total(&m_VorbisFile, -1));
  fprintf(stderr,"Encoded by: %s\n\n", ov_comment(&m_VorbisFile,-1)->vendor);
  
  return true;
}

long CVorbisDecoder::DecodeInterleaved(char* p_PcmOut, unsigned int p_nSize)
{  
  int bitstream = 0; 
  int bytesRead = ov_read(&m_VorbisFile, p_PcmOut, p_nSize, m_nEndianess, 2, 1, &bitstream);
  
  if(bytesRead == 0)
  {
      /* todo: error handling */
    return -1;
  }
  else if(bytesRead < 0) 
  {
    /* todo: error handling */
    /* error in the stream */
    return -1;
  }
  else 
  {
    if(bitstream != 0)
      return -1;
    
    /* calc samples an encode */
    long samplesRead = bytesRead / m_pVorbisInfo->channels / sizeof(short int);
    return samplesRead;
  }  
}

#endif /* DISABLE_TRANSCODING */
