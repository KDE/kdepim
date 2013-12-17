/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#ifndef AUTOCREATESCRIPTUTIL_H
#define AUTOCREATESCRIPTUTIL_H

#include <QString>
#include <QStringList>
#include <QDomNode>

namespace AutoCreateScriptUtil
{
QString createMultiLine(const QString &str);
QString createList(const QString &str, const QChar &separator, bool addEndSemiColon = true);
QString createList(const QStringList &lst, bool addSemiColon = true, bool protectSlash = false);
QStringList createListFromString(QString str);
QString createAddressList(const QString &str, bool addSemiColon = true);
QString negativeString(bool isNegative);
QString tagValueWithCondition(const QString &tag, bool notCondition);
QString tagValue(const QString &tag);
QString strValue(QDomNode &node);
QStringList listValue(const QDomElement &element);
QString listValueToStr(const QDomElement &element);
QString fixListValue(QString valueStr);
QString quoteStr(QString str);
void comboboxItemNotFound(const QString &searchItem, const QString &name, QString &error);
QString createFullWhatsThis(const QString &help, const QString &href);
}

#endif // AUTOCREATESCRIPTUTIL_H
