/*
  statusbarprogresswidget.cpp

  This file is part of KMail, the KDE mail client.

  (C) 2004 KMail Authors
  Includes StatusbarProgressWidget which is based on KIOLittleProgressDlg
  by Matt Koss <koss@miesto.sk>

  KMail is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  KMail is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

  In addition, as a special exception, the copyright holders give
  permission to link the code of this program with any edition of
  the Qt library by Trolltech AS, Norway (or with modified versions
  of Qt that use the same license as Qt), and distribute linked
  combinations including the two.  You must obey the GNU General
  Public License in all respects for all of the code used other than
  Qt.  If you modify this file, you may extend this exception to
  your version of the file, but you are not obligated to do so.  If
  you do not wish to do so, delete this exception statement from
  your version.
*/


#include "ssllabel.h"
using KPIM::SSLLabel;
#include "progressmanager.h"
using KPIM::ProgressItem;
using KPIM::ProgressManager;

#include <kprogress.h>
#include <kiconloader.h>
#include <kdebug.h>

#include <tqtimer.h>
#include <tqlabel.h>
#include <tqpushbutton.h>
#include <tqtooltip.h>
#include <klocale.h>
#include <tqlayout.h>
#include <tqwidgetstack.h>
#include <tqframe.h>

#include "progressdialog.h"
#include "statusbarprogresswidget.h"

using namespace KPIM;

//-----------------------------------------------------------------------------
StatusbarProgressWidget::StatusbarProgressWidget( ProgressDialog* progressDialog, TQWidget* parent, bool button )
  : TQFrame( parent ), mCurrentItem( 0 ), mProgressDialog( progressDialog ),
    mDelayTimer( 0 ), mBusyTimer( 0 )
{
  m_bShowButton = button;
  int w = fontMetrics().width( " 999.9 kB/s 00:00:01 " ) + 8;
  box = new TQHBoxLayout( this, 0, 0 );

  m_pButton = new TQPushButton( this );
  m_pButton->setSizePolicy( TQSizePolicy( TQSizePolicy::Minimum,
                                         TQSizePolicy::Minimum ) );
  m_pButton->setPixmap( SmallIcon( "up" ) );
  box->addWidget( m_pButton  );
  stack = new TQWidgetStack( this );
  stack->setMaximumHeight( fontMetrics().height() );
  box->addWidget( stack );

  m_sslLabel = new SSLLabel( this );
  box->addWidget( m_sslLabel );

  TQToolTip::add( m_pButton, i18n("Open detailed progress dialog") );

  m_pProgressBar = new KProgress( this );
  m_pProgressBar->setLineWidth( 1 );
  m_pProgressBar->setFrameStyle( TQFrame::Box );
  m_pProgressBar->installEventFilter( this );
  m_pProgressBar->setMinimumWidth( w );
  stack->addWidget( m_pProgressBar, 1 );

  m_pLabel = new TQLabel( TQString::null, this );
  m_pLabel->setAlignment( AlignHCenter | AlignVCenter );
  m_pLabel->installEventFilter( this );
  m_pLabel->setMinimumWidth( w );
  stack->addWidget( m_pLabel, 2 );
  m_pButton->setMaximumHeight( fontMetrics().height() );
  setMinimumWidth( minimumSizeHint().width() );

  mode = None;
  setMode();

  connect( m_pButton, TQT_SIGNAL( clicked() ),
           progressDialog, TQT_SLOT( slotToggleVisibility() ) );

  connect ( ProgressManager::instance(), TQT_SIGNAL( progressItemAdded( KPIM::ProgressItem * ) ),
            this, TQT_SLOT( slotProgressItemAdded( KPIM::ProgressItem * ) ) );
  connect ( ProgressManager::instance(), TQT_SIGNAL( progressItemCompleted( KPIM::ProgressItem * ) ),
            this, TQT_SLOT( slotProgressItemCompleted( KPIM::ProgressItem * ) ) );
  connect ( ProgressManager::instance(), TQT_SIGNAL(progressItemUsesBusyIndicator(KPIM::ProgressItem*,bool)),
            this, TQT_SLOT( updateBusyMode() ) );

  connect ( progressDialog, TQT_SIGNAL( visibilityChanged( bool )),
            this, TQT_SLOT( slotProgressDialogVisible( bool ) ) );

  mDelayTimer = new TQTimer( this );
  connect ( mDelayTimer, TQT_SIGNAL( timeout() ),
            this, TQT_SLOT( slotShowItemDelayed() ) );
}

// There are three cases: no progressitem, one progressitem (connect to it directly),
// or many progressitems (display busy indicator). Let's call them 0,1,N.
// In slot..Added we can only end up in 1 or N.
// In slot..Removed we can end up in 0, 1, or we can stay in N if we were already.

void StatusbarProgressWidget::updateBusyMode()
{
  connectSingleItem(); // if going to 1 item
  if ( mCurrentItem ) { // Exactly one item
    delete mBusyTimer;
    mBusyTimer = 0;
    mDelayTimer->start( 1000, true );
  }
  else { // N items
    if ( !mBusyTimer ) {
      mBusyTimer = new TQTimer( this );
      connect( mBusyTimer, TQT_SIGNAL( timeout() ),
               this, TQT_SLOT( slotBusyIndicator() ) );
      mDelayTimer->start( 1000, true );
    }
  }
}

