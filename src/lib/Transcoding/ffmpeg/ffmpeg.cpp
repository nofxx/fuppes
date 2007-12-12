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

#include "ffmpeg.h"

#include <errno.h>

#include <unistd.h>

#if !defined(HAVE_GETRUSAGE) && defined(HAVE_GETPROCESSTIMES)
#include <windows.h>
#endif

#if defined(HAVE_TERMIOS_H)
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <termios.h>
#include <sys/resource.h>
#elif defined(HAVE_CONIO_H)
#include <conio.h>
#endif
#undef time //needed because HAVE_AV_CONFIG_H is defined on top
#include <time.h>

#ifdef WIN32
//#ifdef __MINGW32__
//__declspec(dllimport) void __stdcall Sleep(unsigned long dwMilliseconds);
#  include <windows.h>
#  define usleep(t)    Sleep((t) / 1000)
#  include <fcntl.h>
#  define lseek(f,p,w) _lseeki64((f), (p), (w))
//#endif
#endif

#include "cmdutils.h"

#undef NDEBUG
#include <assert.h>

#if !defined(INFINITY) && defined(HUGE_VAL)
#define INFINITY HUGE_VAL
#endif

#include <iostream>
using namespace std;

//#undef exit

//extern const OptionDef options[];
//static int opt_default(const char *opt, const char *arg);

#define DEFAULT_PASS_LOGFILENAME "ffmpeg2pass"

#ifndef HAVE_AVSTRING_H
void pstrcpy(char *buf, int buf_size, const char *str)
{
    int c;
    char *q = buf;

    if (buf_size <= 0)
        return;

    for(;;) {
        c = *str++;
        if (c == 0 || q >= buf + buf_size - 1)
            break;
        *q++ = c;
    }
    *q = '\0';
}

int strstart(const char *str, const char *val, const char **ptr)
{
    const char *p, *q;
    p = str;
    q = val;
    while (*q != '\0') {
        if (*p != *q)
            return 0;
        p++;
        q++;
    }
    if (ptr)
        *ptr = p;
    return 1;
}
#endif // HAVE_AVSTRING_H  

char *_av_strdup(const char *s)
{
  char *ptr;
  int len;
  len = strlen(s) + 1;
  ptr = (char*)av_malloc(len);
  if (ptr)
      memcpy(ptr, s, len);
  return ptr;
}



int CFFmpeg::read_ffserver_streams(AVFormatContext *s, const char *filename)
{
    int i, err;
    AVFormatContext *ic;

    err = av_open_input_file(&ic, filename, NULL, FFM_PACKET_SIZE, NULL);
    if (err < 0)
        return err;
    /* copy stream format */
    s->nb_streams = ic->nb_streams;
    for(i=0;i<ic->nb_streams;i++) {
        AVStream *st;

        // FIXME: a more elegant solution is needed
        st = (AVStream*)av_mallocz(sizeof(AVStream));
        memcpy(st, ic->streams[i], sizeof(AVStream));
        st->codec = avcodec_alloc_context();
        memcpy(st->codec, ic->streams[i]->codec, sizeof(AVCodecContext));
        s->streams[i] = st;
    }

    av_close_input_file(ic);
    return 0;
}

double
get_sync_ipts(const AVOutputStream *ost, CFFmpeg* pFFmpeg)
{
    const AVInputStream *ist = ost->sync_ist;
    return (double)(ist->pts + pFFmpeg->input_files_ts_offset[ist->file_index] - pFFmpeg->start_time)/AV_TIME_BASE;
}

void write_frame(AVFormatContext *s, AVPacket *pkt, AVCodecContext *avctx, AVBitStreamFilterContext *bsfc){
    while(bsfc){
        AVPacket new_pkt= *pkt;
        int a= av_bitstream_filter_filter(bsfc, avctx, NULL,
                                          &new_pkt.data, &new_pkt.size,
                                          pkt->data, pkt->size,
                                          pkt->flags & PKT_FLAG_KEY);
        if(a){
            av_free_packet(pkt);
            new_pkt.destruct= av_destruct_packet;
        }
        *pkt= new_pkt;

        bsfc= bsfc->next;
    }

    av_interleaved_write_frame(s, pkt);
}

#define MAX_AUDIO_PACKET_SIZE (128 * 1024)

void do_audio_out(AVFormatContext *s,
                         AVOutputStream *ost,
                         AVInputStream *ist,
                         unsigned char *buf, int size, CFFmpeg* pFFmpeg)
{
    uint8_t *buftmp;
    static uint8_t *audio_buf = NULL;
    static uint8_t *audio_out = NULL;
    const int audio_out_size= 4*MAX_AUDIO_PACKET_SIZE;

    int size_out, frame_bytes, ret;
    AVCodecContext *enc= ost->st->codec;

    /* SC: dynamic allocation of buffers */
    if (!audio_buf)
        audio_buf = (uint8_t*)av_malloc(2*MAX_AUDIO_PACKET_SIZE);
    if (!audio_out)
        audio_out = (uint8_t*)av_malloc(audio_out_size);
    if (!audio_buf || !audio_out)
        return;               /* Should signal an error ! */

    if(pFFmpeg->audio_sync_method){
        double delta = get_sync_ipts(ost, pFFmpeg) * enc->sample_rate - ost->sync_opts
                - av_fifo_size(&ost->fifo)/(ost->st->codec->channels * 2);
        double idelta= delta*ist->st->codec->sample_rate / enc->sample_rate;
        int byte_delta= ((int)idelta)*2*ist->st->codec->channels;

        //FIXME resample delay
        if(fabs(delta) > 50){
            if(ist->is_start){
                if(byte_delta < 0){
                    byte_delta= FFMAX(byte_delta, -size);
                    size += byte_delta;
                    buf  -= byte_delta;
                    if(pFFmpeg->verbose > 2)
                        fprintf(stderr, "discarding %d audio samples\n", (int)-delta);
                    if(!size)
                        return;
                    ist->is_start=0;
                }else{
                    static uint8_t *input_tmp= NULL;
                    input_tmp= (uint8_t*)av_realloc(input_tmp, byte_delta + size);

                    if(byte_delta + size <= MAX_AUDIO_PACKET_SIZE)
                        ist->is_start=0;
                    else
                        byte_delta= MAX_AUDIO_PACKET_SIZE - size;

                    memset(input_tmp, 0, byte_delta);
                    memcpy(input_tmp + byte_delta, buf, size);
                    buf= input_tmp;
                    size += byte_delta;
                    if(pFFmpeg->verbose > 2)
                        fprintf(stderr, "adding %d audio samples of silence\n", (int)delta);
                }
            }else if(pFFmpeg->audio_sync_method>1){
                int comp= av_clip(delta, -pFFmpeg->audio_sync_method, pFFmpeg->audio_sync_method);
                assert(ost->audio_resample);
                if(pFFmpeg->verbose > 2)
                    fprintf(stderr, "compensating audio timestamp drift:%f compensation:%d in:%d\n", delta, comp, enc->sample_rate);
//                fprintf(stderr, "drift:%f len:%d opts:%"PRId64" ipts:%"PRId64" fifo:%d\n", delta, -1, ost->sync_opts, (int64_t)(get_sync_ipts(ost) * enc->sample_rate), av_fifo_size(&ost->fifo)/(ost->st->codec->channels * 2));
                av_resample_compensate(*(struct AVResampleContext**)ost->resample, comp, enc->sample_rate);
            }
        }
    }else
        ost->sync_opts= lrintf(get_sync_ipts(ost, pFFmpeg) * enc->sample_rate)
                        - av_fifo_size(&ost->fifo)/(ost->st->codec->channels * 2); //FIXME wrong

    if (ost->audio_resample) {
        buftmp = audio_buf;
        size_out = audio_resample(ost->resample,
                                  (short *)buftmp, (short *)buf,
                                  size / (ist->st->codec->channels * 2));
        size_out = size_out * enc->channels * 2;
    } else {
        buftmp = buf;
        size_out = size;
    }

    /* now encode as many frames as possible */
    if (enc->frame_size > 1) {
        /* output resampled raw samples */
        av_fifo_write(&ost->fifo, buftmp, size_out);

        frame_bytes = enc->frame_size * 2 * enc->channels;

        while (av_fifo_read(&ost->fifo, audio_buf, frame_bytes) == 0) {
            AVPacket pkt;
            av_init_packet(&pkt);

            ret = avcodec_encode_audio(enc, audio_out, audio_out_size,
                                       (short *)audio_buf);
            pFFmpeg->audio_size += ret;
            pkt.stream_index= ost->index;
            pkt.data= audio_out;
            pkt.size= ret;
            if(enc->coded_frame && enc->coded_frame->pts != AV_NOPTS_VALUE)
                pkt.pts= av_rescale_q(enc->coded_frame->pts, enc->time_base, ost->st->time_base);
            pkt.flags |= PKT_FLAG_KEY;
            write_frame(s, &pkt, ost->st->codec, pFFmpeg->bitstream_filters[ost->file_index][pkt.stream_index]);

            ost->sync_opts += enc->frame_size;
        }
    } else {
        AVPacket pkt;
        av_init_packet(&pkt);

        ost->sync_opts += size_out / (2 * enc->channels);

        /* output a pcm frame */
        /* XXX: change encoding codec API to avoid this ? */
        switch(enc->codec->id) {
        case CODEC_ID_PCM_S32LE:
        case CODEC_ID_PCM_S32BE:
        case CODEC_ID_PCM_U32LE:
        case CODEC_ID_PCM_U32BE:
            size_out = size_out << 1;
            break;
        case CODEC_ID_PCM_S24LE:
        case CODEC_ID_PCM_S24BE:
        case CODEC_ID_PCM_U24LE:
        case CODEC_ID_PCM_U24BE:
        case CODEC_ID_PCM_S24DAUD:
            size_out = size_out / 2 * 3;
            break;
        case CODEC_ID_PCM_S16LE:
        case CODEC_ID_PCM_S16BE:
        case CODEC_ID_PCM_U16LE:
        case CODEC_ID_PCM_U16BE:
            break;
        default:
            size_out = size_out >> 1;
            break;
        }
        ret = avcodec_encode_audio(enc, audio_out, size_out,
                                   (short *)buftmp);
        pFFmpeg->audio_size += ret;
        pkt.stream_index= ost->index;
        pkt.data= audio_out;
        pkt.size= ret;
        if(enc->coded_frame && enc->coded_frame->pts != AV_NOPTS_VALUE)
            pkt.pts= av_rescale_q(enc->coded_frame->pts, enc->time_base, ost->st->time_base);
        pkt.flags |= PKT_FLAG_KEY;
        write_frame(s, &pkt, ost->st->codec, pFFmpeg->bitstream_filters[ost->file_index][pkt.stream_index]);
    }
}

