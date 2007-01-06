/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "legacydecrypt.h"

using namespace KPIM;

QString Legacy::decryptKMail(const QString & data)
{
  QString result;
  for ( int i = 0; i < data.length(); ++i )
    result += (data[i].unicode() < 0x20) ? data[i] :
      QChar(0x1001F - data[i].unicode());
  return result;
}

QString Legacy::decryptKNode(const QString & data)
{
  uint i, val, len = data.length();
  QString result;

  for ( i = 0; i < len; ++i )
  {
    val = data[i].toLatin1();
    val -= ' ';
    val = (255-' ') - val;
    result += (char)(val + ' ');
  }

  return result;
}
