/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNODE_OPTIONS_H
#define KNODE_OPTIONS_H

#include <kcmdlineargs.h>
#include <klocale.h>

/** KNode's command line options. */
static KCmdLineOptions knode_options ()
{
  KCmdLineOptions options;
  options.add("+[url]", ki18n("A 'news://server/group' URL"));
  return options;
}

#endif /* KNODE_OPTIONS_H */