void pre_process_video_frame(AVInputStream *ist, AVPicture *picture, void **bufp, CFFmpeg* pFFmpeg)
{
    AVCodecContext *dec;
    AVPicture *picture2;
    AVPicture picture_tmp;
    uint8_t *buf = 0;

    dec = ist->st->codec;

    /* deinterlace : must be done before any resize */
    if (pFFmpeg->do_deinterlace || pFFmpeg->using_vhook) {
        int size;

        /* create temporary picture */
        size = avpicture_get_size(dec->pix_fmt, dec->width, dec->height);
        buf = (uint8_t*)av_malloc(size);
        if (!buf)
            return;

        picture2 = &picture_tmp;
        avpicture_fill(picture2, buf, dec->pix_fmt, dec->width, dec->height);

        if (pFFmpeg->do_deinterlace){
            if(avpicture_deinterlace(picture2, picture,
                                     dec->pix_fmt, dec->width, dec->height) < 0) {
                /* if error, do not deinterlace */
                av_free(buf);
                buf = NULL;
                picture2 = picture;
            }
        } else {
            av_picture_copy(picture2, picture, dec->pix_fmt, dec->width, dec->height);
        }
    } else {
        picture2 = picture;
    }

    /* 2007-07-19 - u-voelkel :: fuppes mod
    if (ENABLE_VHOOK)
        frame_hook_process(picture2, dec->pix_fmt, dec->width, dec->height,
                           1000000 * ist->pts / AV_TIME_BASE); */

    if (picture != picture2)
        *picture = *picture2;
    *bufp = buf;
}

/* we begin to correct av delay at this threshold */
#define AV_DELAY_MAX 0.100

void do_subtitle_out(AVFormatContext *s,
                            AVOutputStream *ost,
                            AVInputStream *ist,
                            AVSubtitle *sub,
                            int64_t pts, CFFmpeg* pFFmpeg)
{
    static uint8_t *subtitle_out = NULL;
    int subtitle_out_max_size = 65536;
    int subtitle_out_size, nb, i;
    AVCodecContext *enc;
    AVPacket pkt;

    if (pts == AV_NOPTS_VALUE) {
        fprintf(stderr, "Subtitle packets must have a pts\n");
        return;
    }

    enc = ost->st->codec;

    if (!subtitle_out) {
        subtitle_out = (uint8_t*)av_malloc(subtitle_out_max_size);
    }

    /* Note: DVB subtitle need one packet to draw them and one other
       packet to clear them */
    /* XXX: signal it in the codec context ? */
    if (enc->codec_id == CODEC_ID_DVB_SUBTITLE)
        nb = 2;
    else
        nb = 1;

    for(i = 0; i < nb; i++) {
        subtitle_out_size = avcodec_encode_subtitle(enc, subtitle_out,
                                                    subtitle_out_max_size, sub);

        av_init_packet(&pkt);
        pkt.stream_index = ost->index;
        pkt.data = subtitle_out;
        pkt.size = subtitle_out_size;
        pkt.pts = av_rescale_q(av_rescale_q(pts, ist->st->time_base, AV_TIME_BASE_Q) + pFFmpeg->input_files_ts_offset[ist->file_index], AV_TIME_BASE_Q,  ost->st->time_base);
        if (enc->codec_id == CODEC_ID_DVB_SUBTITLE) {
            /* XXX: the pts correction is handled here. Maybe handling
               it in the codec would be better */
            if (i == 0)
                pkt.pts += 90 * sub->start_display_time;
            else
                pkt.pts += 90 * sub->end_display_time;
        }
        write_frame(s, &pkt, ost->st->codec, pFFmpeg->bitstream_filters[ost->file_index][pkt.stream_index]);
    }
}



void do_video_out(AVFormatContext *s,
                         AVOutputStream *ost,
                         AVInputStream *ist,
                         AVFrame *in_picture,
                         int *frame_size, CFFmpeg* pFFmpeg)
{
    int nb_frames, i, ret;
    AVFrame *final_picture, *formatted_picture, *resampling_dst, *padding_src;
    AVFrame picture_crop_temp, picture_pad_temp;
    AVCodecContext *enc, *dec;

    avcodec_get_frame_defaults(&picture_crop_temp);
    avcodec_get_frame_defaults(&picture_pad_temp);

    enc = ost->st->codec;
    dec = ist->st->codec;

    /* by default, we output a single frame */
    nb_frames = 1;

    *frame_size = 0;

    if(pFFmpeg->video_sync_method){
        double vdelta;
        vdelta = get_sync_ipts(ost, pFFmpeg) / av_q2d(enc->time_base) - ost->sync_opts;
        //FIXME set to 0.5 after we fix some dts/pts bugs like in avidec.c
        if (vdelta < -1.1)
            nb_frames = 0;
        else if (vdelta > 1.1)
            nb_frames = lrintf(vdelta);
//fprintf(stderr, "vdelta:%f, ost->sync_opts:%"PRId64", ost->sync_ipts:%f nb_frames:%d\n", vdelta, ost->sync_opts, ost->sync_ipts, nb_frames);
        if (nb_frames == 0){
            pFFmpeg->nb_frames_drop++;
            if (pFFmpeg->verbose>2)
                fprintf(stderr, "*** drop!\n");
        }else if (nb_frames > 1) {
            pFFmpeg->nb_frames_dup += nb_frames;
            if (pFFmpeg->verbose>2)
                fprintf(stderr, "*** %d dup!\n", nb_frames-1);
        }
    }else
        ost->sync_opts= lrintf(get_sync_ipts(ost, pFFmpeg) / av_q2d(enc->time_base));

    nb_frames= FFMIN(nb_frames, pFFmpeg->max_frames[CODEC_TYPE_VIDEO] - ost->frame_number);
    if (nb_frames <= 0)
        return;

    if (ost->video_crop) {
        if (av_picture_crop((AVPicture *)&picture_crop_temp, (AVPicture *)in_picture, dec->pix_fmt, ost->topBand, ost->leftBand) < 0) {
            av_log(NULL, AV_LOG_ERROR, "error cropping picture\n");
            return;
        }
        formatted_picture = &picture_crop_temp;
    } else {
        formatted_picture = in_picture;
    }

    final_picture = formatted_picture;
    padding_src = formatted_picture;
    resampling_dst = &ost->pict_tmp;
    if (ost->video_pad) {
        final_picture = &ost->pict_tmp;
        if (ost->video_resample) {
            if (av_picture_crop((AVPicture *)&picture_pad_temp, (AVPicture *)final_picture, enc->pix_fmt, ost->padtop, ost->padleft) < 0) {
                av_log(NULL, AV_LOG_ERROR, "error padding picture\n");
                return;
            }
            resampling_dst = &picture_pad_temp;
        }
    }

    if (ost->video_resample) {
        padding_src = NULL;
        final_picture = &ost->pict_tmp;
#ifdef HAVE_LIBSWSCALE
        sws_scale(ost->img_resample_ctx, formatted_picture->data, formatted_picture->linesize,
              0, ost->resample_height, resampling_dst->data, resampling_dst->linesize);
#endif
    }

    if (ost->video_pad) {
        av_picture_pad((AVPicture*)final_picture, (AVPicture *)padding_src,
                enc->height, enc->width, enc->pix_fmt,
                ost->padtop, ost->padbottom, ost->padleft, ost->padright, pFFmpeg->padcolor);
    }

    /* duplicates frame if needed */
    for(i=0;i<nb_frames;i++) {
        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.stream_index= ost->index;

        if (s->oformat->flags & AVFMT_RAWPICTURE) {
            /* raw pictures are written as AVPicture structure to
               avoid any copies. We support temorarily the older
               method. */
            AVFrame* old_frame = enc->coded_frame;
            enc->coded_frame = dec->coded_frame; //FIXME/XXX remove this hack
            pkt.data= (uint8_t *)final_picture;
            pkt.size=  sizeof(AVPicture);
            if(dec->coded_frame && enc->coded_frame->pts != AV_NOPTS_VALUE)
                pkt.pts= av_rescale_q(enc->coded_frame->pts, enc->time_base, ost->st->time_base);
            if(dec->coded_frame && dec->coded_frame->key_frame)
                pkt.flags |= PKT_FLAG_KEY;

            write_frame(s, &pkt, ost->st->codec, pFFmpeg->bitstream_filters[ost->file_index][pkt.stream_index]);
            enc->coded_frame = old_frame;
        } else {
            AVFrame big_picture;

            big_picture= *final_picture;
            /* better than nothing: use input picture interlaced
               settings */
            big_picture.interlaced_frame = in_picture->interlaced_frame;
            if(pFFmpeg->avctx_opts[CODEC_TYPE_VIDEO]->flags & (CODEC_FLAG_INTERLACED_DCT|CODEC_FLAG_INTERLACED_ME)){
                if(pFFmpeg->top_field_first == -1)
                    big_picture.top_field_first = in_picture->top_field_first;
                else
                    big_picture.top_field_first = pFFmpeg->top_field_first;
            }

            /* handles sameq here. This is not correct because it may
               not be a global option */
            if (pFFmpeg->same_quality) {
                big_picture.quality = ist->st->quality;
            }else
                big_picture.quality = ost->st->quality;
            if(!pFFmpeg->me_threshold)
                big_picture.pict_type = 0;
//            big_picture.pts = AV_NOPTS_VALUE;
            big_picture.pts= ost->sync_opts;
//            big_picture.pts= av_rescale(ost->sync_opts, AV_TIME_BASE*(int64_t)enc->time_base.num, enc->time_base.den);
//av_log(NULL, AV_LOG_DEBUG, "%"PRId64" -> encoder\n", ost->sync_opts);
            ret = avcodec_encode_video(enc,
                                       pFFmpeg->bit_buffer, pFFmpeg->bit_buffer_size,
                                       &big_picture);
            if (ret == -1) {
                fprintf(stderr, "Video encoding failed\n");
                return;
            }
            //enc->frame_number = enc->real_pict_num;
            if(ret>0){
                pkt.data= pFFmpeg->bit_buffer;
                pkt.size= ret;
                if(enc->coded_frame && enc->coded_frame->pts != AV_NOPTS_VALUE)
                    pkt.pts= av_rescale_q(enc->coded_frame->pts, enc->time_base, ost->st->time_base);
/*av_log(NULL, AV_LOG_DEBUG, "encoder -> %"PRId64"/%"PRId64"\n",
   pkt.pts != AV_NOPTS_VALUE ? av_rescale(pkt.pts, enc->time_base.den, AV_TIME_BASE*(int64_t)enc->time_base.num) : -1,
   pkt.dts != AV_NOPTS_VALUE ? av_rescale(pkt.dts, enc->time_base.den, AV_TIME_BASE*(int64_t)enc->time_base.num) : -1);*/

                if(enc->coded_frame && enc->coded_frame->key_frame)
                    pkt.flags |= PKT_FLAG_KEY;
                write_frame(s, &pkt, ost->st->codec, pFFmpeg->bitstream_filters[ost->file_index][pkt.stream_index]);
                *frame_size = ret;
                //fprintf(stderr,"\nFrame: %3d %3d size: %5d type: %d",
                //        enc->frame_number-1, enc->real_pict_num, ret,
                //        enc->pict_type);
                /* if two pass, output log */
                if (ost->logfile && enc->stats_out) {
                    fprintf(ost->logfile, "%s", enc->stats_out);
                }
            }
        }
        ost->sync_opts++;
        ost->frame_number++;
    }
}

