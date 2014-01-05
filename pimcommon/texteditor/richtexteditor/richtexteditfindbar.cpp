/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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

#include "richtexteditfindbar.h"
#include "pimcommon/texteditor/commonwidget/textfindreplacewidget.h"

#include <kicon.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <klineedit.h>
#include <KColorScheme>

#include <QTextEdit>

using namespace PimCommon;

RichTextEditFindBar::RichTextEditFindBar( QTextEdit * view, QWidget * parent )
    : TextEditFindBarBase( parent ),
      mView( view )
{
}

RichTextEditFindBar::~RichTextEditFindBar()
{
}

void RichTextEditFindBar::slotSearchText( bool backward, bool isAutoSearch )
{
    mView->moveCursor(QTextCursor::Start);
    searchText( backward, isAutoSearch );
}

bool RichTextEditFindBar::viewIsReadOnly() const
{
    return mView->isReadOnly();
}

bool RichTextEditFindBar::documentIsEmpty() const
{
    return mView->document()->isEmpty();
}

bool RichTextEditFindBar::searchInDocument(const QString &text, QTextDocument::FindFlags searchOptions)
{
    const bool found = mView->find( text, searchOptions );
    mFindWidget->setFoundMatch( found );
    return found;
}

void RichTextEditFindBar::autoSearchMoveCursor()
{
    QTextCursor cursor = mView->textCursor();
    cursor.setPosition( cursor.selectionStart() );
    mView->setTextCursor( cursor );
}

void RichTextEditFindBar::slotReplaceText()
{
    //FIXME!
    if (mView->textCursor().hasSelection()) {
        if (mView->textCursor().selectedText() == mFindWidget->search()->text()) {
            mView->textCursor().insertText(mReplaceWidget->replace()->text());
            //search next after replace text.
            searchText(false, false);
        }
    } else {
        searchText( false, false );
    }
}

void RichTextEditFindBar::slotReplaceAllText()
{
    //FIXME richtext
    mView->setPlainText(mView->toPlainText().replace(mFindWidget->findRegExp(), mReplaceWidget->replace()->text()));
}

