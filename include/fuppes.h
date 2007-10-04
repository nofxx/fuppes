/***************************************************************************
 *            fuppes.h
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
 
#ifndef _FUPPES_H
#define _FUPPES_H

#ifdef __cplusplus
extern "C" {
#endif

#define FUPPES_FALSE 0
#define FUPPES_TRUE  1

/**
 *  initialize libfuppes
 *    - load and check config
 *    - check transcoding
 *    - init winsocks
 *    - init external libs (e.g. imagemagick)
 *  @return returns FUPPES_OK on success otherwise FUPPES_FALSE
 */
int fuppes_init(int argc, char* argv[], void(*p_log_cb)(const char* sz_log));

/**
 *  set callback for error messages
 */
void fuppes_set_error_callback(void(*p_err_cb)(const char* sz_err));

/**
 *  set callback for notify messages
 */
void fuppes_set_notify_callback(void(*p_notify_cb)(const char* sz_title, const char* sz_msg));
  
/**
 *  set callback for user input messages
 */
void fuppes_set_user_input_callback(void(*p_user_input_cb)(const char* sz_msg, char* sz_result, unsigned int n_buffer_size));
  
/**
 *  start fuppes
 *  @return returns FUPPES_OK on success otherwise FUPPES_FALSE
 */
int fuppes_start();

/**
 *  stop fuppes
 *  @return returns FUPPES_OK on success otherwise FUPPES_FALSE
 */
int fuppes_stop();

/**
 *  cleanup libfuppes
 *    uninitializes all the stuff initialized by "fuppes_init()"
 *  @return returns FUPPES_OK on success otherwise FUPPES_FALSE
 */
int fuppes_cleanup();


int fuppes_is_started();

const char* fuppes_get_version();

void fuppes_print_info();
void fuppes_print_device_settings();
  
void fuppes_set_loglevel(int n_log_level);
void fuppes_inc_loglevel();
  
void fuppes_rebuild_db();
void fuppes_update_db();
void fuppes_update_db_add_new();
void fuppes_update_db_remove_missing();
  
void fuppes_rebuild_vcontainers();

void fuppes_get_http_server_address(char* sz_addr, int n_buff_size);

void fuppes_send_alive();
void fuppes_send_byebye();
void fuppes_send_msearch();

#ifdef __cplusplus
}
#endif
#endif // _FUPPES_H
