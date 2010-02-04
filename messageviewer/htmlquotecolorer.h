/* Copyright 2009 Thomas McGuire <mcguire@kde.org>

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2 of the License or
   ( at your option ) version 3 or, at the discretion of KDE e.V.
   ( which shall act as a proxy as in section 14 of the GPLv3 ), any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef HTMLQUOTECOLORER_H
#define HTMLQUOTECOLORER_H

#include <QColor>

namespace MessageViewer {

/**
 * Little helper class that takes a HTML source as input and finds all
 * lines that are quoted with '>' or '|'. The HTML text is then modified so
 * that the quoted lines appear in the defined quote colors.
 */
class HTMLQuoteColorer
{
  public:

    HTMLQuoteColorer();

    /**
     * Sets the quote color of the specific leve.
     * This class supports 3 levels, from 0 to 2.
     * Level 2 is the most quoted, with three quote signs in front of the line, and
     * level 0 is the least quoted, with one quote sign in front of the line.
     *
     * If you don't call this, the color of the quoting will be black.
     */
    void setQuoteColor( int level, const QColor &color );

    /**
     * Do the work and add nice colors to the HTML.
     * @param htmlSource the input HTML code
     * @return the modified HTML code, without any <body>, <html> or <head> tags
     */
    QString process( const QString &htmlSource );

  private:

    QColor mQuoteColors[3];
};

}

#endif
