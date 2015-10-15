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

#include "richtextcomposeremailquotehighlighter.h"
#include "richtextcomposer.h"
using namespace MessageComposer;

class Q_DECL_HIDDEN MessageComposer::RichTextComposerEmailQuoteHighlighter::RichTextComposerEmailQuoteHighlighterPrivate
{
public:
    QColor col1;
    QColor col2;
    QColor col3;
    QColor misspelledColor;
    bool spellCheckingEnabled;
    RichTextComposer *parent;
};

RichTextComposerEmailQuoteHighlighter::RichTextComposerEmailQuoteHighlighter(RichTextComposer *textEdit, const QColor &normalColor,
        const QColor &quoteDepth1, const QColor &quoteDepth2,
        const QColor &quoteDepth3, const QColor &misspelledColor)
    : Sonnet::Highlighter(textEdit),
      d(new MessageComposer::RichTextComposerEmailQuoteHighlighter::RichTextComposerEmailQuoteHighlighterPrivate())
{
    Q_UNUSED(normalColor);
    // Don't automatically disable the spell checker, for example because there
    // are too many misspelled words. That would also disable quote highlighting.
    // FIXME: disable this spell checking!
    setAutomatic(false);

    setActive(true);
    d->col1 = quoteDepth1;
    d->col2 = quoteDepth2;
    d->col3 = quoteDepth3;
    d->misspelledColor = misspelledColor;
    d->spellCheckingEnabled = false;
    d->parent = textEdit;
}

RichTextComposerEmailQuoteHighlighter::~RichTextComposerEmailQuoteHighlighter()
{
    delete d;
}
void RichTextComposerEmailQuoteHighlighter::setQuoteColor(const QColor &normalColor,
        const QColor &quoteDepth1,
        const QColor &quoteDepth2,
        const QColor &quoteDepth3,
        const QColor &misspelledColor)
{
    Q_UNUSED(normalColor);
    d->col1 = quoteDepth1;
    d->col2 = quoteDepth2;
    d->col3 = quoteDepth3;
    d->misspelledColor = misspelledColor;
}

void RichTextComposerEmailQuoteHighlighter::toggleSpellHighlighting(bool on)
{
    if (on != d->spellCheckingEnabled) {
        d->spellCheckingEnabled = on;
        rehighlight();
    }
}

void RichTextComposerEmailQuoteHighlighter::highlightBlock(const QString &text)
{
    QString simplified = text;
    simplified = simplified.remove(QRegExp(QStringLiteral("\\s"))).
                 replace(QLatin1Char('|'), QLatin1Char('>'));

    while (simplified.startsWith(QStringLiteral(">>>>"))) {
        simplified = simplified.mid(3);
    }

    if (simplified.startsWith(QStringLiteral(">>>"))) {
        setFormat(0, text.length(), d->col3);
    } else if (simplified.startsWith(QStringLiteral(">>"))) {
        setFormat(0, text.length(), d->col2);
    } else if (simplified.startsWith(QStringLiteral(">"))) {
        setFormat(0, text.length(), d->col1);
    } else if (d->parent->isLineQuoted(text)) {
        setFormat(0, text.length(), d->col1);   // FIXME: custom quote prefix
        // can't handle multiple levels
    } else if (d->spellCheckingEnabled) {
        Highlighter::highlightBlock(text);
        return; //setCurrentBlockState already done in Highlighter::highlightBlock
    }
    setCurrentBlockState(0);
}

void RichTextComposerEmailQuoteHighlighter::unsetMisspelled(int start, int count)
{
    Q_UNUSED(start);
    Q_UNUSED(count);
}

void RichTextComposerEmailQuoteHighlighter::setMisspelled(int start, int count)
{
    setMisspelledColor(d->misspelledColor);
    Sonnet::Highlighter::setMisspelled(start, count);
}
