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

#ifndef KSIEVE_KSIEVEUI_XMLPRINTINGSCRIPTBUILDER_H
#define KSIEVE_KSIEVEUI_XMLPRINTINGSCRIPTBUILDER_H

#include <ksieve/scriptbuilder.h>

#include <QDomDocument>

namespace KSieveUi {
class XMLPrintingScriptBuilder : public KSieve::ScriptBuilder
{
public:
    explicit XMLPrintingScriptBuilder();
    ~XMLPrintingScriptBuilder();

    void taggedArgument( const QString &tag );
    void stringArgument( const QString &string, bool multiLine, const QString & /*fixme*/ );
    void numberArgument( unsigned long number, char quantifier );
    void commandStart( const QString &identifier );
    void commandEnd();
    void testStart( const QString &identifier );
    void testEnd();
    void testListStart();
    void testListEnd();
    void blockStart();
    void blockEnd();
    void stringListArgumentStart();
    void stringListArgumentEnd();
    void stringListEntry( const QString &string, bool multiline, const QString &hashComment );
    void hashComment( const QString &comment );
    void bracketComment( const QString &comment );

    void lineFeed();
    void error( const KSieve::Error &error );
    void finished();

    QString result() const;
    QString error() const;
    bool hasError() const;

    void clear();

    QDomDocument toDom() const;

private:
    void write( const char * msg );
    void write( const QByteArray &key, const QString &value );
    void write( const QByteArray & key, const QByteArray &attribute, const QString & value );

    QString mResult;
    QString mError;
    int mIndent;
};
}
#endif
