/* Copyright (C) 2011, 2012, 2013 Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "sievesyntaxhighlighter.h"

using namespace PimCommon;

SieveSyntaxHighlighter::SieveSyntaxHighlighter( QTextDocument *doc )
    : QSyntaxHighlighter( doc )
{
    init();
}

SieveSyntaxHighlighter::~SieveSyntaxHighlighter()
{
}

void SieveSyntaxHighlighter::highlightBlock(const QString &text)
{
    Q_FOREACH (const Rule &rule, m_rules) {
        const QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        int length = 0;
        while (index >= 0 && ( length = expression.matchedLength() ) > 0 ) {
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
}

void SieveSyntaxHighlighter::addCapabilities(const QStringList &capabilities)
{
    // capabilities
    QTextCharFormat keywordFormat;
    keywordFormat.setForeground( Qt::darkGreen );
    keywordFormat.setFontWeight( QFont::Bold );
    QStringList keywords;
    keywords << capabilities;
    Q_FOREACH ( const QString & s, keywords ) {
        const QRegExp regex( QString::fromLatin1("\"%1\"").arg(s), Qt::CaseInsensitive );
        m_rules.append( Rule( regex, keywordFormat ) );
    }
}

void SieveSyntaxHighlighter::init()
{
    // Comments
    QTextCharFormat commentFormat;
    commentFormat.setForeground( Qt::darkYellow );
    QRegExp commentRegex( QLatin1String( "^#.*$" ) );
    m_rules.append( Rule( commentRegex, commentFormat ) );

    commentRegex = QRegExp( QLatin1String( "^/*.*$*/" ) );
    m_rules.append( Rule( commentRegex, commentFormat ) );


    // Keywords
    QTextCharFormat keywordFormat;
    keywordFormat.setForeground( Qt::darkMagenta );
    keywordFormat.setFontWeight( QFont::Bold );
    QStringList keywords;
    keywords << QLatin1String( "\\brequire\\b" );
    keywords << QLatin1String( "\\binclude\\b" );
    keywords << QLatin1String( "\\bglobal\\b" );
    Q_FOREACH ( const QString & s, keywords ) {
        const QRegExp regex( s, Qt::CaseInsensitive );
        m_rules.append( Rule( regex, keywordFormat ) );
    }

    // Text keyword
    QTextCharFormat textKeywordFormat;
    textKeywordFormat.setForeground( Qt::green );
    QStringList textKeywords;
    textKeywords << QLatin1String( "\\btext:" );
    Q_FOREACH ( const QString & s, textKeywords ) {
        const QRegExp regex( s, Qt::CaseInsensitive );
        m_rules.append( Rule( regex, textKeywordFormat ) );
    }

    // Match Type
    QTextCharFormat matchFormat;
    matchFormat.setForeground( Qt::red );
    QStringList matchType;
    matchType << QLatin1String( "\\s:quoteregex\\b" )
              << QLatin1String( "\\s:content\\b" )
              << QLatin1String( "\\s:raw\\b" )
              << QLatin1String( "\\s:type\\b" )
              << QLatin1String( "\\s:subtype\\b" )
              << QLatin1String( "\\s:anychild\\b" )
              << QLatin1String( "\\s:param\\b" )
              << QLatin1String( "\\s:value\\b" )
              << QLatin1String( "\\s:count\\b" )
              << QLatin1String( "\\s:last\\b" )
              << QLatin1String( "\\s:text\\b" )
              << QLatin1String( "\\s:lower\\b" )
              << QLatin1String( "\\s:upper\\b" )
              << QLatin1String( "\\s:lowerfirst\\b" )
              << QLatin1String( "\\s:upperfirst\\b" )
              << QLatin1String( "\\s:quotewilcard\\b" )
              << QLatin1String( "\\s:length\\b" )
              << QLatin1String( "\\s:contains\\b" )
              << QLatin1String( "\\s:matches\\b" )
              << QLatin1String( "\\s:global\\b" )
              << QLatin1String( "\\s:once\\b" )
              << QLatin1String( "\\s:optional\\b" )
              << QLatin1String( "\\s:personal\\b" )
              << QLatin1String( "\\s:is\\b" )
              << QLatin1String( "\\s:over\\b" )
              << QLatin1String( "\\s:under\\b" )
              << QLatin1String( "\\s:localpart\\b" )
              << QLatin1String( "\\s:domain\\b" )
              << QLatin1String( "\\s:user\\b" )
              << QLatin1String( "\\s:detail\\b" )
              << QLatin1String( "\\s:all\\b" )
              << QLatin1String( "\\s:copy\\b" )
              << QLatin1String( "\\s:message\\b" )
              << QLatin1String( "\\s:importance\\b" )
              << QLatin1String( "\\s:seconds\\b" )
              << QLatin1String( "\\stext:\\b" )
              << QLatin1String( "\\s:days\\b" )
              << QLatin1String( "\\s:addresses\\b" )
              << QLatin1String( "\\s:regex\\b")
              << QLatin1String( "\\s:flags\\b" )
              << QLatin1String( "\\s:subject\\b" )
              << QLatin1String( "\\s:create\\b" );
    Q_FOREACH ( const QString & s, matchType ) {
        const QRegExp regex( s, Qt::CaseInsensitive );
        m_rules.append( Rule( regex, matchFormat ) );
    }

    // Control structure
    QTextCharFormat controlFormat;
    controlFormat.setForeground( Qt::green );
    controlFormat.setFontWeight( QFont::Bold );
    QStringList controlType;
    controlType << QLatin1String( "\\bif\\b" )<<QLatin1String( "\\belsif\\b" )<<QLatin1String( "\\belse\\b" );
    Q_FOREACH ( const QString & s, controlType ) {
        const QRegExp regex( s, Qt::CaseInsensitive );
        m_rules.append( Rule( regex, controlFormat ) );
    }

    //Action commands:
    QTextCharFormat actionFormat;
    actionFormat.setForeground( Qt::blue );
    actionFormat.setFontWeight( QFont::Bold );
    QStringList actionType;
    actionType <<QLatin1String( "\\bstop\\b" )
               <<QLatin1String( "\\bkeep\\b" )
               <<QLatin1String( "\\breject\\b" )
               <<QLatin1String( "\\bdiscard\\b" )
               <<QLatin1String( "\\bredirect\\b" )
               <<QLatin1String( "\\bfileinto\\b" )
               <<QLatin1String( "\\bsetflag\\b" )
               <<QLatin1String( "\\baddflag\\b" )
               <<QLatin1String( "\\bremoveflag\\b" )
               <<QLatin1String( "\\hasflag\\b" )
               <<QLatin1String( "\\bdeleteheader\\b" )
               <<QLatin1String( "\\baddheader\\b" )
               <<QLatin1String( "\\bnotify\\b" )
               <<QLatin1String( "\\bset\\b" )
               <<QLatin1String( "\\breturn\\b" )
               <<QLatin1String( "\\bvacation\\b" );
    Q_FOREACH ( const QString & s, actionType ) {
        const QRegExp regex( s, Qt::CaseInsensitive );
        m_rules.append( Rule( regex, actionFormat ) );
    }

    //Test commands:
    QTextCharFormat testFormat;
    testFormat.setForeground( Qt::gray );
    testFormat.setFontWeight( QFont::Bold );
    QStringList testType;
    testType <<QLatin1String( "\\benvelope\\b" )
             <<QLatin1String( "\\baddress\\b" )
             <<QLatin1String( "\\ballof\\b" )
             <<QLatin1String( "\\banyof\\b" )
             <<QLatin1String( "\\bexists\\b" )
             <<QLatin1String( "\\bfalse\\b" )
             <<QLatin1String( "\\bheader\\b" )
             <<QLatin1String( "\\bnot\\b" )
             <<QLatin1String( "\\bsize\\b" )
             <<QLatin1String( "\\bdate\\b" )
             <<QLatin1String( "\\bbody\\b" )
             <<QLatin1String( "\\bcurrentdate\\b" )
             <<QLatin1String( "\\bmailboxexists\\b" )
             <<QLatin1String( "\\btrue\\b" );
    Q_FOREACH ( const QString & s, testType ) {
        const QRegExp regex( s, Qt::CaseInsensitive );
        m_rules.append( Rule( regex, testFormat ) );
    }

    // Literals
    QTextCharFormat literalFormat;
    literalFormat.setForeground( Qt::darkRed );
    QRegExp literalRegex( QLatin1String( "(\"[^\"]*\")" ) );
    m_rules.append( Rule( literalRegex, literalFormat ) );
}

