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

#include "plaintexteditor.h"
#include "plaintextsyntaxspellcheckinghighlighter.h"

#include <KPIMTextEdit/SyntaxHighlighterBase>

using namespace PimCommon;

class PimCommon::PlainTextSyntaxSpellCheckingHighlighter::PlainTextSyntaxSpellCheckingHighlighterPrivate
{
public:
    PlainTextSyntaxSpellCheckingHighlighterPrivate(PlainTextEditor *plainText)
        : editor(plainText),
          spellCheckingEnabled(false)
    {

    }
    QVector<KPIMTextEdit::Rule> rules;
    PlainTextEditor *editor;
    QColor misspelledColor;
    bool spellCheckingEnabled;
};

PlainTextSyntaxSpellCheckingHighlighter::PlainTextSyntaxSpellCheckingHighlighter(PlainTextEditor *plainText, const QColor &misspelledColor)
    : Sonnet::Highlighter(plainText),
      d(new PlainTextSyntaxSpellCheckingHighlighter::PlainTextSyntaxSpellCheckingHighlighterPrivate(plainText))
{
    d->misspelledColor = misspelledColor;
    setAutomatic(false);
}


PlainTextSyntaxSpellCheckingHighlighter::~PlainTextSyntaxSpellCheckingHighlighter()
{
    delete d;
}

void PlainTextSyntaxSpellCheckingHighlighter::toggleSpellHighlighting(bool on)
{
    if (on != d->spellCheckingEnabled) {
        d->spellCheckingEnabled = on;
        rehighlight();
    }
}

void PlainTextSyntaxSpellCheckingHighlighter::setSyntaxHighlighterRules(const QVector<KPIMTextEdit::Rule> &rule)
{
    d->rules = rule;
}

void PlainTextSyntaxSpellCheckingHighlighter::highlightBlock(const QString &text)
{
    Q_FOREACH (const KPIMTextEdit::Rule &rule, d->rules) {
        const QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        int length = 0;
        while (index >= 0 && (length = expression.matchedLength()) > 0) {
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    if (d->spellCheckingEnabled) {
        Highlighter::highlightBlock(text);
    }
}

void PlainTextSyntaxSpellCheckingHighlighter::unsetMisspelled(int start, int count)
{
    Q_UNUSED(start);
    Q_UNUSED(count);
}

void PlainTextSyntaxSpellCheckingHighlighter::setMisspelled(int start, int count)
{
    setMisspelledColor(d->misspelledColor);
    Sonnet::Highlighter::setMisspelled(start, count);
}
