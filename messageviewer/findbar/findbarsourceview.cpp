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
#include "findbarsourceview.h"

#include <KLocale>
#include <KLineEdit>
#include <QPlainTextEdit>
#include <QAction>

using namespace MessageViewer;

FindBarSourceView::FindBarSourceView( QPlainTextEdit * view, QWidget * parent )
  : FindBarBase( parent ), m_view( view )
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
  if ( m_caseSensitiveAct->isChecked() )
    searchOptions |= QTextDocument::FindCaseSensitively;

  if ( isAutoSearch )
  {
    QTextCursor cursor = m_view->textCursor();
    cursor.setPosition( cursor.selectionStart() );
    m_view->setTextCursor( cursor );
  }
  else if( !mLastSearchStr.contains( m_search->text(), Qt::CaseSensitive ))
  {
    clearSelections();
  }
  mLastSearchStr = m_search->text();
  const bool found = m_view->find( mLastSearchStr, searchOptions );

  setFoundMatch( found );
  FindBarBase::messageInfo( backward, isAutoSearch, found );  
}


void FindBarSourceView::clearSelections()
{
  QTextCursor textCursor = m_view->textCursor();
  textCursor.clearSelection();
  textCursor.setPosition( 0 );
  m_view->setTextCursor( textCursor );
                          
  FindBarBase::clearSelections();
}

void FindBarSourceView::updateHighLight(bool)
{
  clearSelections();
}

void FindBarSourceView::updateSensitivity(bool)
{
  QTextDocument::FindFlags searchOptions = 0;
  if ( m_caseSensitiveAct->isChecked() )
    searchOptions |= QTextDocument::FindCaseSensitively;
  mLastSearchStr = m_search->text();
  const bool found = m_view->find( mLastSearchStr, searchOptions );
  setFoundMatch( found );
}


#include "findbarsourceview.moc"
