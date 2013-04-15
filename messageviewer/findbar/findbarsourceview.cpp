/* Copyright (C) 2011, 2012, 2013 Laurent Montel <montel@kde.org>
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
#include "findbarsourceview.h"

#include <KLocale>
#include <KLineEdit>
#include <QPlainTextEdit>
#include <QAction>

using namespace MessageViewer;

FindBarSourceView::FindBarSourceView( QPlainTextEdit * view, QWidget * parent )
    : FindBarBase( parent ), mView( view )
{  
}

FindBarSourceView::~FindBarSourceView()
{
}


void FindBarSourceView::searchText( bool backward, bool isAutoSearch )
{
    QTextDocument::FindFlags searchOptions = 0;
    if ( backward )
        searchOptions |= QTextDocument::FindBackward;
    if ( mCaseSensitiveAct->isChecked() )
        searchOptions |= QTextDocument::FindCaseSensitively;

    if ( isAutoSearch ) {
        QTextCursor cursor = mView->textCursor();
        cursor.setPosition( cursor.selectionStart() );
        mView->setTextCursor( cursor );
    } else if( !mLastSearchStr.contains( mSearch->text(), Qt::CaseSensitive )) {
        clearSelections();
    }
    mLastSearchStr = mSearch->text();
    const bool found = mView->find( mLastSearchStr, searchOptions );

    setFoundMatch( found );
    FindBarBase::messageInfo( backward, isAutoSearch, found );
}


void FindBarSourceView::clearSelections()
{
    QTextCursor textCursor = mView->textCursor();
    textCursor.clearSelection();
    textCursor.setPosition( 0 );
    mView->setTextCursor( textCursor );

    FindBarBase::clearSelections();
}

void FindBarSourceView::updateHighLight(bool)
{
    clearSelections();
}

void FindBarSourceView::updateSensitivity(bool)
{
    QTextDocument::FindFlags searchOptions = 0;
    if ( mCaseSensitiveAct->isChecked() )
        searchOptions |= QTextDocument::FindCaseSensitively;
    mLastSearchStr = mSearch->text();
    const bool found = mView->find( mLastSearchStr, searchOptions );
    setFoundMatch( found );
}


#include "findbarsourceview.moc"
