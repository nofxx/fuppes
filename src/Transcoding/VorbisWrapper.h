/***************************************************************************
 *            VorbisWrapper.h
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

#ifndef _VORBISWRAPPER_H
#define _VORBISWRAPPER_H

#include <string>
#include <vorbis/vorbisfile.h>

#ifdef __cplusplus
extern "C"
{

}
#endif

class CVorbisDecoder
{
  public:
    CVorbisDecoder();
    ~CVorbisDecoder();
  
    bool OpenFile(std::string p_sFileName);
    /**
     * @param   p_PcmOut[]
     * @return  number of decoded samples
     */
    long DecodeInterleaved(char* p_PcmOut, unsigned int p_nSize);
  
  private:
    OggVorbis_File m_VorbisFile;
    FILE*          m_pVorbisFileHandle;
    vorbis_info*   m_pVorbisInfo;
    int            m_nEndianess;
  
};

#endif /* _VORBISWRAPPER_H */

#endif /* DISABLE_TRANSCODING */
