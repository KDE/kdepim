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

#include "richtextcomposer.h"
#include "richtextcomposercontroler.h"
#include "richtextcomposeractions.h"
#include "richtextcomposerimages.h"
#include "richtextcomposeremailquotehighlighter.h"
#include "nestedlisthelper_p.h"
#include "richtextexternalcomposer.h"
#include "richtextcomposersignatures.h"
#include <KLocalizedString>
#include <QTextBlock>
#include <QTextLayout>
#include <QApplication>
#include <QClipboard>

#include "richtextcomposeremailquotedecorator.h"
#include <pimcommon/autocorrection.h>

#include <grantlee/plaintextmarkupbuilder.h>

#include <KActionCollection>
#include <QFileInfo>
#include <QMimeData>

#include <KPIMTextEdit/EMailQuoteHighlighter>

using namespace MessageComposer;

class Q_DECL_HIDDEN RichTextComposer::RichTextComposerPrivate
{
public:
    RichTextComposerPrivate(RichTextComposer *qq)
        : autoCorrection(Q_NULLPTR),
          q(qq),
          forcePlainTextMarkup(false),
          mode(RichTextComposer::Plain)
    {
        composerControler = new RichTextComposerControler(q, q);
        richTextComposerActions = new RichTextComposerActions(composerControler, q);
        externalComposer = new MessageComposer::RichTextExternalComposer(q);
        richTextComposerSignatures = new MessageComposer::RichTextComposerSignatures(q, q);
        q->connect(externalComposer, &RichTextExternalComposer::externalEditorClosed, qq, &RichTextComposer::externalEditorClosed);
        q->connect(externalComposer, &RichTextExternalComposer::externalEditorStarted, qq, &RichTextComposer::externalEditorStarted);
        q->connect(q, &RichTextComposer::textModeChanged, q, &RichTextComposer::slotTextModeChanged);

    }
    QString quotePrefix;
    PimCommon::AutoCorrection *autoCorrection;
    RichTextComposerControler *composerControler;
    RichTextComposerActions *richTextComposerActions;
    MessageComposer::RichTextExternalComposer *externalComposer;
    MessageComposer::RichTextComposerSignatures *richTextComposerSignatures;
    RichTextComposer *q;
    bool forcePlainTextMarkup;
    RichTextComposer::Mode mode;
};

RichTextComposer::RichTextComposer(QWidget *parent)
    : PimCommon::RichTextEditor(parent),
      d(new RichTextComposerPrivate(this))
{
    setAcceptRichText(false);
}

RichTextComposer::~RichTextComposer()
{
    delete d;
}

MessageComposer::RichTextExternalComposer *RichTextComposer::externalComposer() const
{
    return d->externalComposer;
}

MessageComposer::RichTextComposerSignatures *RichTextComposer::composerSignature() const
{
    return d->richTextComposerSignatures;
}

MessageComposer::RichTextComposerControler *RichTextComposer::composerControler() const
{
    return d->composerControler;
}

MessageComposer::RichTextComposerActions *RichTextComposer::composerActions() const
{
    return d->richTextComposerActions;
}

QList<QAction *> RichTextComposer::richTextActionList() const
{
    return d->richTextComposerActions->richTextActionList();
}

void RichTextComposer::createActions(KActionCollection *ac)
{
    d->richTextComposerActions->createActions(ac);
}

void RichTextComposer::updateHighLighter()
{
    MessageComposer::RichTextComposerEmailQuoteHighlighter *hlighter = dynamic_cast<MessageComposer::RichTextComposerEmailQuoteHighlighter *>(highlighter());
    if (hlighter) {
        hlighter->toggleSpellHighlighting(checkSpellingEnabled());
    }

}

void RichTextComposer::clearDecorator()
{
    //Nothing
}

void RichTextComposer::createHighlighter()
{
    MessageComposer::RichTextComposerEmailQuoteHighlighter *highlighter = new MessageComposer::RichTextComposerEmailQuoteHighlighter(this);
    highlighter->toggleSpellHighlighting(checkSpellingEnabled());
    setHighlighterColors(highlighter);
    setHighlighter(highlighter);
}

void RichTextComposer::setHighlighterColors(MessageComposer::RichTextComposerEmailQuoteHighlighter *highlighter)
{
    Q_UNUSED(highlighter);
}

void RichTextComposer::setUseExternalEditor(bool use)
{
    d->externalComposer->setUseExternalEditor(use);
}

void RichTextComposer::setExternalEditorPath(const QString &path)
{
    d->externalComposer->setExternalEditorPath(path);
}

bool RichTextComposer::checkExternalEditorFinished()
{
    return d->externalComposer->checkExternalEditorFinished();
}

