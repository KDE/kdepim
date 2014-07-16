/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef KADDRESSBOOK_OPTIONS_H
#define KADDRESSBOOK_OPTIONS_H

#include <kcmdlineargs.h>
#include <klocale.h>

static KCmdLineOptions kaddressbook_options()
{
  KCmdLineOptions options;
  options.add("import",                ki18n("Import the given file" ));
  options.add( "+[urls]",
               ki18n( "files or urls. "
                      "The user will be asked whether to import." ) );

  return options;
}

#endif // KADDRESSBOOK_OPTIONS_H
