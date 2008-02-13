/***************************************************************************
 *            MadDecoder.cpp
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

#ifndef DISABLE_TRANSCODING
#include "MadDecoder.h"

#ifdef HAVE_MAD

#include "../SharedConfig.h"
#include "../Common/Common.h"
#include <string.h>
#include <limits.h>

using namespace std;

CMadDecoder::CMadDecoder():CAudioDecoderBase()
{
}

CMadDecoder::~CMadDecoder()
{
  if(m_LibHandle)
    FuppesCloseLibrary(m_LibHandle);
}
  
bool CMadDecoder::LoadLib()
{
  #ifdef WIN32
  std::string sLibName = "libmad.dll";
  #else
  std::string sLibName = "libmad.so";
  #endif
  
  if(!CSharedConfig::Shared()->MadLibName().empty()) {
    sLibName = CSharedConfig::Shared()->MadLibName();
  }  
  
  CSharedLog::Log(L_EXT, __FILE__, __LINE__, "try opening %s", sLibName.c_str());
  m_LibHandle = FuppesLoadLibrary(sLibName);   
  
  if(!m_LibHandle) {
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot open library %s", sLibName.c_str());
    return false;
  } 
  
	m_MadStreamInit = (MadStreamInit_t)FuppesGetProcAddress(m_LibHandle, "mad_stream_init");
  if(!m_MadStreamInit) {
		CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol '%s'", "mad_stream_init");
    return false;
  }
  
	m_MadFrameInit = (MadFrameInit_t)FuppesGetProcAddress(m_LibHandle, "mad_frame_init");
  if(!m_MadFrameInit) {
		CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol '%s'", "mad_frame_init");
    return false;
  }
	
	m_MadSynthInit = (MadSynthInit_t)FuppesGetProcAddress(m_LibHandle, "mad_synth_init");
  if(!m_MadSynthInit) {
		CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol '%s'", "mad_synth_init");
    return false;
  }
				
	m_MadStreamBuffer = (MadStreamBuffer_t)FuppesGetProcAddress(m_LibHandle, "mad_stream_buffer");
  if(!m_MadStreamBuffer) {
		CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol '%s'", "mad_stream_buffer");
    return false;
  }
	
	m_MadFrameDecode = (MadFrameDecode_t)FuppesGetProcAddress(m_LibHandle, "mad_frame_decode");
  if(!m_MadFrameDecode) {
		CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol '%s'", "mad_frame_decode");
    return false;
  }
	
	m_MadSynthFrame = (MadSynthFrame_t)FuppesGetProcAddress(m_LibHandle, "mad_synth_frame");
  if(!m_MadSynthFrame) {
		CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol '%s'", "mad_synth_frame");
    return false;
  }
	
	m_MadSynthFinish = (MadSynthFinish_t)FuppesGetProcAddress(m_LibHandle, "mad_synth_finish");
  if(!m_MadSynthFinish) {
		CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol '%s'", "mad_synth_finish");
    //return false;
  }
	
	m_MadFrameFinish = (MadFrameFinish_t)FuppesGetProcAddress(m_LibHandle, "mad_frame_finish");
  if(!m_MadFrameFinish) {
		CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol '%s'", "mad_frame_finish");
    return false;
  }

	m_MadStreamFinish = (MadStreamFinish_t)FuppesGetProcAddress(m_LibHandle, "mad_stream_finish");
  if(!m_MadStreamFinish) {
		CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol '%s'", "mad_stream_finish");
    return false;
  }
		
  return true;
}

typedef struct stats_t
{
//  int currentframe;   /* current frame being played */
  int framecount;     /* total frames in the file   */
  int bitrate;        /* average bitrate            */
  int channels;       /* number of channels (1 or 2 */
  int freq;           /* output sampling frequency  */
  //mad_timer_t length; /* length in time of the decoded file */
  //mad_timer_t pos;    /* poision in time currently being played */
}
stats_t;

