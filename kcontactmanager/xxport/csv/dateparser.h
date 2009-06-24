/*
    This file is part of KContactManager.
    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

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
*/

#ifndef DATEPARSER_H
#define DATEPARSER_H

#include <QtCore/QDate>
#include <QtCore/QString>

/**
  This class parses the date out of a given string with the
  help of a pattern.
  The pattern can contains the following key characters:
    y = year (e.g. 82)
    Y = year (e.g. 1982)
    m = month (e.g. 7, 07 or 12)
    M = month (e.g. 07 or 12)
    d = day (e.g. 3, 03 or 17)
    D = day (e.g. 03 or 17 )
 */
class DateParser
{
  public:
    DateParser( const QString &pattern );
    ~DateParser();

    QDate parse( const QString &dateStr ) const;

  private:
    QString mPattern;
};

#endif