void StatusbarProgressWidget::slotProgressItemAdded( ProgressItem *item )
{
  if ( item->parent() )
    return; // we are only interested in top level items

  updateBusyMode();
}

void StatusbarProgressWidget::slotProgressItemCompleted( ProgressItem *item )
{
  if ( item->parent() ) return; // we are only interested in top level items
  connectSingleItem(); // if going back to 1 item
  if ( ProgressManager::instance()->isEmpty() ) { // No item
    // Done. In 5s the progress-widget will close, then we can clean up the statusbar
    TQTimer::singleShot( 5000, this, TQT_SLOT( slotClean() ) );
  } else if ( mCurrentItem ) { // Exactly one item
    delete mBusyTimer;
    mBusyTimer = 0;
    activateSingleItemMode();
  }
}

void StatusbarProgressWidget::connectSingleItem()
{
  if ( mCurrentItem ) {
    disconnect ( mCurrentItem, TQT_SIGNAL( progressItemProgress( KPIM::ProgressItem *, unsigned int ) ),
                 this, TQT_SLOT( slotProgressItemProgress( KPIM::ProgressItem *, unsigned int ) ) );
    mCurrentItem = 0;
  }
  mCurrentItem = ProgressManager::instance()->singleItem();
  if ( mCurrentItem ) {
    connect ( mCurrentItem, TQT_SIGNAL( progressItemProgress( KPIM::ProgressItem *, unsigned int ) ),
              this, TQT_SLOT( slotProgressItemProgress( KPIM::ProgressItem *, unsigned int ) ) );
  }
}

void StatusbarProgressWidget::activateSingleItemMode()
{
  m_pProgressBar->setTotalSteps( 100 );
  m_pProgressBar->setProgress( mCurrentItem->progress() );
  m_pProgressBar->setPercentageVisible( true );
}

void StatusbarProgressWidget::slotShowItemDelayed()
{
  bool noItems = ProgressManager::instance()->isEmpty();
  if ( mCurrentItem ) {
    activateSingleItemMode();
  } else if ( !noItems ) { // N items
    m_pProgressBar->setTotalSteps( 0 );
    m_pProgressBar->setPercentageVisible( false );
    Q_ASSERT( mBusyTimer );
    if ( mBusyTimer )
      mBusyTimer->start( 100 );
  }

  if ( !noItems && mode == None ) {
    mode = Progress;
    setMode();
  }
}

void StatusbarProgressWidget::slotBusyIndicator()
{
  int p = m_pProgressBar->progress();
  m_pProgressBar->setProgress( p + 10 );
}

void StatusbarProgressWidget::slotProgressItemProgress( ProgressItem *item, unsigned int value )
{
  Q_ASSERT( item == mCurrentItem); // the only one we should be connected to
  m_pProgressBar->setProgress( value );
}

void StatusbarProgressWidget::slotSetSSL( bool ssl )
{
  m_sslLabel->setEncrypted( ssl );
}

void StatusbarProgressWidget::setMode() {
  switch ( mode ) {
  case None:
    if ( m_bShowButton ) {
      m_pButton->hide();
    }
    m_sslLabel->setState( SSLLabel::Done );
    // show the empty label in order to make the status bar look better
    stack->show();
    stack->raiseWidget( m_pLabel );
    break;

#if 0
  case Label:
    if ( m_bShowButton ) {
      m_pButton->show();
    }
    m_sslLabel->setState( m_sslLabel->lastState() );
    stack->show();
    stack->raiseWidget( m_pLabel );
    break;
#endif

  case Progress:
    stack->show();
    stack->raiseWidget( m_pProgressBar );
    if ( m_bShowButton ) {
      m_pButton->show();
    }
    m_sslLabel->setState( m_sslLabel->lastState() );
    break;
  }
}

void StatusbarProgressWidget::slotClean()
{
  // check if a new item showed up since we started the timer. If not, clear
  if ( ProgressManager::instance()->isEmpty() ) {
    m_pProgressBar->setProgress( 0 );
    //m_pLabel->clear();
    mode = None;
    setMode();
  }
}

bool StatusbarProgressWidget::eventFilter( TQObject *, TQEvent *ev )
{
  if ( ev->type() == TQEvent::MouseButtonPress ) {
    TQMouseEvent *e = (TQMouseEvent*)ev;

    if ( e->button() == LeftButton && mode != None ) {    // toggle view on left mouse button
      // Consensus seems to be that we should show/hide the fancy dialog when the user
      // clicks anywhere in the small one.
      mProgressDialog->slotToggleVisibility();
      return true;
    }
  }
  return false;
}

void StatusbarProgressWidget::slotProgressDialogVisible( bool b )
{
  // Update the hide/show button when the detailed one is shown/hidden
  if ( b ) {
    m_pButton->setPixmap( SmallIcon( "down" ) );
    TQToolTip::remove( m_pButton );
    TQToolTip::add( m_pButton, i18n("Hide detailed progress window") );
    setMode();
  } else {
    m_pButton->setPixmap( SmallIcon( "up" ) );
    TQToolTip::remove( m_pButton );
    TQToolTip::add( m_pButton, i18n("Show detailed progress window") );
  }
}

#include "statusbarprogresswidget.moc"