bool CMadDecoder::OpenFile(std::string p_sFileName, CAudioDetails* pAudioDetails)
{     
	cout << "OPEN: " << p_sFileName << endl;
	fflush(stdout);
		
	m_pFile = fopen(p_sFileName.c_str(), "rb");
  if(m_pFile == 0) {
    printf("Error opening input file: \"%s\"\n", p_sFileName.c_str());
    return false;
  }
		
		/* pAudioDetails->nNumChannels   = m_pVorbisInfo->channels;
  pAudioDetails->nSampleRate    = m_pVorbisInfo->rate;
  pAudioDetails->nNumPcmSamples = m_OvPcmTotal(&m_VorbisFile, -1);*/

	//#define INPUT_BUFFER_SIZE	(5*8192)
	unsigned char		InputBuffer[INPUT_BUFFER_SIZE+MAD_BUFFER_GUARD];
		
	fseek(m_pFile, 0, SEEK_END);
  m_nFileSize = ftell(m_pFile);
  fseek(m_pFile, 0, SEEK_SET);

	m_MadStreamInit(&m_Stream);
	m_MadFrameInit(&m_Frame);
	m_MadSynthInit(&m_Synth);
		
		
	/*int nBytesRead = fread(InputBuffer, INPUT_BUFFER_SIZE, 1, m_pFile);
		
	mad_stream_buffer(&m_Stream,InputBuffer,INPUT_BUFFER_SIZE);*/
		
	struct mad_header Header;
	mad_header_init(&Header);
		
	//mad_header_decode(&Header, &m_Stream);
		
	/*cout << Header.bitrate << endl;
	cout << Header.samplerate << endl;
	fflush(stdout);*/
		
		
	//struct mad_stream stream;
  //struct mad_header header; 
  //unsigned char buffer[INPUT_BUFFER_SIZE];
  unsigned int nBuffLen = 0;
  //mad_stream_init (&stream);
  //mad_header_init (&header);
	struct stats_t stats;

	m_nNumFrames = 0;
		
  while(true) {
		
		if(nBuffLen < INPUT_BUFFER_SIZE) {
			int bytes;
      bytes = fread (InputBuffer + nBuffLen, 1, INPUT_BUFFER_SIZE - nBuffLen, m_pFile);
      if(bytes <= 0) 
				break;
      nBuffLen += bytes;
    }
			
    mad_stream_buffer(&m_Stream, InputBuffer, nBuffLen);
    while(true) {
			if(mad_header_decode (&Header, &m_Stream) == -1) {
				if(!MAD_RECOVERABLE(m_Stream.error))
					break;
        if (m_Stream.error == MAD_ERROR_LOSTSYNC) {
          /* ignore LOSTSYNC due to ID3 tags */
          int tagsize = 1; //id3_tag_query (m_Stream.this_frame, m_Stream.bufend - m_Stream.this_frame);
          if (tagsize > 0) {
						mad_stream_skip (&m_Stream, tagsize);
						continue;
          }
        }

				printf("error decoding header at frame %d: %s\n", 
                      m_nNumFrames, 
                      mad_stream_errorstr (&m_Stream));
        continue;
			}

			if(m_nNumFrames == 0) {
				stats.channels = MAD_NCHANNELS(&Header);
        stats.freq = Header.samplerate;
      }
      else {
				if (stats.channels != MAD_NCHANNELS(&Header)) {
					printf ("warning: number of channels varies within file\n");
				}
				if (stats.freq != Header.samplerate){
					printf ("warning: number of channels varies within file\n");
				}
      }

      m_nNumFrames++;
      stats.bitrate += Header.bitrate;

    } // while(true)
			
		if(m_Stream.error != MAD_ERROR_BUFLEN)
			break;
		
		memmove(InputBuffer, m_Stream.next_frame, &InputBuffer[nBuffLen] - m_Stream.next_frame);
		nBuffLen -= m_Stream.next_frame - &InputBuffer[0];

  } // while(true)

  mad_header_finish (&Header);
	fseek(m_pFile, 0, SEEK_SET);	
		
  stats.bitrate    = stats.bitrate/m_nNumFrames;
  stats.framecount = m_nNumFrames;
  //m_nNumFrames = 0;
  printf ("\tchannels = %d\n", stats.channels);
  printf ("\tfrequency= %d\n", stats.freq);
  printf ("\tbitrate  = %d\n", stats.bitrate / 1000);
  printf ("\tframes   = %d\n", stats.framecount);
  //printf ("\tlength   = %ld seconds\n", stats.length.seconds);	

	pAudioDetails->nNumChannels   = stats.channels;
  pAudioDetails->nSampleRate    = stats.freq;
  pAudioDetails->nNumPcmSamples	= m_nNumFrames;
	
	m_MadStreamFinish(&m_Stream);
	m_MadFrameFinish(&m_Frame);
	if(m_MadSynthFinish)
		m_MadSynthFinish(&m_Synth);
		
	mad_stream_init(&m_Stream);
	mad_frame_init(&m_Frame);
	mad_synth_init(&m_Synth);
		
	m_OutputPtr = NULL;
	m_GuardPtr = NULL;
	m_OutputBufferEnd = NULL;
	m_nSynthPos = -1;
		
  return true;
}

void CMadDecoder::CloseFile()
{
	m_MadStreamFinish(&m_Stream);
	m_MadFrameFinish(&m_Frame);
	m_MadSynthFinish(&m_Synth);
		
  fclose(m_pFile);
}

static signed short MadFixedToSshort(mad_fixed_t Fixed)
{
	/* Clipping */
	if(Fixed>=MAD_F_ONE)
		return(SHRT_MAX);
	if(Fixed<=-MAD_F_ONE)
		return(-SHRT_MAX);

	/* Conversion. */
	Fixed=Fixed>>(MAD_F_FRACBITS-15);
	return((signed short)Fixed);
}


