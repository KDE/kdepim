#ifndef _KPILOT_FAKES_H
#define _KPILOT_FAKES_H

/* This file is part of the KDE libraries
   Copyright (c) 2000 The KDE Project

   unsetenv() taken from the GNU C Library.
   Copyright (C) 1992,1995-1999,2000-2002 Free Software Foundation, Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HAVE_SETENV
int setenv(const char *name, const char *value, int overwrite);
#endif /* !HAVE_SETENV */

#ifndef HAVE_UNSETENV
void unsetenv (const char *name);
#endif /* !HAVE_UNSETENV */

#ifndef HAVE_USLEEP
void usleep(unsigned int usec);
#endif /* !HAVE_USLEEP */

#ifndef HAVE_RANDOM
long int random();
void srandom(unsigned int seed);
#endif

#ifndef HAVE_SETEUID
int seteuid(uid_t euid);
#endif

#ifndef HAVE_MKSTEMPS
int mkstemps (char* _template, int suffix_len);
#endif /* !HAVE_MKSTEMPS */

#ifndef HAVE_MKSTEMP
int mkstemp (char* _template);
#endif

#ifndef HAVE_MKDTEMP
char* mkdtemp (char* _template);
#endif /* !HAVE_MKDTEMP */

#ifndef HAVE_REVOKE
int revoke(const char *tty);
#endif

#ifndef HAVE_STRLCPY
unsigned long strlcpy(char* d, const char* s, unsigned long bufsize);
#endif

#ifndef HAVE_STRLCAT
unsigned long strlcat(char* d, const char* s, unsigned long bufsize);
#endif

#ifdef __cplusplus
}
#endif


#endif
