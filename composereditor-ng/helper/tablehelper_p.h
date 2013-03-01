/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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
#ifndef TABLEHELPER_H
#define TABLEHELPER_H

#include <QWebElement>

namespace ComposerEditorNG {

namespace TableHelper {
QWebElement tableBodyWebElement(const QWebElement&element);
QWebElement rowWebElement(const QWebElement&element);
QWebElement tableWebElement(const QWebElement&element);

int tableRowCount(const QWebElement &element);

int tableColumnCount(const QWebElement &element);

QWebElement previousCell(const QWebElement& element);
QWebElement nextCell(const QWebElement& element);

void removeCellContentsFromCurrentRow(const QWebElement& element);

int currentColumn(const QWebElement& element);

int currentCellColSpan(const QWebElement& element);
int currentCellRowSpan(const QWebElement& element);

}
}

#endif // TABLEHELPER_H
