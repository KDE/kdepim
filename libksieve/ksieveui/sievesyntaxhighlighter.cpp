/* Copyright (C) 2011 Laurent Montel <montel@kde.org>
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

using namespace KSieveUi;

SieveSyntaxHighlighter::SieveSyntaxHighlighter( QTextDocument* doc )
  : QSyntaxHighlighter( doc )
{
  init();
}

SieveSyntaxHighlighter::~SieveSyntaxHighlighter()
{
}
  
void SieveSyntaxHighlighter::highlightBlock(const QString& text)
{
  Q_FOREACH(const Rule &rule, m_rules) {
    const QRegExp expression(rule.pattern);
    int index = expression.indexIn(text);
    int length = 0;
    while (index >= 0 && ( length = expression.matchedLength() ) > 0 ) {
      setFormat(index, length, rule.format);
      index = expression.indexIn(text, index + length);
    }
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
  keywords << QLatin1String( "\\brequire\\b" )<<QLatin1String( "\\bstop\\b" );
  Q_FOREACH( const QString & s, keywords ) {
    const QRegExp regex( s, Qt::CaseInsensitive );
    m_rules.append( Rule( regex, keywordFormat ) );
  }


  // Match Type
  QTextCharFormat matchFormat;
  matchFormat.setForeground( Qt::red );
  QStringList matchType;
  matchType << QLatin1String( "\\s:contains\\b" )<<QLatin1String( "\\s:matches\\b" )<<QLatin1String( "\\s:is\\b" )<<QLatin1String( "\\s:over\\b" )<<QLatin1String( "\\s:under\\b" )<<QLatin1String( "\\s:localpart\\b" )<<QLatin1String( "\\s:domain\\b" )<<QLatin1String( "\\s:all\\b" );
  Q_FOREACH( const QString & s, matchType ) {
    const QRegExp regex( s, Qt::CaseInsensitive );
    m_rules.append( Rule( regex, matchFormat ) );
  }

  // Control structure
  QTextCharFormat controlFormat;
  controlFormat.setForeground( Qt::green );
  controlFormat.setFontWeight( QFont::Bold );
  QStringList controlType;
  controlType << QLatin1String( "\\bif\\b" )<<QLatin1String( "\\belsif\\b" )<<QLatin1String( "\\belse\\b" );
  Q_FOREACH( const QString & s, controlType ) {
    const QRegExp regex( s, Qt::CaseInsensitive );
    m_rules.append( Rule( regex, controlFormat ) );
  }

  //Action commands:
  QTextCharFormat actionFormat;
  actionFormat.setForeground( Qt::blue );
  actionFormat.setFontWeight( QFont::Bold );
  QStringList actionType;
  actionType << QLatin1String( "\\bkeep\\b" )<<QLatin1String( "\\breject\\b" )<<QLatin1String( "\\bdiscard\\b" )<<QLatin1String( "\\bredirect\\b" )<<QLatin1String( "\\bfileinto\\b" )<<QLatin1String( "\\bsetflag\\b" )<<QLatin1String( "\\baddflag\\b" )<<QLatin1String("\\bvacation\\b");
  Q_FOREACH( const QString & s, actionType ) {
    const QRegExp regex( s, Qt::CaseInsensitive );
    m_rules.append( Rule( regex, actionFormat ) );
  }

  //Test commands:
  QTextCharFormat testFormat;
  testFormat.setForeground( Qt::gray );
  testFormat.setFontWeight( QFont::Bold );
  QStringList testType;
  testType << QLatin1String( "\\baddress\\b" )<<QLatin1String( "\\ballof\\b" )<<QLatin1String( "\\banyof\\b" )<<QLatin1String( "\\bexists\\b" )<<QLatin1String( "\\bfalse\\b" )<<QLatin1String( "\\bheader\\b" )<<QLatin1String("\\bnot\\b" )<<QLatin1String( "\\bsize\\b" )<<QLatin1String( "\\btrue\\b" );
  Q_FOREACH( const QString & s, testType ) {
    const QRegExp regex( s, Qt::CaseInsensitive );
    m_rules.append( Rule( regex, testFormat ) );
  }

  // Literals
  QTextCharFormat literalFormat;
  literalFormat.setForeground( Qt::darkRed );
  QRegExp literalRegex( QLatin1String( "(\"[^\"]*\")" ) );
  m_rules.append( Rule( literalRegex, literalFormat ) );

  
  
}

#include "sievesyntaxhighlighter.moc"
