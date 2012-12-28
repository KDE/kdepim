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

#include "tablehelper.h"

namespace ComposerEditorNG {

QWebElement TableHelper::tableBodyWebElement(const QWebElement&element)
{
    const QString tagName(element.tagName().toLower());
    if (tagName == QLatin1String("table")) {
        QWebElement tableElement = element.firstChild();
        while (!tableElement.isNull()) {
            if ( tableElement.tagName().toLower() == QLatin1String("tbody") ) {
                return tableElement;
            }
            tableElement = tableElement.nextSibling();
        }
        return QWebElement();
    } else if (tagName == QLatin1String("tbody")) {
        return element;
    } else {
        QWebElement e = element;
        do {
            e = e.parent();
        } while( (e.tagName().toLower() != QLatin1String("tdbody")) && !e.isNull() );
        return e;
    }
}

}