double psnr(double d){
    if(d==0) return INFINITY;
    return -10.0*log(d)/log(10.0);
}

void do_video_stats(AVFormatContext *os, AVOutputStream *ost,
                           int frame_size, CFFmpeg* pFFmpeg)
{
    AVCodecContext *enc;
    int frame_number;
    double ti1, bitrate, avg_bitrate;

    /* this is executed just the first time do_video_stats is called */
    /*if (!pFFmpeg->fvstats) {
        pFFmpeg->fvstats = fopen(pFFmpeg->vstats_filename, "w");
        if (!pFFmpeg->fvstats) {
            perror("fopen");
            exit(1);
        }
    }*/

    enc = ost->st->codec;
    if (enc->codec_type == CODEC_TYPE_VIDEO) {
        frame_number = ost->frame_number;
        /*fprintf(pFFmpeg->fvstats, "frame= %5d q= %2.1f ", frame_number, enc->coded_frame->quality/(float)FF_QP2LAMBDA);
        if (enc->flags&CODEC_FLAG_PSNR)
            fprintf(pFFmpeg->fvstats, "PSNR= %6.2f ", psnr(enc->coded_frame->error[0]/(enc->width*enc->height*255.0*255.0)));

        fprintf(pFFmpeg->fvstats,"f_size= %6d ", frame_size);*/
        /* compute pts value */
        ti1 = ost->sync_opts * av_q2d(enc->time_base);
        if (ti1 < 0.01)
            ti1 = 0.01;

        bitrate = (frame_size * 8) / av_q2d(enc->time_base) / 1000.0;
        avg_bitrate = (double)(pFFmpeg->video_size * 8) / ti1 / 1000.0;
        /*fprintf(pFFmpeg->fvstats, "s_size= %8.0fkB time= %0.3f br= %7.1fkbits/s avg_br= %7.1fkbits/s ",
            (double)pFFmpeg->video_size / 1024, ti1, bitrate, avg_bitrate);
        fprintf(pFFmpeg->fvstats,"type= %c\n", av_get_pict_type_char(enc->coded_frame->pict_type));*/
    }
}

void print_report(AVFormatContext **output_files,
                         AVOutputStream **ost_table, int nb_ostreams,
                         int is_last_report, CFFmpeg* pFFmpeg)
{
    char buf[1024];
    AVOutputStream *ost;
    AVFormatContext *oc, *os;
    int64_t total_size;
    AVCodecContext *enc;
    int frame_number, vid, i;
    double bitrate, ti1, pts;
    static int64_t last_time = -1;
    static int qp_histogram[52];

    if (!is_last_report) {
        int64_t cur_time;
        /* display the report every 0.5 seconds */
        cur_time = av_gettime();
        if (last_time == -1) {
            last_time = cur_time;
            return;
        }
        if ((cur_time - last_time) < 500000)
            return;
        last_time = cur_time;
    }


    oc = output_files[0];

														 
		#if (FFMPEG_VERSION >= 52)
		total_size = url_fsize(oc->pb);
		#else
    total_size = url_fsize(&oc->pb);
		#endif
    if(total_size<0) // FIXME improve url_fsize() so it works with non seekable output too
			#if (FFMPEG_VERSION >= 52)
			total_size = url_ftell(oc->pb);
			#else
			total_size = url_ftell(&oc->pb);
			#endif

    buf[0] = '\0';
    ti1 = 1e10;
    vid = 0;
    for(i=0;i<nb_ostreams;i++) {
        ost = ost_table[i];
        os = output_files[ost->file_index];
        enc = ost->st->codec;
        if (vid && enc->codec_type == CODEC_TYPE_VIDEO) {
            snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "q=%2.1f ",
                    enc->coded_frame->quality/(float)FF_QP2LAMBDA);
        }
        if (!vid && enc->codec_type == CODEC_TYPE_VIDEO) {
            float t = (av_gettime()-pFFmpeg->timer_start) / 1000000.0;

            frame_number = ost->frame_number;
            snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "frame=%5d fps=%3d q=%3.1f ",
                     frame_number, (t>1)?(int)(frame_number/t+0.5) : 0,
                     enc->coded_frame ? enc->coded_frame->quality/(float)FF_QP2LAMBDA : -1);
            if(is_last_report)
                snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "L");
            if(pFFmpeg->qp_hist && enc->coded_frame){
                int j;
                int qp= lrintf(enc->coded_frame->quality/(float)FF_QP2LAMBDA);
                if(qp>=0 && qp<sizeof(qp_histogram)/sizeof(int))
                    qp_histogram[qp]++;
                for(j=0; j<32; j++)
                    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%X", (int)lrintf(log(qp_histogram[j]+1)/log(2)));
            }
            if (enc->flags&CODEC_FLAG_PSNR){
                int j;
                double error, error_sum=0;
                double scale, scale_sum=0;
                char type[3]= {'Y','U','V'};
                snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "PSNR=");
                for(j=0; j<3; j++){
                    if(is_last_report){
                        error= enc->error[j];
                        scale= enc->width*enc->height*255.0*255.0*frame_number;
                    }else{
                        error= enc->coded_frame->error[j];
                        scale= enc->width*enc->height*255.0*255.0;
                    }
                    if(j) scale/=4;
                    error_sum += error;
                    scale_sum += scale;
                    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%c:%2.2f ", type[j], psnr(error/scale));
                }
                snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "*:%2.2f ", psnr(error_sum/scale_sum));
            }
            vid = 1;
        }
        /* compute min output value */
        pts = (double)ost->st->pts.val * av_q2d(ost->st->time_base);
        if ((pts < ti1) && (pts > 0))
            ti1 = pts;
    }
    if (ti1 < 0.01)
        ti1 = 0.01;

    if (pFFmpeg->verbose || is_last_report) {
        bitrate = (double)(total_size * 8) / ti1 / 1000.0;

        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
            "size=%8.0fkB time=%0.1f bitrate=%6.1fkbits/s",
            (double)total_size / 1024, ti1, bitrate);

        if (pFFmpeg->verbose > 1)
          snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), " dup=%d drop=%d",
                  pFFmpeg->nb_frames_dup, pFFmpeg->nb_frames_drop);

        if (pFFmpeg->verbose >= 0)
            fprintf(stderr, "%s    \r", buf);

        fflush(stderr);
    }

    if (is_last_report && pFFmpeg->verbose >= 0){
        int64_t raw= pFFmpeg->audio_size + pFFmpeg->video_size + pFFmpeg->extra_size;
        fprintf(stderr, "\n");
        fprintf(stderr, "video:%1.0fkB audio:%1.0fkB global headers:%1.0fkB muxing overhead %f%%\n",
                pFFmpeg->video_size/1024.0,
                pFFmpeg->audio_size/1024.0,
                pFFmpeg->extra_size/1024.0,
                100.0*(total_size - raw)/raw
        );
    }
}

