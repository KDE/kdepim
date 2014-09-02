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

#include "tablehelper_p.h"

namespace ComposerEditorNG
{
static QString TBODY = QLatin1String("tbody");
static QString TABLE = QLatin1String("table");
static QString ROW = QLatin1String("tr");
static QString CELL = QLatin1String("td");

QWebElement TableHelper::tableBodyWebElement(const QWebElement &element)
{
    const QString tagName(element.tagName().toLower());
    if (tagName == TABLE) {
        QWebElement tableElement = element.firstChild();
        while (!tableElement.isNull()) {
            if (tableElement.tagName().toLower() == TBODY) {
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
        } while ((e.tagName().toLower() != TBODY) && !e.isNull());
        return e;
    }
}

QWebElement TableHelper::rowWebElement(const QWebElement &element)
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
        } while ((e.tagName().toLower() != ROW) && !e.isNull());
        return e;
    }
}

QWebElement TableHelper::tableWebElement(const QWebElement &element)
{
    if (element.tagName().toLower() == TABLE) {
        return element;
    } else {
        QWebElement e = element;
        do {
            e = e.parent();
        } while (e.tagName().toLower() != TABLE);
        return e;
    }
}

QWebElement TableHelper::nextCell(const QWebElement &element)
{
    QWebElement cellElement = element.nextSibling();
    //Next cell
    if (!cellElement.isNull()) {
        return cellElement;
    }
    QWebElement parentElement = element.parent();
    QWebElement firstElement = parentElement.firstChild();
    while (!firstElement.isNull()) {
        if (firstElement == element) {

        }
        firstElement = firstElement.nextSibling();
    }

    //TODO
    return QWebElement();
}

QWebElement TableHelper::previousCell(const QWebElement &element)
{
    QWebElement cellElement = element.previousSibling();
    //previous cell
    if (!cellElement.isNull()) {
        return cellElement;
    }
    //TODO
    return QWebElement();
}

int TableHelper::tableRowCount(const QWebElement &element)
{
    int numberOfRow = 0;
    QWebElement tableBodyElement = tableBodyWebElement(element);
    if (!tableBodyElement.isNull()) {
        //Search all TR
        QWebElement trElement = tableBodyElement.firstChild();
        while (!trElement.isNull()) {
            if (trElement.tagName().toLower() == ROW) {
                numberOfRow++;
            }
            trElement = trElement.nextSibling();
        }
    }
    return numberOfRow;
}

int TableHelper::tableColumnCount(const QWebElement &element)
{
    int numberColumn = 0;
    QWebElement tableBodyElement = tableBodyWebElement(element);
    if (!tableBodyElement.isNull()) {
        //Search all TR
        QWebElement trElement = tableBodyElement.firstChild();
        while (!trElement.isNull()) {
            if (trElement.tagName().toLower() == ROW) {
                QWebElement tdElement = trElement.firstChild();
                int currentNumberColumn = 0;
                while (!tdElement.isNull()) {
                    if (tdElement.tagName().toLower() == CELL) {
                        currentNumberColumn++;
                    }
                    tdElement = tdElement.nextSibling();
                }
                numberColumn = qMax(numberColumn, currentNumberColumn);
            }
            trElement = trElement.nextSibling();
        }
    }
    return numberColumn;
}

int TableHelper::currentColumn(const QWebElement &element)
{
    QWebElement e = element;
    do {
        e = e.parent();
    } while ((e.tagName().toLower() != ROW) && !e.isNull());

    //TODO
    return 0;
}

void TableHelper::removeCellContentsFromCurrentRow(const QWebElement &element)
{
    QWebElement e = element;
    do {
        e = e.parent();
    } while ((e.tagName().toLower() != ROW) && !e.isNull());
    if (!e.isNull()) {
        QWebElement cellElement = e.firstChild();
        while (!cellElement.isNull()) {
            if (cellElement.tagName().toLower() == CELL) {
                cellElement.setInnerXml(QString::fromLatin1("<BR>"));
            }
            cellElement = cellElement.nextSibling();
        }
    }
}

int TableHelper::currentCellColSpan(const QWebElement &element)
{
    QWebElement e = element;
    if (e.tagName().toLower() == CELL) {
        if (e.hasAttribute(QLatin1String("colspan"))) {
            return e.attribute(QLatin1String("colspan")).toInt();
        }
    }
    return -1;
}

int TableHelper::currentCellRowSpan(const QWebElement &element)
{
    QWebElement e = element;
    if (e.tagName().toLower() == CELL) {
        if (e.hasAttribute(QLatin1String("rowspan"))) {
            return e.attribute(QLatin1String("rowspan")).toInt();
        }
    }
    return -1;
}

}
