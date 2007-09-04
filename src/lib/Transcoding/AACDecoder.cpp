/***************************************************************************
 *            AACDecoder.cpp
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

#ifndef DISABLE_TRANSCODING
#include "AACDecoder.h"
#ifdef HAVE_FAAD

CAACDecoder::CAACDecoder()
{
}

CAACDecoder::~CAACDecoder()
{
}
  
bool CAACDecoder::LoadLib()
{
  return false;
}

bool CAACDecoder::OpenFile(std::string p_sFileName, CAudioDetails* pAudioDetails)
{
  return false;
}

void CAACDecoder::CloseFile()
{
}

long CAACDecoder::DecodeInterleaved(char* p_PcmOut, int p_nBufferSize, int* p_nBytesRead)
{
  return 0;
}

unsigned int CAACDecoder::NumPcmSamples()
{
  return 0;
}

#endif // HAVE_FAAD
#endif // DISABLE_TRANSCODING
