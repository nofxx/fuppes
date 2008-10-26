/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            fuppes_plugin.h
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

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _FUPPES_PLUGIN_H
#define _FUPPES_PLUGIN_H
	
#include <string.h>
#include <stdlib.h>
#include "fuppes_plugin_types.h"

#include <stdio.h>
	
inline int set_value(char** out, const char* in)
{	
	if(strlen(in) == 0) {
		return 1;
	}
	
	*out = (char*)realloc(*out, sizeof(char) * (strlen(in) + 1));	
	strcpy(*out, in);
  return 0;
}
	
//static void register_fuppes_plugin(plugin_info* info) = 0;

/*static inline void add_extension(file_ext* info, char* ext)
{
	file_ext* tmp  = info->extension;
	file_ext* next = NULL;
	
	if(tmp) {
		next = (file_ext*)tmp->next;
	}
	else {
		tmp = (file_ext*)malloc(sizeof(file_ext));
		tmp->next = NULL;
		next = tmp;
	}
	
	while(next != NULL) {
		tmp = next;
		next = (file_ext*)next->next;
	}
	
	strcpy(tmp->ext, ext);
	tmp->next = NULL;	
}*/

#endif //_FUPPES_PLUGIN_H

#ifdef __cplusplus
}
#endif
