/*
 * Various utilities for command line tools
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

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#if FFMPEG_VERSION >= 52 && !defined(OLD_INCLUDES_PATH)
#include <libavformat/avformat.h>

	#ifdef HAVE_AVSTRING_H
	#include <libavutil/avstring.h>
	#endif
#else
#include <avformat.h>

	#ifdef HAVE_AVSTRING_H
	#include <avstring.h>
	#endif
#endif

#include "cmdutils.h"

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif



#include "ffmpeg.h"

OptionDef* find_option(OptionDef **po, const char *name) {
  
    while ((*po)->name != NULL) {
        if (!strcmp(name, (*po)->name))
            break;
      po++;      
    }
    return *po;
}

void parse_options(int argc, char **argv, OptionDef **options, CFFmpeg* pFFmpeg)
{
    const char *opt, *arg;
    int optindex, handleoptions=1;
    const OptionDef *po;

    /* parse options */
    optindex = 1;
    while (optindex < argc) {
        opt = argv[optindex++];

        if (handleoptions && opt[0] == '-' && opt[1] != '\0') {
          if (opt[1] == '-' && opt[2] == '\0') {
            handleoptions = 0;
            continue;
          }
            po= find_option(options, opt + 1);
            if (!po->name)
                po= find_option(options, "default");
            if (!po->name) {
                unknown_opt: 
                fprintf(stderr, "%s: unrecognized option '%s'\n", argv[0], opt);
                exit(1);
            }
            arg = NULL;
            if (po->flags & HAS_ARG) {
                arg = argv[optindex++];
                if (!arg) {
                    fprintf(stderr, "%s: missing argument for option '%s'\n", argv[0], opt);
                    exit(1);
                }
            }
            if (po->flags & OPT_STRING) {
                char *str;
                str = _av_strdup(arg);
                *po->u.str_arg = str;
            } else if (po->flags & OPT_BOOL) {
                *po->u.int_arg = 1;
            } else if (po->flags & OPT_INT) {
                *po->u.int_arg = atoi(arg);
            } else if (po->flags & OPT_INT64) {
                *po->u.int64_arg = strtoll(arg, (char **)NULL, 10);
            } else if (po->flags & OPT_FLOAT) {
                *po->u.float_arg = atof(arg);
            } else if (po->flags & OPT_FUNC2) {
                //if(po->u.func2_arg(opt+1, arg)<0)
              
                /*func2_ptr func2_arg;
                func2_arg = po->u.func2_arg;
                if((pFFmpeg->*func2_arg)(opt+1, arg)<0)*/                
                if((pFFmpeg->*(po->u.func2_arg))(opt+1, arg)<0)
                    goto unknown_opt;
            } else {
                //po->u.func_arg(arg);
              
                /*void (CFFmpeg::*func_arg)(const char *);
                func_arg = po->u.func_arg;                
                (pFFmpeg->*func_arg)(arg);*/              
                (pFFmpeg->*(po->u.func_arg))(arg);
            }
        } else {
            parse_arg_file(opt, pFFmpeg);
        }
    }
}

void print_error(const char *filename, int err)
{
    switch(err) {
    case AVERROR(EINVAL):
        fprintf(stderr, "%s: Incorrect image filename syntax.\n"
                "Use '%%d' to specify the image number:\n"
                "  for img1.jpg, img2.jpg, ..., use 'img%%d.jpg';\n"
                "  for img001.jpg, img002.jpg, ..., use 'img%%03d.jpg'.\n",
                filename);
        break;
    case AVERROR_INVALIDDATA:
        fprintf(stderr, "%s: Error while parsing header\n", filename);
        break;
    case AVERROR(EILSEQ):
        fprintf(stderr, "%s: Unknown format\n", filename);
        break;
    case AVERROR(EIO):
        fprintf(stderr, "%s: I/O error occured\n"
                "Usually that means that input file is truncated and/or corrupted.\n",
                filename);
        break;
    case AVERROR(ENOMEM):
        fprintf(stderr, "%s: memory allocation error occured\n", filename);
        break;
    case AVERROR(ENOENT):
        fprintf(stderr, "%s: no such file or directory\n", filename);
        break;
    default:
        fprintf(stderr, "%s: Error while opening file\n", filename);
        break;
    }
}
