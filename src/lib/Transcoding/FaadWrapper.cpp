/***************************************************************************
 *            FaadWrapper.cpp
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

/*
 * this file contains code from FAAD2 version 2.0
 *
 * FAAD2 - Freeware Advanced Audio (AAC) Decoder including SBR decoding
 * Copyright (C) 2003-2004 M. Bakker, Ahead Software AG, http://www.nero.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "FaadWrapper.h"

#ifndef DISABLE_TRANSCODING
#ifdef HAVE_FAAD

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

#include "../SharedLog.h" 
#include "../SharedConfig.h" 

static int adts_sample_rates[] = {96000,88200,64000,48000,44100,32000,24000,22050,16000,12000,11025,8000,7350,0,0,0};

int CFaadWrapper::adts_parse(int *bitrate, float *length)
{
  int frames, frame_length;
  int t_framelength = 0;
  int samplerate;
  float frames_per_sec, bytes_per_frame;

  // Read all frames to ensure correct time and bitrate 
  for (frames = 0; ; frames++)
  {
    if (m_nBufferSize > 7)
    {
      // check syncword 
      if (!((m_Buffer[0] == 0xFF)&&((m_Buffer[1] & 0xF6) == 0xF0)))
        break;

      if (frames == 0)
        samplerate = adts_sample_rates[(m_Buffer[2]&0x3c)>>2];

      frame_length = ((((unsigned int)m_Buffer[3] & 0x3)) << 11)
          | (((unsigned int)m_Buffer[4]) << 3) | (m_Buffer[5] >> 5);

      t_framelength += frame_length;

      if (frame_length > m_nBufferSize)
        break;

      m_nBytesConsumed = frame_length;
      FillBuffer();
    } else {
      break;
    }
  }

  frames_per_sec = (float)samplerate/1024.0f;
  if (frames != 0)
    bytes_per_frame = (float)t_framelength/(float)(frames*1000);
  else
    bytes_per_frame = 0;
  *bitrate = (int)(8. * bytes_per_frame * frames_per_sec + 0.5);
  if (frames_per_sec != 0)
    *length = (float)frames/frames_per_sec;
  else
    *length = 1;

  return 1;
}

#ifdef HAVE_MP4FF_H
uint32_t read_callback(void *user_data, void *buffer, uint32_t length)
{
    return fread(buffer, 1, length, (FILE*)user_data);
}

uint32_t seek_callback(void *user_data, uint64_t position)
{
    return fseek((FILE*)user_data, position, SEEK_SET);
}
#endif // HAVE_MP4FF_H

int CFaadWrapper::write_audio_16bit(char* p_PcmOut, void *sample_buffer, unsigned int samples)
{
  int ret;
  unsigned int i;
  short *sample_buffer16 = (short*)sample_buffer;
  char *data = (char*)malloc(samples * 16 * sizeof(char)/8);
 
  if(m_nOutEndianess == E_LITTLE_ENDIAN) {  
    for (i = 0; i < samples; i++) {
      data[i*2] = (char)(sample_buffer16[i] & 0xFF);
      data[i*2+1] = (char)((sample_buffer16[i] >> 8) & 0xFF);
    }
  }
  else if(m_nOutEndianess == E_BIG_ENDIAN) {
    for (i = 0; i < samples; i++) {
      data[i*2] = (char)((sample_buffer16[i] >> 8) & 0xFF);
      data[i*2+1] = (char)(sample_buffer16[i] & 0xFF);
    }
  }

  memcpy(p_PcmOut, data, samples * 16 * sizeof(char)/8);

  if (data) 
    free(data);

  return samples * 16 * sizeof(char)/8;
}


int CFaadWrapper::DecodeAACfile(char* p_PcmOut)
{
  void *sample_buffer;
  
  do {
    sample_buffer = m_faacDecDecode(hDecoder, &frameInfo, m_Buffer, m_nBufferSize);

    if (frameInfo.error > 0) {
      fprintf(stderr, "decode Error: %s\n",
      m_faacDecGetErrorMessage(frameInfo.error));
      return -1;
    }

    m_nBytesConsumed = frameInfo.bytesconsumed;
    FillBuffer();      
    
    if (first_time && !frameInfo.error) {
      // print some channel info
      //print_channel_info(&frameInfo);   
      first_time = false;
    }

    if ((frameInfo.error == 0) && (frameInfo.samples > 0)) {
      write_audio_16bit(p_PcmOut, sample_buffer, frameInfo.samples);
    }

  } while (frameInfo.samples == 0 && m_nBufferSize > 0);

  if(frameInfo.samples > 0)
    return frameInfo.samples / 2;
  else
    return -1;
}

#ifdef HAVE_MP4FF_H
int CFaadWrapper::GetAACTrack(mp4ff_t *infile)
{
  // find AAC track
  int i, rc;
  int numTracks = m_mp4ff_total_tracks(infile);

  printf("num tracks: %d\n", numTracks);

  for (i = 0; i < numTracks; i++) {
    unsigned char *buff = NULL;
    unsigned int buff_size = 0;
    mp4AudioSpecificConfig mp4ASC;

    m_mp4ff_get_decoder_config(infile, i, &buff, &buff_size);

    if (buff) {
      rc = m_AudioSpecificConfig(buff, buff_size, &mp4ASC);
      free(buff);

      if (rc < 0)
        continue;
      return i;
    }
  }

  // can't decode this
  return -1;
}
#endif // HAVE_MP4FF_H

unsigned long srates[] =
{
    96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000,
    12000, 11025, 8000
};

#ifdef HAVE_MP4FF_H
int CFaadWrapper::DecodeMP4file(char* p_PcmOut)
{
  void *sample_buffer;
  unsigned char *buffer;
  unsigned int  buffer_size;
  
  if(sampleId == numSamples) {
    return -1;
  }  
  
  do {   
    int rc;
    long dur;
    unsigned int sample_count;
    unsigned int delay = 0;

    // get acces unit from MP4 file
    buffer = NULL;
    buffer_size = 0;

    dur = m_mp4ff_get_sample_duration(infile, track, sampleId);   
    rc = m_mp4ff_read_sample(infile, track, sampleId, &buffer,  &buffer_size);        
    if (rc == 0) {
      fprintf(stderr, "Reading from MP4 file failed.\n");
      return -1;
    }

    sample_buffer = m_faacDecDecode(hDecoder, &frameInfo, buffer, buffer_size);
   
    if (buffer)
      free(buffer);

    if (sampleId == 0) 
      dur = 0;

    if (useAacLength || (timescale != samplerate)) {
      sample_count = frameInfo.samples;
    } else {
      sample_count = (unsigned int)(dur * frameInfo.channels);

      if (!useAacLength && !initial && (sampleId < numSamples/2) && (sample_count != frameInfo.samples)) {
        fprintf(stderr, "MP4 seems to have incorrect frame duration, using values from AAC data.\n");
        useAacLength = 1;
        sample_count = frameInfo.samples;
      }
    }

    if (initial && (sample_count < framesize*frameInfo.channels) && (frameInfo.samples > sample_count)) {
      delay = frameInfo.samples - sample_count;
    }
        
    if (first_time && !frameInfo.error) {
      // print some channel info
      //print_channel_info(&frameInfo);
      first_time = false;
    }

    if (sample_count > 0) 
      initial = 0;
    
    if ((frameInfo.error == 0) && (sample_count > 0)) {            
      write_audio_16bit(p_PcmOut, sample_buffer, sample_count);
    }

    if (frameInfo.error > 0) {
      fprintf(stderr, "Warning: %s\n",
      m_faacDecGetErrorMessage(frameInfo.error));
    }
  
  }  while (frameInfo.samples == 0 && initial);  
  
  sampleId++;
  

  if(frameInfo.samples > 0)
    return frameInfo.samples / 2;
  else
    return -1;
}
#endif // HAVE_MP4FF_H

CFaadWrapper::CFaadWrapper()
{
  numSamples = 0;
  hDecoder = NULL;
  m_pFileHandle = NULL;
  m_LibHandle = NULL;
  #ifdef HAVE_MP4FF_H
  m_mp4ffLibHandle = NULL;
  mp4cb = NULL;  
  infile = NULL;
  #endif // HAVE_MP4FF_H
}

CFaadWrapper::~CFaadWrapper()
{
  if(hDecoder)
    m_faacDecClose(hDecoder);
  
  #ifdef HAVE_MP4FF_H
  if(infile)
    m_mp4ff_close(infile);
  
  if(mp4cb)
    free(mp4cb);
  #endif // HAVE_MP4FF_H
  
  if(m_LibHandle)
    FuppesCloseLibrary(m_LibHandle);
  
  #ifdef HAVE_MP4FF_H
  if(m_mp4ffLibHandle)
    FuppesCloseLibrary(m_mp4ffLibHandle);
  #endif // HAVE_MP4FF_H
  
  CloseFile();
}
  
bool CFaadWrapper::LoadLib()
{
  #ifdef WIN32 
  std::string sLibName = "libfaad-0.dll"; 
  #else   
  std::string sLibName = "libfaad.so.0"; 
  #endif 
   
  if(!CSharedConfig::Shared()->FaadLibName().empty()) { 
    sLibName = CSharedConfig::Shared()->FaadLibName();  
  } 
   
  CSharedLog::Shared()->Log(L_EXT, "try opening " + sLibName, __FILE__, __LINE__); 
  m_LibHandle = FuppesLoadLibrary(sLibName);  
  if(!m_LibHandle) { 
    CSharedLog::Shared()->Log(L_EXT, "cannot open library " + sLibName, __FILE__, __LINE__); 
        printf("[WARNING :: AACDecoder] cannot open library %s\n", sLibName.c_str());
    return false; 
  }
 
  m_faacDecOpen = (faacDecOpen_t)FuppesGetProcAddress(m_LibHandle, "faacDecOpen"); 
  if(!m_faacDecOpen) { 
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'faacDecOpen'", __FILE__, __LINE__);    
    return false; 
  }

  m_faacDecGetErrorMessage = (faacDecGetErrorMessage_t)FuppesGetProcAddress(m_LibHandle, "faacDecGetErrorMessage"); 
  if(!m_faacDecGetErrorMessage) { 
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'faacDecGetErrorMessage'", __FILE__, __LINE__);    
    return false; 
  }
  
  m_faacDecGetCurrentConfiguration = (faacDecGetCurrentConfiguration_t)FuppesGetProcAddress(m_LibHandle, "faacDecGetCurrentConfiguration"); 
  if(!m_faacDecGetCurrentConfiguration) { 
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'faacDecGetCurrentConfiguration'", __FILE__, __LINE__);    
  } 

  m_faacDecSetConfiguration = (faacDecSetConfiguration_t)FuppesGetProcAddress(m_LibHandle, "faacDecSetConfiguration"); 
  if(!m_faacDecSetConfiguration) { 
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'faacDecSetConfiguration'", __FILE__, __LINE__);    
  } 

  m_faacDecInit = (faacDecInit_t)FuppesGetProcAddress(m_LibHandle, "faacDecInit"); 
  if(!m_faacDecInit) { 
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'faacDecInit'", __FILE__, __LINE__);    
  } 

  m_faacDecInit2 = (faacDecInit2_t)FuppesGetProcAddress(m_LibHandle, "faacDecInit2"); 
  if(!m_faacDecInit2) { 
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'faacDecInit2'", __FILE__, __LINE__);    
    return false; 
  }
  
  m_faacDecDecode = (faacDecDecode_t)FuppesGetProcAddress(m_LibHandle, "faacDecDecode"); 
  if(!m_faacDecDecode) { 
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'faacDecDecode'", __FILE__, __LINE__);    
    return false; 
  } 

  m_faacDecClose = (faacDecClose_t)FuppesGetProcAddress(m_LibHandle, "faacDecClose"); 
  if(!m_faacDecClose) { 
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'faacDecClose'", __FILE__, __LINE__);    
    return false; 
  } 

  m_AudioSpecificConfig = (AudioSpecificConfig_t)FuppesGetProcAddress(m_LibHandle, "AudioSpecificConfig"); 
  if(!m_AudioSpecificConfig) {
		m_AudioSpecificConfig = (AudioSpecificConfig_t)FuppesGetProcAddress(m_LibHandle, "faacDecAudioSpecificConfig"); 
		if(!m_AudioSpecificConfig) {  
  	  CSharedLog::Shared()->Log(L_EXT, "cannot load symbol '(faacDec)AudioSpecificConfig'", __FILE__, __LINE__);    
      return false;
		}
  }


  #ifdef HAVE_MP4FF_H
  
  #ifdef WIN32 
  sLibName = "libmp4ff-0.dll"; 
  #else   
  sLibName = "libmp4ff.so.0"; 
  #endif 

  if(!CSharedConfig::Shared()->Mp4ffLibName().empty()) { 
    sLibName = CSharedConfig::Shared()->Mp4ffLibName();  
  }
   
  CSharedLog::Shared()->Log(L_EXT, "try opening " + sLibName, __FILE__, __LINE__); 
  m_mp4ffLibHandle = FuppesLoadLibrary(sLibName);  
  if(!m_mp4ffLibHandle) { 
    CSharedLog::Shared()->Log(L_EXT, "cannot open library " + sLibName, __FILE__, __LINE__); 
        printf("[WARNING :: AACDecoder] cannot open library %s\n", sLibName.c_str());
    return false; 
  }  
  

  m_mp4ff_read_sample = (mp4ff_read_sample_t)FuppesGetProcAddress(m_mp4ffLibHandle, "mp4ff_read_sample"); 
  if(!m_mp4ff_read_sample) { 
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'mp4ff_read_sample'", __FILE__, __LINE__);    
    return false; 
  } 
  
  m_mp4ff_time_scale = (mp4ff_time_scale_t)FuppesGetProcAddress(m_mp4ffLibHandle, "mp4ff_time_scale"); 
  if(!m_mp4ff_time_scale) { 
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'mp4ff_time_scale'", __FILE__, __LINE__);    
    return false; 
  } 
  
  m_mp4ff_num_samples = (mp4ff_num_samples_t)FuppesGetProcAddress(m_mp4ffLibHandle, "mp4ff_num_samples"); 
  if(!m_mp4ff_num_samples) { 
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'mp4ff_num_samples'", __FILE__, __LINE__);    
    return false; 
  } 
  
  m_mp4ff_open_read = (mp4ff_open_read_t)FuppesGetProcAddress(m_mp4ffLibHandle, "mp4ff_open_read"); 
  if(!m_mp4ff_open_read) { 
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'mp4ff_open_read'", __FILE__, __LINE__);    
    return false; 
  } 
  
  m_mp4ff_close = (mp4ff_close_t)FuppesGetProcAddress(m_mp4ffLibHandle, "mp4ff_close"); 
  if(!m_mp4ff_close) { 
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'mp4ff_close'", __FILE__, __LINE__);    
    return false; 
  } 
  
  m_mp4ff_get_decoder_config = (mp4ff_get_decoder_config_t)FuppesGetProcAddress(m_mp4ffLibHandle, "mp4ff_get_decoder_config"); 
  if(!m_mp4ff_get_decoder_config) { 
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'mp4ff_get_decoder_config'", __FILE__, __LINE__);    
    return false; 
  } 
  
  m_mp4ff_total_tracks = (mp4ff_total_tracks_t)FuppesGetProcAddress(m_mp4ffLibHandle, "mp4ff_total_tracks"); 
  if(!m_mp4ff_total_tracks) { 
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'mp4ff_total_tracks'", __FILE__, __LINE__);    
    return false; 
  } 
  
  m_mp4ff_get_sample_duration = (mp4ff_get_sample_duration_t)FuppesGetProcAddress(m_mp4ffLibHandle, "mp4ff_get_sample_duration"); 
  if(!m_mp4ff_get_sample_duration) { 
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'mp4ff_get_sample_duration'", __FILE__, __LINE__);    
    return false; 
  }
  
  #endif // HAVE_MP4FF_H
  
	return true;
}

bool CFaadWrapper::OpenFile(std::string p_sFileName, CAudioDetails* pAudioDetails)
{
  if((m_pFileHandle = fopen(p_sFileName.c_str(), "rb")) == NULL) {
    fprintf(stderr, "Cannot open %s\n", p_sFileName.c_str()); 
    return false;
  }
  
	#ifdef _WIN32
  _setmode(_fileno(m_pFileHandle), _O_BINARY);
  #endif
  
  fseek(m_pFileHandle, 0, SEEK_END);
  m_nFileLength = ftell(m_pFileHandle);
  m_nFileRest = m_nFileLength;
  fseek(m_pFileHandle, 0, SEEK_SET);
  
  m_nBufferSize = FAAD_MIN_STREAMSIZE * FAAD_MAX_CHANNELS;
  if(!(m_Buffer = (unsigned char*)malloc(m_nBufferSize))) {
    fprintf(stderr, "Memory allocation error\n");
    return false;
  }    
   
  m_nBufferSize = fread(m_Buffer, 1, m_nBufferSize, m_pFileHandle);
  m_nFileRest -= m_nBufferSize;
  m_nBytesConsumed = 0;
  
  
  if(m_Buffer[4] == 'f' && m_Buffer[5] == 't' && m_Buffer[6] == 'y' && m_Buffer[7] == 'p') {
    printf("is mp4\n");
    m_bIsMp4 = true;
    first_time = true;
    fseek(m_pFileHandle, 0, SEEK_SET);
    #ifdef HAVE_MP4FF_H
    return InitMp4Decoder();
    #endif // HAVE_MP4FF_H
  }  
  else {
    m_bIsMp4 = false;
    first_time = true;
    return InitAACDecoder();
  }
}

void CFaadWrapper::CloseFile()
{
  if(m_pFileHandle)
    fclose(m_pFileHandle);
}

long CFaadWrapper::DecodeInterleaved(char* p_PcmOut, int p_nBufferSize, int* p_nBytesRead)
{
  if(m_bIsMp4) {
    #ifdef HAVE_MP4FF_H
    return DecodeMP4file(p_PcmOut);
    #else
    return -1;
    #endif // HAVE_MP4FF_H
  }
  else {
    return DecodeAACfile(p_PcmOut);
  }
}

unsigned int CFaadWrapper::NumPcmSamples()
{
  printf("num samples %d\n", numSamples);
	return 0; //numSamples;
}

void CFaadWrapper::FillBuffer()
{
  int nRead = 0;
  
  if(m_nBytesConsumed > 0) {
    //printf("consumed %d bytes\n", m_nBytesConsumed);
    memmove(m_Buffer, &m_Buffer[m_nBytesConsumed], m_nBufferSize - m_nBytesConsumed); 
    nRead = fread(&m_Buffer[m_nBufferSize - m_nBytesConsumed], 1, m_nBytesConsumed, m_pFileHandle);
    //printf("read %d bytes rest %d bytes\n", nRead, m_nFileRest);
    m_nFileRest -= nRead;
    if(nRead < m_nBytesConsumed) {
      m_nBufferSize = m_nBufferSize - m_nBytesConsumed + nRead;
      //printf("buffer size %d bytes\n", m_nBufferSize);
    }    
    m_nBytesConsumed = 0;
  }
}

bool CFaadWrapper::InitAACDecoder()
{
  unsigned long samplerate;
  //uint32_t samplerate;
  unsigned char channels;
  int header_type = 0;
  int bitrate = 0;
  float length = 0;  
    
  hDecoder = m_faacDecOpen();

  config = m_faacDecGetCurrentConfiguration(hDecoder);
  config->defObjectType = LC;
  config->outputFormat = FAAD_FMT_16BIT;
  config->downMatrix = 1;
  config->useOldADTSFormat = 0;
  config->dontUpSampleImplicitSBR = 1;   
  m_faacDecSetConfiguration(hDecoder, config);
  
  int tagsize = 0;
  if(!memcmp(m_Buffer, "ID3", 3)) {
      // high bit is not used
    tagsize = (m_Buffer[6] << 21) | (m_Buffer[7] << 14) |
        (m_Buffer[8] <<  7) | (m_Buffer[9] <<  0);

    tagsize += 10;
  }

  // get AAC infos for printing
  header_type = 0;
  if ((m_Buffer[0] == 0xFF) && ((m_Buffer[1] & 0xF6) == 0xF0))
  {
    adts_parse(&bitrate, &length);
    fseek(m_pFileHandle, tagsize, SEEK_SET);      
    m_nBufferSize = fread(m_Buffer, 1, FAAD_MIN_STREAMSIZE * FAAD_MAX_CHANNELS, m_pFileHandle);
    m_nFileRest = m_nFileLength - tagsize - FAAD_MIN_STREAMSIZE * FAAD_MAX_CHANNELS;

    header_type = 1;      
  } else if (memcmp(m_Buffer, "ADIF", 4) == 0) {
    int skip_size = (m_Buffer[4] & 0x80) ? 9 : 0;
    bitrate = ((unsigned int)(m_Buffer[4 + skip_size] & 0x0F)<<19) |
        ((unsigned int)m_Buffer[5 + skip_size]<<11) |
        ((unsigned int)m_Buffer[6 + skip_size]<<3) |
        ((unsigned int)m_Buffer[7 + skip_size] & 0xE0);

    length = (float)m_nFileLength;
    if (length != 0)
    {
        length = ((float)length*8.f)/((float)bitrate) + 0.5f;
    }

    bitrate = (int)((float)bitrate/1000.0f + 0.5f);

    header_type = 2;
    
  }

  if ((m_nBytesConsumed = m_faacDecInit(hDecoder, m_Buffer, m_nBufferSize, &samplerate, &channels)) < 0) {
    fprintf(stderr, "Error initializing decoder library.\n");               
    return false;
  }
  
  FillBuffer();  

  // print AAC file info    
  switch (header_type)
  {
    case 0:
      fprintf(stderr, "RAW\n\n");
      break;
    case 1:
      fprintf(stderr, "ADTS, %.3f sec, %d kbps, %d Hz\n\n",
          length, bitrate, samplerate);
      break;
    case 2:
      fprintf(stderr, "ADIF, %.3f sec, %d kbps, %d Hz\n\n",
          length, bitrate, samplerate);
      break;
  }
}

#ifdef HAVE_MP4FF_H
bool CFaadWrapper::InitMp4Decoder()
{
       
  unsigned char channels;

  mp4AudioSpecificConfig mp4ASC;

  unsigned char *buffer;
  unsigned int buffer_size;

  // for gapless decoding
  useAacLength = 1;
  initial = 1;
  
  
  // initialise the callback structure
  mp4cb = (mp4ff_callback_t*)malloc(sizeof(mp4ff_callback_t));
  mp4cb->read = read_callback;
  mp4cb->seek = seek_callback;
  mp4cb->user_data = m_pFileHandle;

  hDecoder = m_faacDecOpen();

  // set configuration
  config = m_faacDecGetCurrentConfiguration(hDecoder);
  config->outputFormat = FAAD_FMT_16BIT;
  config->downMatrix = 1;
  config->dontUpSampleImplicitSBR = 1;
  m_faacDecSetConfiguration(hDecoder, config);

  infile = m_mp4ff_open_read(mp4cb);
  if (!infile) {
    // unable to open file
    fprintf(stderr, "Error opening input file\n");    
    return false;
  }

  if ((track = GetAACTrack(infile)) < 0) {
    fprintf(stderr, "Unable to find correct AAC sound track in the MP4 file.\n");    
    return false;
  }

  buffer = NULL;
  buffer_size = 0;
  m_mp4ff_get_decoder_config(infile, track, &buffer, &buffer_size);

  if(m_faacDecInit2(hDecoder, buffer, buffer_size, &samplerate, &channels) < 0) {
    // If some error initializing occured, skip the file
    fprintf(stderr, "Error initializing decoder library.\n");    
    return false;
  }

  timescale = m_mp4ff_time_scale(infile, track);
  framesize = 1024;
  useAacLength = 0;

  if(buffer) {
    if (m_AudioSpecificConfig(buffer, buffer_size, &mp4ASC) >= 0) {
      if (mp4ASC.frameLengthFlag == 1) framesize = 960;
      if (mp4ASC.sbr_present_flag == 1) framesize *= 2;
    }
    free(buffer);
  }

  // print some mp4 file info
  fprintf(stderr, "input file info:\n\n");
      
  char *tag = NULL, *item = NULL;
  int k, j;
  char *ot[6] = { "NULL", "MAIN AAC", "LC AAC", "SSR AAC", "LTP AAC", "HE AAC" };
  long samples = m_mp4ff_num_samples(infile, track);
  float f = 1024.0;
  float seconds;
  if (mp4ASC.sbr_present_flag == 1) {
    f = f * 2.0;
  }
  seconds = (float)samples*(float)(f-1.0)/(float)mp4ASC.samplingFrequency;

  fprintf(stderr, "%s\t%.3f secs, %d ch, %d Hz\n\n", ot[(mp4ASC.objectTypeIndex > 5)?0:mp4ASC.objectTypeIndex],
      seconds, mp4ASC.channelsConfiguration, mp4ASC.samplingFrequency);

  numSamples = m_mp4ff_num_samples(infile, track);
  printf("num samples %d\n", numSamples);
  sampleId = 0;

  return true;
}
#endif // HAVE_MP4FF_H

#endif // HAVE_FAAD
#endif // DISABLE_TRANSCODING
