/**
 * kmeditor.h
 *
 * Copyright 2007 Laurent Montel <montel@kde.org>
 * Copyright 2008 Thomas McGuire <mcguire@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */

#ifndef MESSAGECOMPOSER_KMEDITOR_H
#define MESSAGECOMPOSER_KMEDITOR_H

#include "messagecomposer_export.h"

#include <KPIMTextEdit/TextEdit>

namespace KPIMIdentities {
class Signature;
}
namespace PimCommon {
class AutoCorrection;
}
namespace MessageComposer {
class TextPart;
class KMeditorPrivate;

/**
 * The KMeditor class provides a widget to edit and display text,
 * specially geared towards writing e-mails.
 *
 * It offers sevaral additional functions of a KRichTextWidget:
 *
 * @li The ability to use an external editor
 * @li Utility functions like removing whitespace, inserting a file,
 *     adding quotes or rot13'ing the text
 */
class MESSAGECOMPOSER_EXPORT KMeditor : public KPIMTextEdit::TextEdit
{
    Q_OBJECT

public:

    /**
     * Constructs a KMeditor object
     */
    explicit KMeditor( const QString &text, QWidget *parent = 0 );

    /**
     * Constructs a KMeditor object.
     */
    explicit KMeditor( QWidget *parent = 0 );

    /**
     * Constructs a KMeditor object.
     */
    explicit KMeditor( QWidget *parent, const QString& configFile );


    virtual ~KMeditor();

    virtual int quoteLength( const QString& line ) const;
    virtual const QString defaultQuoteSign() const;

    /**
     * Sets a quote prefix. Lines starting with the passed quote prefix will
     * be highlighted as quotes (in addition to lines that are starting with
     * '>' and '|').
     */
    void setQuotePrefixName( const QString &quotePrefix );

    /**
     * @return the quote prefix set before with setQuotePrefixName(), or an empty
     *         string if that was never called.
     */
    virtual QString quotePrefixName() const;

    //Redefine it for each apps
    virtual QString smartQuote( const QString & msg ); //need by kmail

    void setUseExternalEditor( bool use );
    void setExternalEditorPath( const QString & path );
    bool checkExternalEditorFinished();
    void killExternalEditor();

    /**
     * Show the open file dialog and returns the selected URL there.
     * The file dialog has an encoding combobox displayed, and the selected
     * encoding there will be set as the encoding of the URL's fileEncoding().
     */
    KUrl insertFile();

    /**
     * Enables word wrap. Words will be wrapped at the specified column.
     *
     * @param wrapColumn the column where words will be wrapped
     */
    void enableWordWrap( int wrapColumn );

    /**
     * Disables word wrap.
     * Note that words are still wrapped at the end of the editor; no scrollbar
     * will appear.
     */
    void disableWordWrap();

    /**
     * Changes the font of the whole text.
     * Also sets the default font for the document.
     *
     * @param font the font that the whole text will get
     */
    void setFontForWholeText( const QFont &font );

    void setCursorPositionFromStart( unsigned int pos );

    /**
     * @return the line number where the cursor is. This takes word-wrapping
     *         into account. Line numbers start at 0.
     */
    int linePosition();

    /**
     * @return the column numbe where the cursor is.
     */
    int columnNumber();

    /**
     * Reimplemented again to work around a bug (see comment in implementation).
     * FIXME: This is _not_ virtual in the base class
     */
    void ensureCursorVisible();

    /**
     * Cleans the whitespace of the edit's text.
     * Adjacent tabs and spaces will be converted to a single space.
     * Trailing whitespace will be removed.
     * More than 2 newlines in a row will be changed to 2 newlines.
     * Text in quotes or text inside of the given signature will not be
     * cleaned.
     * For undo/redo, this is treated as one operation.
     *
     * @param sig text inside this signature will not be cleaned
     */
    void cleanWhitespace( const KPIMIdentities::Signature &sig );

    /**
     * Replaces all occurrences of the old signature with the new signature.
     * Text in quotes will be ignored.
     * For undo/redo, this is treated as one operation.
     * If the old signature is empty, nothing is done.
     * If the new signature is empty, the old signature including the
     * separator is removed.
     *
     * @param oldSig the old signature, which will be replaced
     * @param newSig the new signature
     * @return @p true if oldSig was found (and replaced) at least once
     */
    bool replaceSignature( const KPIMIdentities::Signature &oldSig,
                           const KPIMIdentities::Signature &newSig );

    /**
     * Fill the given composer MessageComposer::TextPart with what's in the editor currently.
     * @param textPart The MessageComposer::TextPart to fill.
     */
    void fillComposerTextPart( MessageComposer::TextPart* textPart ) const;

    PimCommon::AutoCorrection *autocorrection() const;

    void setAutocorrection(PimCommon::AutoCorrection* autocorrect);

    void setAutocorrectionLanguage(const QString& lang);

    void forcePlainTextMarkup(bool force);

    void insertLink(const QString &url);

    void insertShareLink(const QString &url);

public Q_SLOTS:
    void startExternalEditor();
    void slotAddQuotes();
    void slotPasteAsQuotation();
    void slotRemoveQuotes();
    void slotPasteWithoutFormatting();
    void slotChangeInsertMode();
    void insertPlainTextImplementation();
Q_SIGNALS:

    /**
     * Emitted whenever the foucs is lost or gained
     *
     * @param focusGained true if the focus was gained, false when it was lost
     */
    void focusChanged( bool focusGained );

    /**
     * Emitted when the user uses the up arrow in the first line. The application
     * should then put the focus on the widget above the text edit.
     */
    void focusUp();

    void insertModeChanged();
    void externalEditorStarted();
    void externalEditorClosed();

protected:

    /**
     * Reimplemented to start the external editor and to emit focusUp().
     */
    virtual void keyPressEvent ( QKeyEvent * e );

private:
    KMeditorPrivate *const d;
    friend class KMeditorPrivate;
    Q_PRIVATE_SLOT( d, void ensureCursorVisibleDelayed() )
    Q_PRIVATE_SLOT( d, void slotEditorFinished( int, QProcess::ExitStatus ) )
    Q_PRIVATE_SLOT( d, void slotAddAutoCorrect( QString, QString ) )
};

}

#endif
