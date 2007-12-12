/*
 * Various utilities for command line tools
 * copyright (c) 2003 Fabrice Bellard
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

#ifndef _CMD_UTILS_H
#define _CMD_UTILS_H

#ifdef HAVE_CONFIG_H
#include "../../../config.h"
#endif

class CFFmpeg;

typedef void (CFFmpeg::*func_ptr)(const char*);

typedef int (CFFmpeg::*func2_ptr)(const char *, const char *);

struct OptionDef {
  
    OptionDef()
    {
      name = NULL;
      flags = 0;
      help = NULL;
      argname = NULL;
    }
  
    OptionDef(const char *_name, 
              int _flags, 
              func_ptr _u,
              const char *_help,
              const char *_argname)
    {
      if(_name) {      
        name = (const char*)malloc(sizeof(char*) * strlen(_name) + 1);
        strcpy((char*)name, _name);        
      }
      else {
        name = NULL;
      }
        
      flags = _flags;
      u.func_arg = _u;
      
      if(_help) {      
        help = (const char*)malloc(sizeof(char*) * strlen(_help) + 1);
        strcpy((char*)help, _help);        
      }
      else {
        help = NULL;
      }
      
      if(_argname) {      
        argname = (const char*)malloc(sizeof(char*) * strlen(_argname) + 1);
        strcpy((char*)argname, _argname);        
      }
      else {
        argname = NULL;
      }

    }
  
    OptionDef(const char *_name, 
              int _flags, 
              func2_ptr _u,
              const char *_help,
              const char *_argname)
    {
      if(_name) {      
        name = (const char*)malloc(sizeof(char*) * strlen(_name) + 1);
        strcpy((char*)name, _name);        
      }
      else {
        name = NULL;
      }
        
      flags = _flags;
      u.func2_arg = _u;
      
      if(_help) {      
        help = (const char*)malloc(sizeof(char*) * strlen(_help) + 1);
        strcpy((char*)help, _help);        
      }
      else {
        help = NULL;
      }
      
      if(_argname) {      
        argname = (const char*)malloc(sizeof(char*) * strlen(_argname) + 1);
        strcpy((char*)argname, _argname);        
      }
      else {
        argname = NULL; 
      }

    }
  
		// OPT_STRING
		OptionDef(const char *_name, 
              int _flags, 
              char ** _str,
              const char *_help,
              const char *_argname)
    {
      if(_name) {      
        name = (const char*)malloc(sizeof(char*) * strlen(_name) + 1);
        strcpy((char*)name, _name);        
      }
      else {
        name = NULL;
      }
        
      flags = _flags;
      u.str_arg = _str;
      
      if(_help) {      
        help = (const char*)malloc(sizeof(char*) * strlen(_help) + 1);
        strcpy((char*)help, _help);        
      }
      else {
        help = NULL;
      }
      
      if(_argname) {      
        argname = (const char*)malloc(sizeof(char*) * strlen(_argname) + 1);
        strcpy((char*)argname, _argname);        
      }
      else {
        argname = NULL; 
      }

    }
  		
		
    ~OptionDef()
    {
      if(name) {
        free((char*)name);
      }
      
      if(help) {
        free((char*)help);
      }
      
      if(argname) {
        free((char*)argname);
      }
    }
  
    const char *name;
    int flags;
#define HAS_ARG    0x0001
#define OPT_BOOL   0x0002
#define OPT_EXPERT 0x0004
#define OPT_STRING 0x0008
#define OPT_VIDEO  0x0010
#define OPT_AUDIO  0x0020
#define OPT_GRAB   0x0040
#define OPT_INT    0x0080
#define OPT_FLOAT  0x0100
#define OPT_SUBTITLE 0x0200
#define OPT_FUNC2  0x0400
#define OPT_INT64  0x0800
     union {
        //void (CFFmpeg::*func_arg)(const char *); //FIXME passing error code as int return would be nicer then exit() in the func
        func_ptr func_arg;
        int *int_arg;
        char **str_arg;
        float *float_arg;
        //int (*func2_arg)(const char *, const char *);
        func2_ptr func2_arg;
        int64_t *int64_arg;
    } u;
    const char *help;
    const char *argname;
} ;

//void show_help_options(const OptionDef *options, const char *msg, int mask, int value);
void parse_options(int argc, char **argv, OptionDef **options, CFFmpeg* pFFmpeg);
void parse_arg_file(const char *filename, CFFmpeg* pFFmpeg);

#endif /* _CMD_UTILS_H */
