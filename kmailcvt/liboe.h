/***************************************************************************
                          liboe.h  -  description
                             -------------------
    begin                : Thu Aug 24 2000
    copyright            : (C) 2000 by Hans Dijkema
    email                : kmailcvt@hum.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/* LIBOE 0.92 - UNSTABLE - EXPERIMENTAL OE4 SUPPORT
   - EXPERIMENTAL WINDOWS SOURCE COMPATIBILITY
   Copyright (C) 2000 Stephan B. Nedregård (stephan@micropop.com) */

/*  This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <sys/stat.h>

#ifndef LIBOE_H
#define LIBOE_H

#define OE_CANNOTREAD 1
#define OE_NOTOEBOX   2
#define OE_POSITION   3
#define OE_NOBODY     4

struct oe_list { /* Internal use only */
  fpos_t pos;
  struct oe_list *next;
};
typedef struct oe_list oe_list;

struct oe_internaldata{
  void (*oput)(char*,int newmsg /* =1, new message begins, =2, msg continues =0, msg ends */);
  FILE *oe;
  oe_list *used;
  int success, justheaders, failure;
  int errcode;
  struct stat *stat;
};
typedef struct oe_internaldata oe_data;

/* ---------------------------------------------------- */
   oe_data* oe_readbox(char* filename,void (*writeit)(const char*,int));
/* ---------------------------------------------------- 
   filename - filename of OE box (e.g. "Inbox.dbx")
   writeit  - function to get Unix mailbox feed from liboe
              e.g. void getmailboxstream(char*).

   Writeit receives a mailbox stream from liboe, can be used
   in clever ways to disguise the fact that the mailbox is 
   really an OE mailbox so that standard mailbox parsing
   functions can be used for these as well. 

   Return values: NULL if no stream returned, else a structure
   of type struct oe_internaldata (see above) where *oe and *used
   are always NULL on return.

   Stream (writeit): Returns one line per call.

   ---------------------------------------------------- */


/* ---------------------------------------------------- */
   oe_data* oe_readmbox(char* filename,void (*oput)(char*,int));
/* ---------------------------------------------------- 
   Basically the same as oe_readbox, but reads a standard
   mailbox instead. 
   ---------------------------------------------------- */

#endif


