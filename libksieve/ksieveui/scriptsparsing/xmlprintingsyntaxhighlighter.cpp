/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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


#include "xmlprintingsyntaxhighlighter.h"

XMLPrintingSyntaxHighLighter::XMLPrintingSyntaxHighLighter( QTextDocument *doc )
    : PimCommon::SyntaxHighlighterBase( doc )
{
    init();
}

XMLPrintingSyntaxHighLighter::~XMLPrintingSyntaxHighLighter()
{
}

void XMLPrintingSyntaxHighLighter::init()
{
    QTextCharFormat testFormat;
    testFormat.setForeground( Qt::gray );
    testFormat.setFontWeight( QFont::Bold );
    QStringList testType;
    testType << QLatin1String("require");

    Q_FOREACH ( const QString &s, testType ) {
        const QRegExp regex( s, Qt::CaseInsensitive );
        m_rules.append( Rule( regex, testFormat ) );
    }

    QTextCharFormat quoteFormat;
    quoteFormat.setForeground( Qt::blue );
    quoteFormat.setFontWeight( QFont::Bold );
    QStringList quoteType;
    quoteType << QLatin1String("quoted") << QLatin1String("hash") << QLatin1String("bracket") << QLatin1String("multiline");
    Q_FOREACH ( const QString &s, quoteType ) {
        const QRegExp regex( s, Qt::CaseInsensitive );
        m_rules.append( Rule( regex, quoteFormat ) );
    }


    QTextCharFormat misc;
    misc.setForeground( Qt::red );
    misc.setFontWeight( QFont::Bold );
    QStringList miscType;
    miscType << QLatin1String("control") << QLatin1String("block") << QLatin1String("script")<< QLatin1String("action")<<QLatin1String("comment");
    miscType << QLatin1String("num") << QLatin1String("tag")<< QLatin1String("list")<< QLatin1String("str")<< QLatin1String("test") <<QLatin1String("crlf/");
    Q_FOREACH ( const QString &s, miscType ) {
        const QRegExp regex( s, Qt::CaseInsensitive );
        m_rules.append( Rule( regex, misc ) );
    }

    QTextCharFormat header;
    header.setForeground( Qt::black );
    header.setFontWeight( QFont::Bold );
    m_rules.append( Rule( QRegExp(QLatin1String("<\\?xml.*"), Qt::CaseInsensitive), header ) );

}

