/** -*- c++ -*-
 * progressdialog.cpp
 *
 *  Copyright (c) 2004 Till Adam <adam@kde.org>,
 *                     David Faure <faure@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of this program with any edition of
 *  the Qt library by Trolltech AS, Norway (or with modified versions
 *  of Qt that use the same license as Qt), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt.  If you modify this file, you may extend this exception to
 *  your version of the file, but you are not obligated to do so.  If
 *  you do not wish to do so, delete this exception statement from
 *  your version.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <tqapplication.h>
#include <tqlayout.h>
#include <tqprogressbar.h>
#include <tqtimer.h>
#include <tqheader.h>
#include <tqobject.h>
#include <tqscrollview.h>
#include <tqtoolbutton.h>
#include <tqpushbutton.h>
#include <tqvbox.h>
#include <tqtooltip.h>

#include <klocale.h>
#include <kdialog.h>
#include <kstdguiitem.h>
#include <kiconloader.h>
#include <kdebug.h>

#include "progressdialog.h"
#include "progressmanager.h"
#include "ssllabel.h"
#include <tqwhatsthis.h>

namespace KPIM {

class TransactionItem;

TransactionItemView::TransactionItemView( TQWidget * parent,
                                          const char * name,
                                          WFlags f )
    : TQScrollView( parent, name, f ) {
  setFrameStyle( NoFrame );
  mBigBox = new TQVBox( viewport() );
  mBigBox->setSpacing( 5 );
  addChild( mBigBox );
  setResizePolicy( TQScrollView::AutoOneFit ); // Fit so that the box expands horizontally
}

TransactionItem* TransactionItemView::addTransactionItem( ProgressItem* item, bool first )
{
   TransactionItem *ti = new TransactionItem( mBigBox, item, first );
   ti->hide();
   TQTimer::singleShot( 1000, ti, TQT_SLOT( show() ) );
   return ti;
}

void TransactionItemView::resizeContents( int w, int h )
{
  // (handling of TQEvent::LayoutHint in TQScrollView calls this method)
  //kdDebug(5300) << k_funcinfo << w << "," << h << endl;
  TQScrollView::resizeContents( w, h );
  // Tell the layout in the parent (progressdialog) that our size changed
  updateGeometry();
  // Resize the parent (progressdialog) - this works but resize horizontally too often
  //parentWidget()->adjustSize();

  TQApplication::sendPostedEvents( 0, TQEvent::ChildInserted );
  TQApplication::sendPostedEvents( 0, TQEvent::LayoutHint );
  TQSize sz = parentWidget()->sizeHint();
  int currentWidth = parentWidget()->width();
  // Don't resize to sz.width() every time when it only reduces a little bit
  if ( currentWidth < sz.width() || currentWidth > sz.width() + 100 )
    currentWidth = sz.width();
  parentWidget()->resize( currentWidth, sz.height() );
}

TQSize TransactionItemView::sizeHint() const
{
  return minimumSizeHint();
}

TQSize TransactionItemView::minimumSizeHint() const
{
  int f = 2 * frameWidth();
  // Make room for a vertical scrollbar in all cases, to avoid a horizontal one
  int vsbExt = verticalScrollBar()->sizeHint().width();
  int minw = topLevelWidget()->width() / 3;
  int maxh = topLevelWidget()->height() / 2;
  TQSize sz( mBigBox->minimumSizeHint() );
  sz.setWidth( QMAX( sz.width(), minw ) + f + vsbExt );
  sz.setHeight( QMIN( sz.height(), maxh ) + f );
  return sz;
}


void TransactionItemView::slotLayoutFirstItem()
{
  /*
     The below relies on some details in Qt's behaviour regarding deleting
     objects. This slot is called from the destroyed signal of an item just
     going away. That item is at that point still in the  list of chilren, but
     since the vtable is already gone, it will have type TQObject. The first
     one with both the right name and the right class therefor is what will
     be the first item very shortly. That's the one we want to remove the
     hline for.
  */
  TQObject *o = mBigBox->child( "TransactionItem", "KPIM::TransactionItem" );
  TransactionItem *ti = dynamic_cast<TransactionItem*>( o );
  if ( ti ) {
    ti->hideHLine();
  }
}


// ----------------------------------------------------------------------------

TransactionItem::TransactionItem( TQWidget* parent,
                                  ProgressItem *item, bool first )
  : TQVBox( parent, "TransactionItem" ), mCancelButton( 0 ), mItem( item )

