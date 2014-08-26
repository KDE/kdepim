/*
  Copyright (c) 2012, 2013 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "xmlprintingscriptbuilder.h"
#include <ksieve/parser.h>
using KSieve::Parser;

#include <ksieve/error.h>
#include <QDebug>

using namespace KSieveUi;
XMLPrintingScriptBuilder::XMLPrintingScriptBuilder()
    : KSieve::ScriptBuilder(),
      mIsAction(false)
{
    write(QLatin1String("<?xml version='1.0'?>"));
    write(QLatin1String("<script>"));
}

XMLPrintingScriptBuilder::~XMLPrintingScriptBuilder()
{
}

void XMLPrintingScriptBuilder::taggedArgument(const QString &tag)
{
    write(QLatin1String("tag"), tag);
}

void XMLPrintingScriptBuilder::stringArgument(const QString &string, bool multiLine, const QString & /*fixme*/)
{
    write(QLatin1String("str") , multiLine ? QLatin1String("type=\"multiline\"") : QLatin1String("type=\"quoted\""), string);
}

void XMLPrintingScriptBuilder::numberArgument(unsigned long number, char quantifier)
{
    write(QLatin1String("num"), (quantifier ? QString::fromLatin1("quantifier=\"%1\"").arg(quantifier) : QString()) , QString::number(number));
}

void XMLPrintingScriptBuilder::commandStart(const QString &identifier)
{
    if (identifier == QLatin1String("else") ||
            identifier == QLatin1String("break") ||
            identifier == QLatin1String("require") ||
            identifier == QLatin1String("foreverypart") ||
            identifier == QLatin1String("if") ||
            identifier == QLatin1String("elsif")) {
        write(QString::fromLatin1("<control name=\"%1\">").arg(identifier));
        mIsAction = false;
    } else {
        write(QString::fromLatin1("<action name=\"%1\">").arg(identifier));
        mIsAction = true;
    }
}

void XMLPrintingScriptBuilder::commandEnd()
{
    if (mIsAction) {
        write(QLatin1String("</action>"));
    } else {
        write(QLatin1String("</control>"));
    }
    mIsAction = false;
}

void XMLPrintingScriptBuilder::testStart(const QString &identifier)
{
    write(QString::fromLatin1("<test name=\"%1\">").arg(identifier));
}

void XMLPrintingScriptBuilder::testEnd()
{
    write(QLatin1String("</test>"));
}

void XMLPrintingScriptBuilder::testListStart()
{
    write(QLatin1String("<testlist>"));
}

void XMLPrintingScriptBuilder::testListEnd()
{
    write(QLatin1String("</testlist>"));
}

void XMLPrintingScriptBuilder::blockStart()
{
    write(QLatin1String("<block>"));
}

void XMLPrintingScriptBuilder::blockEnd()
{
    write(QLatin1String("</block>"));
}

void XMLPrintingScriptBuilder::stringListArgumentStart()
{
    write(QLatin1String("<list>"));
}

void XMLPrintingScriptBuilder::stringListArgumentEnd()
{
    write(QLatin1String("</list>"));
}

void XMLPrintingScriptBuilder::stringListEntry(const QString &string, bool multiline, const QString &hashComment)
{
    stringArgument(string, multiline, hashComment);
}

void XMLPrintingScriptBuilder::hashComment(const QString &comment)
{
    write(QLatin1String("comment"), QLatin1String("type=\"hash\""), comment);
}

void XMLPrintingScriptBuilder::bracketComment(const QString &comment)
{
    write(QLatin1String("comment"), QLatin1String("type=\"bracket\""), comment);
}

void XMLPrintingScriptBuilder::lineFeed()
{
    write(QLatin1String("<crlf/>"));
}

void XMLPrintingScriptBuilder::error(const KSieve::Error &error)
{
    mError = QLatin1String("Error: ") + error.asString();
    write(mError);
}

void XMLPrintingScriptBuilder::finished()
{
    write(QLatin1String("</script>"));
}

void XMLPrintingScriptBuilder::write(const QString &msg)
{
    mResult += msg;
}

void XMLPrintingScriptBuilder::write(const QString &key, const QString &value)
{
    if (value.isEmpty()) {
        write(QString::fromLatin1("<%1>").arg(key));
        return;
    }
    write(QString::fromLatin1("<%1>").arg(key));
    write(value);
    write(QString::fromLatin1("</%1>").arg(key));
}

void XMLPrintingScriptBuilder::write(const QString &key, const QString &attribute, const QString &value)
{
    if (value.isEmpty()) {
        write(QString::fromLatin1("<%1/>").arg(key));
        return;
    }

    if (attribute.isEmpty()) {
        write(QString::fromLatin1("<%1>").arg(key));
    } else {
        write(QString::fromLatin1("<%1 %2>").arg(key).arg(attribute));
    }
    write(value);
    write(QString::fromLatin1("</%1>").arg(key));
}

QString XMLPrintingScriptBuilder::result() const
{
    return mResult;
}

QString XMLPrintingScriptBuilder::error() const
{
    return mError;
}

bool XMLPrintingScriptBuilder::hasError() const
{
    return !mError.isEmpty();
}

void XMLPrintingScriptBuilder::clear()
{
    mResult.clear();
    mError.clear();
}

QDomDocument XMLPrintingScriptBuilder::toDom() const
{
    QString errorMsg;
    int errorRow;
    int errorCol;
    QDomDocument doc;
    if (!doc.setContent(mResult, &errorMsg, &errorRow, &errorCol)) {
        qDebug() << "Unable to load document.Parse error in line " << errorRow
                 << ", col " << errorCol << ": " << errorMsg;
        qDebug() << " mResult" << mResult;

    }
    return doc;
}
