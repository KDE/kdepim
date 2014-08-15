/*
* Copyright 2011 Lamarque Souza <lamarque@kde.org>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301  USA
*/

#ifndef KMAIL_MOBILE_OPTIONS_H
#define KMAIL_MOBILE_OPTIONS_H

#include <kcmdlineargs.h>
#include <KLocalizedString>

static KCmdLineOptions kmailMobileOptions()
{
  KCmdLineOptions options;
  options.add("t <address>",    ki18n("Send message to 'address'"));
  options.add("s <subject>",    ki18n("Set subject of message"));
  options.add("c <address>",    ki18n("Send CC: to 'address'"));
  options.add("b <address>",    ki18n("Send BCC: to 'address'"));
  options.add("B <text>",       ki18n("Set body of message"));
  options.add("A <url>",        ki18n("Add an attachment to the mail. This can be repeated"));
  return options;
}

#endif
