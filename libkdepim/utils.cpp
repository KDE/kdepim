/**
 * utils.cpp
 *
 * Copyright (C) 2007 Laurent Montel <montel@kde.org>
 * Copyright (C) 2008 Jarosław Staniek <staniek@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */

#include "utils.h"

using namespace KPIM;
#include <QString>

QString Utils::rot13(const QString &s)
{
  QString r(s);

  for (int i=0; i<r.length(); i++) {
    if ( ( r[i] >= QLatin1Char('A') && r[i] <= QLatin1Char('M') ) ||
         ( r[i] >= QLatin1Char('a') && r[i] <= QLatin1Char('m') ) ) {
      r[i] = (char)((int)QChar(r[i]).toLatin1() + 13);
    } else {
      if  ( ( r[i] >= QLatin1Char('N') && r[i] <= QLatin1Char('Z') ) ||
            ( r[i] >= QLatin1Char('n') && r[i] <= QLatin1Char('z') ) ) {
        r[i] = (char)((int)QChar(r[i]).toLatin1() - 13);
      }
    }
  }

  return r;
}
