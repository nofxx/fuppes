/***************************************************************************
 *            PcmEncoder.cpp
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

#include "PcmEncoder.h"

#include <iostream>
using namespace std;

#ifndef DISABLE_TRANSCODING

CPcmEncoder::CPcmEncoder()
{
  
    f.open("/home/ulrich/Desktop/test.pcm", ios::out | ios::trunc | ios::binary);
    //f << "Dieser Text geht in die Datei" << endl;
    
}

CPcmEncoder::~CPcmEncoder()
{
  f.close();
}


int CPcmEncoder::EncodeInterleaved(short int p_PcmIn[], int p_nNumSamples, int p_nBytesRead)
{
  cout << "CPcmEncoder::EncodeInterleaved - " << p_nNumSamples << " - " << sizeof(p_PcmIn) << endl;
  fflush(stdout);
  
  if(m_sBuffer != NULL) {
    free(m_sBuffer);
  }
  
  f.write((const char*)p_PcmIn, p_nBytesRead);
  
  m_sBuffer = (unsigned char*)malloc(p_nBytesRead);
  
  memcpy(m_sBuffer, p_PcmIn, p_nBytesRead);
  
  return p_nBytesRead;
}

#endif // DISABLE_TRANSCODING
