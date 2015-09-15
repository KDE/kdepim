/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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
inline const QString tbodyStr()
{
    return QStringLiteral("tbodyStr()");
}

inline const QString tableStr()
{
    return QStringLiteral("table");
}

inline const QString trStr()
{
    return QStringLiteral("tr");
}

inline const QString tdStr()
{
    return QStringLiteral("td");
}

QWebElement TableHelper::tableBodyWebElement(const QWebElement &element)
{
    const QString tagName(element.tagName().toLower());
    if (tagName == tableStr()) {
        QWebElement tableElement = element.firstChild();
        while (!tableElement.isNull()) {
            if (tableElement.tagName().toLower() == tbodyStr()) {
                return tableElement;
            }
            tableElement = tableElement.nextSibling();
        }
        return QWebElement();
    } else if (tagName == tbodyStr()) {
        return element;
    } else {
        QWebElement e = element;
        do {
            e = e.parent();
        } while ((e.tagName().toLower() != tbodyStr()) && !e.isNull());
        return e;
    }
}

QWebElement TableHelper::rowWebElement(const QWebElement &element)
{
    const QString tagName(element.tagName().toLower());
    if (tagName == tableStr()) {
        return QWebElement();
    } else if (tagName == trStr()) {
        return element;
    } else {
        QWebElement e = element;
        do {
            e = e.parent();
        } while ((e.tagName().toLower() != trStr()) && !e.isNull());
        return e;
    }
}

QWebElement TableHelper::tableWebElement(const QWebElement &element)
{
    if (element.tagName().toLower() == tableStr()) {
        return element;
    } else {
        QWebElement e = element;
        do {
            e = e.parent();
        } while (e.tagName().toLower() != tableStr());
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
            if (trElement.tagName().toLower() == trStr()) {
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
            if (trElement.tagName().toLower() == trStr()) {
                QWebElement tdElement = trElement.firstChild();
                int currentNumberColumn = 0;
                while (!tdElement.isNull()) {
                    if (tdElement.tagName().toLower() == tdStr()) {
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
    } while ((e.tagName().toLower() != trStr()) && !e.isNull());

    //TODO
    return 0;
}

void TableHelper::removeCellContentsFromCurrentRow(const QWebElement &element)
{
    QWebElement e = element;
    do {
        e = e.parent();
    } while ((e.tagName().toLower() != trStr()) && !e.isNull());
    if (!e.isNull()) {
        QWebElement cellElement = e.firstChild();
        while (!cellElement.isNull()) {
            if (cellElement.tagName().toLower() == tdStr()) {
                cellElement.setInnerXml(QStringLiteral("<BR>"));
            }
            cellElement = cellElement.nextSibling();
        }
    }
}

int TableHelper::currentCellColSpan(const QWebElement &element)
{
    QWebElement e = element;
    if (e.tagName().toLower() == tdStr()) {
        if (e.hasAttribute(QStringLiteral("colspan"))) {
            return e.attribute(QStringLiteral("colspan")).toInt();
        }
    }
    return -1;
}

int TableHelper::currentCellRowSpan(const QWebElement &element)
{
    QWebElement e = element;
    if (e.tagName().toLower() == tdStr()) {
        if (e.hasAttribute(QStringLiteral("rowspan"))) {
            return e.attribute(QStringLiteral("rowspan")).toInt();
        }
    }
    return -1;
}

}
