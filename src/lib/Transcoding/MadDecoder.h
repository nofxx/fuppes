/***************************************************************************
 *            MadDecoder.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2008 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
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
#ifdef HAVE_MAD

#ifndef _MADDECODER_H
#define _MADDECODER_H

#include <string>
#include <mad.h>

#include "WrapperBase.h"

#ifdef __cplusplus
extern "C"
{		
	//void mad_stream_init(struct mad_stream *);
	typedef void (*MadStreamInit_t)(struct mad_stream*);		
	// void mad_frame_init(struct mad_frame *);
	typedef void (*MadFrameInit_t)(struct mad_frame*);
	// void mad_synth_init(struct mad_synth *);
	typedef void (*MadSynthInit_t)(struct mad_synth*);
		
	//void mad_stream_buffer(struct mad_stream *, unsigned char const *, unsigned long);
	typedef void (*MadStreamBuffer_t)(struct mad_stream*, unsigned char const*, unsigned long);
	// int mad_frame_decode(struct mad_frame *, struct mad_stream *);
	typedef int (*MadFrameDecode_t)(struct mad_frame*, struct mad_stream*);
	// void mad_synth_frame(struct mad_synth *, struct mad_frame const *);
	typedef void (*MadSynthFrame_t)(struct mad_synth*, struct mad_frame const*);
	
	// void mad_synth_finish(struct mad_synth *);
	typedef void (*MadSynthFinish_t)(struct mad_synth*);
	//void mad_frame_finish(struct mad_frame *);
	typedef void (*MadFrameFinish_t)(struct mad_frame*);
	//void mad_stream_finish(struct mad_stream *);
	typedef void (*MadStreamFinish_t)(struct mad_stream*);
}
#endif // __cplusplus

class CMadDecoder: public CAudioDecoderBase
{
  public:
    CMadDecoder();
    virtual ~CMadDecoder();
  
    bool LoadLib(); 
    bool OpenFile(std::string p_sFileName, CAudioDetails* pAudioDetails);
    unsigned int NumPcmSamples();
    long DecodeInterleaved(char* p_PcmOut, int p_nBufferSize, int* p_nBytesRead);
    void CloseFile();   

  private:
		FILE* 							m_pFile;
		fuppes_off_t				m_nFileSize;
		struct mad_stream		m_Stream;
		struct mad_frame		m_Frame;
		struct mad_synth		m_Synth;
		
		#define INPUT_BUFFER_SIZE	(5*8192)
		//#define INPUT_BUFFER_SIZE 3831977
		unsigned char				m_InputBuffer[INPUT_BUFFER_SIZE+MAD_BUFFER_GUARD];
		
		unsigned int				m_nNumFrames;
		
		
		char*								m_OutputPtr;
		unsigned char* 			m_GuardPtr;
		const char*					m_OutputBufferEnd;
		int									m_nSynthPos;
			
		
    MadStreamInit_t			m_MadStreamInit;
		MadFrameInit_t			m_MadFrameInit;
		MadSynthInit_t			m_MadSynthInit;
		MadStreamBuffer_t		m_MadStreamBuffer;
		MadFrameDecode_t		m_MadFrameDecode;
		MadSynthFrame_t			m_MadSynthFrame;
		MadSynthFinish_t		m_MadSynthFinish;
		MadFrameFinish_t		m_MadFrameFinish;
		MadStreamFinish_t		m_MadStreamFinish;
};

#endif // _MADDECODER_H

#endif // HAVE_MAD
#endif // DISABLE_TRANSCODING