/* pkt = NULL means EOF (needed to flush decoder buffers) */
int output_packet(AVInputStream *ist, int ist_index,
                         AVOutputStream **ost_table, int nb_ostreams,
                         const AVPacket *pkt, CFFmpeg* pFFmpeg)
{
    AVFormatContext *os;
    AVOutputStream *ost;
    uint8_t *ptr;
    int len, ret, i;
    uint8_t *data_buf;
    int data_size, got_picture;
    AVFrame picture;
    void *buffer_to_free;
    static unsigned int samples_size= 0;
    static short *samples= NULL;
    AVSubtitle subtitle, *subtitle_to_free;
    int got_subtitle;

    if(!pkt){
        ist->pts= ist->next_pts; // needed for last packet if vsync=0
    } else if (pkt->dts != AV_NOPTS_VALUE) { //FIXME seems redundant, as libavformat does this too
        ist->next_pts = ist->pts = av_rescale_q(pkt->dts, ist->st->time_base, AV_TIME_BASE_Q);
    } else {
//        assert(ist->pts == ist->next_pts);
    }

    if (pkt == NULL) {
        /* EOF handling */
        ptr = NULL;
        len = 0;
        goto handle_eof;
    }

    len = pkt->size;
    ptr = pkt->data;
    while (len > 0) {
    handle_eof:
        /* decode the packet if needed */
        data_buf = NULL; /* fail safe */
        data_size = 0;
        subtitle_to_free = NULL;
        if (ist->decoding_needed) {
            switch(ist->st->codec->codec_type) {
            case CODEC_TYPE_AUDIO:{
                if(pkt)
                    samples= (short*)av_fast_realloc(samples, &samples_size, FFMAX(pkt->size*sizeof(*samples), AVCODEC_MAX_AUDIO_FRAME_SIZE));
                data_size= samples_size;
                    /* XXX: could avoid copy if PCM 16 bits with same
                       endianness as CPU */
                ret = avcodec_decode_audio2(ist->st->codec, samples, &data_size,
                                           ptr, len);
                if (ret < 0)
                    goto fail_decode;
                ptr += ret;
                len -= ret;
                /* Some bug in mpeg audio decoder gives */
                /* data_size < 0, it seems they are overflows */
                if (data_size <= 0) {
                    /* no audio frame */
                    continue;
                }
                data_buf = (uint8_t *)samples;
                ist->next_pts += ((int64_t)AV_TIME_BASE/2 * data_size) /
                    (ist->st->codec->sample_rate * ist->st->codec->channels);
                break;}
            case CODEC_TYPE_VIDEO:
                    data_size = (ist->st->codec->width * ist->st->codec->height * 3) / 2;
                    /* XXX: allocate picture correctly */
                    avcodec_get_frame_defaults(&picture);

                    ret = avcodec_decode_video(ist->st->codec,
                                               &picture, &got_picture, ptr, len);
                    ist->st->quality= picture.quality;
                    if (ret < 0)
                        goto fail_decode;
                    if (!got_picture) {
                        /* no picture yet */
                        goto discard_packet;
                    }
                    if (ist->st->codec->time_base.num != 0) {
                        ist->next_pts += ((int64_t)AV_TIME_BASE *
                                          ist->st->codec->time_base.num) /
                            ist->st->codec->time_base.den;
                    }
                    len = 0;
                    break;
            case CODEC_TYPE_SUBTITLE:
                ret = avcodec_decode_subtitle(ist->st->codec,
                                              &subtitle, &got_subtitle, ptr, len);
                if (ret < 0)
                    goto fail_decode;
                if (!got_subtitle) {
                    goto discard_packet;
                }
                subtitle_to_free = &subtitle;
                len = 0;
                break;
            default:
                goto fail_decode;
            }
        } else {
            switch(ist->st->codec->codec_type) {
            case CODEC_TYPE_AUDIO:
                ist->next_pts += ((int64_t)AV_TIME_BASE * ist->st->codec->frame_size) /
                    (ist->st->codec->sample_rate * ist->st->codec->channels);
                break;
            case CODEC_TYPE_VIDEO:
                if (ist->st->codec->time_base.num != 0) {
                    ist->next_pts += ((int64_t)AV_TIME_BASE *
                                      ist->st->codec->time_base.num) /
                        ist->st->codec->time_base.den;
                }
                break;
            }
            data_buf = ptr;
            data_size = len;
            ret = len;
            len = 0;
        }

        buffer_to_free = NULL;
        if (ist->st->codec->codec_type == CODEC_TYPE_VIDEO) {
            pre_process_video_frame(ist, (AVPicture *)&picture,
                                    &buffer_to_free, pFFmpeg);
        }

        // preprocess audio (volume)
        if (ist->st->codec->codec_type == CODEC_TYPE_AUDIO) {
            if (pFFmpeg->audio_volume != 256) {
                short *volp;
                volp = samples;
                for(i=0;i<(data_size / sizeof(short));i++) {
                    int v = ((*volp) * pFFmpeg->audio_volume + 128) >> 8;
                    if (v < -32768) v = -32768;
                    if (v >  32767) v = 32767;
                    *volp++ = v;
                }
            }
        }

        /* frame rate emulation */
        if (ist->st->codec->rate_emu) {
            int64_t pts = av_rescale((int64_t) ist->frame * ist->st->codec->time_base.num, 1000000, ist->st->codec->time_base.den);
            int64_t now = av_gettime() - ist->start;
            if (pts > now)			
                usleep(pts - now);

            ist->frame++;
        }

#if 0
        /* mpeg PTS deordering : if it is a P or I frame, the PTS
           is the one of the next displayed one */
        /* XXX: add mpeg4 too ? */
        if (ist->st->codec->codec_id == CODEC_ID_MPEG1VIDEO) {
            if (ist->st->codec->pict_type != B_TYPE) {
                int64_t tmp;
                tmp = ist->last_ip_pts;
                ist->last_ip_pts  = ist->frac_pts.val;
                ist->frac_pts.val = tmp;
            }
        }
#endif
        /* if output time reached then transcode raw format,
           encode packets and output them */
        if (pFFmpeg->start_time == 0 || ist->pts >= pFFmpeg->start_time)
            for(i=0;i<nb_ostreams;i++) {
                int frame_size;

                ost = ost_table[i];
                if (ost->source_index == ist_index) {
                    os = pFFmpeg->output_files[ost->file_index];

#if 0
                    printf("%d: got pts=%0.3f %0.3f\n", i,
                           (double)pkt->pts / AV_TIME_BASE,
                           ((double)ist->pts / AV_TIME_BASE) -
                           ((double)ost->st->pts.val * ost->st->time_base.num / ost->st->time_base.den));
#endif
                    /* set the input output pts pairs */
                    //ost->sync_ipts = (double)(ist->pts + input_files_ts_offset[ist->file_index] - start_time)/ AV_TIME_BASE;

                    if (ost->encoding_needed) {
                        switch(ost->st->codec->codec_type) {
                        case CODEC_TYPE_AUDIO:
                            do_audio_out(os, ost, ist, data_buf, data_size, pFFmpeg);
                            break;
                        case CODEC_TYPE_VIDEO:
                            do_video_out(os, ost, ist, &picture, &frame_size, pFFmpeg);
                            pFFmpeg->video_size += frame_size;
                            if (pFFmpeg->vstats_filename && frame_size)
                                do_video_stats(os, ost, frame_size, pFFmpeg);
                            break;
                        case CODEC_TYPE_SUBTITLE:
                            do_subtitle_out(os, ost, ist, &subtitle,
                                            pkt->pts, pFFmpeg);
                            break;
                        default:
                            return -1;//abort();
                        }
                    } else {
                        AVFrame avframe; //FIXME/XXX remove this
                        AVPacket opkt;
                        av_init_packet(&opkt);

                        /* no reencoding needed : output the packet directly */
                        /* force the input stream PTS */

                        avcodec_get_frame_defaults(&avframe);
                        ost->st->codec->coded_frame= &avframe;
                        avframe.key_frame = pkt->flags & PKT_FLAG_KEY;

                        if(ost->st->codec->codec_type == CODEC_TYPE_AUDIO)
                            pFFmpeg->audio_size += data_size;
                        else if (ost->st->codec->codec_type == CODEC_TYPE_VIDEO) {
                            pFFmpeg->video_size += data_size;
                            ost->sync_opts++;
                        }

                        opkt.stream_index= ost->index;
                        if(pkt->pts != AV_NOPTS_VALUE)
                            opkt.pts= av_rescale_q(av_rescale_q(pkt->pts, ist->st->time_base, AV_TIME_BASE_Q) + pFFmpeg->input_files_ts_offset[ist->file_index], AV_TIME_BASE_Q,  ost->st->time_base);
                        else
                            opkt.pts= AV_NOPTS_VALUE;

                        {
                            int64_t dts;
                            if (pkt->dts == AV_NOPTS_VALUE)
                                dts = ist->next_pts;
                            else
                                dts= av_rescale_q(pkt->dts, ist->st->time_base, AV_TIME_BASE_Q);
                            opkt.dts= av_rescale_q(dts + pFFmpeg->input_files_ts_offset[ist->file_index], AV_TIME_BASE_Q,  ost->st->time_base);
                        }
                        opkt.flags= pkt->flags;

                        //FIXME remove the following 2 lines they shall be replaced by the bitstream filters
                        if(av_parser_change(ist->st->parser, ost->st->codec, &opkt.data, &opkt.size, data_buf, data_size, pkt->flags & PKT_FLAG_KEY))
                            opkt.destruct= av_destruct_packet;

                        write_frame(os, &opkt, ost->st->codec, pFFmpeg->bitstream_filters[ost->file_index][pkt->stream_index]);
                        ost->st->codec->frame_number++;
                        ost->frame_number++;
                        av_free_packet(&opkt);
                    }
                }
            }
        av_free(buffer_to_free);
        /* XXX: allocate the subtitles in the codec ? */
        if (subtitle_to_free) {
            if (subtitle_to_free->rects != NULL) {
                for (i = 0; i < subtitle_to_free->num_rects; i++) {
                    av_free(subtitle_to_free->rects[i].bitmap);
                    av_free(subtitle_to_free->rects[i].rgba_palette);
                }
                av_freep(&subtitle_to_free->rects);
            }
            subtitle_to_free->num_rects = 0;
            subtitle_to_free = NULL;
        }
    }
 discard_packet:
    if (pkt == NULL) {
        /* EOF handling */

        for(i=0;i<nb_ostreams;i++) {
            ost = ost_table[i];
            if (ost->source_index == ist_index) {
                AVCodecContext *enc= ost->st->codec;
                os = pFFmpeg->output_files[ost->file_index];

                if(ost->st->codec->codec_type == CODEC_TYPE_AUDIO && enc->frame_size <=1)
                    continue;
                if(ost->st->codec->codec_type == CODEC_TYPE_VIDEO && (os->oformat->flags & AVFMT_RAWPICTURE))
                    continue;

                if (ost->encoding_needed) {
                    for(;;) {
                        AVPacket pkt;
                        int fifo_bytes;
                        av_init_packet(&pkt);
                        pkt.stream_index= ost->index;

                        switch(ost->st->codec->codec_type) {
                        case CODEC_TYPE_AUDIO:
                            fifo_bytes = av_fifo_size(&ost->fifo);
                            ret = 0;
                            /* encode any samples remaining in fifo */
                            if(fifo_bytes > 0 && enc->codec->capabilities & CODEC_CAP_SMALL_LAST_FRAME) {
                                int fs_tmp = enc->frame_size;
                                enc->frame_size = fifo_bytes / (2 * enc->channels);
                                if(av_fifo_read(&ost->fifo, (uint8_t *)samples, fifo_bytes) == 0) {
                                    ret = avcodec_encode_audio(enc, pFFmpeg->bit_buffer, pFFmpeg->bit_buffer_size, samples);
                                }
                                enc->frame_size = fs_tmp;
                            }
                            if(ret <= 0) {
                                ret = avcodec_encode_audio(enc, pFFmpeg->bit_buffer, pFFmpeg->bit_buffer_size, NULL);
                            }
                            pFFmpeg->audio_size += ret;
                            pkt.flags |= PKT_FLAG_KEY;
                            break;
                        case CODEC_TYPE_VIDEO:
                            ret = avcodec_encode_video(enc, pFFmpeg->bit_buffer, pFFmpeg->bit_buffer_size, NULL);
                            pFFmpeg->video_size += ret;
                            if(enc->coded_frame && enc->coded_frame->key_frame)
                                pkt.flags |= PKT_FLAG_KEY;
                            if (ost->logfile && enc->stats_out) {
                                fprintf(ost->logfile, "%s", enc->stats_out);
                            }
                            break;
                        default:
                            ret=-1;
                        }

                        if(ret<=0)
                            break;
                        pkt.data= pFFmpeg->bit_buffer;
                        pkt.size= ret;
                        if(enc->coded_frame && enc->coded_frame->pts != AV_NOPTS_VALUE)
                            pkt.pts= av_rescale_q(enc->coded_frame->pts, enc->time_base, ost->st->time_base);
                        write_frame(os, &pkt, ost->st->codec, pFFmpeg->bitstream_filters[ost->file_index][pkt.stream_index]);
                    }
                }
            }
        }
    }

    return 0;
 fail_decode:
    return -1;
}


/*
 * The following code is the main loop of the file converter
 */
