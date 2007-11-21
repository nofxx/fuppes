/*
 * FFmpeg main
 * Copyright (c) 2000-2003 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*  
 *  Modified to use it with FUPPES - Free UPnP Entertainment Service
 *  Wrapped the ffmpeg command line tool in a C++ class
 *  Copyright (C) 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 */

#ifndef _FUPPES_FFMPEG_H
#define _FUPPES_FFMPEG_H

#ifdef HAVE_CONFIG_H
#include "../../../config.h"
#endif

#ifdef __cplusplus 
extern "C" 
{  
  #include <ctype.h>
  #include <string.h>
  #include <avformat.h>
	#include <avutil.h>
  #include <fifo.h> 
#ifdef HAVE_LIBSWSCALE
  #include <swscale.h>
#endif
  #include <math.h>
  #include <stdlib.h>
  #include <limits.h>
  #include <opt.h>
  //#include <mem.h>
  
  #ifdef HAVE_AVSTRING_H
  #include <avstring.h>
  #endif
} 
#endif // __cplusplus 


#include "cmdutils.h"


#ifndef HAVE_AVSTRING_H
void pstrcpy(char *buf, int buf_size, const char *str);
int strstart(const char *str, const char *val, const char **ptr);
#endif // HAVE_AVSTRING_H

char *_av_strdup(const char *s);
void print_error(const char *filename, int err);

struct AVInputStream;

typedef struct AVOutputStream {
    int file_index;          /* file index */
    int index;               /* stream index in the output file */
    int source_index;        /* AVInputStream index */
    AVStream *st;            /* stream in the output file */
    int encoding_needed;     /* true if encoding needed for this stream */
    int frame_number;
    /* input pts and corresponding output pts
       for A/V sync */
    //double sync_ipts;        /* dts from the AVPacket of the demuxer in second units */
    struct AVInputStream *sync_ist; /* input stream to sync against */
    int64_t sync_opts;       /* output frame counter, could be changed to some true timestamp */ //FIXME look at frame_number
    /* video only */
    int video_resample;
    AVFrame pict_tmp;      /* temporary image for resampling */
    struct SwsContext *img_resample_ctx; /* for image resampling */
    int resample_height;

    int video_crop;
    int topBand;             /* cropping area sizes */
    int leftBand;

    int video_pad;
    int padtop;              /* padding area sizes */
    int padbottom;
    int padleft;
    int padright;

    /* audio only */
    int audio_resample;
    ReSampleContext *resample; /* for audio resampling */
    AVFifoBuffer fifo;     /* for compression: one audio fifo per codec */
    FILE *logfile;
} AVOutputStream;

typedef struct AVInputStream {
    int file_index;
    int index;
    AVStream *st;
    int discard;             /* true if stream data should be discarded */
    int decoding_needed;     /* true if the packets must be decoded in 'raw_fifo' */
    int64_t sample_index;      /* current sample */

    int64_t       start;     /* time when read started */
    unsigned long frame;     /* current frame */
    int64_t       next_pts;  /* synthetic pts for cases where pkt.pts
                                is not defined */
    int64_t       pts;       /* current pts */
    int is_start;            /* is 1 at the start and after a discontinuity */
} AVInputStream;

typedef struct AVInputFile {
    int eof_reached;      /* true if eof reached */
    int ist_index;        /* index of first stream in ist_table */
    int buffer_size;      /* current total buffer size */
    int nb_streams;       /* nb streams we are aware of */
} AVInputFile;


/* select an input stream for an output stream */
typedef struct AVStreamMap {
    int file_index;
    int stream_index;
    int sync_file_index;
    int sync_stream_index;
} AVStreamMap;

/** select an input file for an output file */
typedef struct AVMetaDataMap {
    int out_file;
    int in_file;
} AVMetaDataMap;


#define MAX_FILES 1
#define NUM_OPTIONS 30

class CFFmpeg {

