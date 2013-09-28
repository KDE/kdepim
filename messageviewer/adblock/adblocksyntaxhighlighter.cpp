/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "adblocksyntaxhighlighter.h"

using namespace MessageViewer;

AdBlockSyntaxHighlighter::AdBlockSyntaxHighlighter(QTextDocument *doc)
    : QSyntaxHighlighter(doc)
{
    init();
}

AdBlockSyntaxHighlighter::~AdBlockSyntaxHighlighter()
{
    QTextCharFormat commentFormat;
    commentFormat.setForeground( Qt::darkYellow );
    QRegExp commentRegex( QLatin1String( "^!" ) );
    m_rules.append( Rule( commentRegex, commentFormat ) );

    QTextCharFormat exceptionFormat;
    exceptionFormat.setForeground( Qt::green );
    QRegExp exceptionRegex( QLatin1String( "^@@" ) );
    m_rules.append( Rule( exceptionRegex, exceptionFormat ) );
}

void AdBlockSyntaxHighlighter::highlightBlock(const QString &text)
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

void AdBlockSyntaxHighlighter::init()
{
    //TODO
}

#include "adblocksyntaxhighlighter.moc"