int av_encode(AVFormatContext **output_files,
                     int nb_output_files,
                     AVFormatContext **input_files,
                     int nb_input_files,
                     AVStreamMap *stream_maps, int nb_stream_maps, CFFmpeg* pFFmpeg)
{
    int ret, i, j, k, n, nb_istreams = 0, nb_ostreams = 0;
    AVFormatContext *is, *os;
    AVCodecContext *codec, *icodec;
    AVOutputStream *ost, **ost_table = NULL;
    AVInputStream *ist, **ist_table = NULL;
    AVInputFile *file_table;
    int key;

    file_table= (AVInputFile*) av_mallocz(nb_input_files * sizeof(AVInputFile));
    if (!file_table)
        goto fail;

    /* input stream init */
    j = 0;
    for(i=0;i<nb_input_files;i++) {
        is = input_files[i];
        file_table[i].ist_index = j;
        file_table[i].nb_streams = is->nb_streams;
        j += is->nb_streams;
    }
    nb_istreams = j;

    ist_table = (AVInputStream**)av_mallocz(nb_istreams * sizeof(AVInputStream *));
    if (!ist_table)
        goto fail;

    for(i=0;i<nb_istreams;i++) {
        ist = (AVInputStream*)av_mallocz(sizeof(AVInputStream));
        if (!ist)
            goto fail;
        ist_table[i] = ist;
    }
    j = 0;
    for(i=0;i<nb_input_files;i++) {
        is = input_files[i];
        for(k=0;k<is->nb_streams;k++) {
            ist = ist_table[j++];
            ist->st = is->streams[k];
            ist->file_index = i;
            ist->index = k;
            ist->discard = 1; /* the stream is discarded by default
                                 (changed later) */

            if (ist->st->codec->rate_emu) {
                ist->start = av_gettime();
                ist->frame = 0;
            }
        }
    }

    /* output stream init */
    nb_ostreams = 0;
    for(i=0;i<nb_output_files;i++) {
        os = output_files[i];
        if (!os->nb_streams) {
            fprintf(stderr, "Output file does not contain any stream\n");
            return -1;
        }
        nb_ostreams += os->nb_streams;
    }
    if (nb_stream_maps > 0 && nb_stream_maps != nb_ostreams) {
        fprintf(stderr, "Number of stream maps must match number of output streams\n");
        return -1;
    }

    /* Sanity check the mapping args -- do the input files & streams exist? */
    for(i=0;i<nb_stream_maps;i++) {
        int fi = stream_maps[i].file_index;
        int si = stream_maps[i].stream_index;

        if (fi < 0 || fi > nb_input_files - 1 ||
            si < 0 || si > file_table[fi].nb_streams - 1) {
            fprintf(stderr,"Could not find input stream #%d.%d\n", fi, si);
            return -1;
        }
        fi = stream_maps[i].sync_file_index;
        si = stream_maps[i].sync_stream_index;
        if (fi < 0 || fi > nb_input_files - 1 ||
            si < 0 || si > file_table[fi].nb_streams - 1) {
            fprintf(stderr,"Could not find sync stream #%d.%d\n", fi, si);
            return -1;
        }
    }

    ost_table = (AVOutputStream**)av_mallocz(sizeof(AVOutputStream *) * nb_ostreams);
    if (!ost_table)
        goto fail;
    for(i=0;i<nb_ostreams;i++) {
        ost = (AVOutputStream*)av_mallocz(sizeof(AVOutputStream));
        if (!ost)
            goto fail;
        ost_table[i] = ost;
    }

    n = 0;
    for(k=0;k<nb_output_files;k++) {
        os = output_files[k];
        for(i=0;i<os->nb_streams;i++) {
            int found;
            ost = ost_table[n++];
            ost->file_index = k;
            ost->index = i;
            ost->st = os->streams[i];
            if (nb_stream_maps > 0) {
                ost->source_index = file_table[stream_maps[n-1].file_index].ist_index +
                    stream_maps[n-1].stream_index;

                /* Sanity check that the stream types match */
                if (ist_table[ost->source_index]->st->codec->codec_type != ost->st->codec->codec_type) {
                    fprintf(stderr, "Codec type mismatch for mapping #%d.%d -> #%d.%d\n",
                        stream_maps[n-1].file_index, stream_maps[n-1].stream_index,
                        ost->file_index, ost->index);
                    return -1;
                }

            } else {
                /* get corresponding input stream index : we select the first one with the right type */
                found = 0;
                for(j=0;j<nb_istreams;j++) {
                    ist = ist_table[j];
                    if (ist->discard &&
                        ist->st->codec->codec_type == ost->st->codec->codec_type) {
                        ost->source_index = j;
                        found = 1;
                        break;
                    }
                }

                if (!found) {
                    /* try again and reuse existing stream */
                    for(j=0;j<nb_istreams;j++) {
                        ist = ist_table[j];
                        if (ist->st->codec->codec_type == ost->st->codec->codec_type) {
                            ost->source_index = j;
                            found = 1;
                        }
                    }
                    if (!found) {
                        fprintf(stderr, "Could not find input stream matching output stream #%d.%d\n",
                                ost->file_index, ost->index);
                        return -1;
                    }
                }
            }
            ist = ist_table[ost->source_index];
            ist->discard = 0;
            ost->sync_ist = (nb_stream_maps > 0) ?
                ist_table[file_table[stream_maps[n-1].sync_file_index].ist_index +
                         stream_maps[n-1].sync_stream_index] : ist;
        }
    }

    /* for each output stream, we compute the right encoding parameters */
    for(i=0;i<nb_ostreams;i++) {
        ost = ost_table[i];
        ist = ist_table[ost->source_index];

        codec = ost->st->codec;
        icodec = ist->st->codec;

        if (ost->st->stream_copy) {
            /* if stream_copy is selected, no need to decode or encode */
            codec->codec_id = icodec->codec_id;
            codec->codec_type = icodec->codec_type;
            if(!codec->codec_tag) codec->codec_tag = icodec->codec_tag;
            codec->bit_rate = icodec->bit_rate;
            codec->extradata= icodec->extradata;
            codec->extradata_size= icodec->extradata_size;
            if(av_q2d(icodec->time_base) > av_q2d(ist->st->time_base) && av_q2d(ist->st->time_base) < 1.0/1000)
                codec->time_base = icodec->time_base;
            else
                codec->time_base = ist->st->time_base;
            switch(codec->codec_type) {
            case CODEC_TYPE_AUDIO:
                codec->sample_rate = icodec->sample_rate;
                codec->channels = icodec->channels;
                codec->frame_size = icodec->frame_size;
                codec->block_align= icodec->block_align;
                break;
            case CODEC_TYPE_VIDEO:
                if(pFFmpeg->using_vhook) {
                    fprintf(stderr,"-vcodec copy and -vhook are incompatible (frames are not decoded)\n");
                    return -1;
                }
                codec->pix_fmt = icodec->pix_fmt;
                codec->width = icodec->width;
                codec->height = icodec->height;
                codec->has_b_frames = icodec->has_b_frames;
                break;
            case CODEC_TYPE_SUBTITLE:
                break;
            default:
                return -1;//abort();
            }
        } else {
            switch(codec->codec_type) {
            case CODEC_TYPE_AUDIO:
                if (av_fifo_init(&ost->fifo, 2 * MAX_AUDIO_PACKET_SIZE))
                    goto fail;

                if (codec->channels == icodec->channels &&
                    codec->sample_rate == icodec->sample_rate) {
                    ost->audio_resample = 0;
                } else {
                    if (codec->channels != icodec->channels &&
                        (icodec->codec_id == CODEC_ID_AC3 ||
                         icodec->codec_id == CODEC_ID_DTS)) {
                        /* Special case for 5:1 AC3 and DTS input */
                        /* and mono or stereo output      */
                        /* Request specific number of channels */
                        icodec->channels = codec->channels;
                        if (codec->sample_rate == icodec->sample_rate)
                            ost->audio_resample = 0;
                        else {
                            ost->audio_resample = 1;
                        }
                    } else {
                        ost->audio_resample = 1;
                    }
                }
                if(pFFmpeg->audio_sync_method>1)
                    ost->audio_resample = 1;

                if(ost->audio_resample){
                    ost->resample = audio_resample_init(codec->channels, icodec->channels,
                                                    codec->sample_rate, icodec->sample_rate);
                    if(!ost->resample){
                        printf("Can't resample.  Aborting.\n");
                        return -1; //abort();
                    }
                }
                ist->decoding_needed = 1;
                ost->encoding_needed = 1;
                break;
            case CODEC_TYPE_VIDEO:
                ost->video_crop = ((pFFmpeg->frame_leftBand + pFFmpeg->frame_rightBand + pFFmpeg->frame_topBand + pFFmpeg->frame_bottomBand) != 0);
                ost->video_pad = ((pFFmpeg->frame_padleft + pFFmpeg->frame_padright + pFFmpeg->frame_padtop + pFFmpeg->frame_padbottom) != 0);
                ost->video_resample = ((codec->width != icodec->width -
                                (pFFmpeg->frame_leftBand + pFFmpeg->frame_rightBand) +
                                (pFFmpeg->frame_padleft + pFFmpeg->frame_padright)) ||
                        (codec->height != icodec->height -
                                (pFFmpeg->frame_topBand  + pFFmpeg->frame_bottomBand) +
                                (pFFmpeg->frame_padtop + pFFmpeg->frame_padbottom)) ||
                        (codec->pix_fmt != icodec->pix_fmt));
                if (ost->video_crop) {
                    ost->topBand = pFFmpeg->frame_topBand;
                    ost->leftBand = pFFmpeg->frame_leftBand;
                }
                if (ost->video_pad) {
                    ost->padtop = pFFmpeg->frame_padtop;
                    ost->padleft = pFFmpeg->frame_padleft;
                    ost->padbottom = pFFmpeg->frame_padbottom;
                    ost->padright = pFFmpeg->frame_padright;
                    if (!ost->video_resample) {
                        avcodec_get_frame_defaults(&ost->pict_tmp);
                        if( avpicture_alloc( (AVPicture*)&ost->pict_tmp, codec->pix_fmt,
                                         codec->width, codec->height ) )
                            goto fail;
                    }
                }
                if (ost->video_resample) {
                    avcodec_get_frame_defaults(&ost->pict_tmp);
                    if( avpicture_alloc( (AVPicture*)&ost->pict_tmp, codec->pix_fmt,
                                         codec->width, codec->height ) ) {
                        fprintf(stderr, "Cannot allocate temp picture, check pix fmt\n");
                        return -1;
                    }
                    pFFmpeg->sws_flags = av_get_int(pFFmpeg->sws_opts, "sws_flags", NULL);
#ifdef HAVE_LIBSWSCALE
                    ost->img_resample_ctx = sws_getContext(
                            icodec->width - (pFFmpeg->frame_leftBand + pFFmpeg->frame_rightBand),
                            icodec->height - (pFFmpeg->frame_topBand + pFFmpeg->frame_bottomBand),
                            icodec->pix_fmt,
                            codec->width - (pFFmpeg->frame_padleft + pFFmpeg->frame_padright),
                            codec->height - (pFFmpeg->frame_padtop + pFFmpeg->frame_padbottom),
                            codec->pix_fmt,
                            pFFmpeg->sws_flags, NULL, NULL, NULL);
#endif
                    if (ost->img_resample_ctx == NULL) {
                        fprintf(stderr, "Cannot get resampling context\n");
                        return -1;
                    }
                    ost->resample_height = icodec->height - (pFFmpeg->frame_topBand + pFFmpeg->frame_bottomBand);
                }
                ost->encoding_needed = 1;
                ist->decoding_needed = 1;
                break;
            case CODEC_TYPE_SUBTITLE:
                ost->encoding_needed = 1;
                ist->decoding_needed = 1;
                break;
            default:
                return -1;//abort();
                break;
            }
            /* two pass mode */
            if (ost->encoding_needed &&
                (codec->flags & (CODEC_FLAG_PASS1 | CODEC_FLAG_PASS2))) {
                char logfilename[1024];
                FILE *f;
                int size;
                char *logbuffer;

                snprintf(logfilename, sizeof(logfilename), "%s-%d.log",
                         pFFmpeg->pass_logfilename ?
                         pFFmpeg->pass_logfilename : DEFAULT_PASS_LOGFILENAME, i);
                if (codec->flags & CODEC_FLAG_PASS1) {
                    f = fopen(logfilename, "w");
                    if (!f) {
                        perror(logfilename);
                        return -1;
                    }
                    ost->logfile = f;
                } else {
                    /* read the log file */
                    f = fopen(logfilename, "r");
                    if (!f) {
                        perror(logfilename);
                        return -1;
                    }
                    fseek(f, 0, SEEK_END);
                    size = ftell(f);
                    fseek(f, 0, SEEK_SET);
                    logbuffer = (char*)av_malloc(size + 1);
                    if (!logbuffer) {
                        fprintf(stderr, "Could not allocate log buffer\n");
                        return -1;
                    }
                    size = fread(logbuffer, 1, size, f);
                    fclose(f);
                    logbuffer[size] = '\0';
                    codec->stats_in = logbuffer;
                }
            }
        }
        if(codec->codec_type == CODEC_TYPE_VIDEO){
            int size= codec->width * codec->height;
            pFFmpeg->bit_buffer_size= FFMAX(pFFmpeg->bit_buffer_size, 4*size);
        }
    }

    if (!pFFmpeg->bit_buffer)
        pFFmpeg->bit_buffer = (uint8_t*)av_malloc(pFFmpeg->bit_buffer_size);
    if (!pFFmpeg->bit_buffer)
        goto fail;

    /* dump the file output parameters - cannot be done before in case
       of stream copy */
    for(i=0;i<nb_output_files;i++) {
        dump_format(output_files[i], i, output_files[i]->filename, 1);
    }

    /* dump the stream mapping */
    if (pFFmpeg->verbose >= 0) {
        fprintf(stderr, "Stream mapping:\n");
        for(i=0;i<nb_ostreams;i++) {
            ost = ost_table[i];
            fprintf(stderr, "  Stream #%d.%d -> #%d.%d",
                    ist_table[ost->source_index]->file_index,
                    ist_table[ost->source_index]->index,
                    ost->file_index,
                    ost->index);
            if (ost->sync_ist != ist_table[ost->source_index])
                fprintf(stderr, " [sync #%d.%d]",
                        ost->sync_ist->file_index,
                        ost->sync_ist->index);
            fprintf(stderr, "\n");
        }
    }

    /* open each encoder */
    for(i=0;i<nb_ostreams;i++) {
        ost = ost_table[i];
        if (ost->encoding_needed) {
            AVCodec *codec;
            codec = avcodec_find_encoder(ost->st->codec->codec_id);
            if (!codec) {
                fprintf(stderr, "Unsupported codec for output stream #%d.%d\n",
                        ost->file_index, ost->index);
                return -1;
            }
            if (avcodec_open(ost->st->codec, codec) < 0) {
                fprintf(stderr, "Error while opening codec for output stream #%d.%d - maybe incorrect parameters such as bit_rate, rate, width or height\n",
                        ost->file_index, ost->index);
                return -1;
            }
            pFFmpeg->extra_size += ost->st->codec->extradata_size;
        }
    }

    /* open each decoder */
    for(i=0;i<nb_istreams;i++) {
        ist = ist_table[i];
        if (ist->decoding_needed) {
            AVCodec *codec;
            codec = avcodec_find_decoder(ist->st->codec->codec_id);
            if (!codec) {
                fprintf(stderr, "Unsupported codec (id=%d) for input stream #%d.%d\n",
                        ist->st->codec->codec_id, ist->file_index, ist->index);
                return -1;
            }
            if (avcodec_open(ist->st->codec, codec) < 0) {
                fprintf(stderr, "Error while opening codec for input stream #%d.%d\n",
                        ist->file_index, ist->index);
                return -1;
            }
            //if (ist->st->codec->codec_type == CODEC_TYPE_VIDEO)
            //    ist->st->codec->flags |= CODEC_FLAG_REPEAT_FIELD;
        }
    }

    /* init pts */
    for(i=0;i<nb_istreams;i++) {
        ist = ist_table[i];
        is = input_files[ist->file_index];
        ist->pts = 0;
        ist->next_pts = av_rescale_q(ist->st->start_time, ist->st->time_base, AV_TIME_BASE_Q);
        if(ist->st->start_time == AV_NOPTS_VALUE)
            ist->next_pts=0;
        if(pFFmpeg->input_files_ts_offset[ist->file_index])
            ist->next_pts= AV_NOPTS_VALUE;
        ist->is_start = 1;
    }

    /* set meta data information from input file if required */
    for (i=0;i<pFFmpeg->nb_meta_data_maps;i++) {
        AVFormatContext *out_file;
        AVFormatContext *in_file;

        int out_file_index = pFFmpeg->meta_data_maps[i].out_file;
        int in_file_index = pFFmpeg->meta_data_maps[i].in_file;
        if ( out_file_index < 0 || out_file_index >= nb_output_files ) {
            fprintf(stderr, "Invalid output file index %d map_meta_data(%d,%d)\n", out_file_index, out_file_index, in_file_index);
            ret = AVERROR(EINVAL);
            goto fail;
        }
        if ( in_file_index < 0 || in_file_index >= nb_input_files ) {
            fprintf(stderr, "Invalid input file index %d map_meta_data(%d,%d)\n", in_file_index, out_file_index, in_file_index);
            ret = AVERROR(EINVAL);
            goto fail;
        }

        out_file = output_files[out_file_index];
        in_file = input_files[in_file_index];

        strcpy(out_file->title, in_file->title);
        strcpy(out_file->author, in_file->author);
        strcpy(out_file->copyright, in_file->copyright);
        strcpy(out_file->comment, in_file->comment);
        strcpy(out_file->album, in_file->album);
        out_file->year = in_file->year;
        out_file->track = in_file->track;
        strcpy(out_file->genre, in_file->genre);
    }

    /* open files and write file headers */
    for(i=0;i<nb_output_files;i++) {
        os = output_files[i];
        if (av_write_header(os) < 0) {
            fprintf(stderr, "Could not write header for output file #%d (incorrect codec parameters ?)\n", i);
            ret = AVERROR(EINVAL);
            goto fail;
        }
    }

    /*if ( !using_stdin && verbose >= 0) {
        fprintf(stderr, "Press [q] to stop encoding\n");
        url_set_interrupt_cb(decode_interrupt_cb);
    }
    term_init(); */

    key = -1;
    pFFmpeg->timer_start = av_gettime();

    for(; ;) { //received_sigterm == 0;) {
        int file_index, ist_index;
        AVPacket pkt;
        double ipts_min;
        double opts_min;

    redo:
        ipts_min= 1e100;
        opts_min= 1e100;
        /* if 'q' pressed, exits */
        /*if (!using_stdin) {
            if (q_pressed)
                break;
            /* read_key() returns 0 on EOF */
            /*key = read_key();
            if (key == 'q')
                break;
        }*/

        /* select the stream that we must read now by looking at the
           smallest output pts */
        file_index = -1;
        for(i=0;i<nb_ostreams;i++) {
            double ipts, opts;
            ost = ost_table[i];
            os = output_files[ost->file_index];
            ist = ist_table[ost->source_index];
            if(ost->st->codec->codec_type == CODEC_TYPE_VIDEO)
                opts = ost->sync_opts * av_q2d(ost->st->codec->time_base);
            else
                opts = ost->st->pts.val * av_q2d(ost->st->time_base);
            ipts = (double)ist->pts;
            if (!file_table[ist->file_index].eof_reached){
                if(ipts < ipts_min) {
                    ipts_min = ipts;
                    if(pFFmpeg->input_sync ) file_index = ist->file_index;
                }
                if(opts < opts_min) {
                    opts_min = opts;
                    if(!pFFmpeg->input_sync) file_index = ist->file_index;
                }
            }
            if(ost->frame_number >= pFFmpeg->max_frames[ost->st->codec->codec_type]){
                file_index= -1;
                break;
            }
        }
        /* if none, if is finished */
        if (file_index < 0) {
            break;
        }

        /* finish if recording time exhausted */
        if (pFFmpeg->recording_time > 0 && opts_min >= (pFFmpeg->recording_time / 1000000.0))
            break;

        /* finish if limit size exhausted */
				#if (FFMPEG_VERSION >= 52)
        if (pFFmpeg->limit_filesize != 0 && pFFmpeg->limit_filesize < url_ftell(output_files[0]->pb))
				#else
				if (pFFmpeg->limit_filesize != 0 && pFFmpeg->limit_filesize < url_ftell(&output_files[0]->pb))
				#endif
            break;
				
				

        /* read a frame from it and output it in the fifo */
        is = input_files[file_index];
        if (av_read_frame(is, &pkt) < 0) {
            file_table[file_index].eof_reached = 1;
            if (pFFmpeg->opt_shortest)
                break;
            else
                continue;
        }

        /*if (pFFmpeg->do_pkt_dump) {
            av_pkt_dump_log(NULL, AV_LOG_DEBUG, &pkt, pFFmpeg->do_hex_dump);
        }*/
				
        /* the following test is needed in case new streams appear
           dynamically in stream : we ignore them */
        if (pkt.stream_index >= file_table[file_index].nb_streams)
            goto discard_packet;
        ist_index = file_table[file_index].ist_index + pkt.stream_index;
        ist = ist_table[ist_index];
        if (ist->discard)
            goto discard_packet;

//        fprintf(stderr, "next:%"PRId64" dts:%"PRId64" off:%"PRId64" %d\n", ist->next_pts, pkt.dts, input_files_ts_offset[ist->file_index], ist->st->codec->codec_type);
        if (pkt.dts != AV_NOPTS_VALUE && ist->next_pts != AV_NOPTS_VALUE) {
            int64_t delta= av_rescale_q(pkt.dts, ist->st->time_base, AV_TIME_BASE_Q) - ist->next_pts;
            if(FFABS(delta) > 1LL*pFFmpeg->dts_delta_threshold*AV_TIME_BASE && !pFFmpeg->copy_ts){
                pFFmpeg->input_files_ts_offset[ist->file_index]-= delta;
                /*if (verbose > 2)
                    fprintf(stderr, "timestamp discontinuity %"PRId64", new offset= %"PRId64"\n", delta, input_files_ts_offset[ist->file_index]); */
                for(i=0; i<file_table[file_index].nb_streams; i++){
                    int index= file_table[file_index].ist_index + i;
                    ist_table[index]->next_pts += delta;
                    ist_table[index]->is_start=1;
                }
            }
        }

        //fprintf(stderr,"read #%d.%d size=%d\n", ist->file_index, ist->index, pkt.size);
        if (output_packet(ist, ist_index, ost_table, nb_ostreams, &pkt, pFFmpeg) < 0) {

            if (pFFmpeg->verbose >= 0)
                fprintf(stderr, "Error while decoding stream #%d.%d\n",
                        ist->file_index, ist->index);

            av_free_packet(&pkt);
            goto redo;
        }

    discard_packet:
        av_free_packet(&pkt);

        /* dump report by using the output first video and audio streams */
        print_report(output_files, ost_table, nb_ostreams, 0, pFFmpeg);
    }

    /* at the end of stream, we must flush the decoder buffers */
    for(i=0;i<nb_istreams;i++) {
        ist = ist_table[i];
        if (ist->decoding_needed) {
            output_packet(ist, i, ost_table, nb_ostreams, NULL, pFFmpeg);
        }
    }

    //term_exit();

    /* write the trailer if needed and close file */
    for(i=0;i<nb_output_files;i++) {
        os = output_files[i];
        av_write_trailer(os);
    }

    /* dump report by using the first video and audio streams */
    print_report(output_files, ost_table, nb_ostreams, 1, pFFmpeg);

    /* close each encoder */
    for(i=0;i<nb_ostreams;i++) {
        ost = ost_table[i];
        if (ost->encoding_needed) {
            av_freep(&ost->st->codec->stats_in);
            avcodec_close(ost->st->codec);
        }
    }

    /* close each decoder */
    for(i=0;i<nb_istreams;i++) {
        ist = ist_table[i];
        if (ist->decoding_needed) {
            avcodec_close(ist->st->codec);
        }
    }

    /* finished ! */

    ret = 0;
 fail1:
    av_freep(&pFFmpeg->bit_buffer);
    av_free(file_table);

    if (ist_table) {
        for(i=0;i<nb_istreams;i++) {
            ist = ist_table[i];
            av_free(ist);
        }
        av_free(ist_table);
    }
    if (ost_table) {
        for(i=0;i<nb_ostreams;i++) {
            ost = ost_table[i];
            if (ost) {
                if (ost->logfile) {
                    fclose(ost->logfile);
                    ost->logfile = NULL;
                }
                av_fifo_free(&ost->fifo); /* works even if fifo is not
                                             initialized but set to zero */
                av_free(ost->pict_tmp.data[0]);
#ifdef HAVE_LIBSWSCALE
                if (ost->video_resample)
                    sws_freeContext(ost->img_resample_ctx);
#endif
                if (ost->audio_resample)
                    audio_resample_close(ost->resample);
                av_free(ost);
            }
        }
        av_free(ost_table);
    }
    return ret;
 fail:
    ret = AVERROR(ENOMEM);
    goto fail1;
}

