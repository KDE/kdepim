/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include <KLocalizedString>

#include <QStringList>
using namespace KSieveUi;

QString AutoCreateScriptUtil::createMultiLine(const QString &str)
{
    const QString result = QStringLiteral("\n%1\n.\n;\n").arg(str);
    return result;
}

QString AutoCreateScriptUtil::createList(const QString &str, QChar separator, bool addEndSemiColon)
{
    const QStringList list = str.trimmed().split(separator);
    const int count = list.count();
    switch (count) {
    case 0:
        return QString();
    case 1:
        return QLatin1String("\"") + list.first() + QLatin1String("\"");
    default: {
        const QString result = createList(list, addEndSemiColon);
        return result;
    }
    }
}

QString AutoCreateScriptUtil::quoteStr(QString str)
{
    return str.replace(QLatin1String("\""), QStringLiteral("\\\""));
}

QString AutoCreateScriptUtil::createList(const QStringList &lst, bool addSemiColon, bool protectSlash)
{
    QString result;
    result = QStringLiteral("[");
    bool wasFirst = true;
    Q_FOREACH (QString str, lst) {
        if (protectSlash) {
            str = str.replace(QLatin1Char('\\'), QStringLiteral("\\\\"));
        }
        result += (wasFirst ? QString() : QStringLiteral(",")) + QStringLiteral(" \"%1\"").arg(quoteStr(str));
        wasFirst = false;
    }
    result += QLatin1String(" ]");
    if (addSemiColon) {
        result += QLatin1Char(';');
    }

    return result;
}

QStringList AutoCreateScriptUtil::createListFromString(QString str)
{
    QStringList lst;
    if (str.startsWith(QLatin1Char('[')) && str.endsWith(QLatin1String("];"))) {
        str.remove(0, 1);
        str.remove(str.length() - 2, 2);
    } else if (str.startsWith(QLatin1Char('[')) && str.endsWith(QLatin1String("]"))) {
        str.remove(0, 1);
        str.remove(str.length() - 1, 1);
    } else {
        return lst;
    }
    lst = str.split(QStringLiteral(", "));
    QStringList resultLst;
    Q_FOREACH (QString s, lst) {
        s.remove(QStringLiteral("\""));
        resultLst << s.trimmed();
    }
    lst = resultLst;
    return lst;
}

QString AutoCreateScriptUtil::createAddressList(const QString &str, bool addSemiColon)
{
    if (str.trimmed().startsWith(QLatin1Char('[')) && str.trimmed().endsWith(QLatin1Char(']'))) {
        return str;
    }
    return createList(str, QLatin1Char(';'), addSemiColon);
}

QString AutoCreateScriptUtil::negativeString(bool isNegative)
{
    return (isNegative ? QStringLiteral("not ") : QString());
}

QString AutoCreateScriptUtil::tagValueWithCondition(const QString &tag, bool notCondition)
{
    return (notCondition ? QStringLiteral("[NOT]") : QString()) + QLatin1Char(':') + tag;
}

QString AutoCreateScriptUtil::tagValue(const QString &tag)
{
    return QLatin1Char(':') + tag;
}

QString AutoCreateScriptUtil::strValue(QDomNode &node)
{
    node = node.nextSibling();
    QDomElement textElement = node.toElement();
    if (!textElement.isNull()) {
        const QString textElementTagName = textElement.tagName();
        if (textElementTagName == QLatin1String("str")) {
            return textElement.text();
        }
    }
    return QString();
}

QString AutoCreateScriptUtil::listValueToStr(const QDomElement &element)
{
    const QStringList lst = AutoCreateScriptUtil::listValue(element);
    //Don't add semicolon
    return createList(lst, false);
}

QStringList AutoCreateScriptUtil::listValue(const QDomElement &element)
{
    QStringList lst;
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("str")) {
                lst << e.text();
            }
        }
        node = node.nextSibling();
    }
    return lst;
}

QString AutoCreateScriptUtil::fixListValue(QString valueStr)
{
    if (!(valueStr.startsWith(QLatin1Char('[')) && valueStr.endsWith(QLatin1Char(']')))) {
        valueStr = QStringLiteral("\"%1\"").arg(valueStr);
    }

    return valueStr;
}

void AutoCreateScriptUtil::comboboxItemNotFound(const QString &searchItem, const QString &name, QString &error)
{
    error += i18n("Cannot find item \"%1\" in widget \"%2\"", searchItem, name);
}

QString AutoCreateScriptUtil::createFullWhatsThis(const QString &help, const QString &href)
{
    if (href.isEmpty()) {
        return help;
    }
    const QString fullWhatsThis = QLatin1String("<qt>") + help + QStringLiteral("<br><a href=\'%1\'>%2</a></qt>").arg(href).arg(i18n("More information"));
    return fullWhatsThis;
}