  public:  
    CFFmpeg()
    {
      nb_input_files = 0;
      nb_output_files = 0;
      
      nb_stream_maps = 0;     
      nb_meta_data_maps = 0;

      file_iformat = NULL;
      file_oformat = NULL;
      
      frame_width = 0;
      frame_height = 0;
      frame_aspect_ratio = 0;
      frame_pix_fmt = PIX_FMT_NONE;
      frame_padtop = 0;
      frame_padbottom = 0;
      frame_padleft = 0;
      frame_padright = 0;
      
      padcolor[0] = 16;
      padcolor[1] = 128;
      padcolor[2] = 128; /* default to black */
        
      frame_topBand = 0;
      frame_bottomBand = 0;
      frame_leftBand = 0;
      frame_rightBand = 0;
      
      max_frames[0] = INT_MAX;
      max_frames[1] = INT_MAX;
      max_frames[2] = INT_MAX;
      max_frames[3] = INT_MAX;
      
      frame_rate = (AVRational) {25,1};

      video_qscale = 0;
      video_qdiff = 3;
      
      intra_matrix = NULL;
      inter_matrix = NULL;
      video_rc_override_string=NULL;
      
      video_rc_eq = (char*)malloc(sizeof(char*) * (strlen("tex^qComp") + 1));
      strcpy(video_rc_eq, "tex^qComp");
      video_rc_eq[strlen("tex^qComp")] = '\0';
      
      video_disable = 0;
      video_discard = 0;
      video_codec_id = CODEC_ID_NONE;
      video_codec_tag = 0;
      same_quality = 0;
      do_deinterlace = 0;
      strict = 0;
      top_field_first = -1;
      me_threshold = 0;
      intra_dc_precision = 8;
      loop_input = 0;
      loop_output = AVFMT_NOOUTPUTLOOP;
      qp_hist = 0;  
      
        
      intra_only = 0;
      audio_sample_rate = 44100;
      #define QSCALE_NONE -99999
      audio_qscale = QSCALE_NONE;
      audio_disable = 0;
      audio_channels = 1;
      audio_codec_id = CODEC_ID_NONE;
      audio_codec_tag = 0;
      audio_language = NULL;

      subtitle_codec_id = CODEC_ID_NONE;
      subtitle_language = NULL;

      mux_preload= 0.5;
      mux_max_delay= 0.7;

      recording_time = 0;
      start_time = 0;
      rec_timestamp = 0;
      input_ts_offset = 0;
      file_overwrite = 1; //0;
      str_title = NULL;
      str_author = NULL;
      str_copyright = NULL;
      str_comment = NULL;
      str_album = NULL;
      do_benchmark = 0;
      do_hex_dump = 0;
      do_pkt_dump = 0;
      do_psnr = 0;
      do_pass = 0;
      pass_logfilename = NULL;
      audio_stream_copy = 0;
      video_stream_copy = 0;
      subtitle_stream_copy = 0;
      video_sync_method= 1;
      audio_sync_method= 0;
      copy_ts= 0;
      opt_shortest = 0; //
      video_global_header = 0;         
        
      rate_emu = 0;

      video_channel = 0;    

      audio_volume = 256;

      using_stdin = 0;
      using_vhook = 0;
      verbose = 1;
      thread_count= 1;
      q_pressed = 0;
      video_size = 0;
      audio_size = 0;
      extra_size = 0;
      nb_frames_dup = 0;
      nb_frames_drop = 0;
      
      limit_filesize = 0; //

      pgmyuv_compatibility_hack=0;
      dts_delta_threshold = 10;

			#ifdef HAVE_LIBSWSCALE
      sws_flags = SWS_BICUBIC;      
			#endif
				
      video_bitstream_filters=NULL;
      audio_bitstream_filters=NULL;      
      
      opt_names = (const char**)malloc(sizeof(void*));
      opt_name_count = 0;
      
      bit_buffer_size= 1024*256;
      bit_buffer= NULL;
      
      
      opt_counter = 0;
      
      //options = (OptionDef)malloc(sizeof(OptionDef) * 20);
      
      options[opt_counter++] = new OptionDef("f", HAS_ARG, (void(CFFmpeg::*)(const char*))&CFFmpeg::opt_format, "force format", "fmt");
      options[opt_counter++] = new OptionDef("i", HAS_ARG, (void(CFFmpeg::*)(const char*))&CFFmpeg::opt_input_file, "input file name", "filename");
      //options[o++] = (OptionDef) { "y", OPT_BOOL, {(void(CFFmpeg::*)(const char*))&this->file_overwrite}, "overwrite output files" };
      /*options[o++] = (OptionDef) { "map", HAS_ARG | OPT_EXPERT, {(void(*)(const char*))opt_map}, "set input stream mapping", "file:stream[:syncfile:syncstream]" };
      options[o++] = (OptionDef) { "map_meta_data", HAS_ARG | OPT_EXPERT, {(void(*)(const char*))opt_map_meta_data}, "set meta data information of outfile from infile", "outfile:infile" };
      //options[o++] = (OptionDef) { "t", HAS_ARG, {(void(*)(const char*))opt_recording_time}, "set the recording time", "duration" };
      options[o++] = (OptionDef) { "fs", HAS_ARG | OPT_INT64, {(void(*)(const char*))&limit_filesize}, "set the limit file size in bytes", "limit_size" };      
      options[o++] = (OptionDef) { "ss", HAS_ARG, {(void(*)(const char*))opt_start_time}, "set the start time offset", "time_off" };
      
      options[o++] = (OptionDef) { "itsoffset", HAS_ARG, {(void(*)(const char*))opt_input_ts_offset}, "set the input ts offset", "time_off" };*/
      
      options[opt_counter++] = new OptionDef("title", HAS_ARG | OPT_STRING, &this->str_title, "set the title", "string" );
      /*options[o++] = (OptionDef) { "timestamp", HAS_ARG, {(void(*)(const char*))&opt_rec_timestamp}, "set the timestamp", "time" };
      options[o++] = (OptionDef) { "author", HAS_ARG | OPT_STRING, {(void(*)(const char*))&str_author}, "set the author", "string" };
      options[o++] = (OptionDef) { "copyright", HAS_ARG | OPT_STRING, {(void(*)(const char*))&str_copyright}, "set the copyright", "string" };
      options[o++] = (OptionDef) { "comment", HAS_ARG | OPT_STRING, {(void(*)(const char*))&str_comment}, "set the comment", "string" };
      options[o++] = (OptionDef) { "album", HAS_ARG | OPT_STRING, {(void(*)(const char*))&str_album}, "set the album", "string" };
      options[o++] = (OptionDef) { "benchmark", OPT_BOOL | OPT_EXPERT, {(void(*)(const char*))&do_benchmark},
        "add timings for benchmarking" };
      options[o++] = (OptionDef) { "dump", OPT_BOOL | OPT_EXPERT, {(void(*)(const char*))&do_pkt_dump},
        "dump each input packet" };
      options[o++] = (OptionDef) { "hex", OPT_BOOL | OPT_EXPERT, {(void(*)(const char*))&do_hex_dump},
        "when dumping packets, also dump the payload" };
      options[o++] = (OptionDef) { "re", OPT_BOOL | OPT_EXPERT, {(void(*)(const char*))&rate_emu}, "read input at native frame rate", "" };
      options[o++] = (OptionDef) { "loop_input", OPT_BOOL | OPT_EXPERT, {(void(*)(const char*))&loop_input}, "loop (current only works with images)" };
      options[o++] = (OptionDef) { "loop_output", HAS_ARG | OPT_INT | OPT_EXPERT, {(void(*)(const char*))&loop_output}, "number of times to loop output in formats that support looping (0 loops forever)", "" };
      options[o++] = (OptionDef) { "v", HAS_ARG, {(void(*)(const char*))opt_verbose}, "control amount of logging", "verbose" };
      options[o++] = (OptionDef) { "target", HAS_ARG, {(void(*)(const char*))opt_target}, "specify target file type (\"vcd\", \"svcd\", \"dvd\", \"dv\", \"dv50\", \"pal-vcd\", \"ntsc-svcd\", ...)", "type" };
      options[o++] = (OptionDef) { "threads", HAS_ARG | OPT_EXPERT, {(void(*)(const char*))opt_thread_count}, "thread count", "count" };
      options[o++] = (OptionDef) { "vsync", HAS_ARG | OPT_INT | OPT_EXPERT, {(void(*)(const char*))&video_sync_method}, "video sync method", "" };
      options[o++] = (OptionDef) { "async", HAS_ARG | OPT_INT | OPT_EXPERT, {(void(*)(const char*))&audio_sync_method}, "audio sync method", "" };
      options[o++] = (OptionDef) { "vglobal", HAS_ARG | OPT_INT | OPT_EXPERT, {(void(*)(const char*))&video_global_header}, "video global header storage type", "" };
      options[o++] = (OptionDef) { "copyts", OPT_BOOL | OPT_EXPERT, {(void(*)(const char*))&copy_ts}, "copy timestamps" };
      options[o++] = (OptionDef) { "shortest", OPT_BOOL | OPT_EXPERT, {(void(*)(const char*))&opt_shortest}, "finish encoding within shortest input" };
      options[o++] = (OptionDef) { "dts_delta_threshold", HAS_ARG | OPT_INT | OPT_EXPERT, {(void(*)(const char*))&dts_delta_threshold}, "timestamp discontinuity delta threshold", "" };
*/
        
      // video options 
      //options[o++] = (OptionDef) { "vframes", OPT_INT | HAS_ARG | OPT_VIDEO, {(void(*)(const char*))&max_frames[CODEC_TYPE_VIDEO]}, "set the number of video frames to record", "number" };
      //options[o++] = (OptionDef) { "dframes", OPT_INT | HAS_ARG, {(void(*)(const char*))&max_frames[CODEC_TYPE_DATA]}, "set the number of data frames to record", "number" };
      options[opt_counter++] = new OptionDef( "r", HAS_ARG | OPT_VIDEO, (void(CFFmpeg::*)(const char*))&CFFmpeg::opt_frame_rate, "set frame rate (Hz value, fraction or abbreviation)", "rate" );
      options[opt_counter++] = new OptionDef( "s", HAS_ARG | OPT_VIDEO, (void(CFFmpeg::*)(const char*))&CFFmpeg::opt_frame_size, "set frame size (WxH or abbreviation)", "size" );
      options[opt_counter++] = new OptionDef( "aspect", HAS_ARG | OPT_VIDEO, (void(CFFmpeg::*)(const char*))&CFFmpeg::opt_frame_aspect_ratio, "set aspect ratio (4:3, 16:9 or 1.3333, 1.7777)", "aspect" );
      /*options[o++] = (OptionDef) { "pix_fmt", HAS_ARG | OPT_EXPERT | OPT_VIDEO, {(void(*)(const char*))&CFFmpeg::opt_frame_pix_fmt}, "set pixel format, 'list' as argument shows all the pixel formats supported", "format" };
      options[o++] = (OptionDef) { "croptop", HAS_ARG | OPT_VIDEO, {(void(*)(const char*))opt_frame_crop_top}, "set top crop band size (in pixels)", "size" };
      options[o++] = (OptionDef) { "cropbottom", HAS_ARG | OPT_VIDEO, {(void(*)(const char*))opt_frame_crop_bottom}, "set bottom crop band size (in pixels)", "size" };
      options[o++] = (OptionDef) { "cropleft", HAS_ARG | OPT_VIDEO, {(void(*)(const char*))opt_frame_crop_left}, "set left crop band size (in pixels)", "size" };
      options[o++] = (OptionDef) { "cropright", HAS_ARG | OPT_VIDEO, {(void(*)(const char*))opt_frame_crop_right}, "set right crop band size (in pixels)", "size" };
      options[o++] = (OptionDef) { "padtop", HAS_ARG | OPT_VIDEO, {(void(*)(const char*))opt_frame_pad_top}, "set top pad band size (in pixels)", "size" };
      options[o++] = (OptionDef) { "padbottom", HAS_ARG | OPT_VIDEO, {(void(*)(const char*))opt_frame_pad_bottom}, "set bottom pad band size (in pixels)", "size" };
      options[o++] = (OptionDef) { "padleft", HAS_ARG | OPT_VIDEO, {(void(*)(const char*))opt_frame_pad_left}, "set left pad band size (in pixels)", "size" };
      options[o++] = (OptionDef) { "padright", HAS_ARG | OPT_VIDEO, {(void(*)(const char*))opt_frame_pad_right}, "set right pad band size (in pixels)", "size" };
      options[o++] = (OptionDef) { "padcolor", HAS_ARG | OPT_VIDEO, {(void(*)(const char*))opt_pad_color}, "set color of pad bands (Hex 000000 thru FFFFFF)", "color" };
      options[o++] = (OptionDef) { "intra", OPT_BOOL | OPT_EXPERT | OPT_VIDEO, {(void(*)(const char*))&intra_only}, "use only intra frames"};
      options[o++] = (OptionDef) { "vn", OPT_BOOL | OPT_VIDEO, {(void(*)(const char*))&video_disable}, "disable video" };
      options[o++] = (OptionDef) { "vdt", OPT_INT | HAS_ARG | OPT_EXPERT | OPT_VIDEO, {(void(*)(const char*))&video_discard}, "discard threshold", "n" };
      options[o++] = (OptionDef) { "qscale", HAS_ARG | OPT_EXPERT | OPT_VIDEO, {(void(*)(const char*))opt_qscale}, "use fixed video quantizer scale (VBR)", "q" };
      options[o++] = (OptionDef) { "qdiff", HAS_ARG | OPT_EXPERT | OPT_VIDEO, {(void(*)(const char*))opt_qdiff}, "max difference between the quantizer scale (VBR)", "q" };
      options[o++] = (OptionDef) { "rc_eq", HAS_ARG | OPT_EXPERT | OPT_VIDEO, {(void(*)(const char*))opt_video_rc_eq}, "set rate control equation", "equation" };
      options[o++] = (OptionDef) { "rc_override", HAS_ARG | OPT_EXPERT | OPT_VIDEO, {(void(*)(const char*))opt_video_rc_override_string}, "rate control override for specific intervals", "override" };*/
      options[opt_counter++] = new OptionDef ("vcodec", HAS_ARG | OPT_VIDEO, (void(CFFmpeg::*)(const char*))&CFFmpeg::opt_video_codec, "force video codec ('copy' to copy stream)", "codec" );
      /*options[o++] = (OptionDef) { "me_threshold", HAS_ARG | OPT_EXPERT | OPT_VIDEO, {(void(*)(const char*))opt_me_threshold}, "motion estimaton threshold",  "" };
      options[o++] = (OptionDef) { "strict", HAS_ARG | OPT_EXPERT | OPT_VIDEO, {(void(*)(const char*))opt_strict}, "how strictly to follow the standards", "strictness" };
      options[o++] = (OptionDef) { "sameq", OPT_BOOL | OPT_VIDEO, {(void(*)(const char*))&same_quality}, "use same video quality as source (implies VBR)" };
      options[o++] = (OptionDef) { "pass", HAS_ARG | OPT_VIDEO, {(void(*)(const char*))&opt_pass}, "select the pass number (1 or 2)", "n" };
      options[o++] = (OptionDef) { "passlogfile", HAS_ARG | OPT_STRING | OPT_VIDEO, {(void(*)(const char*))&pass_logfilename}, "select two pass log file name", "file" };
      options[o++] = (OptionDef) { "deinterlace", OPT_BOOL | OPT_EXPERT | OPT_VIDEO, {(void(*)(const char*))&do_deinterlace}, "deinterlace pictures" };
      options[o++] = (OptionDef) { "psnr", OPT_BOOL | OPT_EXPERT | OPT_VIDEO, {(void(*)(const char*))&do_psnr}, "calculate PSNR of compressed frames" };
      options[o++] = (OptionDef) { "vstats", OPT_EXPERT | OPT_VIDEO, {(void(*)(const char*))&opt_vstats}, "dump video coding statistics to file" };
      options[o++] = (OptionDef) { "vstats_file", HAS_ARG | OPT_EXPERT | OPT_VIDEO, {(void(*)(const char*))opt_vstats_file}, "dump video coding statistics to file", "file" };

      options[o++] = (OptionDef) { "intra_matrix", HAS_ARG | OPT_EXPERT | OPT_VIDEO, {(void(*)(const char*))opt_intra_matrix}, "specify intra matrix coeffs", "matrix" };
      options[o++] = (OptionDef) { "inter_matrix", HAS_ARG | OPT_EXPERT | OPT_VIDEO, {(void(*)(const char*))opt_inter_matrix}, "specify inter matrix coeffs", "matrix" };
      options[o++] = (OptionDef) { "top", HAS_ARG | OPT_EXPERT | OPT_VIDEO, {(void(*)(const char*))opt_top_field_first}, "top=1/bottom=0/auto=-1 field first", "" };
      options[o++] = (OptionDef) { "dc", OPT_INT | HAS_ARG | OPT_EXPERT | OPT_VIDEO, {(void(*)(const char*))&intra_dc_precision}, "intra_dc_precision", "precision" };
      options[o++] = (OptionDef) { "vtag", HAS_ARG | OPT_EXPERT | OPT_VIDEO, {(void(*)(const char*))opt_video_tag}, "force video tag/fourcc", "fourcc/tag" };
      options[o++] = (OptionDef) { "newvideo", OPT_VIDEO, {(void(*)(const char*))opt_new_video_stream}, "add a new video stream to the current output stream" };
      options[o++] = (OptionDef) { "qphist", OPT_BOOL | OPT_EXPERT | OPT_VIDEO, {(void(*)(const char*))&qp_hist }, "show QP histogram" };
*/
      // audio options 
      //options[o++] = (OptionDef) { "aframes", OPT_INT | HAS_ARG | OPT_AUDIO, {(void(*)(const char*))&max_frames[CODEC_TYPE_AUDIO]}, "set the number of audio frames to record", "number" };
      //options[o++] = (OptionDef) { "aq", OPT_FLOAT | HAS_ARG | OPT_AUDIO, {(void(*)(const char*))&audio_qscale}, "set audio quality (codec-specific)", "quality", };
      options[opt_counter++] = new OptionDef( "ar", HAS_ARG | OPT_AUDIO, (void(CFFmpeg::*)(const char*))&CFFmpeg::opt_audio_rate, "set audio sampling rate (in Hz)", "rate" );
      options[opt_counter++] = new OptionDef( "ac", HAS_ARG | OPT_AUDIO, (void(CFFmpeg::*)(const char*))&CFFmpeg::opt_audio_channels, "set number of audio channels", "channels" );
      //options[o++] = (OptionDef) { "an", OPT_BOOL | OPT_AUDIO, {(void(*)(const char*))&audio_disable}, "disable audio" };
      options[opt_counter++] = new OptionDef( "acodec", HAS_ARG | OPT_AUDIO, (void(CFFmpeg::*)(const char*))&CFFmpeg::opt_audio_codec, "force audio codec ('copy' to copy stream)", "codec" );
      /*options[o++] = (OptionDef) { "atag", HAS_ARG | OPT_EXPERT | OPT_AUDIO, {(void(*)(const char*))opt_audio_tag}, "force audio tag/fourcc", "fourcc/tag" };
      options[o++] = (OptionDef) { "vol", OPT_INT | HAS_ARG | OPT_AUDIO, {(void(*)(const char*))&audio_volume}, "change audio volume (256=normal)" , "volume" }; //
      options[o++] = (OptionDef) { "newaudio", OPT_AUDIO, {(void(*)(const char*))opt_new_audio_stream}, "add a new audio stream to the current output stream" };
      options[o++] = (OptionDef) { "alang", HAS_ARG | OPT_STRING | OPT_AUDIO, {(void(*)(const char*))&audio_language}, "set the ISO 639 language code (3 letters) of the current audio stream" , "code" };

      // subtitle options 
      options[o++] = (OptionDef) { "scodec", HAS_ARG | OPT_SUBTITLE, {(void(*)(const char*))opt_subtitle_codec}, "force subtitle codec ('copy' to copy stream)", "codec" };
      options[o++] = (OptionDef) { "newsubtitle", OPT_SUBTITLE, {(void(*)(const char*))opt_new_subtitle_stream}, "add a new subtitle stream to the current output stream" };
      options[o++] = (OptionDef) { "slang", HAS_ARG | OPT_STRING | OPT_SUBTITLE, {(void(*)(const char*))&subtitle_language}, "set the ISO 639 language code (3 letters) of the current subtitle stream" , "code" };

      // grab options 
      options[o++] = (OptionDef) { "vc", HAS_ARG | OPT_EXPERT | OPT_VIDEO | OPT_GRAB, {(void(*)(const char*))opt_video_channel}, "set video grab channel (DV1394 only)", "channel" };
      options[o++] = (OptionDef) { "tvstd", HAS_ARG | OPT_EXPERT | OPT_VIDEO | OPT_GRAB, {(void(*)(const char*))opt_video_standard}, "set television standard (NTSC, PAL (SECAM))", "standard" };
      options[o++] = (OptionDef) { "isync", OPT_BOOL | OPT_EXPERT | OPT_GRAB, {(void(*)(const char*))&input_sync}, "sync read on input", "" };

      // muxer options 
      options[o++] = (OptionDef) { "muxdelay", OPT_FLOAT | HAS_ARG | OPT_EXPERT, {(void(*)(const char*))&mux_max_delay}, "set the maximum demux-decode delay", "seconds" };
      options[o++] = (OptionDef) { "muxpreload", OPT_FLOAT | HAS_ARG | OPT_EXPERT, {(void(*)(const char*))&mux_preload}, "set the initial demux-decode delay", "seconds" };

      options[o++] = (OptionDef) { "absf", HAS_ARG | OPT_AUDIO | OPT_EXPERT, {(void(*)(const char*))opt_audio_bsf}, "", "bitstream filter" };
      options[o++] = (OptionDef) { "vbsf", HAS_ARG | OPT_VIDEO | OPT_EXPERT, {(void(*)(const char*))opt_video_bsf}, "", "bitstream filter" };*/

      options[opt_counter++] = new OptionDef( "default", OPT_FUNC2 | HAS_ARG | OPT_AUDIO | OPT_VIDEO | OPT_EXPERT, (int(CFFmpeg::*)(const char*, const char*))&CFFmpeg::opt_default, "generic catch all option", "" );      
      options[opt_counter++] = new OptionDef();           

      
      
    }
      