/*#if 0
int file_read(const char *filename)
{
    URLContext *h;
    unsigned char buffer[1024];
    int len, i;

    if (url_open(&h, filename, O_RDONLY) < 0) {
        printf("could not open '%s'\n", filename);
        return -1;
    }
    for(;;) {
        len = url_read(h, buffer, sizeof(buffer));
        if (len <= 0)
            break;
        for(i=0;i<len;i++) putchar(buffer[i]);
    }
    url_close(h);
    return 0;
}
#endif */





/*#ifdef CONFIG_VHOOK
static void add_frame_hooker(const char *arg)
{
    int argc = 0;
    char *argv[64];
    int i;
    char *args = av_strdup(arg);

    using_vhook = 1;

    argv[0] = strtok(args, " ");
    while (argc < 62 && (argv[++argc] = strtok(NULL, " "))) {
    }

    i = frame_hook_add(argc, argv);

    if (i != 0) {
        fprintf(stderr, "Failed to add video hook function: %s\n", arg);
        exit(1);
    }
}
#endif*/



void CFFmpeg::check_audio_video_inputs(int *has_video_ptr, int *has_audio_ptr)
{
    int has_video, has_audio, i, j;
    AVFormatContext *ic;

    has_video = 0;
    has_audio = 0;
    for(j=0;j<nb_input_files;j++) {
        ic = input_files[j];
        for(i=0;i<ic->nb_streams;i++) {
            AVCodecContext *enc = ic->streams[i]->codec;
            switch(enc->codec_type) {
            case CODEC_TYPE_AUDIO:
                has_audio = 1;
                break;
            case CODEC_TYPE_VIDEO:
                has_video = 1;
                break;
            case CODEC_TYPE_DATA:
            case CODEC_TYPE_UNKNOWN:
            case CODEC_TYPE_SUBTITLE:
                break;
            default:
                return;//abort();
            }
        }
    }
    *has_video_ptr = has_video;
    *has_audio_ptr = has_audio;
}

