/* Copyright (C) 2012 Laurent Montel <montel@kde.org>
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

#include "sievefindbar.h"

// qt/kde includes
#include <QtCore/QTimer>
#include <QLabel>
#include <QLayout>
#include <QMenu>
#include <QToolButton>
#include <QEvent>
#include <QKeyEvent>
#include <kicon.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <KColorScheme>
#include <QPlainTextEdit>

using namespace KSieveUi;

SieveFindBar::SieveFindBar( QPlainTextEdit * view, QWidget * parent )
  : QWidget( parent ), m_view( view )
{
  QHBoxLayout * lay = new QHBoxLayout( this );
  lay->setMargin( 2 );

  QToolButton * closeBtn = new QToolButton( this );
  closeBtn->setIcon( KIcon( "dialog-close" ) );
  closeBtn->setIconSize( QSize( 24, 24 ) );
  closeBtn->setToolTip( i18n( "Close" ) );

#ifndef QT_NO_ACCESSIBILITY
  closeBtn->setAccessibleName( i18n( "Close" ) );
#endif

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
  QMenu *optionsMenu = new QMenu( optionsBtn );
  m_caseSensitiveAct = optionsMenu->addAction( i18n( "Case sensitive" ) );
  m_caseSensitiveAct->setCheckable( true );

  optionsBtn->setMenu( optionsMenu );
  lay->addWidget( optionsBtn );

  connect( closeBtn, SIGNAL(clicked()), this, SLOT(closeBar()) );
  connect( m_findNextBtn, SIGNAL(clicked()), this, SLOT(findNext()) );
  connect( m_findPrevBtn, SIGNAL(clicked()), this, SLOT(findPrev()) );
  connect( m_caseSensitiveAct, SIGNAL(toggled(bool)), this, SLOT(caseSensitivityChanged(bool)) );
  connect( m_search, SIGNAL(textChanged(QString)), this, SLOT(autoSearch(QString)) );
  connect( m_search, SIGNAL(clearButtonClicked()), this, SLOT(slotClearSearch()) );
  setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
  hide();
}

SieveFindBar::~SieveFindBar()
{
}

void SieveFindBar::setText( const QString&text )
{
  m_search->setText( text );
}

QString SieveFindBar::text() const
{
  return m_search->text();
}

void SieveFindBar::focusAndSetCursor()
{
  setFocus();
  m_search->selectAll();
  m_search->setFocus();
}

void SieveFindBar::slotClearSearch()
{
  clearSelections();
}

void SieveFindBar::autoSearch( const QString& str )
{
  const bool isNotEmpty = ( !str.isEmpty() );
  m_findPrevBtn->setEnabled( isNotEmpty );
  m_findNextBtn->setEnabled( isNotEmpty );
  if ( isNotEmpty )
    QTimer::singleShot( 0, this, SLOT(slotSearchText()) );
  else
    clearSelections();
}

void SieveFindBar::slotSearchText( bool backward, bool isAutoSearch )
{
  searchText( backward, isAutoSearch );  
}

void SieveFindBar::messageInfo( bool backward, bool isAutoSearch, bool found )
{
  if ( !found && !isAutoSearch ) {
    if ( backward ) {
      KMessageBox::information( this, i18n( "Beginning of message reached.\nPhrase '%1' could not be found." ,mLastSearchStr ) ); 
    } else {
      KMessageBox::information( this, i18n( "End of message reached.\nPhrase '%1' could not be found.", mLastSearchStr ) ); 
    }
  }
}


void SieveFindBar::setFoundMatch( bool match )
{
  QString styleSheet;

  if (!m_search->text().isEmpty()) {
    KColorScheme::BackgroundRole bgColorScheme;

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

void SieveFindBar::searchText( bool backward, bool isAutoSearch )
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
  messageInfo( backward, isAutoSearch, found );  

}


void SieveFindBar::findNext()
{
  searchText( false, false );
}

void SieveFindBar::findPrev()
{
  searchText( true, false );
}

void SieveFindBar::caseSensitivityChanged(bool b)
{
  updateSensitivity( b );
}

void SieveFindBar::updateSensitivity( bool )
{
  QTextDocument::FindFlags searchOptions = 0;
  if ( m_caseSensitiveAct->isChecked() )
    searchOptions |= QTextDocument::FindCaseSensitively;
  mLastSearchStr = m_search->text();
  const bool found = m_view->find( mLastSearchStr, searchOptions );
  setFoundMatch( found );

}

void SieveFindBar::slotHighlightAllChanged(bool b)
{
  updateHighLight(b);
}

void SieveFindBar::updateHighLight( bool )
{
  QTextCursor textCursor = m_view->textCursor();
  textCursor.clearSelection();
  textCursor.setPosition( 0 );
  m_view->setTextCursor( textCursor );
  clearSelections();
}

void SieveFindBar::clearSelections()
{
  setFoundMatch( false );
}

void SieveFindBar::closeBar()
{
  // Make sure that all old searches are cleared
  m_search->setText( QString() );
  clearSelections();
  hide();
}

bool SieveFindBar::event(QEvent* e)
{
    // Close the bar when pressing Escape.
    // Not using a QShortcut for this because it could conflict with
    // window-global actions (e.g. Emil Sedgh binds Esc to "close tab").
    // With a shortcut override we can catch this before it gets to kactions.
    if (e->type() == QEvent::ShortcutOverride || e->type() == QEvent::KeyPress) {
        QKeyEvent* kev = static_cast<QKeyEvent* >(e);
        if (kev->key() == Qt::Key_Escape) {
            e->accept();
            closeBar();
            return true;
        }
        else if ( kev->key() == Qt::Key_Enter ||
                  kev->key() == Qt::Key_Return ) {
          e->accept();
          if ( kev->modifiers() & Qt::ShiftModifier )
            findPrev();
          else if ( kev->modifiers() == Qt::NoModifier )
            findNext();
          return true;
        }
    } 
    return QWidget::event(e);
}

#include "sievefindbar.moc"
