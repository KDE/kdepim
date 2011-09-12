/* Copyright (C) 2011 Laurent Montel <montel@kde.org>
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

#include <config-messageviewer.h>

#include "findbarmailwebview.h"
#include "mailwebview.h"

#include <KLocale>
#include <KLineEdit>
#include <QMenu>

using namespace MessageViewer;

FindBarMailWebView::FindBarMailWebView( MailWebView * view, QWidget * parent )
  : FindBarBase( parent ), m_view( view )
{  
#ifndef MESSAGEVIEWER_FINDBAR_NO_HIGHLIGHT_ALL
  QMenu * options = optionsMenu();
  m_highlightAll = options->addAction( i18n( "Highlight all matches" ) );
  m_highlightAll->setCheckable( true );
  connect( m_highlightAll, SIGNAL(toggled(bool)), this, SLOT(highlightAllChanged()) );
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
  if ( m_caseSensitiveAct->isChecked() )
    searchOptions |= MailWebView::FindCaseSensitively;
#ifndef MESSAGEVIEWER_FINDBAR_NO_HIGHLIGHT_ALL
  if ( m_highlightAll->isChecked() )
    searchOptions |= MailWebView::HighlightAllOccurrences;
#endif

  if( !mLastSearchStr.contains( m_search->text(), Qt::CaseSensitive ) )
  {
    clearSelections();
  }
  mLastSearchStr = m_search->text();
  const bool found = m_view->findText( mLastSearchStr, searchOptions );

  setFoundMatch( found );
  FindBarBase::messageInfo( backward, isAutoSearch, found );  
}


void FindBarMailWebView::clearSelections()
{
  m_view->clearFindSelection();
  FindBarBase::clearSelections();
}

#include "findbarmailwebview.moc"
