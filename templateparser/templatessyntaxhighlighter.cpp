/* Copyright (C) 2011-2014 Laurent Montel <montel@kde.org>
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
#include "templatesutil.h"
#include <QPalette>

using namespace TemplateParser;

TemplatesSyntaxHighlighter::TemplatesSyntaxHighlighter(QTextDocument *doc)
    : PimCommon::SyntaxHighlighterBase(doc)
{
    init();
}

TemplatesSyntaxHighlighter::~TemplatesSyntaxHighlighter()
{
}

void TemplatesSyntaxHighlighter::init()
{
    QTextCharFormat keywordFormat;
    QPalette palette;
    keywordFormat.setForeground(palette.link());

    QStringList keywords;
    keywords << Util::keywords();

    Q_FOREACH (const QString &s, keywords) {
        const QRegExp regex(s, Qt::CaseInsensitive);
        m_rules.append(Rule(regex, keywordFormat));
    }

    QTextCharFormat keywordWithArgsFormat;
    keywordWithArgsFormat.setForeground(palette.link());
    QStringList keywordsWithArgs;
    keywordsWithArgs << Util::keywordsWithArgs();

    Q_FOREACH (const QString &s, keywordsWithArgs) {
        const QRegExp regex(s, Qt::CaseInsensitive);
        m_rules.append(Rule(regex, keywordWithArgsFormat));
    }
}

