/*
    config_data.c

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

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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

/* include "config-chiasmus.h" */
#include "config_data.h"

#include "kleo/cryptoconfig.h" /* for the enum values */

/*
  This data structure uses C99 designated initializers. This is why
  it's in a .c file, and not in a .cpp file. If your compiler doesn't
  like this, just don't compile Chiasmus support, or live without the
  wrapper option :) Sorry.
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
        "path", I18N_NOOP("Path to Chiasmus executable"),
        Level_Basic, ArgType_Path,
        { PATH "/usr/local/bin/chiasmus" }, /* in the absence of C99, we assume path is first in the union here */
        0, 0, 1
    },
    {
        "keydir", I18N_NOOP("Key directory"),
        Level_Basic, ArgType_DirPath,
        { PATH "~/.chiasmus/keys" },
        0, 0, 1 /* FIXME: should be a list */
    },
#ifdef HAVE_C99_INITIALIZERS
    {
        "show-output", I18N_NOOP("Show output from chiasmus operations"),
        Level_Expert, ArgType_None,
        { .boolean = { .value = 0, .numTimesSet = 0 } },
        0, 0, 1
    },
    {
        "symcryptrun-class", I18N_NOOP("SymCryptRun class to use"),
        Level_Expert, ArgType_String,
        { .string = "confucius" },
        0, 0, 1
    },
    {
        "timeout", I18N_NOOP("Timeout in seconds for Chiasmus operations"),
        Level_Advanced, ArgType_UInt,
        { .unsigned_integer = 60 },
        0, 0, 1
    },
#endif /* HAVE_C99_INITIALIZERS */
};
const unsigned int kleo_chiasmus_config_entries_dim
    = sizeof kleo_chiasmus_config_entries / sizeof * kleo_chiasmus_config_entries ;

