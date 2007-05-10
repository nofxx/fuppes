/***************************************************************************
 *            VorbisWrapper.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#ifndef DISABLE_TRANSCODING
#ifdef HAVE_VORBIS

#ifndef _VORBISWRAPPER_H
#define _VORBISWRAPPER_H

#include "../Common/Common.h"
#include <string>
#include <vorbis/vorbisfile.h>

#include "WrapperBase.h"

#ifdef __cplusplus
extern "C"
{
  /* int ov_open(FILE *f,OggVorbis_File *vf,char *initial,long ibytes); */
  typedef int (*OvOpen_t)(FILE*, OggVorbis_File*, char*, long);

  /* vorbis_info *ov_info(OggVorbis_File *vf,int link); */
  typedef vorbis_info* (*OvInfo_t)(OggVorbis_File*, int);
  
  /* vorbis_comment *ov_comment(OggVorbis_File *vf,int link); */
  typedef vorbis_comment* (*OvComment_t)(OggVorbis_File*, int);
  
  /* long ov_read(OggVorbis_File *vf, char *buffer, int length, int bigendianp, int word, int sgned, int *bitstream); */
  typedef long (*OvRead_t)(OggVorbis_File*, char*, int, int, int, int, int*); 

  /* int ov_clear(OggVorbis_File *vf); */
  typedef int (*OvClear_t)(OggVorbis_File*);

}
#endif

class CVorbisDecoder: public CAudioDecoderBase
{
  public:
    CVorbisDecoder();
    virtual ~CVorbisDecoder();
  
    bool LoadLib();
  
    bool OpenFile(std::string p_sFileName);
    void CloseFile();
  
    /**
     * @param   p_PcmOut[]
     * @return  number of decoded samples
     */
    long DecodeInterleaved(char* p_PcmOut, unsigned int p_nSize);
  
  private:
    fuppesLibHandle  m_LibHandle;      
          
    OggVorbis_File m_VorbisFile;
    FILE*          m_pVorbisFileHandle;
    vorbis_info*   m_pVorbisInfo;
    int            m_nEndianess;
  
    
    OvOpen_t       m_OvOpen;
    OvInfo_t       m_OvInfo;
    OvComment_t    m_OvComment;
    OvRead_t       m_OvRead;
    OvClear_t      m_OvClear;
  
};

#endif // _VORBISWRAPPER_H

#endif // HAVE_VORBIS
#endif // DISABLE_TRANSCODING
