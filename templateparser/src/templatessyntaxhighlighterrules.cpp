/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "templatessyntaxhighlighterrules.h"

#include <QTextListFormat>
#include "templatesutil.h"
#include <QPalette>
#include <KPIMTextEdit/SyntaxHighlighterBase>
using namespace TemplateParser;

TemplatesSyntaxHighlighterRules::TemplatesSyntaxHighlighterRules()
{
    init();
}

void TemplatesSyntaxHighlighterRules::init()
{
    QTextCharFormat keywordFormat;
    QPalette palette;
    keywordFormat.setForeground(palette.link());

    const QStringList keywords = QStringList() << Util::keywords();

    Q_FOREACH (const QString &s, keywords) {
        const QRegularExpression regex(s, QRegularExpression::CaseInsensitiveOption);
        mRules.append(KPIMTextEdit::Rule(regex, keywordFormat));
    }

    QTextCharFormat keywordWithArgsFormat;
    keywordWithArgsFormat.setForeground(palette.link());
    const QStringList keywordsWithArgs = QStringList() << Util::keywordsWithArgsForCompleter();

    Q_FOREACH (const QString &s, keywordsWithArgs) {
        const QRegularExpression regex(s, QRegularExpression::CaseInsensitiveOption);
        mRules.append(KPIMTextEdit::Rule(regex, keywordWithArgsFormat));
    }

}

QVector<KPIMTextEdit::Rule> TemplatesSyntaxHighlighterRules::rules() const
{
    return mRules;
}

