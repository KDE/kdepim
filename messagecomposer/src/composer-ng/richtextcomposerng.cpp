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

#include "richtextcomposerng.h"
#include <kpimtextedit/richtextcomposercontroler.h>
#include <kpimtextedit/richtextcomposerimages.h>
#include "richtextcomposersignatures.h"
#include <pimcommon/autocorrection.h>

#include <QTextBlock>

#include <part/textpart.h>
#include "settings/messagecomposersettings.h"
#include <grantlee/markupdirector.h>
#include <grantlee/plaintextmarkupbuilder.h>

using namespace MessageComposer;

class MessageComposer::RichTextComposerNgPrivate
{
public:
    RichTextComposerNgPrivate(RichTextComposerNg *q)
        : autoCorrection(Q_NULLPTR),
          richtextComposer(q)
    {
        richTextComposerSignatures = new MessageComposer::RichTextComposerSignatures(richtextComposer, richtextComposer);
    }

    void fixHtmlFontSize(QString &cleanHtml);
    QString toCleanHtml() const;
    PimCommon::AutoCorrection *autoCorrection;
    RichTextComposerNg *richtextComposer;
    MessageComposer::RichTextComposerSignatures *richTextComposerSignatures;
};

RichTextComposerNg::RichTextComposerNg(QWidget *parent)
    : KPIMTextEdit::RichTextComposer(parent),
      d(new MessageComposer::RichTextComposerNgPrivate(this))
{

}

RichTextComposerNg::~RichTextComposerNg()
{
    delete d;
}

MessageComposer::RichTextComposerSignatures *RichTextComposerNg::composerSignature() const
{
    return d->richTextComposerSignatures;
}

PimCommon::AutoCorrection *RichTextComposerNg::autocorrection() const
{
    return d->autoCorrection;
}

void RichTextComposerNg::setAutocorrection(PimCommon::AutoCorrection *autocorrect)
{
    d->autoCorrection = autocorrect;
}

void RichTextComposerNg::setAutocorrectionLanguage(const QString &lang)
{
    if (d->autoCorrection) {
        d->autoCorrection->setLanguage(lang);
    }
}

static bool isSpecial(const QTextCharFormat &charFormat)
{
    return charFormat.isFrameFormat() || charFormat.isImageFormat() ||
           charFormat.isListFormat() || charFormat.isTableFormat() || charFormat.isTableCellFormat();
}

bool RichTextComposerNg::processAutoCorrection(QKeyEvent *e)
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
                return true;
            }
        }
    }
    return false;
}

void RichTextComposerNgPrivate::fixHtmlFontSize(QString &cleanHtml)
{
    static const QString FONTSTYLEREGEX = QStringLiteral("<span style=\".*font-size:(.*)pt;.*</span>");
    QRegExp styleRegex(FONTSTYLEREGEX);
    styleRegex.setMinimal(true);

    int offset = styleRegex.indexIn(cleanHtml, 0);
    while (offset != -1) {
        // replace all the matching text with the new line text
        bool ok = false;
        const QString fontSizeStr = styleRegex.cap(1);
        const int ptValue = fontSizeStr.toInt(&ok);
        if (ok) {
            double emValue = (double)ptValue / 12;
            const QString emValueStr = QString::number(emValue, 'g', 2);
            cleanHtml.replace(styleRegex.pos(1), QString(fontSizeStr + QLatin1String("px")).length(), emValueStr + QLatin1String("em"));
        }
        // advance the search offset to just beyond the last replace
        offset += styleRegex.matchedLength();
        // find the next occurrence
        offset = styleRegex.indexIn(cleanHtml, offset);
    }
}

