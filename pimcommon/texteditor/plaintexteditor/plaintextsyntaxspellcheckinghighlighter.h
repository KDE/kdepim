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

#ifndef PLAINTEXTSYNTAXSPELLCHECKINGHIGHLIGHTER_H
#define PLAINTEXTSYNTAXSPELLCHECKINGHIGHLIGHTER_H

#include "pimcommon_export.h"

#include <Sonnet/Highlighter>
namespace KPIMTextEdit
{
class Rule;
}

namespace PimCommon
{
class PlainTextEditor;
class PIMCOMMON_EXPORT PlainTextSyntaxSpellCheckingHighlighter: public Sonnet::Highlighter
{
public:
    explicit PlainTextSyntaxSpellCheckingHighlighter(PlainTextEditor *plainText, const QColor &misspelledColor = Qt::red);
    ~PlainTextSyntaxSpellCheckingHighlighter();

    void toggleSpellHighlighting(bool on);

    /**
     * Reimplemented to highlight quote blocks.
     */
    void highlightBlock(const QString &text) Q_DECL_OVERRIDE;

    void setSyntaxHighlighterRules(const QVector<KPIMTextEdit::Rule> &rule);
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
    class PlainTextSyntaxSpellCheckingHighlighterPrivate;
    PlainTextSyntaxSpellCheckingHighlighterPrivate *const d;
};
}
#endif // PLAINTEXTSYNTAXSPELLCHECKINGHIGHLIGHTER_H
