/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "storageserviceprogressdialog.h"
#include "storageserviceprogresswidget.h"

#include <KVBox>

#include <QScrollBar>
#include <QLayout>

using namespace PimCommon;

ProgressIndicatorView::ProgressIndicatorView(QWidget *parent)
    : QScrollArea( parent )
{
    setFrameStyle( NoFrame );
    mBigBox = new KVBox( this );
    setWidget( mBigBox );
    setWidgetResizable( true );
    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
}

ProgressIndicatorView::~ProgressIndicatorView()
{

}

StorageServiceProgressWidget *ProgressIndicatorView::addTransactionItem( PimCommon::StorageServiceAbstract *service, bool first )
{
    StorageServiceProgressWidget *ti = new StorageServiceProgressWidget( service, mBigBox);//, first );
    mBigBox->layout()->addWidget( ti );

    resize( mBigBox->width(), mBigBox->height() );

    return ti;
}


void ProgressIndicatorView::resizeEvent ( QResizeEvent *event )
{
    // Tell the layout in the parent (StorageServiceProgressDialog) that our size changed
    updateGeometry();

    QSize sz = parentWidget()->sizeHint();
    int currentWidth = parentWidget()->width();

    // Don't resize to sz.width() every time when it only reduces a little bit
    if ( currentWidth < sz.width() || currentWidth > sz.width() + 100 ) {
        currentWidth = sz.width();
    }
    parentWidget()->resize( currentWidth, sz.height() );

    QScrollArea::resizeEvent( event );
}

QSize ProgressIndicatorView::sizeHint() const
{
    return minimumSizeHint();
}

QSize ProgressIndicatorView::minimumSizeHint() const
{
    int f = 2 * frameWidth();
    // Make room for a vertical scrollbar in all cases, to avoid a horizontal one
    int vsbExt = verticalScrollBar()->sizeHint().width();
    int minw = topLevelWidget()->width() / 3;
    int maxh = topLevelWidget()->height() / 2;
    QSize sz( mBigBox->minimumSizeHint() );
    sz.setWidth( qMax( sz.width(), minw ) + f + vsbExt );
    sz.setHeight( qMin( sz.height(), maxh ) + f );
    return sz;
}

void ProgressIndicatorView::slotLayoutFirstItem()
{
    //This slot is called whenever a TransactionItem is deleted, so this is a
    //good place to call updateGeometry(), so our parent takes the new size
    //into account and resizes.
    updateGeometry();

    /*
     The below relies on some details in Qt's behaviour regarding deleting
     objects. This slot is called from the destroyed signal of an item just
     going away. That item is at that point still in the  list of chilren, but
     since the vtable is already gone, it will have type QObject. The first
     one with both the right name and the right class therefor is what will
     be the first item very shortly. That's the one we want to remove the
     hline for.
  */
#if 0

    TransactionItem *ti = mBigBox->findChild<KPIM::TransactionItem*>( QLatin1String("TransactionItem") );
    if ( ti ) {
        ti->hideHLine();
    }
#endif
}



StorageServiceProgressDialog::StorageServiceProgressDialog(QWidget *alignWidget, QWidget *parent)
    : KPIM::OverlayWidget(alignWidget, parent)
{
    setFrameStyle( QFrame::Panel | QFrame::Sunken ); // QFrame

    setAutoFillBackground( true );

    mScrollView = new ProgressIndicatorView( this );
    layout()->addWidget( mScrollView );
}

StorageServiceProgressDialog::~StorageServiceProgressDialog()
{

}


void StorageServiceProgressDialog::slotTransactionAdded( /*ProgressItem *item*/ )
{
#if 0
    if ( item->parent() ) {
        if ( mTransactionsToListviewItems.contains( item->parent() ) ) {
            TransactionItem * parent = mTransactionsToListviewItems[ item->parent() ];
            parent->addSubTransaction( item );
        }
    } else {
        const bool first = mTransactionsToListviewItems.empty();
        StorageServiceProgressWidget *ti = mScrollView->addTransactionItem( item, first );
        if ( ti ) {
            mTransactionsToListviewItems.insert( item, ti );
        }
        if ( first && mWasLastShown ) {
            QTimer::singleShot( 1000, this, SLOT(slotShow()) );
        }
    }
#endif
}

void StorageServiceProgressDialog::slotTransactionCompleted( /*ProgressItem *item*/ )
{
#if 0
    if ( mTransactionsToListviewItems.contains( item ) ) {
        TransactionItem *ti = mTransactionsToListviewItems[ item ];
        mTransactionsToListviewItems.remove( item );
        ti->setItemComplete();
        QTimer::singleShot( 3000, ti, SLOT(deleteLater()) );
        // see the slot for comments as to why that works
        connect ( ti, SIGNAL(destroyed()),
                  mScrollView, SLOT(slotLayoutFirstItem()) );
    }
    // This was the last item, hide.
    if ( mTransactionsToListviewItems.empty() ) {
        QTimer::singleShot( 3000, this, SLOT(slotHide()) );
    }
#endif
}

void StorageServiceProgressDialog::slotTransactionCanceled( /*ProgressItem **/ )
{
}

void StorageServiceProgressDialog::slotTransactionProgress( /*ProgressItem *item, unsigned int progress*/ )
{
    /*
    if ( mTransactionsToListviewItems.contains( item ) ) {
        TransactionItem *ti = mTransactionsToListviewItems[ item ];
        ti->setProgress( progress );
    }
    */
}


void StorageServiceProgressDialog::slotTransactionUsesBusyIndicator( /*KPIM::ProgressItem *item, bool value*/ )
{
#if 0
    if ( mTransactionsToListviewItems.contains( item ) ) {
        TransactionItem *ti = mTransactionsToListviewItems[ item ];
        if ( value ) {
            ti->setTotalSteps( 0 );
        } else {
            ti->setTotalSteps( 100 );
        }
    }
#endif
}

void StorageServiceProgressDialog::slotShow()
{
    setVisible( true );
}

void StorageServiceProgressDialog::slotHide()
{
    /*
    // check if a new item showed up since we started the timer. If not, hide
    if ( mTransactionsToListviewItems.isEmpty() ) {
        setVisible( false );
    }
    */
}

void StorageServiceProgressDialog::slotClose()
{
    mWasLastShown = false;
    setVisible( false );
}

void StorageServiceProgressDialog::setVisible( bool b )
{
#if 0
    OverlayWidget::setVisible( b );
    emit visibilityChanged( b );
#endif
}

void StorageServiceProgressDialog::slotToggleVisibility()
{
#if 0
    /* Since we are only hiding with a timeout, there is a short period of
   * time where the last item is still visible, but clicking on it in
   * the statusbarwidget should not display the dialog, because there
   * are no items to be shown anymore. Guard against that.
   */
    mWasLastShown = isHidden();
    if ( !isHidden() || !mTransactionsToListviewItems.isEmpty() ) {
        setVisible( isHidden() );
    }
#endif
}
