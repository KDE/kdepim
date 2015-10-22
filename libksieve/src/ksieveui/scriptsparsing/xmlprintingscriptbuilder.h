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

#ifndef KSIEVE_KSIEVEUI_XMLPRINTINGSCRIPTBUILDER_H
#define KSIEVE_KSIEVEUI_XMLPRINTINGSCRIPTBUILDER_H

#include "ksieveui_export.h"
#include <ksieve/scriptbuilder.h>

#include <QDomDocument>

namespace KSieveUi
{
class KSIEVEUI_EXPORT XMLPrintingScriptBuilder : public KSieve::ScriptBuilder
{
public:
    explicit XMLPrintingScriptBuilder();
    ~XMLPrintingScriptBuilder();

    void taggedArgument(const QString &tag) Q_DECL_OVERRIDE;
    void stringArgument(const QString &string, bool multiLine, const QString & /*fixme*/) Q_DECL_OVERRIDE;
    void numberArgument(unsigned long number, char quantifier) Q_DECL_OVERRIDE;
    void commandStart(const QString &identifier, int lineNumber) Q_DECL_OVERRIDE;
    void commandEnd(int lineNumber) Q_DECL_OVERRIDE;
    void testStart(const QString &identifier) Q_DECL_OVERRIDE;
    void testEnd() Q_DECL_OVERRIDE;
    void testListStart() Q_DECL_OVERRIDE;
    void testListEnd() Q_DECL_OVERRIDE;
    void blockStart(int lineNumber) Q_DECL_OVERRIDE;
    void blockEnd(int lineNumber) Q_DECL_OVERRIDE;
    void stringListArgumentStart() Q_DECL_OVERRIDE;
    void stringListArgumentEnd() Q_DECL_OVERRIDE;
    void stringListEntry(const QString &string, bool multiline, const QString &hashComment) Q_DECL_OVERRIDE;
    void hashComment(const QString &comment) Q_DECL_OVERRIDE;
    void bracketComment(const QString &comment) Q_DECL_OVERRIDE;

    void lineFeed() Q_DECL_OVERRIDE;
    void error(const KSieve::Error &error) Q_DECL_OVERRIDE;
    void finished() Q_DECL_OVERRIDE;

    QString result() const;
    QString error() const;
    bool hasError() const;

    void clear();

    QDomDocument toDom() const;

private:
    void write(const QString &msg);
    void write(const QString &key, const QString &value);
    void write(const QString &key, const QString &attribute, const QString &value);

    QString mResult;
    QString mError;
    bool mIsAction;
};
}
#endif
