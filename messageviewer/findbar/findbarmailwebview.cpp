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

#include "findbarmailwebview.h"
#include "viewer/mailwebview.h"

#include <KLocale>
#include <KLineEdit>
#include <QMenu>
using namespace MessageViewer;

FindBarMailWebView::FindBarMailWebView( MailWebView * view, QWidget * parent )
    : FindBarBase( parent ), mView( view )
{  
#ifndef MESSAGEVIEWER_FINDBAR_NO_HIGHLIGHT_ALL
    QMenu *options = optionsMenu();
    mHighlightAll = options->addAction( i18n( "Highlight all matches" ) );
    mHighlightAll->setCheckable( true );
    connect( mHighlightAll, SIGNAL(toggled(bool)), this, SLOT(slotHighlightAllChanged(bool)) );
#endif
}

FindBarMailWebView::~FindBarMailWebView()
{
}

void FindBarMailWebView::searchText( bool backward, bool isAutoSearch )
{
    MailWebView::FindFlags searchOptions = MailWebView::FindWrapsAroundDocument;

    if ( backward )
        searchOptions |= MailWebView::FindBackward;
    if ( mCaseSensitiveAct->isChecked() )
        searchOptions |= MailWebView::FindCaseSensitively;
#ifndef MESSAGEVIEWER_FINDBAR_NO_HIGHLIGHT_ALL
    if ( mHighlightAll->isChecked() )
        searchOptions |= MailWebView::HighlightAllOccurrences;
#endif
    const QString searchWord( mSearch->text() );
    if( !isAutoSearch && !mLastSearchStr.contains( searchWord, Qt::CaseSensitive ) )
    {
        clearSelections();
    }
    mView->findText(QString(), MailWebView::HighlightAllOccurrences); //Clear an existing highligh

    mLastSearchStr = searchWord;
    const bool found = mView->findText( mLastSearchStr, searchOptions );

    setFoundMatch( found );
    FindBarBase::messageInfo( backward, isAutoSearch, found );
}

void FindBarMailWebView::updateHighLight(bool highLight)
{
#ifndef MESSAGEVIEWER_FINDBAR_NO_HIGHLIGHT_ALL
    bool found = false;
    if ( highLight ) {
        MailWebView::FindFlags searchOptions = MailWebView::FindWrapsAroundDocument;
        if ( mCaseSensitiveAct->isChecked() )
            searchOptions |= MailWebView::FindCaseSensitively;
        searchOptions |= MailWebView::HighlightAllOccurrences;
        found = mView->findText(mLastSearchStr, searchOptions);
    } else {
        found = mView->findText(QString(), MailWebView::HighlightAllOccurrences);
    }
    setFoundMatch( found );
#endif
}

void FindBarMailWebView::updateSensitivity( bool sensitivity )
{
    MailWebView::FindFlags searchOptions = MailWebView::FindWrapsAroundDocument;
    if ( sensitivity ) {
        searchOptions |= MailWebView::FindCaseSensitively;
        mView->findText(QString(), MailWebView::HighlightAllOccurrences); //Clear an existing highligh
    }
#ifndef MESSAGEVIEWER_FINDBAR_NO_HIGHLIGHT_ALL
    if ( mHighlightAll->isChecked() )
        searchOptions |= MailWebView::HighlightAllOccurrences;
#endif
    const bool found = mView->findText(mLastSearchStr, searchOptions);
    setFoundMatch( found );
}


void FindBarMailWebView::clearSelections()
{
    mView->clearFindSelection();
    FindBarBase::clearSelections();
}

#include "findbarmailwebview.moc"