{
  setSpacing( 2 );
  setMargin( 2 );
  setSizePolicy( TQSizePolicy( TQSizePolicy::Preferred, TQSizePolicy::Fixed ) );

  mFrame = new TQFrame( this );
  mFrame->setFrameShape( TQFrame::HLine );
  mFrame->setFrameShadow( TQFrame::Raised );
  mFrame->show();
  setStretchFactor( mFrame, 3 );

  TQHBox *h = new TQHBox( this );
  h->setSpacing( 5 );

  mItemLabel = new TQLabel( item->label(), h );
  // always interpret the label text as RichText, but disable word wrapping
  mItemLabel->setTextFormat( Qt::RichText );
  mItemLabel->setAlignment( Qt::AlignAuto | Qt::AlignVCenter | Qt::SingleLine );
  h->setSizePolicy( TQSizePolicy( TQSizePolicy::Preferred, TQSizePolicy::Fixed ) );

  mProgress = new TQProgressBar( 100, h );
  mProgress->setProgress( item->progress() );

  if ( item->canBeCanceled() ) {
    mCancelButton = new TQPushButton( SmallIcon( "cancel" ), TQString::null, h );
    TQToolTip::add( mCancelButton, i18n("Cancel this operation.") );
    connect ( mCancelButton, TQT_SIGNAL( clicked() ),
              this, TQT_SLOT( slotItemCanceled() ));
  }

  h = new TQHBox( this );
  h->setSpacing( 5 );
  h->setSizePolicy( TQSizePolicy( TQSizePolicy::Preferred, TQSizePolicy::Fixed ) );
  mSSLLabel = new SSLLabel( h );
  mSSLLabel->setSizePolicy( TQSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed ) );
  mItemStatus = new TQLabel( item->status(), h );
  // always interpret the status text as RichText, but disable word wrapping
  mItemStatus->setTextFormat( Qt::RichText );
  mItemStatus->setAlignment( Qt::AlignAuto | Qt::AlignVCenter | Qt::SingleLine );
  // richtext leads to sizeHint acting as if wrapping was enabled though,
  // so make sure we only ever have the height of one line.
  mItemStatus->setSizePolicy( TQSizePolicy( TQSizePolicy::Preferred, TQSizePolicy::Ignored ) );
  mItemStatus->setFixedHeight( mItemLabel->sizeHint().height() );
  setCrypto( item->usesCrypto() );
  if( first ) hideHLine();
}

TransactionItem::~TransactionItem()
{
}

void TransactionItem::hideHLine()
{
    mFrame->hide();
}

void TransactionItem::setProgress( int progress )
{
  mProgress->setProgress( progress );
}

void TransactionItem::setLabel( const TQString& label )
{
  mItemLabel->setText( label );
}

void TransactionItem::setStatus( const TQString& status )
{
  mItemStatus->setText( status );
}

void TransactionItem::setCrypto( bool on )
{
  if (on)
    mSSLLabel->setEncrypted( true );
  else
    mSSLLabel->setEncrypted( false );

  mSSLLabel->setState( mSSLLabel->lastState() );
}

void TransactionItem::setTotalSteps( int totalSteps )
{
  mProgress->setTotalSteps( totalSteps );
}

void TransactionItem::slotItemCanceled()
{
  if ( mItem )
    mItem->cancel();
}


void TransactionItem::addSubTransaction( ProgressItem* /*item*/ )
{

}


// ---------------------------------------------------------------------------

ProgressDialog::ProgressDialog( TQWidget* alignWidget, TQWidget* parent, const char* name )
    : OverlayWidget( alignWidget, parent, name ), mWasLastShown( false )
{
    setFrameStyle( TQFrame::Panel | TQFrame::Sunken ); // QFrame
    setSpacing( 0 ); // QHBox
    setMargin( 1 );

    mScrollView = new TransactionItemView( this, "ProgressScrollView" );

    // No more close button for now, since there is no more autoshow
    /*
    TQVBox* rightBox = new TQVBox( this );
    TQToolButton* pbClose = new TQToolButton( rightBox );
    pbClose->setAutoRaise(true);
    pbClose->setSizePolicy( TQSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed ) );
    pbClose->setFixedSize( 16, 16 );
    pbClose->setIconSet( KGlobal::iconLoader()->loadIconSet( "fileclose", KIcon::Small, 14 ) );
    TQToolTip::add( pbClose, i18n( "Hide detailed progress window" ) );
    connect(pbClose, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotClose()));
    TQWidget* spacer = new TQWidget( rightBox ); // don't let the close button take up all the height
    rightBox->setStretchFactor( spacer, 100 );
    */

    /*
     * Get the singleton ProgressManager item which will inform us of
     * appearing and vanishing items.
     */
    ProgressManager *pm = ProgressManager::instance();
    connect ( pm, TQT_SIGNAL( progressItemAdded( KPIM::ProgressItem* ) ),
              this, TQT_SLOT( slotTransactionAdded( KPIM::ProgressItem* ) ) );
    connect ( pm, TQT_SIGNAL( progressItemCompleted( KPIM::ProgressItem* ) ),
              this, TQT_SLOT( slotTransactionCompleted( KPIM::ProgressItem* ) ) );
    connect ( pm, TQT_SIGNAL( progressItemProgress( KPIM::ProgressItem*, unsigned int ) ),
              this, TQT_SLOT( slotTransactionProgress( KPIM::ProgressItem*, unsigned int ) ) );
    connect ( pm, TQT_SIGNAL( progressItemStatus( KPIM::ProgressItem*, const TQString& ) ),
              this, TQT_SLOT( slotTransactionStatus( KPIM::ProgressItem*, const TQString& ) ) );
    connect ( pm, TQT_SIGNAL( progressItemLabel( KPIM::ProgressItem*, const TQString& ) ),
              this, TQT_SLOT( slotTransactionLabel( KPIM::ProgressItem*, const TQString& ) ) );
    connect ( pm, TQT_SIGNAL( progressItemUsesCrypto(KPIM::ProgressItem*, bool) ),
              this, TQT_SLOT( slotTransactionUsesCrypto( KPIM::ProgressItem*, bool ) ) );
    connect ( pm, TQT_SIGNAL( progressItemUsesBusyIndicator(KPIM::ProgressItem*, bool) ),
              this, TQT_SLOT( slotTransactionUsesBusyIndicator( KPIM::ProgressItem*, bool ) ) );
    connect ( pm, TQT_SIGNAL( showProgressDialog() ),
              this, TQT_SLOT( slotShow() ) );
}

