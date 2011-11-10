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

#include "templatessyntaxhighlighter.h"

using namespace TemplateParser;

TemplatesSyntaxHighlighter::TemplatesSyntaxHighlighter( QTextDocument* doc )
  : QSyntaxHighlighter( doc )
{
  init();
}

TemplatesSyntaxHighlighter::~TemplatesSyntaxHighlighter()
{
}
  
void TemplatesSyntaxHighlighter::highlightBlock(const QString& text)
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

void TemplatesSyntaxHighlighter::init()
{
  QTextCharFormat keywordFormat;
  keywordFormat.setForeground( Qt::blue );
  keywordFormat.setFontWeight( QFont::Bold );

  QStringList keywords;
  keywords<< QLatin1String("%QUOTE") << QLatin1String("%FORCEDPLAIN")
          <<QLatin1String("%FORCEDHTML")<<QLatin1String("%QHEADERS")
          <<QLatin1String("%HEADERS")<<QLatin1String("%TEXT")
          <<QLatin1String("%OTEXTSIZE")<<QLatin1String("%OTEXT")
          <<QLatin1String("%OADDRESSEESADDR")<<QLatin1String("%CCADDR")
          <<QLatin1String("%CCNAME")<<QLatin1String("%CCFNAME")
          <<QLatin1String("%CCLNAME")<<QLatin1String("%TOADDR")
          <<QLatin1String("%TONAME")<<QLatin1String("%TOFNAME")
          <<QLatin1String("%TOLNAME")<<QLatin1String("%TOLIST")
          <<QLatin1String("%FROMADDR")<<QLatin1String("%FROMNAME")
          <<QLatin1String("%FROMFNAME")<<QLatin1String("%FROMLNAME")
          <<QLatin1String("%FULLSUBJECT")<<QLatin1String("%FULLSUBJ")
          <<QLatin1String("%MSGID")<<QLatin1String("%HEADER( ")
          <<QLatin1String("%OCCADDR")<<QLatin1String("%OCCNAME")
          <<QLatin1String("%OCCFNAME")<<QLatin1String("%OCCLNAME")
          <<QLatin1String("%OTOADDR")<<QLatin1String("%OTONAME")
          <<QLatin1String("%OTOFNAME")<<QLatin1String("%OTOLNAME")
          <<QLatin1String("%OTOLIST")<<QLatin1String("%OTO")
          <<QLatin1String("%OFROMADDR")<<QLatin1String("%OFROMNAME")
          <<QLatin1String("%OFROMFNAME")<<QLatin1String("%OFROMLNAME")
          <<QLatin1String("%OFULLSUBJECT")<<QLatin1String("%OFULLSUBJ")
          <<QLatin1String("%OMSGID")<<QLatin1String("%DATEEN")
          <<QLatin1String("%DATESHORT")<<QLatin1String("%DATE")
          <<QLatin1String("%DOW")<<QLatin1String("%TIMELONGEN")
          <<QLatin1String("%TIMELONG")<<QLatin1String("%TIME")
          <<QLatin1String("%ODATEEN")<<QLatin1String("%ODATESHORT")
          <<QLatin1String("%ODATE")<<QLatin1String("%ODOW")
          <<QLatin1String("%OTIMELONGEN")<<QLatin1String("%OTIMELONG")
          <<QLatin1String("%OTIME")<<QLatin1String("%BLANK")
          <<QLatin1String("%NOP")<<QLatin1String("%CLEAR")
          <<QLatin1String("%DEBUGOFF")<<QLatin1String("%DEBUG")
          <<QLatin1String("%CURSOR")<<QLatin1String("%SIGNATURE");
  Q_FOREACH( const QString & s, keywords ) {
    const QRegExp regex( s, Qt::CaseInsensitive );
    m_rules.append( Rule( regex, keywordFormat ) );
  }

  QTextCharFormat keywordWithArgsFormat;
  keywordWithArgsFormat.setForeground( Qt::blue );
  keywordWithArgsFormat.setFontWeight( QFont::Bold );
  QStringList keywordsWithArgs;
  keywordsWithArgs << QLatin1String("%REM=\".*\"%-")<<QLatin1String("%INSERT=\".*\"")<<QLatin1String("%SYSTEM=\".*\"")<<
    QLatin1String("%PUT=\".*\"")<<QLatin1String("%QUOTEPIPE=\".*\"")<<QLatin1String("%MSGPIPE=\".*\"")<<
    QLatin1String("%BODYPIPE=\".*\"")<<QLatin1String("%CLEARPIPE=\".*\"")<<QLatin1String("%TEXTPIPE=\".*\"")<<
    QLatin1String("%OHEADER=\".*\"")<<QLatin1String("%HEADER=\".*\"");
  
  Q_FOREACH( const QString & s, keywordsWithArgs ) {
    const QRegExp regex( s, Qt::CaseInsensitive );
    m_rules.append( Rule( regex, keywordWithArgsFormat ) );
  }

}

#include "templatessyntaxhighlighter.moc"