void RichTextComposer::killExternalEditor()
{
    d->externalComposer->killExternalEditor();
}

RichTextComposer::Mode RichTextComposer::textMode() const
{
    return d->mode;
}

PimCommon::AutoCorrection *RichTextComposer::autocorrection() const
{
    return d->autoCorrection;
}

void RichTextComposer::setAutocorrection(PimCommon::AutoCorrection *autocorrect)
{
    d->autoCorrection = autocorrect;
}

void RichTextComposer::setAutocorrectionLanguage(const QString &lang)
{
    if (d->autoCorrection) {
        d->autoCorrection->setLanguage(lang);
    }
}

void RichTextComposer::enableWordWrap(int wrapColumn)
{
    setWordWrapMode(QTextOption::WordWrap);
    setLineWrapMode(QTextEdit::FixedColumnWidth);
    setLineWrapColumnOrWidth(wrapColumn);
}

void RichTextComposer::disableWordWrap()
{
    setLineWrapMode(QTextEdit::WidgetWidth);
}

int RichTextComposer::linePosition() const
{
    const QTextCursor cursor = textCursor();
    const QTextDocument *doc = document();
    QTextBlock block = doc->begin();
    int lineCount = 0;

    // Simply using cursor.block.blockNumber() would not work since that does not
    // take word-wrapping into account, i.e. it is possible to have more than one
    // line in a block.
    //
    // What we have to do therefore is to iterate over the blocks and count the
    // lines in them. Once we have reached the block where the cursor is, we have
    // to iterate over each line in it, to find the exact line in the block where
    // the cursor is.
    while (block.isValid()) {
        const QTextLayout *layout = block.layout();

        // If the current block has the cursor in it, iterate over all its lines
        if (block == cursor.block()) {

            // Special case: Cursor at end of single non-wrapped line, exit early
            // in this case as the logic below can't handle it
            if (block.lineCount() == layout->lineCount()) {
                return lineCount;
            }

            const int cursorBasePosition = cursor.position() - block.position();
            const int numberOfLine(layout->lineCount());
            for (int i = 0; i < numberOfLine; ++i) {
                QTextLine line = layout->lineAt(i);
                if (cursorBasePosition >= line.textStart() &&
                        cursorBasePosition < line.textStart() + line.textLength()) {
                    break;
                }
                lineCount++;
            }
            return lineCount;
        } else {
            // No, cursor is not in the current block
            lineCount += layout->lineCount();
        }

        block = block.next();
    }

    // Only gets here if the cursor block can't be found, shouldn't happen except
    // for an empty document maybe
    return lineCount;
}

int RichTextComposer::columnNumber() const
{
    const QTextCursor cursor = textCursor();
    return cursor.columnNumber();
}

void RichTextComposer::forcePlainTextMarkup(bool force)
{
    d->forcePlainTextMarkup = force;
}

void RichTextComposer::insertPlainTextImplementation()
{
    if (d->forcePlainTextMarkup) {
        Grantlee::PlainTextMarkupBuilder *pb = new Grantlee::PlainTextMarkupBuilder();

        Grantlee::MarkupDirector *pmd = new Grantlee::MarkupDirector(pb);
        pmd->processDocument(document());
        const QString plainText = pb->getResult();
        document()->setPlainText(plainText);
        delete pmd;
        delete pb;
    } else {
        document()->setPlainText(document()->toPlainText());
    }
}

void RichTextComposer::slotChangeInsertMode()
{
    setOverwriteMode(!overwriteMode());
    Q_EMIT insertModeChanged();
}

void RichTextComposer::activateRichText()
{
    if (d->mode == RichTextComposer::Plain) {
        setAcceptRichText(true);
        d->mode = RichTextComposer::Rich;
        Q_EMIT textModeChanged(d->mode);
    }
}

void RichTextComposer::switchToPlainText()
{
    if (d->mode == RichTextComposer::Rich) {
        d->mode = RichTextComposer::Plain;
        // TODO: Warn the user about this?
        insertPlainTextImplementation();
        setAcceptRichText(false);
        Q_EMIT textModeChanged(d->mode);
    }
}

QString RichTextComposer::textOrHtml() const
{
    if (textMode() == Rich) {
        return d->composerControler->toCleanHtml();
    } else {
        return toPlainText();
    }
}

void RichTextComposer::setTextOrHtml(const QString &text)
{
    // might be rich text
    if (Qt::mightBeRichText(text)) {
        if (d->mode == RichTextComposer::Plain) {
            activateRichText();
        }
        setHtml(text);
    } else {
        setPlainText(text);
    }
}