void ProgressDialog::closeEvent( TQCloseEvent* e )
{
  e->accept();
  hide();
}


/*
 *  Destructor
 */
ProgressDialog::~ProgressDialog()
{
    // no need to delete child widgets.
}

void ProgressDialog::slotTransactionAdded( ProgressItem *item )
{
   TransactionItem *parent = 0;
   if ( item->parent() ) {
     if ( mTransactionsToListviewItems.contains( item->parent() ) ) {
       parent = mTransactionsToListviewItems[ item->parent() ];
       parent->addSubTransaction( item );
     }
   } else {
     const bool first = mTransactionsToListviewItems.empty();
     TransactionItem *ti = mScrollView->addTransactionItem( item, first );
     if ( ti )
       mTransactionsToListviewItems.replace( item, ti );
     if ( first && mWasLastShown )
       TQTimer::singleShot( 1000, this, TQT_SLOT( slotShow() ) );

   }
}

void ProgressDialog::slotTransactionCompleted( ProgressItem *item )
{
   if ( mTransactionsToListviewItems.contains( item ) ) {
     TransactionItem *ti = mTransactionsToListviewItems[ item ];
     mTransactionsToListviewItems.remove( item );
     ti->setItemComplete();
     TQTimer::singleShot( 3000, ti, TQT_SLOT( deleteLater() ) );
     // see the slot for comments as to why that works
     connect ( ti, TQT_SIGNAL( destroyed() ),
               mScrollView, TQT_SLOT( slotLayoutFirstItem() ) );
   }
   // This was the last item, hide.
   if ( mTransactionsToListviewItems.empty() )
     TQTimer::singleShot( 3000, this, TQT_SLOT( slotHide() ) );
}

void ProgressDialog::slotTransactionCanceled( ProgressItem* )
{
}

void ProgressDialog::slotTransactionProgress( ProgressItem *item,
                                              unsigned int progress )
{
   if ( mTransactionsToListviewItems.contains( item ) ) {
     TransactionItem *ti = mTransactionsToListviewItems[ item ];
     ti->setProgress( progress );
   }
}

void ProgressDialog::slotTransactionStatus( ProgressItem *item,
                                            const TQString& status )
{
   if ( mTransactionsToListviewItems.contains( item ) ) {
     TransactionItem *ti = mTransactionsToListviewItems[ item ];
     ti->setStatus( status );
   }
}

void ProgressDialog::slotTransactionLabel( ProgressItem *item,
                                           const TQString& label )
{
   if ( mTransactionsToListviewItems.contains( item ) ) {
     TransactionItem *ti = mTransactionsToListviewItems[ item ];
     ti->setLabel( label );
   }
}


void ProgressDialog::slotTransactionUsesCrypto( ProgressItem *item,
                                                bool value )
{
   if ( mTransactionsToListviewItems.contains( item ) ) {
     TransactionItem *ti = mTransactionsToListviewItems[ item ];
     ti->setCrypto( value );
   }
}

void ProgressDialog::slotTransactionUsesBusyIndicator( KPIM::ProgressItem *item, bool value )
{
  if ( mTransactionsToListviewItems.contains( item ) ) {
     TransactionItem *ti = mTransactionsToListviewItems[ item ];
     if ( value )
       ti->setTotalSteps( 0 );
     else
       ti->setTotalSteps( 100 );
  }
}

void ProgressDialog::slotShow()
{
   setVisible( true );
}

void ProgressDialog::slotHide()
{
  // check if a new item showed up since we started the timer. If not, hide
  if ( mTransactionsToListviewItems.isEmpty() ) {
    setVisible( false );
  }
}

void ProgressDialog::slotClose()
{
  mWasLastShown = false;
  setVisible( false );
}

void ProgressDialog::setVisible( bool b )
{
  if ( b )
    show();
  else
    hide();
  emit visibilityChanged( b );
}

void ProgressDialog::slotToggleVisibility()
{
  /* Since we are only hiding with a timeout, there is a short period of
   * time where the last item is still visible, but clicking on it in
   * the statusbarwidget should not display the dialog, because there
   * are no items to be shown anymore. Guard against that.
   */
  mWasLastShown = !isShown();
  if ( isShown() || !mTransactionsToListviewItems.isEmpty() )
    setVisible( !isShown() );
}

}

#include "progressdialog.moc"
