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

#include "kpimtextedit/richtexteditor.h"
#include "messagecomposer_export.h"
#include <KIdentityManagement/Signature>
class KActionCollection;
namespace PimCommon
{
class AutoCorrection;
}

namespace MessageComposer
{
class RichTextComposerSignatures;
class RichTextComposerControler;
class RichTextComposerActions;
class RichTextExternalComposer;
class RichTextComposerEmailQuoteHighlighter;
class MESSAGECOMPOSER_EXPORT RichTextComposer : public KPIMTextEdit::RichTextEditor
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
    void switchToPlainText();

    void setTextOrHtml(const QString &text);
    QString textOrHtml() const;

    virtual void setHighlighterColors(MessageComposer::RichTextComposerEmailQuoteHighlighter *highlighter);

    void setUseExternalEditor(bool use);
    void setExternalEditorPath(const QString &path);
    bool checkExternalEditorFinished();
    void killExternalEditor();

    //Redefine it for each apps
    virtual QString smartQuote(const QString &msg);    //need by kmail

    void setQuotePrefixName(const QString &quotePrefix);
    QString quotePrefixName() const;

    void setCursorPositionFromStart(unsigned int pos);
    int quoteLength(const QString &line) const;
    bool isLineQuoted(const QString &line) const;
    const QString defaultQuoteSign() const;
    void createActions(KActionCollection *ac);

    QList<QAction *> richTextActionList() const;
    void insertSignature(const KIdentityManagement::Signature &signature, KIdentityManagement::Signature::Placement placement, KIdentityManagement::Signature::AddedText addedText);

    MessageComposer::RichTextComposerSignatures *composerSignature() const;
    MessageComposer::RichTextComposerControler *composerControler() const;
    MessageComposer::RichTextExternalComposer *externalComposer() const;
    MessageComposer::RichTextComposerActions *composerActions() const;
    void createHighlighter() Q_DECL_OVERRIDE;
public Q_SLOTS:
    void insertPlainTextImplementation();
    void slotChangeInsertMode();

Q_SIGNALS:
    void insertModeChanged();
    /**
     * Emitted whenever the text mode is changed.
     *
     * @param mode The new text mode
     */
    void textModeChanged(MessageComposer::RichTextComposer::Mode mode);
    /**
     * Emitted when the user uses the up arrow in the first line. The application
     * should then put the focus on the widget above the text edit.
     */
    void focusUp();

    void externalEditorStarted();
    void externalEditorClosed();

protected:
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    Sonnet::SpellCheckDecorator *createSpellCheckDecorator() Q_DECL_OVERRIDE;
    void insertFromMimeData(const QMimeData *source) Q_DECL_OVERRIDE;
    bool canInsertFromMimeData(const QMimeData *source) const Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void clearDecorator() Q_DECL_OVERRIDE;
    void updateHighLighter() Q_DECL_OVERRIDE;
    bool processKeyEvent(QKeyEvent *e);
private Q_SLOTS:
    void slotTextModeChanged(MessageComposer::RichTextComposer::Mode mode);

private:
    void evaluateListSupport(QKeyEvent *event);
    void evaluateReturnKeySupport(QKeyEvent *event);
    bool processAutoCorrection(QKeyEvent *e);
    class RichTextComposerPrivate;
    RichTextComposerPrivate *const d;
};
}
#endif // RICHTEXTCOMPOSER_H
