/* Copyright (C) 2010 Torgny Nyblom <nyblom@kde.org>
 * Copyright (C) 2010,2011, 2012 Laurent Montel <montel@kde.org>
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

#include "findbarbase.h"

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

using namespace MessageViewer;

FindBarBase::FindBarBase( QWidget * parent )
  : QWidget( parent )
{
  QHBoxLayout * lay = new QHBoxLayout( this );
  lay->setMargin( 2 );

  QToolButton * closeBtn = new QToolButton( this );
  closeBtn->setIcon( KIcon( "dialog-close" ) );
  closeBtn->setIconSize( QSize( 16, 16 ) );
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
  m_optionsMenu = new QMenu( optionsBtn );
  m_caseSensitiveAct = m_optionsMenu->addAction( i18n( "Case sensitive" ) );
  m_caseSensitiveAct->setCheckable( true );

  optionsBtn->setMenu( m_optionsMenu );
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

FindBarBase::~FindBarBase()
{
}

QMenu* FindBarBase::optionsMenu()
{
  return m_optionsMenu;
}

QString FindBarBase::text() const
{
  return m_search->text();
}

void FindBarBase::setText( const QString&text )
{
  m_search->setText( text );
}

void FindBarBase::focusAndSetCursor()
{
  setFocus();
  m_search->selectAll();
  m_search->setFocus();
}

void FindBarBase::slotClearSearch()
{
  clearSelections();
}

void FindBarBase::autoSearch( const QString& str )
{
  const bool isNotEmpty = ( !str.isEmpty() );
  m_findPrevBtn->setEnabled( isNotEmpty );
  m_findNextBtn->setEnabled( isNotEmpty );
  if ( isNotEmpty )
    QTimer::singleShot( 0, this, SLOT(slotSearchText()) );
  else
    clearSelections();
}

void FindBarBase::slotSearchText( bool backward, bool isAutoSearch )
{
  searchText( backward, isAutoSearch );  
}

void FindBarBase::messageInfo( bool backward, bool isAutoSearch, bool found )
{
  if ( !found && !isAutoSearch ) {
    if ( backward ) {
      KMessageBox::information( this, i18n( "Beginning of message reached.\nPhrase '%1' could not be found." ,mLastSearchStr ) ); 
    } else {
      KMessageBox::information( this, i18n( "End of message reached.\nPhrase '%1' could not be found.", mLastSearchStr ) ); 
    }
  }
}


void FindBarBase::setFoundMatch( bool match )
{
#ifndef QT_NO_STYLE_STYLESHEET
  QString styleSheet;

  if (!m_search->text().isEmpty()) {
    if (mNegativeBackground.isEmpty()) {
      KStatefulBrush bgBrush(KColorScheme::View, KColorScheme::PositiveBackground);
      mPositiveBackground = QString::fromLatin1("QLineEdit{ background-color:%1 }").arg(bgBrush.brush(m_search).color().name());
      bgBrush = KStatefulBrush(KColorScheme::View, KColorScheme::NegativeBackground);
      mNegativeBackground = QString::fromLatin1("QLineEdit{ background-color:%1 }").arg(bgBrush.brush(m_search).color().name());
    }
    if (match)
      styleSheet = mPositiveBackground;
    else
      styleSheet = mNegativeBackground;
  }
  m_search->setStyleSheet(styleSheet);
#endif

}

void FindBarBase::searchText( bool backward, bool isAutoSearch )
{
  Q_UNUSED( backward );
  Q_UNUSED( isAutoSearch );
}


void FindBarBase::findNext()
{
  searchText( false, false );
}

void FindBarBase::findPrev()
{
  searchText( true, false );
}

void FindBarBase::caseSensitivityChanged(bool b)
{
  updateSensitivity( b );
}

void FindBarBase::updateSensitivity( bool )
{
}

void FindBarBase::slotHighlightAllChanged(bool b)
{
  updateHighLight(b);
}

void FindBarBase::updateHighLight( bool )
{
}

void FindBarBase::clearSelections()
{
  setFoundMatch( false );
}

void FindBarBase::closeBar()
{
  // Make sure that all old searches are cleared
  m_search->setText( QString() );
  clearSelections();
  hide();
}

bool FindBarBase::event(QEvent* e)
{
    // Close the bar when pressing Escape.
    // Not using a QShortcut for this because it could conflict with
    // window-global actions (e.g. Emil Sedgh binds Esc to "close tab").
    // With a shortcut override we can catch this before it gets to kactions.
    const bool shortCutOverride = (e->type() == QEvent::ShortcutOverride);
    if ( shortCutOverride || e->type() == QEvent::KeyPress ) {
        QKeyEvent* kev = static_cast<QKeyEvent* >(e);
        if (kev->key() == Qt::Key_Escape) {
            if ( shortCutOverride ) {
                e->accept();
                return true;
            }
            e->accept();
            closeBar();
            return true;
        }
        else if ( kev->key() == Qt::Key_Enter ||
                  kev->key() == Qt::Key_Return ) {
          e->accept();
          if ( shortCutOverride ) {
              return true;
          }
          if ( kev->modifiers() & Qt::ShiftModifier )
            findPrev();
          else if ( kev->modifiers() == Qt::NoModifier )
            findNext();
          return true;
        }
    }
    return QWidget::event(e);
}

#include "findbarbase.moc"
