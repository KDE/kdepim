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

#ifndef RICHTEXTCOMPOSER_H
#define RICHTEXTCOMPOSER_H

#include "pimcommon/texteditor/richtexteditor/richtexteditor.h"
#include "messagecomposer_export.h"
namespace PimCommon
{
class AutoCorrection;
}

namespace MessageComposer
{
class MESSAGECOMPOSER_EXPORT RichTextComposer : public PimCommon::RichTextEditor
{
    Q_OBJECT
public:
    explicit RichTextComposer(QWidget *parent = Q_NULLPTR);
    ~RichTextComposer();

    enum Mode {
        Plain,    ///< Plain text mode
        Rich      ///< Rich text mode
    };

    /**
     * @return The current text mode
     */
    Mode textMode() const;

    PimCommon::AutoCorrection *autocorrection() const;
    void setAutocorrection(PimCommon::AutoCorrection *autocorrect);
    void setAutocorrectionLanguage(const QString &lang);

    /**
     * Enables word wrap. Words will be wrapped at the specified column.
     *
     * @param wrapColumn the column where words will be wrapped
     */
    void enableWordWrap(int wrapColumn);

    /**
     * Disables word wrap.
     * Note that words are still wrapped at the end of the editor; no scrollbar
     * will appear.
     */
    void disableWordWrap();

    /**
     * @return the line number where the cursor is. This takes word-wrapping
     *         into account. Line numbers start at 0.
     */
    int linePosition() const;

    /**
     * @return the column numbe where the cursor is.
     */
    int columnNumber() const;

    void forcePlainTextMarkup(bool force);

    void activateRichText();

public Q_SLOTS:
    void insertPlainTextImplementation();    
    void slotChangeInsertMode();
    void slotPasteAsQuotation();
    void slotPasteWithoutFormatting();
Q_SIGNALS:
    void insertModeChanged();
    /**
     * Emitted whenever the text mode is changed.
     *
     * @param mode The new text mode
     */
    void textModeChanged(MessageComposer::RichTextComposer::Mode mode);

private:
    class RichTextComposerPrivate;
    RichTextComposerPrivate *const d;
};
}
#endif // RICHTEXTCOMPOSER_H
