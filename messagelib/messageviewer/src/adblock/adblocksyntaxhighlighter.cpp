/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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
    : KPIMTextEdit::SyntaxHighlighterBase(doc)
{
    init();
}

AdBlockSyntaxHighlighter::~AdBlockSyntaxHighlighter()
{
}

void AdBlockSyntaxHighlighter::init()
{
    QTextCharFormat commentFormat;
    commentFormat.setForeground(Qt::darkYellow);
    QRegularExpression commentRegex(QStringLiteral("^!.*"));
    m_rules.append(KPIMTextEdit::Rule(commentRegex, commentFormat));

    QTextCharFormat exceptionFormat;
    exceptionFormat.setForeground(Qt::magenta);
    QRegularExpression exceptionRegex(QStringLiteral("^@@.*"));
    m_rules.append(KPIMTextEdit::Rule(exceptionRegex, exceptionFormat));

    QTextCharFormat headerFormat;
    headerFormat.setForeground(Qt::red);
    QRegularExpression headerRegex(QStringLiteral("^\\[.*"));
    m_rules.append(KPIMTextEdit::Rule(headerRegex, headerFormat));

    QTextCharFormat elementFormat;
    elementFormat.setForeground(Qt::blue);
    QRegularExpression elementRegex(QStringLiteral(".*##.*"));
    m_rules.append(KPIMTextEdit::Rule(elementRegex, elementFormat));
}

