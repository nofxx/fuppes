/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            decoder_twolame.c
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005-2008 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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


#ifdef __cplusplus
extern "C" {
#endif
		
#include <string.h>
#include <twolame.h>

#define TWOLAME_MAX_BUFFER 32768
		
void register_fuppes_plugin(plugin_info* plugin)
{
	strcpy(plugin->plugin_name, "twolame");
	plugin->plugin_type = PT_AUDIO_ENCODER;
}

void unregister_fuppes_plugin(plugin_info* info)
{
}

#ifdef __cplusplus
}
#endif