long CMadDecoder::DecodeInterleaved(char* p_PcmOut, int p_nBufferSize, int* p_nBytesRead)
{
	if(!m_OutputPtr)
		m_OutputPtr = p_PcmOut;
	if(!m_OutputBufferEnd)
		m_OutputBufferEnd = p_PcmOut + p_nBufferSize;
	
	int	i;
	//unsigned long	FrameCount=0;


	while(true) {

		if(m_nSynthPos >= 0) {
			*p_nBytesRead = 0;
			m_OutputPtr = p_PcmOut;
				
			//cout << "synth: " << m_nSynthPos << " of " << m_Synth.pcm.length << endl;
				
			long nSamples = 0;
				
			for(i = m_nSynthPos; i < m_Synth.pcm.length; i++) {
				signed short	Sample;

				#warning todo: endianess
				/* Left channel */
				Sample=MadFixedToSshort(m_Synth.pcm.samples[0][i]);
				*(m_OutputPtr++)=Sample>>8;
				*(m_OutputPtr++)=Sample&0xff;

				/* Right channel. If the decoded stream is monophonic then
				 * the right output channel is the same as the left one.
				 */
				if(MAD_NCHANNELS(&m_Frame.header)==2)
					Sample=MadFixedToSshort(m_Synth.pcm.samples[1][i]);
				*(m_OutputPtr++)=Sample>>8;
				*(m_OutputPtr++)=Sample&0xff;			

				*p_nBytesRead = (*p_nBytesRead) + 4;
					
				nSamples++;
					
				/* Flush the output buffer if it is full. */
				if(m_OutputPtr == m_OutputBufferEnd) {
					m_nSynthPos = i + 1;
					cout << "out buffer full " << *p_nBytesRead << " - samples : " << nSamples << " pos " << m_nSynthPos << endl;
					return nSamples;
				}
			}

			cout << "no samples left " << nSamples << " of " << m_Synth.pcm.length << " - pos " << m_nSynthPos << endl;
			cout << "out buffer: " << *p_nBytesRead << " of " << p_nBufferSize << endl;
			m_nSynthPos = -1;
			return nSamples;
		}
			

		if(m_Stream.buffer==NULL || m_Stream.error==MAD_ERROR_BUFLEN)	{
			size_t			ReadSize,	Remaining;
			unsigned char	*ReadStart;

			// frames left
			if(m_Stream.next_frame!=NULL) {
				cout << "frames left" << endl;
				fflush(stdout);
				Remaining = m_Stream.bufend - m_Stream.next_frame;
				memmove(m_InputBuffer, m_Stream.next_frame, Remaining);
				ReadStart = m_InputBuffer + Remaining;
				ReadSize  = INPUT_BUFFER_SIZE - Remaining;
			}
			else {
				cout << "buffer is empty" << endl;
				fflush(stdout);
				ReadSize  = INPUT_BUFFER_SIZE;
				ReadStart = m_InputBuffer;
				Remaining = 0;
			}
				
			// fill buffer
			cout << "read " << ReadSize << " bytes - remaining: " << Remaining  << endl;
			fflush(stdout);
			ReadSize = fread(ReadStart, 1, ReadSize, m_pFile);
				
			if(ReadSize<=0) {
				return -1;
			}

			// eof
			if(m_nFileSize == ftell(m_pFile)) {
				cout << "eof" << endl;
				fflush(stdout);
				m_GuardPtr = ReadStart + ReadSize;
				memset(m_GuardPtr, 0, MAD_BUFFER_GUARD);
				ReadSize += MAD_BUFFER_GUARD;
			}

			// fill mad buffer
			mad_stream_buffer(&m_Stream, m_InputBuffer, ReadSize + Remaining);
			m_Stream.error = (mad_error)0;

		} // read file and fill buffer 

		// decode
		/*cout << "decode ";
		fflush(stdout);*/
		int nDec = mad_frame_decode(&m_Frame, &m_Stream);
		/*cout << nDec << endl;
		fflush(stdout);*/
		if(nDec > 0) {
				
			if(MAD_RECOVERABLE(m_Stream.error))	{
				cout << "recover" << endl;
				fflush(stdout);
				continue;
			}
			else {
				if(m_Stream.error == MAD_ERROR_BUFLEN) {
					cout << "buflen" << endl;
					fflush(stdout);
					continue;
				}
				else {
					cout << "fatal ::" << mad_stream_errorstr(&m_Stream) << endl;						
					fflush(stdout);
					return -1;
				}
			}
				
		}
				
		//cout << "synth frame" << endl;
		//fflush(stdout);
		mad_synth_frame(&m_Synth,&m_Frame);
		m_nSynthPos = 0;
	} // while true

	return -1;	
}

unsigned int CMadDecoder::NumPcmSamples()
{  
  return 0; //m_nNumFrames;
}

#endif // HAVE_MAD
#endif // DISABLE_TRANSCODING
