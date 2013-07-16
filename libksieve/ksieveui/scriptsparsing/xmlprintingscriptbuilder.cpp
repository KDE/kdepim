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
#include <QDebug>

XMLPrintingScriptBuilder::XMLPrintingScriptBuilder()
    : KSieve::ScriptBuilder(),
      mIndent( 0 )
{
}

XMLPrintingScriptBuilder::~XMLPrintingScriptBuilder()
{
}

void XMLPrintingScriptBuilder::taggedArgument( const QString & tag )
{
    write( "tag", tag );
}

void XMLPrintingScriptBuilder::stringArgument( const QString & string, bool multiLine, const QString & /*fixme*/ )
{
    write( multiLine ? "string type=\"multiline\"" : "string type=\"quoted\"", string );
}

void XMLPrintingScriptBuilder::numberArgument( unsigned long number, char quantifier )
{
    const QString txt = QLatin1String("number") + ( quantifier ? QString::fromLatin1(" quantifier=\"%1\"").arg( quantifier ) : QString() ) ;
    write( txt.toLatin1(), QString::number( number ) );
}

void XMLPrintingScriptBuilder::commandStart( const QString & identifier )
{
    write( "<command>" );
    ++mIndent;
    write( "identifier", identifier );
}

void XMLPrintingScriptBuilder::commandEnd()
{
    --mIndent;
    write( "</command>" );
}

void XMLPrintingScriptBuilder::testStart( const QString & identifier )
{
    write( "<test>" );
    ++mIndent;
    write( "identifier", identifier );
}

void XMLPrintingScriptBuilder::testEnd()
{
    --mIndent;
    write( "</test>" );
}

void XMLPrintingScriptBuilder::testListStart()
{
    write( "<testlist>" );
    ++mIndent;
}

void XMLPrintingScriptBuilder::testListEnd()
{
    --mIndent;
    write( "</testlist>" );
}

void XMLPrintingScriptBuilder::blockStart()
{
    write( "<block>" );
    ++mIndent;
}

void XMLPrintingScriptBuilder::blockEnd()
{
    --mIndent;
    write( "</block>" );
}

void XMLPrintingScriptBuilder::stringListArgumentStart()
{
    write( "<stringlist>" );
    ++mIndent;
}

void XMLPrintingScriptBuilder::stringListArgumentEnd()
{
    --mIndent;
    write( "</stringlist>" );
}

void XMLPrintingScriptBuilder::stringListEntry( const QString & string, bool multiline, const QString & hashComment )
{
    stringArgument( string, multiline, hashComment );
}

void XMLPrintingScriptBuilder::hashComment( const QString & comment )
{
    write( "comment type=\"hash\"", comment );
}

void XMLPrintingScriptBuilder::bracketComment( const QString & comment )
{
    write( "comment type=\"bracket\"", comment );
}

void XMLPrintingScriptBuilder::lineFeed()
{
    write( "<crlf/>" );
}

void XMLPrintingScriptBuilder::error( const KSieve::Error & error )
{
    mIndent = 0;
    write( (QLatin1String("Error: ") + error.asString()).toLatin1() );
}

void XMLPrintingScriptBuilder::finished()
{
    --mIndent;
    write( "</script>" );
}

void XMLPrintingScriptBuilder::write( const char * msg )
{
    for ( int i = 2*mIndent ; i > 0 ; --i ) {
        qDebug() << " ";
        mResult += QLatin1String(" ");
    }
    qDebug() << msg;
    mResult += QString::fromUtf8(msg) + QLatin1Char('\n');
}

void XMLPrintingScriptBuilder::write( const QByteArray & key, const QString & value )
{
    if ( value.isEmpty() ) {
        write( "<" + key + "/>" );
        return;
    }
    write( "<" + key + ">" );
    ++mIndent;
    write( value.toUtf8().data() );
    --mIndent;
    write( "</" + key + ">" );
}

QString XMLPrintingScriptBuilder::result() const
{
    return mResult;
}