void RichTextComposer::evaluateReturnKeySupport(QKeyEvent *event)
{
    if (event->key() ==  Qt::Key_Return) {
        QTextCursor cursor = textCursor();
        int oldPos = cursor.position();
        int blockPos = cursor.block().position();

        //selection all the line.
        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        QString lineText = cursor.selectedText();
        if (((oldPos - blockPos)  > 0) &&
                ((oldPos - blockPos) < int(lineText.length()))) {
            bool isQuotedLine = false;
            int bot = 0; // bot = begin of text after quote indicators
            while (bot < lineText.length()) {
                if ((lineText[bot] == QChar::fromLatin1('>')) ||
                        (lineText[bot] == QChar::fromLatin1('|'))) {
                    isQuotedLine = true;
                    ++bot;
                } else if (lineText[bot].isSpace()) {
                    ++bot;
                } else {
                    break;
                }
            }
            evaluateListSupport(event);
            // duplicate quote indicators of the previous line before the new
            // line if the line actually contained text (apart from the quote
            // indicators) and the cursor is behind the quote indicators
            if (isQuotedLine &&
                    (bot != lineText.length()) &&
                    ((oldPos - blockPos) >= int(bot))) {
                // The cursor position might have changed unpredictably if there was selected
                // text which got replaced by a new line, so we query it again:
                cursor.movePosition(QTextCursor::StartOfBlock);
                cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
                QString newLine = cursor.selectedText();

                // remove leading white space from the new line and instead
                // add the quote indicators of the previous line
                int leadingWhiteSpaceCount = 0;
                while ((leadingWhiteSpaceCount < newLine.length()) &&
                        newLine[leadingWhiteSpaceCount].isSpace()) {
                    ++leadingWhiteSpaceCount;
                }
                newLine = newLine.replace(0, leadingWhiteSpaceCount, lineText.left(bot));
                cursor.insertText(newLine);
                //cursor.setPosition( cursor.position() + 2 );
                cursor.movePosition(QTextCursor::StartOfBlock);
                setTextCursor(cursor);
            }
        } else {
            evaluateListSupport(event);
        }
    } else {
        evaluateListSupport(event);
    }
}

void RichTextComposer::evaluateListSupport(QKeyEvent *event)
{
    bool handled = false;
    if (textCursor().currentList()) {
        // handled is False if the key press event was not handled or not completely
        // handled by the Helper class.
        handled = d->composerControler->nestedListHelper()->handleBeforeKeyPressEvent(event);
    }

    if (!handled) {
        PimCommon::RichTextEditor::keyPressEvent(event);
    }

    if (textCursor().currentList()) {
        d->composerControler->nestedListHelper()->handleAfterKeyPressEvent(event);
    }
    Q_EMIT cursorPositionChanged();
}

static bool isSpecial(const QTextCharFormat &charFormat)
{
    return charFormat.isFrameFormat() || charFormat.isImageFormat() ||
           charFormat.isListFormat() || charFormat.isTableFormat() || charFormat.isTableCellFormat();
}

void RichTextComposer::processAutoCorrection(QKeyEvent *e)
{
    if (d->autoCorrection && d->autoCorrection->isEnabledAutoCorrection()) {
        if ((e->key() == Qt::Key_Space) || (e->key() == Qt::Key_Enter) || (e->key() == Qt::Key_Return)) {
            if (!isLineQuoted(textCursor().block().text()) && !textCursor().hasSelection()) {
                const QTextCharFormat initialTextFormat = textCursor().charFormat();
                const bool richText = (textMode() == RichTextComposer::Rich);
                int position = textCursor().position();
                const bool addSpace = d->autoCorrection->autocorrect(richText, *document(), position);
                QTextCursor cur = textCursor();
                cur.setPosition(position);
                if (overwriteMode() && e->key() == Qt::Key_Space) {
                    if (addSpace) {
                        const QChar insertChar = QLatin1Char(' ');
                        if (!cur.atBlockEnd()) {
                            cur.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 1);
                        }
                        if (richText && !isSpecial(initialTextFormat)) {
                            cur.insertText(insertChar, initialTextFormat);
                        } else {
                            cur.insertText(insertChar);
                        }
                        setTextCursor(cur);
                    }
                } else {
                    const bool spacePressed = (e->key() == Qt::Key_Space);
                    const QChar insertChar = spacePressed ? QLatin1Char(' ') : QLatin1Char('\n');
                    if (richText && !isSpecial(initialTextFormat)) {
                        if ((spacePressed && addSpace) || !spacePressed) {
                            cur.insertText(insertChar, initialTextFormat);
                        }
                    } else {
                        if ((spacePressed && addSpace) || !spacePressed) {
                            cur.insertText(insertChar);
                        }
                    }
                    setTextCursor(cur);
                }
            }
        }
    }
}

