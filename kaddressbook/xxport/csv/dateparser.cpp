/*
  This file is part of KAddressBook.
  Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "dateparser.h"

DateParser::DateParser(const QString &pattern)
    : mPattern(pattern)
{
}

DateParser::~DateParser()
{
}

QDateTime DateParser::parse(const QString &dateStr) const
{
    int year, month, day, hour, minute, second;
    year = month = day = hour = minute = second = 0;

    int currPos = 0;
    for (int i = 0; i < mPattern.length(); ++i) {
        if (mPattern[ i ] == QLatin1Char('y')) {   // 19YY
            if (currPos + 1 < dateStr.length()) {
                year = 1900 + dateStr.midRef(currPos, 2).toInt();
                currPos += 2;
            } else {
                return QDateTime();
            }
        } else if (mPattern[ i ] == QLatin1Char('Y')) {   // YYYY
            if (currPos + 3 < dateStr.length()) {
                year = dateStr.midRef(currPos, 4).toInt();
                currPos += 4;
            } else {
                return QDateTime();
            }
        } else if (mPattern[ i ] == QLatin1Char('m')) {   // M or MM
            if (currPos + 1 < dateStr.length()) {
                if (dateStr[ currPos ].isDigit()) {
                    if (dateStr[ currPos + 1 ].isDigit()) {
                        month = dateStr.midRef(currPos, 2).toInt();
                        currPos += 2;
                        continue;
                    }
                }
            }
            if (currPos < dateStr.length()) {
                if (dateStr[ currPos ].isDigit()) {
                    month = dateStr.midRef(currPos, 1).toInt();
                    currPos++;
                    continue;
                }
            }

            return QDateTime();
        } else if (mPattern[ i ] == QLatin1Char('M')) {   // 0M or MM
            if (currPos + 1 < dateStr.length()) {
                month = dateStr.midRef(currPos, 2).toInt();
                currPos += 2;
            } else {
                return QDateTime();
            }
        } else if (mPattern[ i ] == QLatin1Char('d')) {   // D or DD
            if (currPos + 1 < dateStr.length()) {
                if (dateStr[ currPos ].isDigit()) {
                    if (dateStr[ currPos + 1 ].isDigit()) {
                        day = dateStr.midRef(currPos, 2).toInt();
                        currPos += 2;
                        continue;
                    }
                }
            }
            if (currPos < dateStr.length()) {
                if (dateStr[ currPos ].isDigit()) {
                    day = dateStr.midRef(currPos, 1).toInt();
                    currPos++;
                    continue;
                }
            }

            return QDateTime();
        } else if (mPattern[ i ] == QLatin1Char('D')) {   // 0D or DD
            if (currPos + 1 < dateStr.length()) {
                day = dateStr.midRef(currPos, 2).toInt();
                currPos += 2;
            } else {
                return QDateTime();
            }
        } else if (mPattern[ i ] == QLatin1Char('H')) {   // 0H or HH
            if (currPos + 1 < dateStr.length()) {
                hour = dateStr.midRef(currPos, 2).toInt();
                currPos += 2;
            } else {
                return QDateTime();
            }
        } else if (mPattern[ i ] == QLatin1Char('I')) {   // 0I or II
            if (currPos + 1 < dateStr.length()) {
                minute = dateStr.midRef(currPos, 2).toInt();
                currPos += 2;
            } else {
                return QDateTime();
            }
        } else if (mPattern[ i ] == QLatin1Char('S')) {   // 0S or SS
            if (currPos + 1 < dateStr.length()) {
                second = dateStr.midRef(currPos, 2).toInt();
                currPos += 2;
            } else {
                return QDateTime();
            }
        } else {
            currPos++;
        }
    }

    return QDateTime(QDate(year, month, day), QTime(hour, minute, second));
}
