/* APIEmulation.h
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



#undef jp_logf

#include "libplugin.h"

#define PREF_RCFILE 0
#define PREF_TIME 1
#define PREF_SHORTDATE 2
#define PREF_LONGDATE 3
#define PREF_FDOW 4 /*First Day Of the Week */
#define PREF_SHOW_DELETED 5
#define PREF_SHOW_MODIFIED 6
#define PREF_HIDE_COMPLETED 7
#define PREF_HIGHLIGHT 8
#define PREF_PORT 9
#define PREF_RATE 10
#define PREF_USER 11
#define PREF_USER_ID 12
#define PREF_PC_ID 13
#define PREF_NUM_BACKUPS 14
#define PREF_WINDOW_WIDTH 15
#define PREF_WINDOW_HEIGHT 16
#define PREF_DATEBOOK_PANE 17
#define PREF_ADDRESS_PANE 18
#define PREF_TODO_PANE 19
#define PREF_MEMO_PANE 20
#define PREF_USE_DB3 21
#define PREF_LAST_APP 22
#define PREF_PRINT_THIS_MANY 23
#define PREF_PRINT_ONE_PER_PAGE 24
#define PREF_NUM_BLANK_LINES 25
#define PREF_PRINT_COMMAND 26
#define PREF_CHAR_SET 27
#define PREF_SYNC_DATEBOOK 28
#define PREF_SYNC_ADDRESS 29
#define PREF_SYNC_TODO 30
#define PREF_SYNC_MEMO 31
#define PREF_SYNC_MEMO32 32
#define PREF_ADDRESS_NOTEBOOK_PAGE 33
#define PREF_OUTPUT_HEIGHT 34
#define PREF_OPEN_ALARM_WINDOWS 35
#define PREF_DO_ALARM_COMMAND 36
#define PREF_ALARM_COMMAND 37
#define PREF_REMIND_IN 38
#define PREF_REMIND_UNITS 39
#define PREF_PASSWORD 40
#define PREF_MEMO32_MODE 41
#define PREF_PAPER_SIZE 42
#define PREF_DATEBOOK_EXPORT_FILENAME 43
#define PREF_DATEBOOK_IMPORT_PATH 44
#define PREF_ADDRESS_EXPORT_FILENAME 45
#define PREF_ADDRESS_IMPORT_PATH 46
#define PREF_TODO_EXPORT_FILENAME 47
#define PREF_TODO_IMPORT_PATH 48
#define PREF_MEMO_EXPORT_FILENAME 49
#define PREF_MEMO_IMPORT_PATH 50

#define NUM_PREFS 51

#define MAX_PREF_NUM_BACKUPS 99

#define PREF_MDY 0
#define PREF_DMY 1
#define PREF_YMD 2

#define CHAR_SET_ENGLISH  0
#define CHAR_SET_JAPANESE 1
#define CHAR_SET_1250     2 /* Czech */
#define CHAR_SET_1251     3 /* Russian; palm koi8-r, host win1251 */
#define CHAR_SET_1251_B   4 /* Russian; palm win1251, host koi8-r */
#define CHAR_SET_TRADITIONAL_CHINESE  5 /* Taiwan Chinese */
#define CHAR_SET_KOREAN   6 /* Korean Hangul */
#define NUM_CHAR_SETS     7

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
/* FIXME: Need a policy.  Should all symbols available to
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
// keyring,expense
/*************************************
 * convert char code
 *************************************/
#define charset_j2p(buf, max_len, char_set)  {\
	if (char_set == CHAR_SET_JAPANESE) Euc2Sjis(buf, max_len);\
	if (char_set == CHAR_SET_1250) Lat2Win(buf,max_len);\
	if (char_set == CHAR_SET_1251) koi8_to_win1251(buf, max_len);\
	if (char_set == CHAR_SET_1251_B) win1251_to_koi8(buf, max_len);}
#define charset_p2j(buf, max_len, char_set) {\
        if (char_set == CHAR_SET_JAPANESE) Sjis2Euc(buf, max_len);\
        if (char_set == CHAR_SET_1250) Win2Lat(buf,max_len);\
        if (char_set == CHAR_SET_1251) win1251_to_koi8(buf, max_len);\
        if (char_set == CHAR_SET_1251_B) koi8_to_win1251(buf, max_len);}

void jp_charset_p2j(unsigned char *buf, int max_len);
void jp_charset_j2p(unsigned char *buf, int max_len);


/*
// backup:

// expense
dialog_save_changed_record
gdk_color_alloc

// keyring
// needs -lcrypt
des_ecb3_encrypt
des_set_key
dialog_save_changed_record
gdk_color_alloc
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
