/*
    This file is part of KAddressBook.
    Copyright (C) 1999 Don Sanders <sanders@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef KADDRESSBOOK_OPTIONS_H
#define KADDRESSBOOK_OPTIONS_H

#include <kcmdlineargs.h>
#include <klocale.h>

static KCmdLineOptions kaddressbook_options[] =
{
  { "a", 0 , 0 },
  { "addr <email>", I18N_NOOP( "Shows contact editor with given email address" ), 0 },
  { "uid <uid>", I18N_NOOP( "Shows contact editor with given uid" ), 0 },
  { "editor-only", I18N_NOOP( "Launches in editor only mode" ), 0 },
  { "new-contact", I18N_NOOP( "Launches editor for the new contact" ), 0 },
  { "+[URL]", I18N_NOOP( "Import the given vCard" ), 0 },
  KCmdLineLastOption
};

#endif /* KADDRESSBOOK_OPTIONS_H */

