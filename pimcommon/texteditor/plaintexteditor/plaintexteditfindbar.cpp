/* Copyright (C) 2012, 2013 Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "plaintexteditfindbar.h"
#include "pimcommon/texteditor/commonwidget/textfindreplacewidget.h"

#include <klocale.h>
#include <kmessagebox.h>
#include <klineedit.h>
#include <KColorScheme>

#include <QPlainTextEdit>

using namespace PimCommon;

PlainTextEditFindBar::PlainTextEditFindBar( QPlainTextEdit * view, QWidget * parent )
    : TextEditFindBarBase( parent ),
      mView( view )
{
}

PlainTextEditFindBar::~PlainTextEditFindBar()
{
}

void PlainTextEditFindBar::slotSearchText( bool backward, bool isAutoSearch )
{
    mView->moveCursor(QTextCursor::Start);
    searchText( backward, isAutoSearch );
}

bool PlainTextEditFindBar::viewIsReadOnly() const
{
    return mView->isReadOnly();
}

bool PlainTextEditFindBar::documentIsEmpty() const
{
    return mView->document()->isEmpty();
}

bool PlainTextEditFindBar::searchInDocument(const QString &text, QTextDocument::FindFlags searchOptions)
{
    const bool found = mView->find( text, searchOptions );
    mFindWidget->setFoundMatch( found );
    return found;
}

void PlainTextEditFindBar::autoSearchMoveCursor()
{
    QTextCursor cursor = mView->textCursor();
    cursor.setPosition( cursor.selectionStart() );
    mView->setTextCursor( cursor );
}

void PlainTextEditFindBar::slotReplaceText()
{
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

void PlainTextEditFindBar::slotReplaceAllText()
{
    mView->setPlainText(mView->toPlainText().replace(mFindWidget->findRegExp(), mReplaceWidget->replace()->text()));
}

