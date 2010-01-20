/* This file is part of the KDE project
 *
 * Copyright (C) 2010 Torgny Nyblom <kde nyblom org>
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

#include "findbar.h"

// qt/kde includes
#include <QtCore/QTimer>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QMenu>
#include <QtGui/QToolButton>
#include <QtWebKit/QWebView>
#include <kicon.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <klineedit.h>

using namespace MessageViewer;

FindBar::FindBar( QWebView * view, QWidget * parent )
  : QWidget( parent ), m_view( view )
{
  m_searchOptions |= QWebPage::FindWrapsAroundDocument;
  QHBoxLayout * lay = new QHBoxLayout( this );
  lay->setMargin( 2 );
  
  QToolButton * closeBtn = new QToolButton( this );
  closeBtn->setIcon( KIcon( "dialog-close" ) );
  closeBtn->setIconSize( QSize( 24, 24 ) );
  closeBtn->setToolTip( i18n( "Close" ) );
  closeBtn->setAutoRaise( true );
  lay->addWidget( closeBtn );

  QLabel * label = new QLabel( i18nc( "Find text", "F&ind:" ), this );
  lay->addWidget( label );

  m_search = new KLineEdit( this );
  m_search->setToolTip( i18n( "Text to search for" ) );
  label->setBuddy( m_search );
  lay->addWidget( m_search );
  
  QPushButton * findNextBtn = new QPushButton( KIcon( "go-down-search" ), i18nc( "Find and go to the next search match", "Next" ), this );
  findNextBtn->setToolTip( i18n( "Jump to next match" ) );
  lay->addWidget( findNextBtn );
  
  QPushButton * findPrevBtn = new QPushButton( KIcon( "go-up-search" ), i18nc( "Find and go to the previous search match", "Previous" ), this );
  findPrevBtn->setToolTip( i18n( "Jump to previous match" ) );
  lay->addWidget( findPrevBtn );
  
  QPushButton * optionsBtn = new QPushButton( this );
  optionsBtn->setText( i18n( "Options" ) );
  optionsBtn->setToolTip( i18n( "Modify search behavior" ) );
  QMenu * optionsMenu = new QMenu( optionsBtn );
  m_caseSensitiveAct = optionsMenu->addAction( i18n( "Case sensitive" ) );
  m_caseSensitiveAct->setCheckable( true );
  m_highlightAll = optionsMenu->addAction( i18n( "Highlight all matches" ) );
  m_highlightAll->setCheckable( true );
  optionsBtn->setMenu( optionsMenu );
  lay->addWidget( optionsBtn );
  
  connect( closeBtn, SIGNAL( clicked() ), this, SLOT( closeBar() ) );
  connect( findNextBtn, SIGNAL( clicked() ), this, SLOT( findNext() ) );
  connect( findPrevBtn, SIGNAL( clicked() ), this, SLOT( findPrev() ) );
  connect( m_caseSensitiveAct, SIGNAL( toggled( bool ) ), this, SLOT( caseSensitivityChanged() ) );
  connect( m_highlightAll, SIGNAL( toggled( bool ) ), this, SLOT( highlightAllChanged() ) );
  connect( m_search, SIGNAL( userTextChanged( QString ) ), this, SLOT( autoSearch( QString ) ) );
  hide();
}

FindBar::~FindBar()
{
}

QString FindBar::text() const
{
  return m_search->text();
}

void FindBar::focusAndSetCursor()
{
  setFocus();
  m_search->selectAll();
  m_search->setFocus();
}

void FindBar::autoSearch( QString str )
{
  if( str.length() > 2 )
    QTimer::singleShot(0, this, SLOT( find() ) );
  else
    clearSelections();
}

void FindBar::findNext()
{
  m_searchOptions ^= QWebPage::FindBackward;
  find();
}

void FindBar::findPrev()
{
  m_searchOptions |= QWebPage::FindBackward;
  find();
}

void FindBar::find()
{
  if( !mLastSearchStr.contains( m_search->text(), Qt::CaseSensitive ) )
  {
    clearSelections();
  }
  mLastSearchStr = m_search->text();
  m_view->findText( mLastSearchStr, m_searchOptions );
}

void FindBar::caseSensitivityChanged()
{
  clearSelections();
  if ( m_caseSensitiveAct->isChecked() )
  {
    m_searchOptions |= QWebPage::FindCaseSensitively;
  } else {
    m_searchOptions ^= QWebPage::FindCaseSensitively;
  }
}

void FindBar::highlightAllChanged()
{
  clearSelections();
  if ( m_highlightAll->isChecked() )
  {
    m_searchOptions |= QWebPage::HighlightAllOccurrences;
  } else {
    m_searchOptions ^= QWebPage::HighlightAllOccurrences;
  }
}

void FindBar::clearSelections()
{
  if ( m_highlightAll->isChecked() )
  {
    m_view->findText( QString(), m_searchOptions ^ QWebPage::FindCaseSensitively );
  } else {
    m_view->findText( QString(), m_searchOptions | QWebPage::FindCaseSensitively );
  }
  //WEBKIT: TODO: Find a way to unselect last selection
  //m_view->triggerPageAction( QWebPage::MoveToStartOfDocument );
  //m_view->triggerPageAction( QWebPage::SelectStartOfDocument );
}

void FindBar::closeBar()
{
  // Make sure that all old searches are cleared
  m_search->setText( QString() );
  clearSelections();
  hide();
}

#include "findbar.moc"
