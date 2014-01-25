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

void ProgressIndicatorView::resizeEvent ( QResizeEvent *event )
{
    // Tell the layout in the parent (progressdialog) that our size changed
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
