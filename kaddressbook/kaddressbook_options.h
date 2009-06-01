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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef KADDRESSBOOK_OPTIONS_H
#define KADDRESSBOOK_OPTIONS_H

#include <kcmdlineargs.h>
#include <klocale.h>

static KCmdLineOptions kaddressbook_options ()
{
  KCmdLineOptions options;
  options.add( "a" );
  options.add( "addr <email>", ki18n( "Shows contact editor with given email address" ) );
  options.add( "uid <uid>", ki18n( "Shows contact editor with given uid" ) );
  options.add( "editor-only", ki18n( "Launches in editor only mode" ) );
  options.add( "new-contact", ki18n( "Launches editor for the new contact" ) );
  options.add( "document <file>", ki18n( "Work on given file" ) );
  options.add( "+[URL]", ki18n( "Import the given vCard" ) );
  return options;
}

#endif /* KADDRESSBOOK_OPTIONS_H */

