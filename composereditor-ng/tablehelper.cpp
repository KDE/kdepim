/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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

#include "tablehelper_p.h"

#include <QDebug>

namespace ComposerEditorNG {
static QString TBODY = QLatin1String("tbody");
static QString TABLE = QLatin1String("table");
static QString ROW = QLatin1String("tr");

QWebElement TableHelper::tableBodyWebElement(const QWebElement&element)
{
    const QString tagName(element.tagName().toLower());
    if (tagName == TABLE) {
        QWebElement tableElement = element.firstChild();
        while (!tableElement.isNull()) {
            if ( tableElement.tagName().toLower() == TBODY ) {
                return tableElement;
            }
            tableElement = tableElement.nextSibling();
        }
        return QWebElement();
    } else if (tagName == TBODY) {
        return element;
    } else {
        QWebElement e = element;
        do {
            e = e.parent();
        } while( (e.tagName().toLower() != TBODY) && !e.isNull() );
        return e;
    }
}

QWebElement TableHelper::rowWebElement(const QWebElement&element)
{
    const QString tagName(element.tagName().toLower());
    if (tagName == TABLE) {
        return QWebElement();
    } else if (tagName == ROW) {
        return element;
    } else {
        QWebElement e = element;
        do {
            e = e.parent();
        } while( (e.tagName().toLower() != ROW) && !e.isNull() );
        return e;
    }
    return QWebElement();
}

QWebElement TableHelper::tableWebElement(const QWebElement&element)
{
    if(element.tagName().toLower() == TABLE) {
        return element;
    } else {
        QWebElement e = element;
        do {
            e = e.parent();
        } while(e.tagName().toLower() != TABLE);
        return e;
    }
}


}
