/*
   RFC 2060 (IMAP4rev1) client.

   Copyright (C) 2000 Rik Hemsley (rikkus) <rik@kde.org>

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
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/ 

#include <qtextstream.h>
#include <qfile.h>

#include "IMAPClient.h"

namespace IMAP {

static QFile * logfile = 0;

void log(const QString & s)
{
  if (0 == logfile)
    return;

  if (!logfile->isOpen()) {
    logfile->setName("log");
    if (!logfile->open(IO_WriteOnly))
      qDebug("Couldn't open logfile");
  }

  QTextStream t(logfile);
  t << s;
  logfile->flush();
}

  QString
flagsString(ulong flags)
{
  QString s;

  if (flags & Seen)
    s += "\\Seen ";
  if (flags & Answered)
    s += "\\Answered ";
  if (flags & Flagged)
    s += "\\Flagged ";
  if (flags & Deleted)
    s += "\\Deleted ";
  if (flags & Draft)
    s += "\\Draft ";
  if (flags & Recent)
    s += "\\Recent ";

  if (s[s.length() - 1] == ' ')
    s.truncate(s.length() - 1);

  return s;
}

} // end namespace IMAP
