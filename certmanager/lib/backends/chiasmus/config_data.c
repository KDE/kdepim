/*
    chiasmusbackend.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2005 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "config_data.h"

#include "kleo/cryptoconfig.h" /* for the enum values */

/*
  This data structure uses C99 designated initializers. This is why
  it's in a .c file, and not in a .cpp file. If your compiler doesn't
  like this, just don't compile Chiasmus support :) Sorry.
*/

#ifdef PATH
# undef PATH
#endif 
#ifdef HAVE_C99_INITIALIZERS
# define PATH .path =
#else
# define PATH
#endif

#define I18N_NOOP(x) (x)

const struct kleo_chiasmus_config_data kleo_chiasmus_config_entries[] = {
  {
    "path", I18N_NOOP( "Path to executable" ),
    Level_Basic, ArgType_Path,
    { PATH "/usr/local/bin/chiasmus" }, /* in the absence of C99, we assume path is first in the union here */
    0, 0, 1
  },
  {
    "keydir", I18N_NOOP( "Key directory" ),
    Level_Basic, ArgType_Path,
    { PATH "~/.chiasmus/keys" },
    0, 0, 1
  },
#ifdef HAVE_C99_INITIALIZERS
  {
    "booltest", "Bool test",
    Level_Expert, ArgType_None,
    { .boolean = { 1, 31 } },
    0, 0, 0
  },
  {
    "inttest", "Int test",
    Level_Expert, ArgType_Int,
    { .integer = 111 },
    0, 0, 0
  },
  { "uinttest", "UInt test",
    Level_Expert, ArgType_UInt,
    { .unsigned_integer = 112 },
    0, 0, 0
  },
  { "stringtest", "String test",
    Level_Expert, ArgType_String,
    { .string = "Hello World!" },
    0, 0, 0
  },
#endif /* HAVE_C99_INITIALIZERS */
};
const unsigned int kleo_chiasmus_config_entries_dim
  = sizeof kleo_chiasmus_config_entries / sizeof *kleo_chiasmus_config_entries ;

