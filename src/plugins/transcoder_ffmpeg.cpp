/***************************************************************************
 *            transcoder_ffmpeg.c
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

#include "../../include/fuppes_plugin.h"

#include "ffmpeg/ffmpeg.h"
#include <string>
#include <sstream>

void parseFFmpegArgs(const char* args, char* argv[], int* argc)
{
  std::string sParams = args;
  int     nChar = ' ';
  char*   sChar = NULL;
  std::string  sArg;  
  
  while((sChar = strchr(sParams.c_str(), nChar)) || !sParams.empty()) {    
    
    if(sChar) {
      sArg = sParams.substr(0, sChar - sParams.c_str());      
      sParams = sParams.substr(sChar - sParams.c_str() + 1, sParams.length());
    }
    else {
      sArg = sParams;      
      sParams = "";
    }
   
    argv[*argc] = (char*)malloc((strlen(sArg.c_str()) + 1) * sizeof(char));
    strcpy(argv[*argc], sArg.c_str());  
    *argc = (*argc) + 1;
  }  
}

#ifdef __cplusplus
extern "C" {
#endif

#if FFMPEG_VERSION >= 52 && !defined(OLD_INCLUDES_PATH)
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#else
#include <avformat.h>
#include <avcodec.h>
#endif

#include <string.h>
	
void register_fuppes_plugin(plugin_info* info)
{
	strcpy(info->plugin_name, "ffmpeg");
	info->plugin_type = PT_TRANSCODER;
	
	av_register_all();
}


typedef struct {
  int       numArgs;
  char*     szArgs[40]; 
  CFFmpeg*  ffmpeg;
} pluginData_t;
  
int fuppes_transcoder_transcode(plugin_info* plugin,
                                const char* inputFile,
                                const char* outputFile,
																const char* audioCodec, 
																const char* videoCodec,
                                int videoBitrate,
                                int audioBitrate,
                                int audioSamplerate,
																const char* ffmpegParams)
{
  plugin->user_data = malloc(sizeof(pluginData_t));
  pluginData_t* data = (pluginData_t*)plugin->user_data;
  data->numArgs = 0;
                                  
  data->szArgs[data->numArgs] = (char*)malloc((strlen("ffmpeg") + 1) * sizeof(char));
  strcpy(data->szArgs[data->numArgs++], "ffmpeg");  
  
  data->szArgs[data->numArgs] = (char*)malloc((strlen("-i") + 1) * sizeof(char));
  strcpy(data->szArgs[data->numArgs++], "-i");  
  
  data->szArgs[data->numArgs] = (char*)malloc((strlen(inputFile)  + 1) * sizeof(char));
  strcpy(data->szArgs[data->numArgs++], inputFile);  

  // video setting
  data->szArgs[data->numArgs] = (char*)malloc((strlen("-vcodec") + 1) * sizeof(char));
  strcpy(data->szArgs[data->numArgs++], "-vcodec");  
  
  data->szArgs[data->numArgs] = (char*)malloc((strlen(videoCodec) + 1) * sizeof(char));
  strcpy(data->szArgs[data->numArgs++], videoCodec);
  
  if(videoBitrate > 0) {
    std::stringstream sBitRate;
    sBitRate << videoBitrate;
  
    data->szArgs[data->numArgs] = (char*)malloc((strlen("-b") + 1) * sizeof(char));
    strcpy(data->szArgs[data->numArgs++], "-b");
  
    data->szArgs[data->numArgs] = (char*)malloc((strlen(sBitRate.str().c_str()) + 1) * sizeof(char));
    strcpy(data->szArgs[data->numArgs++], sBitRate.str().c_str());
  }
    
  // audio settings 
  data->szArgs[data->numArgs] = (char*)malloc((strlen("-acodec") + 1) * sizeof(char));
  strcpy(data->szArgs[data->numArgs++], "-acodec");
  
  data->szArgs[data->numArgs] = (char*)malloc((strlen(audioCodec) + 1) * sizeof(char));
  strcpy(data->szArgs[data->numArgs++], audioCodec);  
  
  if(audioSamplerate > 0) {
    std::stringstream sSampleRate;
    sSampleRate << audioSamplerate;
    
    data->szArgs[data->numArgs] = (char*)malloc((strlen("-ar") + 1) * sizeof(char));
    strcpy(data->szArgs[data->numArgs++], "-ar");
  
    data->szArgs[data->numArgs] = (char*)malloc((strlen(sSampleRate.str().c_str()) + 1) * sizeof(char));
    strcpy(data->szArgs[data->numArgs++], sSampleRate.str().c_str());
  }
  
  
  if(audioBitrate > 0) {
    std::stringstream sBitRate;
    sBitRate << audioBitrate;
    
    data->szArgs[data->numArgs] = (char*)malloc((strlen("-ab") + 1) * sizeof(char));
    strcpy(data->szArgs[data->numArgs++], "-ab");
  
    data->szArgs[data->numArgs] = (char*)malloc((strlen(sBitRate.str().c_str()) + 1) * sizeof(char));
    strcpy(data->szArgs[data->numArgs++], sBitRate.str().c_str());
  }
                                  

  parseFFmpegArgs(ffmpegParams, data->szArgs, &data->numArgs);

  
	data->szArgs[data->numArgs] = (char*)malloc((strlen(outputFile) + 1) * sizeof(char));
  strcpy(data->szArgs[data->numArgs++], outputFile);
                                  

  ((pluginData_t*)plugin->user_data)->ffmpeg = new CFFmpeg();
  ((pluginData_t*)plugin->user_data)->ffmpeg->ffmpeg_main(data->numArgs, data->szArgs);

                                  
  for(int i = 0; i < ((pluginData_t*)plugin->user_data)->numArgs; i++) {
    free(((pluginData_t*)plugin->user_data)->szArgs[i]);
  }

  delete ((pluginData_t*)plugin->user_data)->ffmpeg;                                  
  delete (pluginData_t*)plugin->user_data;
  plugin->user_data = NULL;

	return 0;
}

void unregister_fuppes_plugin(plugin_info* info)
{
}

#ifdef __cplusplus
}
#endif
