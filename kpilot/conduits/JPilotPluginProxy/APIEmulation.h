/* libplugin.h
 *
 * Copyright (C) 1999 by Judd Montgomery
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __APIEMULATION_H__
#define __APIEMULATION_H__

#include <stdio.h>
#include <stdlib.h>
#include "libplugin.h"



#undef jp_logf

#define MAX_PREF_VALUE 80

#define INTTYPE 1
#define CHARTYPE 2

typedef struct {
   char *name;
   int usertype;
   int filetype;
   long ivalue;
   char *svalue;
   int svalue_size;
} prefType;


int jpilot_logf(int level, char *format, ...);
int jp_logf(int level, char *format, ...);
/* FIXME: Need a policy.  Should all symbols avaliable to
 * plugins start with jp or jpilot?
 */
//#define jp_logf jpilot_logf


// backup, mail, mal:
int get_home_file_name(char *file, char *full_name, int max_size);
// mail:
FILE *jp_open_home_file(char *filename, char *mode);
// backup, mal
int jp_get_pref (prefType prefs[], int which, long *n, const char **ret);
int jp_set_pref (prefType prefs[], int which, long n, const char *string);
// mal
void jp_pref_init(prefType prefs[], int count);
char *pref_lstrncpy_realloc(char **dest, const char *src, int *size, int max_size);
// backup, mal
int jp_pref_read_rc_file(char *filename, prefType prefs[], int num_prefs);
int jp_pref_write_rc_file(char *filename, prefType prefs[], int num_prefs);


/*
// backup:

// expense
dialog_save_changed_record
gdk_color_alloc
jp_charset_p2j

// keyring
// needs -lcrypt
des_ecb3_encrypt
des_set_key
dialog_save_changed_record
gdk_color_alloc
jp_charset_j2p
jp_charset_p2j
MD5

// mail
gdk_color_alloc
get_app_info_size
get_next_unique_pc_id
rename_file

// mal


//libsynctime should work!!!

*/
#endif
