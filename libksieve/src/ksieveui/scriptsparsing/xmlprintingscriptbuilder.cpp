/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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
#include "libksieve_debug.h"

using namespace KSieveUi;
XMLPrintingScriptBuilder::XMLPrintingScriptBuilder()
    : KSieve::ScriptBuilder(),
      mIsAction(false)
{
    write(QStringLiteral("<?xml version='1.0'?>"));
    write(QStringLiteral("<script>"));
}

XMLPrintingScriptBuilder::~XMLPrintingScriptBuilder()
{
}

void XMLPrintingScriptBuilder::taggedArgument(const QString &tag)
{
    write(QStringLiteral("tag"), tag);
}

void XMLPrintingScriptBuilder::stringArgument(const QString &string, bool multiLine, const QString & /*fixme*/)
{
    write(QStringLiteral("str"), multiLine ? QStringLiteral("type=\"multiline\"") : QStringLiteral("type=\"quoted\""), string);
}

void XMLPrintingScriptBuilder::numberArgument(unsigned long number, char quantifier)
{
    write(QStringLiteral("num"), (quantifier ? QStringLiteral("quantifier=\"%1\"").arg(quantifier) : QString()), QString::number(number));
}

void XMLPrintingScriptBuilder::commandStart(const QString &identifier, int lineNumber)
{
    Q_UNUSED(lineNumber);
    if (identifier == QLatin1String("else") ||
            identifier == QLatin1String("break") ||
            identifier == QLatin1String("require") ||
            identifier == QLatin1String("foreverypart") ||
            identifier == QLatin1String("if") ||
            identifier == QLatin1String("elsif")) {
        write(QStringLiteral("<control name=\"%1\">").arg(identifier));
        mIsAction = false;
    } else {
        write(QStringLiteral("<action name=\"%1\">").arg(identifier));
        mIsAction = true;
    }
}

void XMLPrintingScriptBuilder::commandEnd(int lineNumber)
{
    Q_UNUSED(lineNumber);
    if (mIsAction) {
        write(QStringLiteral("</action>"));
    } else {
        write(QStringLiteral("</control>"));
    }
    mIsAction = false;
}

void XMLPrintingScriptBuilder::testStart(const QString &identifier)
{
    write(QStringLiteral("<test name=\"%1\">").arg(identifier));
}

void XMLPrintingScriptBuilder::testEnd()
{
    write(QStringLiteral("</test>"));
}

void XMLPrintingScriptBuilder::testListStart()
{
    write(QStringLiteral("<testlist>"));
}

void XMLPrintingScriptBuilder::testListEnd()
{
    write(QStringLiteral("</testlist>"));
}

void XMLPrintingScriptBuilder::blockStart(int lineNumber)
{
    Q_UNUSED(lineNumber);
    write(QStringLiteral("<block>"));
}

void XMLPrintingScriptBuilder::blockEnd(int lineNumber)
{
    Q_UNUSED(lineNumber);
    write(QStringLiteral("</block>"));
}

void XMLPrintingScriptBuilder::stringListArgumentStart()
{
    write(QStringLiteral("<list>"));
}

void XMLPrintingScriptBuilder::stringListArgumentEnd()
{
    write(QStringLiteral("</list>"));
}

void XMLPrintingScriptBuilder::stringListEntry(const QString &string, bool multiline, const QString &hashComment)
{
    stringArgument(string, multiline, hashComment);
}

void XMLPrintingScriptBuilder::hashComment(const QString &comment)
{
    write(QStringLiteral("comment"), QStringLiteral("type=\"hash\""), comment);
}

void XMLPrintingScriptBuilder::bracketComment(const QString &comment)
{
    write(QStringLiteral("comment"), QStringLiteral("type=\"bracket\""), comment);
}

void XMLPrintingScriptBuilder::lineFeed()
{
    write(QStringLiteral("<crlf/>"));
}

void XMLPrintingScriptBuilder::error(const KSieve::Error &error)
{
    mError = QStringLiteral("Error: ") + error.asString();
    write(mError);
}

void XMLPrintingScriptBuilder::finished()
{
    write(QStringLiteral("</script>"));
}

void XMLPrintingScriptBuilder::write(const QString &msg)
{
    mResult += msg;
}

void XMLPrintingScriptBuilder::write(const QString &key, const QString &value)
{
    if (value.isEmpty()) {
        write(QStringLiteral("<%1>").arg(key));
        return;
    }
    write(QStringLiteral("<%1>").arg(key));
    write(value);
    write(QStringLiteral("</%1>").arg(key));
}

void XMLPrintingScriptBuilder::write(const QString &key, const QString &attribute, const QString &value)
{
    if (value.isEmpty()) {
        write(QStringLiteral("<%1/>").arg(key));
        return;
    }

    if (attribute.isEmpty()) {
        write(QStringLiteral("<%1>").arg(key));
    } else {
        write(QStringLiteral("<%1 %2>").arg(key, attribute));
    }
    write(value);
    write(QStringLiteral("</%1>").arg(key));
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
        qCDebug(LIBKSIEVE_LOG) << "Unable to load document.Parse error in line " << errorRow
                               << ", col " << errorCol << ": " << errorMsg;
        qCDebug(LIBKSIEVE_LOG) << " mResult" << mResult;

    }
    return doc;
}
