/***************************************************************************
 *            LameWrapper.h
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
 
#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif 

#ifndef DISABLE_TRANSCODING
#ifdef  HAVE_LAME

#ifndef _LAMEWRAPPER_H
#define _LAMEWRAPPER_H

#include "WrapperBase.h"
#include "../Common/Common.h"
#include <lame/lame.h>
#include <string>


extern "C"
{ 
  typedef lame_global_flags LameGlobalFlags;
  
  /* lame_global_flags* lame_init() */
  typedef lame_global_flags* (*LameInit_t)();
  /* const char* get_lame_version() */  
  typedef const char*        (*LameGetVersion_t)();
  /* void lame_init_params(lame_global_flags*) */
  typedef void               (*LameInitParams_t)(lame_global_flags*);
  /* void lame_print_config(lame_global_flags*) */
  typedef void               (*LamePrintConfig_t)(lame_global_flags*);
  
  /* int lame_set_compression_ratio(lame_global_flags *, float); */
  typedef int  (*LameSetCompressionRatio_t)(lame_global_flags*, float);
  /* float lame_get_compression_ratio(const lame_global_flags *); */
  typedef float (*LameGetCompressionRatio_t)(lame_global_flags*);
  /* int lame_set_brate(lame_global_flags *, int); */
  typedef int   (*LameSetBrate_t)(lame_global_flags*, int);
  /* int lame_get_brate(const lame_global_flags *); */
  typedef int   (*LameGetBrate_t)(lame_global_flags*);
  
  /* mode = 0,1,2,3 = stereo, jstereo, dual channel (not supported), mono
  default: lame picks based on compression ration and input channels */
  //int CDECL lame_set_mode(lame_global_flags *, MPEG_mode);
  typedef int (*LameSetMode_t)(lame_global_flags*, MPEG_mode);
  
  
  /* quality=0..9.  0=best (very slow).  9=worst.
  recommended:  2     near-best quality, not too slow
                5     good quality, fast
                7     ok quality, really fast */
  
  /* int CDECL lame_set_quality(lame_global_flags *, int); */
  typedef int (*LameSetQuality_t)(lame_global_flags*, int);
  
  /* int CDECL lame_get_quality(const lame_global_flags *); */
  typedef int (*LameGetQuality_t)(const lame_global_flags*);  
  
  
  /* int lame_encode_buffer(lame_global_flags* gf, 
                            short int leftpcm[],
                            short int rightpcm[],
                            int num_samples,
                            char *mp3buffer,
                            int mp3buffer_size) */
  
  typedef int (*LameEncodeBufferInterleaved_t)(lame_global_flags*, short int[], int, unsigned char*, int);                    
               
  //lame_encode_flush(gf, mp3buffer, LAME_MAXMP3BUFFER);
  typedef int (*LameEncodeFlush_t)(lame_global_flags*, unsigned char*, int);
  
  // int  lame_close (lame_global_flags*);
  typedef int (*LameClose_t)(lame_global_flags*);
  
  // id3 functions
  
  typedef void (*Id3TagInit_t)(lame_global_flags*);

  typedef void (*Id3TagV1Only_t)(lame_global_flags*);

  typedef void (*Id3TagV2Only_t)(lame_global_flags*);

  typedef void (*Id3TagAddV2_t)(lame_global_flags*);
  
  typedef void(*Id3TagPadV2_t)(lame_global_flags*);
  
  /* pad version 1 tag with spaces instead of nulls */
  //extern void id3tag_space_v1 (lame_global_flags *gfp);

  typedef void (*Id3TagSetTitle_t)(lame_global_flags*, const char*);
  
  typedef void (*Id3TagSetArtist_t)(lame_global_flags*, const char*);
  
  typedef void (*Id3TagSetAlbum_t)(lame_global_flags*, const char*);
  
  typedef void (*Id3TagSetYear_t)(lame_global_flags*, const char*);
  
  typedef void (*Id3TagSetComment_t)(lame_global_flags*, const char*);
  
  typedef void (*Id3TagSetTrack_t)(lame_global_flags*, const char*);  
  
  typedef int (*Id3TagSetGenre_t)(lame_global_flags*, const char*);
  
}


typedef struct LAME_BITRATE_MAPPING_t {
  int   nBitRate;
  float fLameRate;
} LAME_BITRATE_MAPPING_t;

/*
  1. create
  2. call LoadLib()
  3. set compression ratio and other properties
  4. call Init()
  5. encoding using EncodeInterleaved() 
  6. flush calling Flush()
  7. LameWrapper will clean up when it's deleted
*/ 

class CLameWrapper: public CAudioEncoderBase
{  
  public:
		CLameWrapper();
    virtual ~CLameWrapper();
    bool LoadLib();
  
    void Init();
    void PrintConfig();
    std::string GetVersion();    
    void SetTranscodingSettings(CTranscodingSettings* pTranscodingSettings);
    //void SetCompressionRatio(LAME_BITRATE p_nCompressionRatio);
    //void SetBitrate(int p_nBitrate);
  
    int   EncodeInterleaved(short int p_PcmIn[], int p_nNumSamples, int p_nBytesRead);
    int   Flush();
    unsigned char* GetEncodedBuffer() { return m_sMp3Buffer; }
    
    
    unsigned int GuessContentLength(unsigned int p_nNumPcmSamples);
    
    void GetMp3Tail(char* p_szBuffer);
  
  private:
    fuppesLibHandle  m_LibHandle;
    LameGlobalFlags* m_LameGlobalFlags;
    unsigned char    m_sMp3Buffer[LAME_MAXMP3BUFFER];
  
    LameInit_t        m_LameInit;
    LameGetVersion_t  m_LameGetVersion;
    LameInitParams_t  m_LameInitParams; 
    LamePrintConfig_t m_LamePrintConfig;
  
    LameSetCompressionRatio_t     m_LameSetCompressionRatio;
    LameGetCompressionRatio_t     m_LameGetCompressionRatio;
    LameSetBrate_t                m_LameSetBrate;
    LameGetBrate_t                m_LameGetBrate;
    LameSetMode_t                 m_LameSetMode;
    LameSetQuality_t              m_LameSetQuality;
    LameGetQuality_t              m_LameGetQuality;
  
    LameEncodeBufferInterleaved_t m_LameEncodeBufferInterleaved;
    LameEncodeFlush_t             m_LameEncodeFlush;
  
    LameClose_t   m_LameClose;
    
    int m_nBitRate;
    int m_nSampleRate;
    int m_nChannels;
  
    char szMp3Tail[128];
  
    // id3
    Id3TagInit_t        m_Id3TagInit;
    Id3TagV1Only_t      m_Id3TagV1Only;
    Id3TagV2Only_t      m_Id3TagV2Only;
    Id3TagAddV2_t       m_Id3TagAddV2;
    Id3TagPadV2_t       m_Id3TagPadV2;
    Id3TagSetTitle_t    m_Id3TagSetTitle;
    Id3TagSetArtist_t   m_Id3TagSetArtist;
    Id3TagSetAlbum_t    m_Id3TagSetAlbum;
    Id3TagSetYear_t     m_Id3TagSetYear;
    Id3TagSetComment_t  m_Id3TagSetComment;
    Id3TagSetTrack_t    m_Id3TagSetTrack;  
    Id3TagSetGenre_t    m_Id3TagSetGenre;
};

#endif // _LAMEWRAPPER_H
#endif // HAVE_LAME
#endif // DISABLE_TRANSCODING