void CFFmpeg::new_video_stream(AVFormatContext *oc)
{
    AVStream *st;
    AVCodecContext *video_enc;
    int codec_id;

    st = av_new_stream(oc, oc->nb_streams);
    if (!st) {
        fprintf(stderr, "Could not alloc stream\n");
        return;
    }
    avcodec_get_context_defaults2(st->codec, CODEC_TYPE_VIDEO);
    bitstream_filters[nb_output_files][oc->nb_streams - 1]= video_bitstream_filters;
    video_bitstream_filters= NULL;

    if(thread_count>1)
        avcodec_thread_init(st->codec, thread_count);

    video_enc = st->codec;

    if(video_codec_tag)
        video_enc->codec_tag= video_codec_tag;

    if(   (video_global_header&1)
       || (video_global_header==0 && (oc->oformat->flags & AVFMT_GLOBALHEADER))){
        video_enc->flags |= CODEC_FLAG_GLOBAL_HEADER;
        avctx_opts[CODEC_TYPE_VIDEO]->flags|= CODEC_FLAG_GLOBAL_HEADER;
    }
    if(video_global_header&2){
        video_enc->flags2 |= CODEC_FLAG2_LOCAL_HEADER;
        avctx_opts[CODEC_TYPE_VIDEO]->flags2|= CODEC_FLAG2_LOCAL_HEADER;
    }

    if (video_stream_copy) {
        st->stream_copy = 1;
        video_enc->codec_type = CODEC_TYPE_VIDEO;
    } else {
        char *p;
        int i;
        AVCodec *codec;

        codec_id = av_guess_codec(oc->oformat, NULL, oc->filename, NULL, CODEC_TYPE_VIDEO);
        if (video_codec_id != CODEC_ID_NONE)
            codec_id = video_codec_id;

        video_enc->codec_id = (CodecID)codec_id;
        codec = avcodec_find_encoder((CodecID)codec_id);

        for(i=0; i<opt_name_count; i++){
             const AVOption *opt;
             double d= av_get_double(avctx_opts[CODEC_TYPE_VIDEO], opt_names[i], &opt);
             if(d==d && (opt->flags&AV_OPT_FLAG_VIDEO_PARAM) && (opt->flags&AV_OPT_FLAG_ENCODING_PARAM))
                 av_set_double(video_enc, opt_names[i], d);
        }

        video_enc->time_base.den = frame_rate.num;
        video_enc->time_base.num = frame_rate.den;
        if(codec && codec->supported_framerates){
            const AVRational *p= codec->supported_framerates;
            AVRational req= (AVRational){frame_rate.num, frame_rate.den};
            const AVRational *best=NULL;
            AVRational best_error= (AVRational){INT_MAX, 1};
            for(; p->den!=0; p++){
                AVRational error= av_sub_q(req, *p);
                if(error.num <0) error.num *= -1;
                if(av_cmp_q(error, best_error) < 0){
                    best_error= error;
                    best= p;
                }
            }
            video_enc->time_base.den= best->num;
            video_enc->time_base.num= best->den;
        }

        video_enc->width = frame_width + frame_padright + frame_padleft;
        video_enc->height = frame_height + frame_padtop + frame_padbottom;
        video_enc->sample_aspect_ratio = av_d2q(frame_aspect_ratio*video_enc->height/video_enc->width, 255);
        video_enc->pix_fmt = frame_pix_fmt;

        if(codec && codec->pix_fmts){
            const enum PixelFormat *p= codec->pix_fmts;
            for(; *p!=-1; p++){
                if(*p == video_enc->pix_fmt)
                    break;
            }
            if(*p == -1)
                video_enc->pix_fmt = codec->pix_fmts[0];
        }

        if (intra_only)
            video_enc->gop_size = 0;
        if (video_qscale || same_quality) {
            video_enc->flags |= CODEC_FLAG_QSCALE;
            video_enc->global_quality=
                st->quality = FF_QP2LAMBDA * video_qscale;
        }

        if(intra_matrix)
            video_enc->intra_matrix = intra_matrix;
        if(inter_matrix)
            video_enc->inter_matrix = inter_matrix;

        video_enc->max_qdiff = video_qdiff;
        video_enc->rc_eq = video_rc_eq;
        video_enc->thread_count = thread_count;
        p= video_rc_override_string;
        for(i=0; p; i++){
            int start, end, q;
            int e=sscanf(p, "%d,%d,%d", &start, &end, &q);
            if(e!=3){
                fprintf(stderr, "error parsing rc_override\n");
                return;
            }
            video_enc->rc_override=
                (RcOverride*)av_realloc(video_enc->rc_override,
                           sizeof(RcOverride)*(i+1));
            video_enc->rc_override[i].start_frame= start;
            video_enc->rc_override[i].end_frame  = end;
            if(q>0){
                video_enc->rc_override[i].qscale= q;
                video_enc->rc_override[i].quality_factor= 1.0;
            }
            else{
                video_enc->rc_override[i].qscale= 0;
                video_enc->rc_override[i].quality_factor= -q/100.0;
            }
            p= strchr(p, '/');
            if(p) p++;
        }
        video_enc->rc_override_count=i;
        if (!video_enc->rc_initial_buffer_occupancy)
            video_enc->rc_initial_buffer_occupancy = video_enc->rc_buffer_size*3/4;
        video_enc->me_threshold= me_threshold;
        video_enc->intra_dc_precision= intra_dc_precision - 8;
        video_enc->strict_std_compliance = strict;

        if (do_psnr)
            video_enc->flags|= CODEC_FLAG_PSNR;

        /* two pass mode */
        if (do_pass) {
            if (do_pass == 1) {
                video_enc->flags |= CODEC_FLAG_PASS1;
            } else {
                video_enc->flags |= CODEC_FLAG_PASS2;
            }
        }
    }

    /* reset some key parameters */
    video_disable = 0;
    video_codec_id = CODEC_ID_NONE;
    video_stream_copy = 0;
}

