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

#include "autocreatescriptutil_p.h"
#include <QStringList>
#include <QDebug>

QString AutoCreateScriptUtil::createMultiLine(const QString &str)
{
    const QString result = QString::fromLatin1("\n %1\n.\n;\n").arg(str);
    return result;
}

QString AutoCreateScriptUtil::createList(const QString &str, const QChar &separator)
{
    const QStringList list = str.trimmed().split(separator);
    const int count = list.count();
    switch(count) {
    case 0:
        return QString();
    case 1:
        return QLatin1String("\"") + list.first() + QLatin1String("\"");
    default: {
        const QString result = createList(list);
        return result;
    }
    }
}

QString AutoCreateScriptUtil::createList(const QStringList &lst)
{
    QString result;
    result = QLatin1String("[");
    bool wasFirst = true;
    Q_FOREACH (const QString &str, lst) {
        result += (wasFirst ? QString() : QLatin1String(",")) + QString::fromLatin1(" \"%1\"").arg(str);
        wasFirst = false;
    }
    result += QLatin1String(" ];");

    return result;
}

QStringList AutoCreateScriptUtil::createListFromString(QString str)
{
    QStringList lst;
    if (str.startsWith(QLatin1Char('[')) && str.endsWith(QLatin1String("];"))) {
        str.remove(0,1);
        str.remove(str.length()-2, 2);
    } else if (str.startsWith(QLatin1Char('[')) && str.endsWith(QLatin1String("]"))) {
        str.remove(0,1);
        str.remove(str.length()-1, 1);
    } else {
        return lst;
    }
    lst = str.split(QLatin1String(", "));
    QStringList resultLst;
    Q_FOREACH (QString s, lst) {
        s.remove(QLatin1String("\""));
        resultLst<<s.trimmed();
    }
    lst = resultLst;
    return lst;
}

QString AutoCreateScriptUtil::createAddressList(const QString &str)
{
    return createList(str, QLatin1Char(';'));
}


QString AutoCreateScriptUtil::negativeString(bool isNegative)
{
    return (isNegative ? QLatin1String("not ") : QString());
}
