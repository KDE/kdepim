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
#include "messagecomposer/composer-ng/richtextcomposer.h"
using namespace MessageComposer;

class MessageComposer::RichTextComposerEmailQuoteHighlighter::RichTextComposerEmailQuoteHighlighterPrivate
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
    : Sonnet::Highlighter(textEdit, textEdit->spellCheckingConfigFileName()),
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
}

QString RichTextComposerEmailQuoteHighlighter::highlightText(const QString &text,
        const QColor &quoteDepth1,
        const QColor &quoteDepth2,
        const QColor &quoteDepth3)
{
    const QStringList splitList = text.split(QLatin1Char('\n'));
    QString result;
    QStringList::const_iterator it = splitList.constBegin();
    QStringList::const_iterator end = splitList.constEnd();
    while (it != end) {
        result.append(highlightParagraph((*it) + QLatin1Char('\n'),
                                         quoteDepth1, quoteDepth2, quoteDepth3));
        ++it;
    }
    return result;
}

QString RichTextComposerEmailQuoteHighlighter::highlightParagraph(const QString &text,
        const QColor &quoteDepth1,
        const QColor &quoteDepth2,
        const QColor &quoteDepth3)
{
    QString simplified = text;
    simplified = simplified.remove(QRegExp(QLatin1String("\\s"))).
                 replace(QLatin1Char('|'), QLatin1Char('>')).
                 replace(QLatin1String("&gt;"), QLatin1String(">"));

    while (simplified.startsWith(QLatin1String(">>>>"))) {
        simplified = simplified.mid(3);
    }

    QString result(QLatin1String("<font color=\"%1\">%2</font>"));
    if (simplified.startsWith(QLatin1String(">>>"))) {
        return result.arg(quoteDepth3.name(), text);
    } else if (simplified.startsWith(QLatin1String(">>"))) {
        return result.arg(quoteDepth2.name(), text);
    } else if (simplified.startsWith(QLatin1String(">"))) {
        return result.arg(quoteDepth1.name(), text);
    }

    return text;
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
    simplified = simplified.remove(QRegExp(QLatin1String("\\s"))).
                 replace(QLatin1Char('|'), QLatin1Char('>'));

    while (simplified.startsWith(QLatin1String(">>>>"))) {
        simplified = simplified.mid(3);
    }

    if (simplified.startsWith(QLatin1String(">>>"))) {
        setFormat(0, text.length(), d->col3);
    } else if (simplified.startsWith(QLatin1String(">>"))) {
        setFormat(0, text.length(), d->col2);
    } else if (simplified.startsWith(QLatin1String(">"))) {
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