void CFFmpeg::new_audio_stream(AVFormatContext *oc)
{
    AVStream *st;
    AVCodecContext *audio_enc;
    int codec_id, i;

    st = av_new_stream(oc, oc->nb_streams);
    if (!st) {
        fprintf(stderr, "Could not alloc stream\n");
        return;
    }
    avcodec_get_context_defaults2(st->codec, CODEC_TYPE_AUDIO);

    bitstream_filters[nb_output_files][oc->nb_streams - 1]= audio_bitstream_filters;
    audio_bitstream_filters= NULL;

    if(thread_count>1)
        avcodec_thread_init(st->codec, thread_count);

    audio_enc = st->codec;
    audio_enc->codec_type = CODEC_TYPE_AUDIO;
    audio_enc->strict_std_compliance = strict;

    if(audio_codec_tag)
        audio_enc->codec_tag= audio_codec_tag;

    if (oc->oformat->flags & AVFMT_GLOBALHEADER) {
        audio_enc->flags |= CODEC_FLAG_GLOBAL_HEADER;
        avctx_opts[CODEC_TYPE_AUDIO]->flags|= CODEC_FLAG_GLOBAL_HEADER;
    }
    if (audio_stream_copy) {
        st->stream_copy = 1;
        audio_enc->channels = audio_channels;
    } else {
        codec_id = av_guess_codec(oc->oformat, NULL, oc->filename, NULL, CODEC_TYPE_AUDIO);

        for(i=0; i<opt_name_count; i++){
            const AVOption *opt;
            double d= av_get_double(avctx_opts[CODEC_TYPE_AUDIO], opt_names[i], &opt);
            if(d==d && (opt->flags&AV_OPT_FLAG_AUDIO_PARAM) && (opt->flags&AV_OPT_FLAG_ENCODING_PARAM))
                av_set_double(audio_enc, opt_names[i], d);
        }

        if (audio_codec_id != CODEC_ID_NONE)
            codec_id = audio_codec_id;
        audio_enc->codec_id = (CodecID)codec_id;

        if (audio_qscale > QSCALE_NONE) {
            audio_enc->flags |= CODEC_FLAG_QSCALE;
            audio_enc->global_quality = st->quality = FF_QP2LAMBDA * audio_qscale;
        }
        audio_enc->thread_count = thread_count;
        audio_enc->channels = audio_channels;
    }
    audio_enc->sample_rate = audio_sample_rate;
    audio_enc->time_base= (AVRational){1, audio_sample_rate};
    if (audio_language) {
        #ifdef HAVE_AVSTRING_H
        av_strlcpy(st->language, audio_language, sizeof(st->language));
        #else
        pstrcpy(st->language, sizeof(st->language), audio_language);
        #endif
        av_free(audio_language);
        audio_language = NULL;
    }

    /* reset some key parameters */
    audio_disable = 0;
    audio_codec_id = CODEC_ID_NONE;
    audio_stream_copy = 0;
}



int64_t getutime(void)
{
#ifdef HAVE_GETRUSAGE
    struct rusage rusage;

    getrusage(RUSAGE_SELF, &rusage);
    return (rusage.ru_utime.tv_sec * 1000000LL) + rusage.ru_utime.tv_usec;
#elif defined(HAVE_GETPROCESSTIMES)
    HANDLE proc;
    FILETIME c, e, k, u;
    proc = GetCurrentProcess();
    GetProcessTimes(proc, &c, &e, &k, &u);
    return ((int64_t) u.dwHighDateTime << 32 | u.dwLowDateTime) / 10;
#else
  return av_gettime();
#endif
}

#if defined(CONFIG_FFM_DEMUXER) || defined(CONFIG_FFM_MUXER)
extern int ffm_nopts;
#endif

void show_formats(void)
{
    AVInputFormat *ifmt;
    AVOutputFormat *ofmt;
    URLProtocol *up;
    AVCodec *p, *p2;
    const char *last_name;

    printf("File formats:\n");
    last_name= "000";
    for(;;){
        int decode=0;
        int encode=0;
        const char *name=NULL;
        const char *long_name=NULL;

        for(ofmt = first_oformat; ofmt != NULL; ofmt = ofmt->next) {
            if((name == NULL || strcmp(ofmt->name, name)<0) &&
                strcmp(ofmt->name, last_name)>0){
                name= ofmt->name;
                long_name= ofmt->long_name;
                encode=1;
            }
        }
        for(ifmt = first_iformat; ifmt != NULL; ifmt = ifmt->next) {
            if((name == NULL || strcmp(ifmt->name, name)<0) &&
                strcmp(ifmt->name, last_name)>0){
                name= ifmt->name;
                long_name= ifmt->long_name;
                encode=0;
            }
            if(name && strcmp(ifmt->name, name)==0)
                decode=1;
        }
        if(name==NULL)
            break;
        last_name= name;

        printf(
            " %s%s %-15s %s\n",
            decode ? "D":" ",
            encode ? "E":" ",
            name,
            long_name ? long_name:" ");
    }
    printf("\n");

    printf("Codecs:\n");
    last_name= "000";
    for(;;){
        int decode=0;
        int encode=0;
        int cap=0;
        const char *type_str;

        p2=NULL;
        for(p = first_avcodec; p != NULL; p = p->next) {
            if((p2==NULL || strcmp(p->name, p2->name)<0) &&
                strcmp(p->name, last_name)>0){
                p2= p;
                decode= encode= cap=0;
            }
            if(p2 && strcmp(p->name, p2->name)==0){
                if(p->decode) decode=1;
                if(p->encode) encode=1;
                cap |= p->capabilities;
            }
        }
        if(p2==NULL)
            break;
        last_name= p2->name;

        switch(p2->type) {
        case CODEC_TYPE_VIDEO:
            type_str = "V";
            break;
        case CODEC_TYPE_AUDIO:
            type_str = "A";
            break;
        case CODEC_TYPE_SUBTITLE:
            type_str = "S";
            break;
        default:
            type_str = "?";
            break;
        }
        printf(
            " %s%s%s%s%s%s %s",
            decode ? "D": (/*p2->decoder ? "d":*/" "),
            encode ? "E":" ",
            type_str,
            cap & CODEC_CAP_DRAW_HORIZ_BAND ? "S":" ",
            cap & CODEC_CAP_DR1 ? "D":" ",
            cap & CODEC_CAP_TRUNCATED ? "T":" ",
            p2->name);
       /* if(p2->decoder && decode==0)
            printf(" use %s for decoding", p2->decoder->name);*/
        printf("\n");
    }
    printf("\n");

    printf("Supported file protocols:\n");
    for(up = first_protocol; up != NULL; up = up->next)
        printf(" %s:", up->name);
    printf("\n");

    printf("Frame size, frame rate abbreviations:\n ntsc pal qntsc qpal sntsc spal film ntsc-film sqcif qcif cif 4cif\n");
    printf("\n");
    printf(
"Note, the names of encoders and decoders do not always match, so there are\n"
"several cases where the above table shows encoder only or decoder only entries\n"
"even though both encoding and decoding are supported. For example, the h263\n"
"decoder corresponds to the h263 and h263p encoders, for file formats it is even\n"
"worse.\n");   
}


void parse_arg_file(const char *filename, CFFmpeg* pFFmpeg)
{
  pFFmpeg->opt_output_file(filename);
}

int CFFmpeg::ffmpeg_main(int argc, char* argv[])
{    
    int i;
    int64_t ti;

  
    for(i=0; i<CODEC_TYPE_NB; i++){
        avctx_opts[i]= avcodec_alloc_context2((CodecType)i);
    }
    avformat_opts = av_alloc_format_context();
#ifdef HAVE_LIBSWSCALE
    sws_opts = sws_getContext(16,16,0, 16,16,0, sws_flags, NULL,NULL,NULL);
#endif

    
    /* parse options */
    parse_options(argc, argv, options, this);

    /* file converter / grab */
    if (nb_output_files <= 0) {
        fprintf(stderr, "Must supply at least one output file\n");
        return 0;
    }

    if (nb_input_files == 0) {
        fprintf(stderr, "Must supply at least one input file\n");
        return 0;
    }

    ti = getutime();  
  
    av_encode(output_files, nb_output_files, input_files, nb_input_files,
              stream_maps, nb_stream_maps, this);
    ti = getutime() - ti;
    /*if (do_benchmark) {
        printf("bench: utime=%0.3fs\n", ti / 1000000.0);
    }*/


  //ffm
    
    return 0;
}
/*
void ffmpeg_close()
{
  int i;
      // close files 
    for(i=0;i<nb_output_files;i++) {
        // maybe av_close_output_file ??? 
        AVFormatContext *s = output_files[i];
        int j;
        if (!(s->oformat->flags & AVFMT_NOFILE))
            url_fclose(&s->pb);
        for(j=0;j<s->nb_streams;j++) {
            av_free(s->streams[j]->codec);
            s->streams[j]->codec = NULL;
            av_free(s->streams[j]);
            s->streams[j] = NULL;
        }
        av_free(s);
    }
    for(i=0;i<nb_input_files;i++)
        av_close_input_file(input_files[i]);

    //av_free_static();

    av_free(intra_matrix);
    intra_matrix = NULL;
    av_free(inter_matrix);
    inter_matrix = NULL;

    if (fvstats)
        fclose(fvstats);
    av_free(vstats_filename);
    vstats_filename = NULL;

    av_free(opt_names);
    opt_names = NULL;

    av_free(video_standard);
    video_standard = NULL;
*/
  
/*#ifdef CONFIG_POWERPC_PERF
    extern void powerpc_display_perf_report(void);
    powerpc_display_perf_report();
#endif // CONFIG_POWERPC_PERF 
} */

/*} 
#endif // __cplusplus  */