void RichTextComposerNg::fillComposerTextPart(MessageComposer::TextPart *textPart)
{
    if (composerControler()->isFormattingUsed() && MessageComposer::MessageComposerSettings::self()->improvePlainTextOfHtmlMessage()) {
        Grantlee::PlainTextMarkupBuilder *pb = new Grantlee::PlainTextMarkupBuilder();

        Grantlee::MarkupDirector *pmd = new Grantlee::MarkupDirector(pb);
        pmd->processDocument(document());
        const QString plainText = pb->getResult();
        textPart->setCleanPlainText(composerControler()->toCleanPlainText(plainText));
        QTextDocument *doc = new QTextDocument(plainText);
        doc->adjustSize();

        textPart->setWrappedPlainText(composerControler()->toWrappedPlainText(doc));
        delete doc;
        delete pmd;
        delete pb;
    } else {
        textPart->setCleanPlainText(composerControler()->toCleanPlainText());
        textPart->setWrappedPlainText(composerControler()->toWrappedPlainText());
    }
    textPart->setWordWrappingEnabled(lineWrapMode() == QTextEdit::FixedColumnWidth);
    if (composerControler()->isFormattingUsed()) {
        QString cleanHtml = d->toCleanHtml();
        d->fixHtmlFontSize(cleanHtml);
        textPart->setCleanHtml(cleanHtml);
        textPart->setEmbeddedImages(composerControler()->composerImages()->embeddedImages());
    }

}

QString RichTextComposerNgPrivate::toCleanHtml() const
{
    QString result = richtextComposer->toHtml();

    static const QString EMPTYLINEHTML = QStringLiteral(
            "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; "
            "margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; \">&nbsp;</p>");

    // Qt inserts various style properties based on the current mode of the editor (underline,
    // bold, etc), but only empty paragraphs *also* have qt-paragraph-type set to 'empty'.
    static const QString EMPTYLINEREGEX = QStringLiteral(
            "<p style=\"-qt-paragraph-type:empty;(.*)</p>");

    static const QString OLLISTPATTERNQT = QStringLiteral(
            "<ol style=\"margin-top: 0px; margin-bottom: 0px; margin-left: 0px;");

    static const QString ULLISTPATTERNQT = QStringLiteral(
            "<ul style=\"margin-top: 0px; margin-bottom: 0px; margin-left: 0px;");

    static const QString ORDEREDLISTHTML = QStringLiteral(
            "<ol style=\"margin-top: 0px; margin-bottom: 0px;");

    static const QString UNORDEREDLISTHTML = QStringLiteral(
                "<ul style=\"margin-top: 0px; margin-bottom: 0px;");

    // fix 1 - empty lines should show as empty lines - MS Outlook treats margin-top:0px; as
    // a non-existing line.
    // Although we can simply remove the margin-top style property, we still get unwanted results
    // if you have three or more empty lines. It's best to replace empty <p> elements with <p>&nbsp;</p>.

    QRegExp emptyLineFinder(EMPTYLINEREGEX);
    emptyLineFinder.setMinimal(true);

    // find the first occurrence
    int offset = emptyLineFinder.indexIn(result, 0);
    while (offset != -1) {
        // replace all the matching text with the new line text
        result.replace(offset, emptyLineFinder.matchedLength(), EMPTYLINEHTML);
        // advance the search offset to just beyond the last replace
        offset += EMPTYLINEHTML.length();
        // find the next occurrence
        offset = emptyLineFinder.indexIn(result, offset);
    }

    // fix 2a - ordered lists - MS Outlook treats margin-left:0px; as
    // a non-existing number; e.g: "1. First item" turns into "First Item"
    result.replace(OLLISTPATTERNQT, ORDEREDLISTHTML);

    // fix 2b - unordered lists - MS Outlook treats margin-left:0px; as
    // a non-existing bullet; e.g: "* First bullet" turns into "First Bullet"
    result.replace(ULLISTPATTERNQT, UNORDEREDLISTHTML);

    return result;
}

static bool isCursorAtEndOfLine(const QTextCursor &cursor)
{
    QTextCursor testCursor = cursor;
    testCursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    return !testCursor.hasSelection();
}

static void insertSignatureHelper(const QString &signature,
                                  RichTextComposerNg *textEdit,
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

void RichTextComposerNg::insertSignature(const KIdentityManagement::Signature &signature, KIdentityManagement::Signature::Placement placement, KIdentityManagement::Signature::AddedText addedText)
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
                composerControler()->composerImages()->loadImage(image->image, image->name, image->name);
            }
        }

    }
}

QString RichTextComposerNg::toCleanHtml() const
{
    return d->toCleanHtml();
}