    ~CFFmpeg() 
    { 
      free(opt_names); 
      
      for(int i = opt_counter - 1; i >= 0; i--) {
        delete options[i];
      }
      
    }
  
    int ffmpeg_main(int argc, char **argv);
  
  
  
  
void opt_format(const char *arg)
{
    /* compatibility stuff for pgmyuv */
    if (!strcmp(arg, "pgmyuv")) {
        pgmyuv_compatibility_hack=1;
//        opt_image_format(arg);
        arg = "image2";
        fprintf(stderr, "pgmyuv format is deprecated, use image2\n");
    }

    file_iformat = av_find_input_format(arg);
    file_oformat = guess_format(arg, NULL, NULL);
    if (!file_iformat && !file_oformat) {
        fprintf(stderr, "Unknown input or output format: %s\n", arg);
        return;
    }
}  
  
void opt_video_rc_eq(char *arg)
{
    video_rc_eq = arg;
}

void opt_video_rc_override_string(char *arg)
{
    video_rc_override_string = arg;
}

void opt_me_threshold(const char *arg)
{
    me_threshold = atoi(arg);
}

void opt_verbose(const char *arg)
{
    verbose = atoi(arg);
    av_log_level = atoi(arg);
}

void opt_frame_rate(const char *arg)
{
#ifdef HAVE_AV_PARSE_VIDEO_FUNCTS
    if (av_parse_video_frame_rate(&frame_rate, arg) < 0) {
#else
		if (parse_frame_rate(&frame_rate.num, &frame_rate.den, arg) < 0) {
#endif
        fprintf(stderr, "Incorrect frame rate\n");
        return;
    }
}

void opt_frame_crop_top(const char *arg)
{
    frame_topBand = atoi(arg);
    if (frame_topBand < 0) {
        fprintf(stderr, "Incorrect top crop size\n");
        return;
    }
    if ((frame_topBand % 2) != 0) {
        fprintf(stderr, "Top crop size must be a multiple of 2\n");
        return;
    }
    if ((frame_topBand) >= frame_height){
        fprintf(stderr, "Vertical crop dimensions are outside the range of the original image.\nRemember to crop first and scale second.\n");
        return;
    }
    frame_height -= frame_topBand;
}

void opt_frame_crop_bottom(const char *arg)
{
    frame_bottomBand = atoi(arg);
    if (frame_bottomBand < 0) {
        fprintf(stderr, "Incorrect bottom crop size\n");
        return;
    }
    if ((frame_bottomBand % 2) != 0) {
        fprintf(stderr, "Bottom crop size must be a multiple of 2\n");
        return;
    }
    if ((frame_bottomBand) >= frame_height){
        fprintf(stderr, "Vertical crop dimensions are outside the range of the original image.\nRemember to crop first and scale second.\n");
        return;
    }
    frame_height -= frame_bottomBand;
}

void opt_frame_crop_left(const char *arg)
{
    frame_leftBand = atoi(arg);
    if (frame_leftBand < 0) {
        fprintf(stderr, "Incorrect left crop size\n");
        return;
    }
    if ((frame_leftBand % 2) != 0) {
        fprintf(stderr, "Left crop size must be a multiple of 2\n");
        return;
    }
    if ((frame_leftBand) >= frame_width){
        fprintf(stderr, "Horizontal crop dimensions are outside the range of the original image.\nRemember to crop first and scale second.\n");
        return;
    }
    frame_width -= frame_leftBand;
}

void opt_frame_crop_right(const char *arg)
{
    frame_rightBand = atoi(arg);
    if (frame_rightBand < 0) {
        fprintf(stderr, "Incorrect right crop size\n");
        return;
    }
    if ((frame_rightBand % 2) != 0) {
        fprintf(stderr, "Right crop size must be a multiple of 2\n");
        return;
    }
    if ((frame_rightBand) >= frame_width){
        fprintf(stderr, "Horizontal crop dimensions are outside the range of the original image.\nRemember to crop first and scale second.\n");
        return;
    }
    frame_width -= frame_rightBand;
}

void opt_frame_size(const char *arg)
{
#ifdef HAVE_AV_PARSE_VIDEO_FUNCTS
    if (av_parse_video_frame_size(&frame_width, &frame_height, arg) < 0) {
#else
		if (parse_image_size(&frame_width, &frame_height, arg) < 0) {
#endif
        fprintf(stderr, "Incorrect frame size\n");
        return;
    }
    if ((frame_width % 2) != 0 || (frame_height % 2) != 0) {
        fprintf(stderr, "Frame size must be a multiple of 2\n");
        return;
    }
}


#define SCALEBITS 10
#define ONE_HALF  (1 << (SCALEBITS - 1))
#define FIX(x)    ((int) ((x) * (1<<SCALEBITS) + 0.5))

#define RGB_TO_Y(r, g, b) \
((FIX(0.29900) * (r) + FIX(0.58700) * (g) + \
  FIX(0.11400) * (b) + ONE_HALF) >> SCALEBITS)

#define RGB_TO_U(r1, g1, b1, shift)\
(((- FIX(0.16874) * r1 - FIX(0.33126) * g1 +         \
     FIX(0.50000) * b1 + (ONE_HALF << shift) - 1) >> (SCALEBITS + shift)) + 128)

#define RGB_TO_V(r1, g1, b1, shift)\
(((FIX(0.50000) * r1 - FIX(0.41869) * g1 -           \
   FIX(0.08131) * b1 + (ONE_HALF << shift) - 1) >> (SCALEBITS + shift)) + 128)

void opt_pad_color(const char *arg) {
    /* Input is expected to be six hex digits similar to
       how colors are expressed in html tags (but without the #) */
    int rgb = strtol(arg, NULL, 16);
    int r,g,b;

    r = (rgb >> 16);
    g = ((rgb >> 8) & 255);
    b = (rgb & 255);

    padcolor[0] = RGB_TO_Y(r,g,b);
    padcolor[1] = RGB_TO_U(r,g,b,0);
    padcolor[2] = RGB_TO_V(r,g,b,0);
}

void opt_frame_pad_top(const char *arg)
{
    frame_padtop = atoi(arg);
    if (frame_padtop < 0) {
        fprintf(stderr, "Incorrect top pad size\n");
        return;
    }
    if ((frame_padtop % 2) != 0) {
        fprintf(stderr, "Top pad size must be a multiple of 2\n");
        return;
    }
}

void opt_frame_pad_bottom(const char *arg)
{
    frame_padbottom = atoi(arg);
    if (frame_padbottom < 0) {
        fprintf(stderr, "Incorrect bottom pad size\n");
        return;
    }
    if ((frame_padbottom % 2) != 0) {
        fprintf(stderr, "Bottom pad size must be a multiple of 2\n");
        return;
    }
}


void opt_frame_pad_left(const char *arg)
{
    frame_padleft = atoi(arg);
    if (frame_padleft < 0) {
        fprintf(stderr, "Incorrect left pad size\n");
        return;
    }
    if ((frame_padleft % 2) != 0) {
        fprintf(stderr, "Left pad size must be a multiple of 2\n");
        return;
    }
}


void opt_frame_pad_right(const char *arg)
{
    frame_padright = atoi(arg);
    if (frame_padright < 0) {
        fprintf(stderr, "Incorrect right pad size\n");
        return;
    }
    if ((frame_padright % 2) != 0) {
        fprintf(stderr, "Right pad size must be a multiple of 2\n");
        return;
    }
}

/*void list_pix_fmts(void)
{
    int i;
    char pix_fmt_str[128];
    for (i=-1; i < PIX_FMT_NB; i++) {
        avcodec_pix_fmt_string (pix_fmt_str, sizeof(pix_fmt_str), i);
        fprintf(stdout, "%s\n", pix_fmt_str);
    }
}*/

void opt_frame_pix_fmt(const char *arg)
{
    if (strcmp(arg, "list"))
        frame_pix_fmt = avcodec_get_pix_fmt(arg);
    else {
        //list_pix_fmts();
        return;
    }
}

void opt_frame_aspect_ratio(const char *arg)
{
    int x = 0, y = 0;
    double ar = 0;
    const char *p;

    p = strchr(arg, ':');
    if (p) {
        x = strtol(arg, (char **)&arg, 10);
        if (arg == p)
            y = strtol(arg+1, (char **)&arg, 10);
        if (x > 0 && y > 0)
            ar = (double)x / (double)y;
    } else
        ar = strtod(arg, (char **)&arg);

    if (!ar) {
        fprintf(stderr, "Incorrect aspect ratio specification.\n");
        return;
    }
    frame_aspect_ratio = ar;
}

void opt_qscale(const char *arg)
{
    video_qscale = atof(arg);
    if (video_qscale <= 0 ||
        video_qscale > 255) {
        fprintf(stderr, "qscale must be > 0.0 and <= 255\n");
        return;
    }
}

void opt_qdiff(const char *arg)
{
    video_qdiff = atoi(arg);
    if (video_qdiff < 0 ||
        video_qdiff > 31) {
        fprintf(stderr, "qdiff must be >= 1 and <= 31\n");
        return;
    }
}

void opt_strict(const char *arg)
{
    strict= atoi(arg);
}

void opt_top_field_first(const char *arg)
{
    top_field_first= atoi(arg);
}

void opt_thread_count(const char *arg)
{
    thread_count= atoi(arg);
#if !defined(HAVE_THREADS)
    if (verbose >= 0)
        fprintf(stderr, "Warning: not compiled with thread support, using thread emulation\n");
#endif
}

void opt_audio_rate(const char *arg)
{
    audio_sample_rate = atoi(arg);
}

void opt_audio_channels(const char *arg)
{
    audio_channels = atoi(arg);
}

void opt_video_channel(const char *arg)
{
    video_channel = strtol(arg, NULL, 0);
}

void opt_video_standard(const char *arg)
{
    video_standard = av_strdup(arg);
}

void opt_codec(int *pstream_copy, int *pcodec_id,
                      int codec_type, const char *arg)
{
    AVCodec *p;

    if (!strcmp(arg, "copy")) {
        *pstream_copy = 1;
    } else {
        p = first_avcodec;
        while (p) {
            if (!strcmp(p->name, arg) && p->type == codec_type)
                break;
            p = p->next;
        }
        if (p == NULL) {
            fprintf(stderr, "Unknown codec '%s'\n", arg);
            return;
        } else {
            *pcodec_id = p->id;
        }
    }
}

void opt_audio_codec(const char *arg)
{
    opt_codec(&audio_stream_copy, &audio_codec_id, CODEC_TYPE_AUDIO, arg);
}

void opt_audio_tag(const char *arg)
{
    char *tail;
    audio_codec_tag= strtol(arg, &tail, 0);

    if(!tail || *tail)
        audio_codec_tag= arg[0] + (arg[1]<<8) + (arg[2]<<16) + (arg[3]<<24);
}

void opt_video_tag(const char *arg)
{
    char *tail;
    video_codec_tag= strtol(arg, &tail, 0);

    if(!tail || *tail)
        video_codec_tag= arg[0] + (arg[1]<<8) + (arg[2]<<16) + (arg[3]<<24);
}  

  
void opt_video_codec(const char *arg)
{
    opt_codec(&video_stream_copy, &video_codec_id, CODEC_TYPE_VIDEO, arg);
}

void opt_subtitle_codec(const char *arg)
{
    opt_codec(&subtitle_stream_copy, &subtitle_codec_id, CODEC_TYPE_SUBTITLE, arg);
}

void opt_map(const char *arg)
{
    AVStreamMap *m;
    const char *p;

    p = arg;
    m = &stream_maps[nb_stream_maps++];

    m->file_index = strtol(arg, (char **)&p, 0);
    if (*p)
        p++;

    m->stream_index = strtol(p, (char **)&p, 0);
    if (*p) {
        p++;
        m->sync_file_index = strtol(p, (char **)&p, 0);
        if (*p)
            p++;
        m->sync_stream_index = strtol(p, (char **)&p, 0);
    } else {
        m->sync_file_index = m->file_index;
        m->sync_stream_index = m->stream_index;
    }
}

void opt_map_meta_data(const char *arg)
{
    AVMetaDataMap *m;
    const char *p;

    p = arg;
    m = &meta_data_maps[nb_meta_data_maps++];

    m->out_file = strtol(arg, (char **)&p, 0);
    if (*p)
        p++;

    m->in_file = strtol(p, (char **)&p, 0);
}

void opt_recording_time(const char *arg)
{
    recording_time = parse_date(arg, 1);
}

void opt_start_time(const char *arg)
{
    start_time = parse_date(arg, 1);
}

void opt_rec_timestamp(const char *arg)
{
    rec_timestamp = parse_date(arg, 0) / 1000000;
}

void opt_input_ts_offset(const char *arg)
{
    input_ts_offset = parse_date(arg, 1);
}

void opt_input_file(const char *filename)
{
    AVFormatContext *ic;
    AVFormatParameters params, *ap = &params;
    int err, i, ret, rfps, rfps_base;
    int64_t timestamp;

    if (!strcmp(filename, "-"))
        filename = "pipe:";

    using_stdin |= !strncmp(filename, "pipe:", 5) ||
                   !strcmp( filename, "/dev/stdin" );

    /* get default parameters from command line */
    ic = av_alloc_format_context();

    memset(ap, 0, sizeof(*ap));
    ap->prealloced_context = 1;
    ap->sample_rate = audio_sample_rate;
    ap->channels = audio_channels;
    ap->time_base.den = frame_rate.num;
    ap->time_base.num = frame_rate.den;
	ap->width = frame_width + frame_padleft + frame_padright;
    ap->height = frame_height + frame_padtop + frame_padbottom;
    ap->pix_fmt = frame_pix_fmt;
    ap->channel = video_channel;
    ap->standard = video_standard;
    ap->video_codec_id = (CodecID)video_codec_id;
    ap->audio_codec_id = (CodecID)audio_codec_id;
    if(pgmyuv_compatibility_hack)
        ap->video_codec_id= CODEC_ID_PGMYUV;
  
    for(i=0; i<opt_name_count; i++){
        const AVOption *opt;
        double d= av_get_double(avformat_opts, opt_names[i], &opt);
        if(d==d && (opt->flags&AV_OPT_FLAG_DECODING_PARAM))
            av_set_double(ic, opt_names[i], d);
    }
    /* open the input file with generic libav function */
    err = av_open_input_file(&ic, filename, file_iformat, 0, ap);
    if (err < 0) {
        print_error(filename, err);
        return;
    }

    ic->loop_input = loop_input;

    /* If not enough info to get the stream parameters, we decode the
       first frames to get it. (used in mpeg case for example) */
    ret = av_find_stream_info(ic);
    if (ret < 0 && verbose >= 0) {
        fprintf(stderr, "%s: could not find codec parameters\n", filename);
        return;
    }

    timestamp = start_time;
    /* add the stream start time */
    if (ic->start_time != AV_NOPTS_VALUE)
        timestamp += ic->start_time;

    /* if seeking requested, we execute it */
    if (start_time != 0) {
        ret = av_seek_frame(ic, -1, timestamp, AVSEEK_FLAG_BACKWARD);
        if (ret < 0) {
            fprintf(stderr, "%s: could not seek to position %0.3f\n",
                    filename, (double)timestamp / AV_TIME_BASE);
        }
        /* reset seek info */
        start_time = 0;
    }

    /* update the current parameters so that they match the one of the input stream */
    for(i=0;i<ic->nb_streams;i++) {
        int j;
        AVCodecContext *enc = ic->streams[i]->codec;
        if(thread_count>1)
            avcodec_thread_init(enc, thread_count);
        enc->thread_count= thread_count;
        switch(enc->codec_type) {
        case CODEC_TYPE_AUDIO:
            for(j=0; j<opt_name_count; j++){
                const AVOption *opt;
                double d= av_get_double(avctx_opts[CODEC_TYPE_AUDIO], opt_names[j], &opt);
                if(d==d && (opt->flags&AV_OPT_FLAG_AUDIO_PARAM) && (opt->flags&AV_OPT_FLAG_DECODING_PARAM))
                    av_set_double(enc, opt_names[j], d);
            }
            //fprintf(stderr, "\nInput Audio channels: %d", enc->channels);
            audio_channels = enc->channels;
            audio_sample_rate = enc->sample_rate;
            if(audio_disable)
                ic->streams[i]->discard= AVDISCARD_ALL;
            break;
        case CODEC_TYPE_VIDEO:
            for(j=0; j<opt_name_count; j++){
                const AVOption *opt;
                double d= av_get_double(avctx_opts[CODEC_TYPE_VIDEO], opt_names[j], &opt);
                if(d==d && (opt->flags&AV_OPT_FLAG_VIDEO_PARAM) && (opt->flags&AV_OPT_FLAG_DECODING_PARAM))
                    av_set_double(enc, opt_names[j], d);
            }
            frame_height = enc->height;
            frame_width = enc->width;
            frame_aspect_ratio = av_q2d(enc->sample_aspect_ratio) * enc->width / enc->height;
            frame_pix_fmt = enc->pix_fmt;
            rfps      = ic->streams[i]->r_frame_rate.num;
            rfps_base = ic->streams[i]->r_frame_rate.den;
            if(enc->lowres) enc->flags |= CODEC_FLAG_EMU_EDGE;
            if(me_threshold)
                enc->debug |= FF_DEBUG_MV;

            if (enc->time_base.den != rfps || enc->time_base.num != rfps_base) {

                if (verbose >= 0)
                    fprintf(stderr,"\nSeems stream %d codec frame rate differs from container frame rate: %2.2f (%d/%d) -> %2.2f (%d/%d)\n",
                            i, (float)enc->time_base.den / enc->time_base.num, enc->time_base.den, enc->time_base.num,

                    (float)rfps / rfps_base, rfps, rfps_base);
            }
            /* update the current frame rate to match the stream frame rate */
            frame_rate.num = rfps;
            frame_rate.den = rfps_base;

            enc->rate_emu = rate_emu;
            if(video_disable)
                ic->streams[i]->discard= AVDISCARD_ALL;
            else if(video_discard)
                ic->streams[i]->discard= (AVDiscard)video_discard;
            break;
        case CODEC_TYPE_DATA:
            break;
        case CODEC_TYPE_SUBTITLE:
            break;
        case CODEC_TYPE_UNKNOWN:
            break;
        default:
            return; //abort();
        }
    }

    input_files[nb_input_files] = ic;
    input_files_ts_offset[nb_input_files] = input_ts_offset - (copy_ts ? 0 : timestamp);
    /* dump the file content */
    if (verbose >= 0)
        dump_format(ic, nb_input_files, filename, 0);

    nb_input_files++;
    file_iformat = NULL;
    file_oformat = NULL;

    video_channel = 0;

    rate_emu = 0;
}  
  

void parse_matrix_coeffs(uint16_t *dest, const char *str)
{
    int i;
    const char *p = str;
    for(i = 0;; i++) {
        dest[i] = atoi(p);
        if(i == 63)
            break;
        p = strchr(p, ',');
        if(!p) {
            fprintf(stderr, "Syntax error in matrix \"%s\" at coeff %d\n", str, i);
            return;
        }
        p++;
    }
}

void opt_inter_matrix(const char *arg)
{
    inter_matrix = (uint16_t*)av_mallocz(sizeof(uint16_t) * 64);
    parse_matrix_coeffs(inter_matrix, arg);
}

void opt_intra_matrix(const char *arg)
{
    intra_matrix = (uint16_t*)av_mallocz(sizeof(uint16_t) * 64);
    parse_matrix_coeffs(intra_matrix, arg);
}

void opt_target(const char *arg)
{
    int norm = -1;
    static const char *const frame_rates[] = {"25", "30000/1001", "24000/1001"};

    if(!strncmp(arg, "pal-", 4)) {
        norm = 0;
        arg += 4;
    } else if(!strncmp(arg, "ntsc-", 5)) {
        norm = 1;
        arg += 5;
    } else if(!strncmp(arg, "film-", 5)) {
        norm = 2;
        arg += 5;
    } else {
        int fr;
        /* Calculate FR via float to avoid int overflow */
        fr = (int)(frame_rate.num * 1000.0 / frame_rate.den);
        if(fr == 25000) {
            norm = 0;
        } else if((fr == 29970) || (fr == 23976)) {
            norm = 1;
        } else {
            /* Try to determine PAL/NTSC by peeking in the input files */
            if(nb_input_files) {
                int i, j;
                for(j = 0; j < nb_input_files; j++) {
                    for(i = 0; i < input_files[j]->nb_streams; i++) {
                        AVCodecContext *c = input_files[j]->streams[i]->codec;
                        if(c->codec_type != CODEC_TYPE_VIDEO)
                            continue;
                        fr = c->time_base.den * 1000 / c->time_base.num;
                        if(fr == 25000) {
                            norm = 0;
                            break;
                        } else if((fr == 29970) || (fr == 23976)) {
                            norm = 1;
                            break;
                        }
                    }
                    if(norm >= 0)
                        break;
                }
            }
        }
        if(verbose && norm >= 0)
            fprintf(stderr, "Assuming %s for target.\n", norm ? "NTSC" : "PAL");
    }

    if(norm < 0) {
        fprintf(stderr, "Could not determine norm (PAL/NTSC/NTSC-Film) for target.\n");
        fprintf(stderr, "Please prefix target with \"pal-\", \"ntsc-\" or \"film-\",\n");
        fprintf(stderr, "or set a framerate with \"-r xxx\".\n");
       return;
    }

    if(!strcmp(arg, "vcd")) {

        opt_video_codec("mpeg1video");
        opt_audio_codec("mp2");
        opt_format("vcd");

        opt_frame_size(norm ? "352x240" : "352x288");
        opt_frame_rate(frame_rates[norm]);
        opt_default("gop", norm ? "18" : "15");

        opt_default("b", "1150000");
        opt_default("maxrate", "1150000");
        opt_default("minrate", "1150000");
        opt_default("bufsize", "327680"); // 40*1024*8;

        opt_default("ab", "224000");
        audio_sample_rate = 44100;
        audio_channels = 2;

        opt_default("packetsize", "2324");
        opt_default("muxrate", "1411200"); // 2352 * 75 * 8;

        /* We have to offset the PTS, so that it is consistent with the SCR.
           SCR starts at 36000, but the first two packs contain only padding
           and the first pack from the other stream, respectively, may also have
           been written before.
           So the real data starts at SCR 36000+3*1200. */
        mux_preload= (36000+3*1200) / 90000.0; //0.44
    } else if(!strcmp(arg, "svcd")) {

        opt_video_codec("mpeg2video");
        opt_audio_codec("mp2");
        opt_format("svcd");

        opt_frame_size(norm ? "480x480" : "480x576");
        opt_frame_rate(frame_rates[norm]);
        opt_default("gop", norm ? "18" : "15");

        opt_default("b", "2040000");
        opt_default("maxrate", "2516000");
        opt_default("minrate", "0"); //1145000;
        opt_default("bufsize", "1835008"); //224*1024*8;
        opt_default("flags", "+SCAN_OFFSET");


        opt_default("ab", "224000");
        audio_sample_rate = 44100;

        opt_default("packetsize", "2324");

    } else if(!strcmp(arg, "dvd")) {

        opt_video_codec("mpeg2video");
        opt_audio_codec("ac3");
        opt_format("dvd");

        opt_frame_size(norm ? "720x480" : "720x576");
        opt_frame_rate(frame_rates[norm]);
        opt_default("gop", norm ? "18" : "15");

        opt_default("b", "6000000");
        opt_default("maxrate", "9000000");
        opt_default("minrate", "0"); //1500000;
        opt_default("bufsize", "1835008"); //224*1024*8;

        opt_default("packetsize", "2048");  // from www.mpucoder.com: DVD sectors contain 2048 bytes of data, this is also the size of one pack.
        opt_default("muxrate", "10080000"); // from mplex project: data_rate = 1260000. mux_rate = data_rate * 8

        opt_default("ab", "448000");
        audio_sample_rate = 48000;

    } else if(!strncmp(arg, "dv", 2)) {

        opt_format("dv");

        opt_frame_size(norm ? "720x480" : "720x576");
        opt_frame_pix_fmt(!strncmp(arg, "dv50", 4) ? "yuv422p" :
                                             (norm ? "yuv411p" : "yuv420p"));
        opt_frame_rate(frame_rates[norm]);

        audio_sample_rate = 48000;
        audio_channels = 2;

    } else {
        fprintf(stderr, "Unknown target: %s\n", arg);
        return;
    }
}

void opt_vstats_file (const char *arg)
{
    av_free (vstats_filename);
    vstats_filename=av_strdup (arg);
}

void opt_vstats (void)
{
    char filename[40];
    time_t today2 = time(NULL);
    struct tm *today = localtime(&today2);

    snprintf(filename, sizeof(filename), "vstats_%02d%02d%02d.log", today->tm_hour, today->tm_min,
             today->tm_sec);
    opt_vstats_file(filename);
}

void opt_video_bsf(const char *arg)
{
    AVBitStreamFilterContext *bsfc= av_bitstream_filter_init(arg); //FIXME split name and args for filter at '='
    AVBitStreamFilterContext **bsfp;

    if(!bsfc){
        fprintf(stderr, "Unknown bitstream filter %s\n", arg);
        return;
    }

    bsfp= &video_bitstream_filters;
    while(*bsfp)
        bsfp= &(*bsfp)->next;

    *bsfp= bsfc;
}

//FIXME avoid audio - video code duplication
void opt_audio_bsf(const char *arg)
{
    AVBitStreamFilterContext *bsfc= av_bitstream_filter_init(arg); //FIXME split name and args for filter at '='
    AVBitStreamFilterContext **bsfp;

    if(!bsfc){
        fprintf(stderr, "Unknown bitstream filter %s\n", arg);
        return;
    }

    bsfp= &audio_bitstream_filters;
    while(*bsfp)
        bsfp= &(*bsfp)->next;

    *bsfp= bsfc;
}

int opt_default(const char *opt, const char *arg){
  
  int type;
    const AVOption *o= NULL;
    int opt_types[]={AV_OPT_FLAG_VIDEO_PARAM, AV_OPT_FLAG_AUDIO_PARAM, 0, AV_OPT_FLAG_SUBTITLE_PARAM, 0};

    for(type=0; type<CODEC_TYPE_NB; type++){
        const AVOption *o2 = av_find_opt(avctx_opts[0], opt, NULL, opt_types[type], opt_types[type]);
        if(o2)
            o = av_set_string(avctx_opts[type], opt, arg);
    }
    if(!o)
        o = av_set_string(avformat_opts, opt, arg);
    if(!o)
        o = av_set_string(sws_opts, opt, arg);
    if(!o){
        if(opt[0] == 'a')
            o = av_set_string(avctx_opts[CODEC_TYPE_AUDIO], opt+1, arg);
        else if(opt[0] == 'v')
            o = av_set_string(avctx_opts[CODEC_TYPE_VIDEO], opt+1, arg);
        else if(opt[0] == 's')
            o = av_set_string(avctx_opts[CODEC_TYPE_SUBTITLE], opt+1, arg);
    }
    if(!o)
        return -1;

//    av_log(NULL, AV_LOG_ERROR, "%s:%s: %f 0x%0X\n", opt, arg, av_get_double(avctx_opts, opt, NULL), (int)av_get_int(avctx_opts, opt, NULL));

    //FIXME we should always use avctx_opts, ... for storing options so there wont be any need to keep track of whats set over this
    opt_names= (const char**)av_realloc(opt_names, sizeof(void*)*(opt_name_count+1));
    opt_names[opt_name_count++]= o->name;

#if defined(CONFIG_FFM_DEMUXER) || defined(CONFIG_FFM_MUXER)
    /* disable generate of real time pts in ffm (need to be supressed anyway) */
    if(avctx_opts[0]->flags & CODEC_FLAG_BITEXACT)
        ffm_nopts = 1;
#endif

    if(avctx_opts[0]->debug)
        av_log_level = AV_LOG_DEBUG;
    return 0;
}


void opt_new_subtitle_stream(void)
{
    AVFormatContext *oc;
    AVStream *st;
    AVCodecContext *subtitle_enc;
    int i;

    if (nb_output_files <= 0) {
        fprintf(stderr, "At least one output file must be specified\n");
        return;
    }
    oc = output_files[nb_output_files - 1];

    st = av_new_stream(oc, oc->nb_streams);
    if (!st) {
        fprintf(stderr, "Could not alloc stream\n");
        return;
    }
    avcodec_get_context_defaults2(st->codec, CODEC_TYPE_SUBTITLE);

    subtitle_enc = st->codec;
    subtitle_enc->codec_type = CODEC_TYPE_SUBTITLE;
    if (subtitle_stream_copy) {
        st->stream_copy = 1;
    } else {
        for(i=0; i<opt_name_count; i++){
             const AVOption *opt;
             double d= av_get_double(avctx_opts[CODEC_TYPE_SUBTITLE], opt_names[i], &opt);
             if(d==d && (opt->flags&AV_OPT_FLAG_SUBTITLE_PARAM) && (opt->flags&AV_OPT_FLAG_ENCODING_PARAM))
                 av_set_double(subtitle_enc, opt_names[i], d);
        }
        subtitle_enc->codec_id = (CodecID)subtitle_codec_id;
    }

    if (subtitle_language) {
        #ifdef HAVE_AVSTRING_H
        av_strlcpy(st->language, subtitle_language, sizeof(st->language));
        #else
        pstrcpy(st->language, sizeof(st->language), subtitle_language);      
        #endif
        av_free(subtitle_language);
        subtitle_language = NULL;
    }

    subtitle_codec_id = CODEC_ID_NONE;
    subtitle_stream_copy = 0;
}

void opt_new_audio_stream(void)
{
    AVFormatContext *oc;
    if (nb_output_files <= 0) {
        fprintf(stderr, "At least one output file must be specified\n");
        return;
    }
    oc = output_files[nb_output_files - 1];
    new_audio_stream(oc);
}

void opt_new_video_stream(void)
{
    AVFormatContext *oc;
    if (nb_output_files <= 0) {
        fprintf(stderr, "At least one output file must be specified\n");
        return;
    }
    oc = output_files[nb_output_files - 1];
    new_video_stream(oc);
}

void opt_output_file(const char *filename)
{
    AVFormatContext *oc;
    int use_video, use_audio, input_has_video, input_has_audio, i;
    AVFormatParameters params, *ap = &params;

    if (!strcmp(filename, "-"))
        filename = "pipe:";

    oc = av_alloc_format_context();

    if (!file_oformat) {
        file_oformat = guess_format(NULL, filename, NULL);
        if (!file_oformat) {
            fprintf(stderr, "Unable to find a suitable output format for '%s'\n",
                    filename);
            return;
        }
    }

    oc->oformat = file_oformat;
    #ifdef HAVE_AVSTRING_H
    av_strlcpy(oc->filename, filename, sizeof(oc->filename));
    #else
    pstrcpy(oc->filename, sizeof(oc->filename), filename);
    #endif

    if (!strcmp(file_oformat->name, "ffm") &&
        #ifdef HAVE_AVSTRING_H
        av_strstart(filename, "http:", NULL)
        #else
        strstart(filename, "http:", NULL)
        #endif
        ) {
        /* special case for files sent to ffserver: we get the stream
           parameters from ffserver */
        if (read_ffserver_streams(oc, filename) < 0) {
            fprintf(stderr, "Could not read stream parameters from '%s'\n", filename);
            return;
        }
    } else {
        use_video = file_oformat->video_codec != CODEC_ID_NONE || video_stream_copy || video_codec_id != CODEC_ID_NONE;
        use_audio = file_oformat->audio_codec != CODEC_ID_NONE || audio_stream_copy || audio_codec_id != CODEC_ID_NONE;

        /* disable if no corresponding type found and at least one
           input file */
        if (nb_input_files > 0) {
            check_audio_video_inputs(&input_has_video, &input_has_audio);
            if (!input_has_video)
                use_video = 0;
            if (!input_has_audio)
                use_audio = 0;
        }

        /* manual disable */
        if (audio_disable) {
            use_audio = 0;
        }
        if (video_disable) {
            use_video = 0;
        }

        if (use_video) {
            new_video_stream(oc);
        }

        if (use_audio) {
            new_audio_stream(oc);
        }

        oc->timestamp = rec_timestamp;

        #ifdef HAVE_AVSTRING_H
        if (str_title)
            av_strlcpy(oc->title, str_title, sizeof(oc->title));
        if (str_author)
            av_strlcpy(oc->author, str_author, sizeof(oc->author));
        if (str_copyright)
            av_strlcpy(oc->copyright, str_copyright, sizeof(oc->copyright));
        if (str_comment)
            av_strlcpy(oc->comment, str_comment, sizeof(oc->comment));
        if (str_album)
            av_strlcpy(oc->album, str_album, sizeof(oc->album));
        #else
        if (str_title)
            pstrcpy(oc->title, sizeof(oc->title), str_title);
        if (str_author)
            pstrcpy(oc->author, sizeof(oc->author), str_author);
        if (str_copyright)
            pstrcpy(oc->copyright, sizeof(oc->copyright), str_copyright);
        if (str_comment)
            pstrcpy(oc->comment, sizeof(oc->comment), str_comment);
        if (str_album)
            pstrcpy(oc->album, sizeof(oc->album), str_album);
        #endif
    }

    output_files[nb_output_files++] = oc;

    /* check filename in case of an image number is expected */
    if (oc->oformat->flags & AVFMT_NEEDNUMBER) {
        if (!av_filename_number_test(oc->filename)) {
            //print_error(oc->filename, AVERROR_NUMEXPECTED);
            return;
        }
    }

    if (!(oc->oformat->flags & AVFMT_NOFILE)) {
        /* test if it already exists to avoid loosing precious files */
        /*if (!file_overwrite &&
            (strchr(filename, ':') == NULL ||
             #ifdef HAVE_AVSTRING_H
             av_strstart(filename, "file:", NULL)
             #else
             strstart(filename, "file:", NULL)
             #endif             
             )) {
            if (url_exist(filename)) {
                int c;

                if ( !using_stdin ) {
                    fprintf(stderr,"File '%s' already exists. Overwrite ? [y/N] ", filename);
                    fflush(stderr);
                    c = getchar();
                    if (toupper(c) != 'Y') {
                        fprintf(stderr, "Not overwriting - exiting\n");
                        return;
                    }
                                }
                                else {
                    fprintf(stderr,"File '%s' already exists. Exiting.\n", filename);
                    return;
                                }
            }
        }*/

        /* open the file */
        if (url_fopen(&oc->pb, filename, URL_WRONLY) < 0) {
            fprintf(stderr, "Could not open '%s'\n", filename);
            return;
        }
    }

    memset(ap, 0, sizeof(*ap));
    if (av_set_parameters(oc, ap) < 0) {
        fprintf(stderr, "%s: Invalid encoding parameters\n",
                oc->filename);
        return;
    }

    oc->preload= (int)(mux_preload*AV_TIME_BASE);
    oc->max_delay= (int)(mux_max_delay*AV_TIME_BASE);
    oc->loop_output = loop_output;

    for(i=0; i<opt_name_count; i++){
        const AVOption *opt;
        double d = av_get_double(avformat_opts, opt_names[i], &opt);
        if(d==d && (opt->flags&AV_OPT_FLAG_ENCODING_PARAM))
            av_set_double(oc, opt_names[i], d);
    }

    /* reset some options */
    file_oformat = NULL;
    file_iformat = NULL;
}

/* same option as mencoder */
void opt_pass(const char *pass_str)
{
    int pass;
    pass = atoi(pass_str);
    if (pass != 1 && pass != 2) {
        fprintf(stderr, "pass number can be only 1 or 2\n");
        return;
    }
    do_pass = pass;
}

  public:
    OptionDef* options[NUM_OPTIONS];
    
    AVFormatContext *input_files[MAX_FILES];
    int64_t input_files_ts_offset[MAX_FILES];
    int nb_input_files; // = 0;

    AVFormatContext *output_files[MAX_FILES];
    int nb_output_files; // = 0;

    AVStreamMap stream_maps[MAX_FILES];
    int nb_stream_maps;

    AVMetaDataMap meta_data_maps[MAX_FILES];
    int nb_meta_data_maps;

    AVInputFormat *file_iformat;
    AVOutputFormat *file_oformat;
    int frame_width; //  = 0;
    int frame_height; // = 0;
    float frame_aspect_ratio; // = 0;
    enum PixelFormat frame_pix_fmt; // = PIX_FMT_NONE;
    int frame_padtop; //  = 0;
    int frame_padbottom; // = 0;
    int frame_padleft; //  = 0;
    int frame_padright; // = 0;
    int padcolor[3]; //
    int frame_topBand; //  = 0;
    int frame_bottomBand; // = 0;
    int frame_leftBand; //  = 0;
    int frame_rightBand; // = 0;
    int max_frames[4]; // = {INT_MAX, INT_MAX, INT_MAX, INT_MAX};
	AVRational frame_rate; // = (AVRational) {25,1};
    float video_qscale; // = 0;
    int video_qdiff; // = 3;
    uint16_t *intra_matrix; // = NULL;
    uint16_t *inter_matrix; // = NULL;
    char *video_rc_override_string; //=NULL;
    char *video_rc_eq; //="tex^qComp";
    int video_disable; // = 0;
    int video_discard; // = 0;
    int video_codec_id; // = CODEC_ID_NONE;
    int video_codec_tag; // = 0;
    int same_quality; // = 0;
    int do_deinterlace; // = 0;
    int strict; // = 0;
    int top_field_first; // = -1;
    int me_threshold; // = 0;
    int intra_dc_precision; // = 8;
    int loop_input; // = 0;
    int loop_output; // = AVFMT_NOOUTPUTLOOP;
    int qp_hist; // = 0;

    int intra_only; // = 0;
    int audio_sample_rate; // = 44100;
    
    float audio_qscale; // = QSCALE_NONE;
    int audio_disable; // = 0;
    int audio_channels; // = 1;
    int audio_codec_id; // = CODEC_ID_NONE;
    int audio_codec_tag; // = 0;
    char *audio_language; // = NULL;

    int subtitle_codec_id; // = CODEC_ID_NONE;
    char *subtitle_language; // = NULL;

    float mux_preload; //= 0.5;
    float mux_max_delay; //= 0.7;

    int64_t recording_time; // = 0;
    int64_t start_time; // = 0;
    int64_t rec_timestamp; // = 0;
    int64_t input_ts_offset; // = 0;
    int file_overwrite; // = 0;
    char *str_title; // = NULL;
    char *str_author; // = NULL;
    char *str_copyright; // = NULL;
    char *str_comment; // = NULL;
    char *str_album; // = NULL;
    int do_benchmark; // = 0;
    int do_hex_dump; // = 0;
    int do_pkt_dump; // = 0;
    int do_psnr; // = 0;
    int do_pass; // = 0;
    char *pass_logfilename; // = NULL;
    int audio_stream_copy; // = 0;
    int video_stream_copy; // = 0;
    int subtitle_stream_copy; // = 0;
    int video_sync_method; //= 1;
    int audio_sync_method; //= 0;
    int copy_ts; //= 0;
    int opt_shortest; // = 0; //
    int video_global_header; // = 0;
    char *vstats_filename;
    FILE *fvstats;

    int rate_emu; // = 0;

    int  video_channel; // = 0;
    char *video_standard;

    int audio_volume; // = 256;

    int using_stdin; // = 0;
    int using_vhook; // = 0;
    int verbose; // = 1;
    int thread_count; //= 1;
    int q_pressed; // = 0;
    int64_t video_size; // = 0;
    int64_t audio_size; // = 0;
    int64_t extra_size; // = 0;
    int nb_frames_dup; // = 0;
    int nb_frames_drop; // = 0;
    int input_sync;
    uint64_t limit_filesize; // = 0; //

    int pgmyuv_compatibility_hack; //=0;
    int dts_delta_threshold; // = 10;

    int sws_flags; // = SWS_BICUBIC;

    const char **opt_names;
    int opt_name_count;
    AVCodecContext *avctx_opts[CODEC_TYPE_NB];
    AVFormatContext *avformat_opts;
    struct SwsContext *sws_opts;
    int64_t timer_start;

    AVBitStreamFilterContext *video_bitstream_filters; //=NULL;
    AVBitStreamFilterContext *audio_bitstream_filters; //=NULL;
    AVBitStreamFilterContext *bitstream_filters[MAX_FILES][MAX_STREAMS];    


    void new_video_stream(AVFormatContext *oc);
    void new_audio_stream(AVFormatContext *oc);

    int read_ffserver_streams(AVFormatContext *s, const char *filename);
    void check_audio_video_inputs(int *has_video_ptr, int *has_audio_ptr);


    int bit_buffer_size; //= 1024*256;
    uint8_t *bit_buffer; //= NULL;

    int opt_counter;

};

#endif // _FUPPES_FFMPEG_H
