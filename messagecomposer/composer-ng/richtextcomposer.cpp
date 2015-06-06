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
#include <KLocalizedString>
#include <QTextBlock>
#include <QTextLayout>
#include <QApplication>
#include <QClipboard>

#include <autocorrection/autocorrection.h>

#include <grantlee/plaintextmarkupbuilder.h>

using namespace MessageComposer;

class RichTextComposer::RichTextComposerPrivate
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
    }
    PimCommon::AutoCorrection *autoCorrection;
    RichTextComposerControler *composerControler;
    RichTextComposerActions *richTextComposerActions;
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

void RichTextComposer::slotPasteAsQuotation()
{
#ifndef QT_NO_CLIPBOARD
    if (hasFocus()) {
        const QString s = QApplication::clipboard()->text();
        if (!s.isEmpty()) {
            // FIXME insertPlainText(d->addQuotesToText(s));
        }
    }
#endif
}

void RichTextComposer::slotPasteWithoutFormatting()
{
#ifndef QT_NO_CLIPBOARD
    if (hasFocus()) {
        const QString s = QApplication::clipboard()->text();
        if (!s.isEmpty()) {
            insertPlainText(s);
        }
    }
#endif
}

void RichTextComposer::activateRichText()
{
    if (d->mode == RichTextComposer::Plain) {
        setAcceptRichText(true);
        d->mode = RichTextComposer::Rich;
        Q_EMIT textModeChanged(d->mode);
    }
}