bool RichTextComposer::processKeyEvent(QKeyEvent *e)
{
    if (d->externalComposer->useExternalEditor() &&
            (e->key() != Qt::Key_Shift) &&
            (e->key() != Qt::Key_Control) &&
            (e->key() != Qt::Key_Meta) &&
            (e->key() != Qt::Key_CapsLock) &&
            (e->key() != Qt::Key_NumLock) &&
            (e->key() != Qt::Key_ScrollLock) &&
            (e->key() != Qt::Key_Alt) &&
            (e->key() != Qt::Key_AltGr)) {
        if (!d->externalComposer->isInProgress()) {
            d->externalComposer->startExternalEditor();
        }
        return true;
    }

    if (e->key() == Qt::Key_Up && e->modifiers() != Qt::ShiftModifier &&
            textCursor().block().position() == 0 &&
            textCursor().block().layout()->lineForTextPosition(textCursor().position()).lineNumber() == 0) {
        textCursor().clearSelection();
        Q_EMIT focusUp();
    } else if (e->key() == Qt::Key_Backtab && e->modifiers() == Qt::ShiftModifier) {
        textCursor().clearSelection();
        Q_EMIT focusUp();
    } else {
        processAutoCorrection(e);
        evaluateReturnKeySupport(e);
    }
    return true;
}

void RichTextComposer::keyPressEvent(QKeyEvent *e)
{
    processKeyEvent(e);
}

Sonnet::SpellCheckDecorator *RichTextComposer::createSpellCheckDecorator()
{
    return new MessageComposer::RichTextComposerEmailQuoteDecorator(this);
}

QString RichTextComposer::smartQuote(const QString &msg)
{
    return msg;
}

void RichTextComposer::setQuotePrefixName(const QString &quotePrefix)
{
    d->quotePrefix = quotePrefix;
}

QString RichTextComposer::quotePrefixName() const
{
    if (!d->quotePrefix.simplified().isEmpty()) {
        return d->quotePrefix;
    } else {
        return QStringLiteral(">");
    }
}

int RichTextComposer::quoteLength(const QString &line) const
{
    if (!d->quotePrefix.simplified().isEmpty()) {
        if (line.startsWith(d->quotePrefix)) {
            return d->quotePrefix.length();
        } else {
            return 0;
        }
    } else {
        bool quoteFound = false;
        int startOfText = -1;
        const int lineLength(line.length());
        for (int i = 0; i < lineLength; ++i) {
            if (line[i] == QLatin1Char('>') || line[i] == QLatin1Char('|')) {
                quoteFound = true;
            } else if (line[i] != QLatin1Char(' ')) {
                startOfText = i;
                break;
            }
        }
        if (quoteFound) {
            if (startOfText == -1) {
                startOfText = line.length() - 1;
            }
            return startOfText;
        } else {
            return 0;
        }
    }
}

void RichTextComposer::setCursorPositionFromStart(unsigned int pos)
{
    d->composerControler->setCursorPositionFromStart(pos);
}

bool RichTextComposer::isLineQuoted(const QString &line) const
{
    return quoteLength(line) > 0;
}

const QString RichTextComposer::defaultQuoteSign() const
{
    if (!d->quotePrefix.simplified().isEmpty()) {
        return d->quotePrefix;
    } else {
        return QStringLiteral("> ");
    }
}

void RichTextComposer::insertFromMimeData(const QMimeData *source)
{
    // Add an image if that is on the clipboard
    if (textMode() == RichTextComposer::Rich && source->hasImage()) {
        const QImage image = qvariant_cast<QImage>(source->imageData());
        QFileInfo fi;
        d->composerControler->composerImages()->insertImage(image, fi);
        return;
    }

    // Attempt to paste HTML contents into the text edit in plain text mode,
    // prevent this and prevent plain text instead.
    if (textMode() == RichTextComposer::Plain && source->hasHtml()) {
        if (source->hasText()) {
            insertPlainText(source->text());
            return;
        }
    }

    PimCommon::RichTextEditor::insertFromMimeData(source);
}

bool RichTextComposer::canInsertFromMimeData(const QMimeData *source) const
{
    if (source->hasHtml() && textMode() == RichTextComposer::Rich) {
        return true;
    }

    if (source->hasText()) {
        return true;
    }

    if (textMode() == RichTextComposer::Rich && source->hasImage()) {
        return true;
    }

    return PimCommon::RichTextEditor::canInsertFromMimeData(source);
}

