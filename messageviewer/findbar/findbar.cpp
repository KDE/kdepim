/* Copyright (C) 2010 Torgny Nyblom <kde@nyblom.org>
 * Copyright (C) 2010 Laurent Montel <montel@kde.org>
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

#include "findbar.h"
#include "mailwebview.h"

// qt/kde includes
#include <QtCore/QTimer>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QMenu>
#include <QtGui/QToolButton>
#include <QEvent>
#include <QKeyEvent>
#include <kicon.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <klineedit.h>
#include <KColorScheme>

using namespace MessageViewer;

FindBar::FindBar( MailWebView * view, QWidget * parent )
  : QWidget( parent ), m_view( view )
{
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
  m_search->setClearButtonShown( true );
  label->setBuddy( m_search );
  lay->addWidget( m_search );

  m_findNextBtn = new QPushButton( KIcon( "go-down-search" ), i18nc( "Find and go to the next search match", "Next" ), this );
  m_findNextBtn->setToolTip( i18n( "Jump to next match" ) );
  lay->addWidget( m_findNextBtn );
  m_findNextBtn->setEnabled( false );

  m_findPrevBtn = new QPushButton( KIcon( "go-up-search" ), i18nc( "Find and go to the previous search match", "Previous" ), this );
  m_findPrevBtn->setToolTip( i18n( "Jump to previous match" ) );
  lay->addWidget( m_findPrevBtn );
  m_findPrevBtn->setEnabled( false );

  QPushButton * optionsBtn = new QPushButton( this );
  optionsBtn->setText( i18n( "Options" ) );
  optionsBtn->setToolTip( i18n( "Modify search behavior" ) );
  QMenu * optionsMenu = new QMenu( optionsBtn );
  m_caseSensitiveAct = optionsMenu->addAction( i18n( "Case sensitive" ) );
  m_caseSensitiveAct->setCheckable( true );
#ifndef MESSAGEVIEWER_FINDBAR_NO_HIGHLIGHT_ALL
  m_highlightAll = optionsMenu->addAction( i18n( "Highlight all matches" ) );
  m_highlightAll->setCheckable( true );
#endif
  optionsBtn->setMenu( optionsMenu );
  lay->addWidget( optionsBtn );

  connect( closeBtn, SIGNAL( clicked() ), this, SLOT( closeBar() ) );
  connect( m_findNextBtn, SIGNAL( clicked() ), this, SLOT( findNext() ) );
  connect( m_findPrevBtn, SIGNAL( clicked() ), this, SLOT( findPrev() ) );
  connect( m_caseSensitiveAct, SIGNAL( toggled( bool ) ), this, SLOT( caseSensitivityChanged() ) );
#ifndef MESSAGEVIEWER_FINDBAR_NO_HIGHLIGHT_ALL
  connect( m_highlightAll, SIGNAL( toggled( bool ) ), this, SLOT( highlightAllChanged() ) );
#endif
  connect( m_search, SIGNAL( textEdited( QString ) ), this, SLOT( autoSearch( QString ) ) );
  connect( m_search, SIGNAL( clearButtonClicked() ), this, SLOT( slotClearSearch() ) );
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

void FindBar::slotClearSearch()
{
  clearSelections();
}

void FindBar::autoSearch( const QString& str )
{
  m_findPrevBtn->setEnabled( !str.isEmpty() );
  m_findNextBtn->setEnabled( !str.isEmpty() );
  if( str.length() > 2 )
    QTimer::singleShot(0, this, SLOT( searchText() ) );
  else
    clearSelections();
}

void FindBar::searchText( bool backward)
{
  MailWebView::FindFlags searchOptions = MailWebView::FindWrapsAroundDocument;

  if ( backward )
    searchOptions = MailWebView::FindBackward;
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
  bool found = m_view->findText( mLastSearchStr, searchOptions );
  setFoundMatch( found );
}

void FindBar::setFoundMatch( bool match )
{
  QString styleSheet;

  if (!m_search->text().isEmpty()) {
    KColorScheme::BackgroundRole bgColorScheme;

    m_findPrevBtn->setEnabled( match );
    m_findNextBtn->setEnabled( match );

    if (match)
      bgColorScheme = KColorScheme::PositiveBackground;
    else
      bgColorScheme = KColorScheme::NegativeBackground;

    KStatefulBrush bgBrush(KColorScheme::View, bgColorScheme);

    styleSheet = QString("QLineEdit{ background-color:%1 }")
                 .arg(bgBrush.brush(m_search).color().name());
  }

#ifndef QT_NO_STYLE_STYLESHEET
  m_search->setStyleSheet(styleSheet);
#endif

}

void FindBar::findNext()
{
  searchText( false );
}

void FindBar::findPrev()
{
  searchText( true );
}

void FindBar::caseSensitivityChanged()
{
  clearSelections();
}

void FindBar::highlightAllChanged()
{
  clearSelections();
}

void FindBar::clearSelections()
{
  m_view->clearFindSelection();
  setFoundMatch( false );
}

void FindBar::closeBar()
{
  // Make sure that all old searches are cleared
  m_search->setText( QString() );
  clearSelections();
  hide();
}

bool FindBar::event(QEvent* e)
{
    // Close the bar when pressing Escape.
    // Not using a QShortcut for this because it could conflict with
    // window-global actions (e.g. Emil Sedgh binds Esc to "close tab").
    // With a shortcut override we can catch this before it gets to kactions.
    if (e->type() == QEvent::ShortcutOverride) {
        QKeyEvent* kev = static_cast<QKeyEvent* >(e);
        if (kev->key() == Qt::Key_Escape) {
            e->accept();
            closeBar();
            return true;
        }
        else if ( kev->key() == Qt::Key_Enter ||
                  kev->key() == Qt::Key_Return ) {
          e->accept();
          findNext();
          return true;
        }
    }
    return QWidget::event(e);
}

#include "findbar.moc"
