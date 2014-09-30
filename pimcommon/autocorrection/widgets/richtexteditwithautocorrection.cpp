/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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


#include "richtexteditwithautocorrection.h"
#include "pimcommon/autocorrection/autocorrection.h"

#include <QKeyEvent>

using namespace PimCommon;

RichTextEditWithAutoCorrection::RichTextEditWithAutoCorrection(QWidget *parent)
    : PimCommon::RichTextEditor(parent),
      mAutoCorrection(new PimCommon::AutoCorrection()),
      mNeedToDelete(true)
{
}

RichTextEditWithAutoCorrection::~RichTextEditWithAutoCorrection()
{
    if (mNeedToDelete)
        delete mAutoCorrection;
}

void RichTextEditWithAutoCorrection::setAutocorrection(PimCommon::AutoCorrection *autocorrect)
{
    mNeedToDelete = false;
    delete mAutoCorrection;
    mAutoCorrection = autocorrect;
}

AutoCorrection *RichTextEditWithAutoCorrection::autocorrection() const
{
    return mAutoCorrection;
}

void RichTextEditWithAutoCorrection::setAutocorrectionLanguage(const QString &language)
{
    mAutoCorrection->setLanguage(language);
}

static bool isSpecial( const QTextCharFormat &charFormat )
{
    return charFormat.isFrameFormat() || charFormat.isImageFormat() ||
            charFormat.isListFormat() || charFormat.isTableFormat() || charFormat.isTableCellFormat();
}

void RichTextEditWithAutoCorrection::keyPressEvent ( QKeyEvent *e )
{
    if (mAutoCorrection && mAutoCorrection->isEnabledAutoCorrection()) {
        if ((e->key() == Qt::Key_Space) || (e->key() == Qt::Key_Enter) || (e->key() == Qt::Key_Return)) {
            if (!textCursor().hasSelection()) {
                const QTextCharFormat initialTextFormat = textCursor().charFormat();
                const bool richText = acceptRichText();
                int position = textCursor().position();
                const bool addSpace = mAutoCorrection->autocorrect(richText, *document(), position);
                QTextCursor cur = textCursor();
                cur.setPosition(position);
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
                return;
            }
        }
    }
    PimCommon::RichTextEditor::keyPressEvent( e );
}