static bool isCursorAtEndOfLine(const QTextCursor &cursor)
{
    QTextCursor testCursor = cursor;
    testCursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    return !testCursor.hasSelection();
}

static void insertSignatureHelper(const QString &signature,
                                  RichTextComposer *textEdit,
                                  KIdentityManagement::Signature::Placement placement,
                                  bool isHtml,
                                  bool addNewlines)
{
    if (!signature.isEmpty()) {

        // Save the modified state of the document, as inserting a signature
        // shouldn't change this. Restore it at the end of this function.
        bool isModified = textEdit->document()->isModified();

        // Move to the desired position, where the signature should be inserted
        QTextCursor cursor = textEdit->textCursor();
        QTextCursor oldCursor = cursor;
        cursor.beginEditBlock();

        if (placement == KIdentityManagement::Signature::End) {
            cursor.movePosition(QTextCursor::End);
        } else if (placement == KIdentityManagement::Signature::Start) {
            cursor.movePosition(QTextCursor::Start);
        } else if (placement == KIdentityManagement::Signature::AtCursor) {
            cursor.movePosition(QTextCursor::StartOfLine);
        }
        textEdit->setTextCursor(cursor);

        QString lineSep;
        if (addNewlines) {
            if (isHtml) {
                lineSep = QStringLiteral("<br>");
            } else {
                lineSep = QLatin1Char('\n');
            }
        }

        // Insert the signature and newlines depending on where it was inserted.
        int newCursorPos = -1;
        QString headSep;
        QString tailSep;

        if (placement == KIdentityManagement::Signature::End) {
            // There is one special case when re-setting the old cursor: The cursor
            // was at the end. In this case, QTextEdit has no way to know
            // if the signature was added before or after the cursor, and just
            // decides that it was added before (and the cursor moves to the end,
            // but it should not when appending a signature). See bug 167961
            if (oldCursor.position() == textEdit->toPlainText().length()) {
                newCursorPos = oldCursor.position();
            }
            headSep = lineSep;
        } else if (placement == KIdentityManagement::Signature::Start) {
            // When prepending signatures, add a couple of new lines before
            // the signature, and move the cursor to the beginning of the QTextEdit.
            // People tends to insert new text there.
            newCursorPos = 0;
            headSep = lineSep + lineSep;
            if (!isCursorAtEndOfLine(cursor)) {
                tailSep = lineSep;
            }
        } else if (placement == KIdentityManagement::Signature::AtCursor) {
            if (!isCursorAtEndOfLine(cursor)) {
                tailSep = lineSep;
            }
        }

        const QString full_signature = headSep + signature + tailSep;
        if (isHtml) {
            textEdit->insertHtml(full_signature);
        } else {
            textEdit->insertPlainText(full_signature);
        }

        cursor.endEditBlock();
        if (newCursorPos != -1) {
            oldCursor.setPosition(newCursorPos);
        }

        textEdit->setTextCursor(oldCursor);
        textEdit->ensureCursorVisible();

        textEdit->document()->setModified(isModified);

        if (isHtml) {
            textEdit->activateRichText();
        }
    }
}

void RichTextComposer::insertSignature(const KIdentityManagement::Signature &signature, KIdentityManagement::Signature::Placement placement, KIdentityManagement::Signature::AddedText addedText)
{
    if (signature.isEnabledSignature()) {
        QString signatureStr;
        if (placement & KIdentityManagement::Signature::AddSeparator) {
            signatureStr = signature.withSeparator();
        } else {
            signatureStr = signature.rawText();
        }

        insertSignatureHelper(signatureStr, this, placement,
                              (signature.isInlinedHtml() &&
                               signature.type() == KIdentityManagement::Signature::Inlined),
                              (addedText & KIdentityManagement::Signature::AddNewLines));

        // We added the text of the signature above, now it is time to add the images as well.
        if (signature.isInlinedHtml()) {
            foreach (const KIdentityManagement::Signature::EmbeddedImagePtr &image, signature.embeddedImages()) {
                d->composerControler->composerImages()->loadImage(image->image, image->name, image->name);
            }
        }

    }
}

void RichTextComposer::mouseReleaseEvent(QMouseEvent *event)
{
    if (d->composerControler->painterActive()) {
        d->composerControler->disablePainter();
        d->richTextComposerActions->uncheckActionFormatPainter();
    }
    PimCommon::RichTextEditor::mouseReleaseEvent(event);
}

void RichTextComposer::slotTextModeChanged(MessageComposer::RichTextComposer::Mode mode)
{
    d->composerControler->textModeChanged(mode);
    d->richTextComposerActions->textModeChanged(mode);
}
