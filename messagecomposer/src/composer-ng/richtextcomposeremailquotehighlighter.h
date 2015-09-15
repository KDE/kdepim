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

#ifndef RICHTEXTCOMPOSEREMAILQUOTEHIGHLIGHTER_H
#define RICHTEXTCOMPOSEREMAILQUOTEHIGHLIGHTER_H

#include <sonnet/highlighter.h>
#include "messagecomposer_export.h"
namespace MessageComposer
{
class RichTextComposer;
class MESSAGECOMPOSER_EXPORT RichTextComposerEmailQuoteHighlighter : public Sonnet::Highlighter
{
public:

    /**
     * Constructor. See setQuoteColor() for the parameters.
     * FIXME: Default colors don't obey color scheme
     */
    explicit RichTextComposerEmailQuoteHighlighter(RichTextComposer *textEdit,
            const QColor &normalColor = Qt::black,
            const QColor &quoteDepth1 = QColor(0x00, 0x80, 0x00),
            const QColor &quoteDepth2 = QColor(0x00, 0x80, 0x00),
            const QColor &quoteDepth3 = QColor(0x00, 0x80, 0x00),
            const QColor &misspelledColor = Qt::red);

    ~RichTextComposerEmailQuoteHighlighter();

    /**
     * Sets the colors used for highlighting quoted text and spelling mistakes.
     *
     * @param quoteDepth1 color for text quoted 1 level deep
     * @param quoteDepth2 color for text quoted 2 level deep
     * @param quoteDepth3 color for text quoted 3 level deep
     * @param misspelledColor color in which misspelled words will be underlined
     * @param normalColor will be ignored, only provided for KNode
     *                    compatibility.
     */
    void setQuoteColor(const QColor &normalColor,
                       const QColor &quoteDepth1,
                       const QColor &quoteDepth2,
                       const QColor &quoteDepth3,
                       const QColor &misspelledColor = Qt::red);

    /**
     * Turns spellcheck highlighting on or off.
     *
     * @param on if true, spelling mistakes will be highlighted
     */
    void toggleSpellHighlighting(bool on);

    /**
     * Reimplemented to highlight quote blocks.
     */
    void highlightBlock(const QString &text) Q_DECL_OVERRIDE;
protected:

    /**
     * Reimplemented, the base version sets the text color to black, which
     * is not what we want. We do nothing, the format is already reset by
     * Qt.
     * @param start the beginning of text
     * @param count the amount of characters to set
     */
    void unsetMisspelled(int start, int count) Q_DECL_OVERRIDE;

    /**
     * Reimplemented to set the color of the misspelled word to a color
     * defined by setQuoteColor().
     */
    void setMisspelled(int start, int count) Q_DECL_OVERRIDE;

private:
    class RichTextComposerEmailQuoteHighlighterPrivate;
    RichTextComposerEmailQuoteHighlighterPrivate *const d;
};

}

#endif // RICHTEXTCOMPOSEREMAILQUOTEHIGHLIGHTER_H
