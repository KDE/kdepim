/*
  Copyright 2010 Olivier Trichet <nive@nivalis.org>

  Permission to use, copy, modify, and distribute this software
  and its documentation for any purpose and without fee is hereby
  granted, provided that the above copyright notice appear in all
  copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

#include "attachment_view.h"

#include "../kncomposer.h"

#include <QHeaderView>
#include <QKeyEvent>


namespace KNode {
namespace Composer {

// -- AttachmentView ---------------------------------------------------

AttachmentView::AttachmentView( QWidget *parent )
  : QTreeWidget( parent )
{
  QHeaderView *h = header();
  h->setMovable( false );
  h->setResizeMode( QHeaderView::Interactive );
  h->setStretchLastSection( true );
}

AttachmentView::~AttachmentView()
{
}



void AttachmentView::removeCurrentAttachment()
{
  QList<QTreeWidgetItem *> items = selectedItems();
  foreach( QTreeWidgetItem *item, items ) {
    takeTopLevelItem( indexOfTopLevelItem( item ) );

    AttachmentViewItem *avi = static_cast< AttachmentViewItem * >( item );
    bool lastItem = ( topLevelItemCount() == 0 );
    emit attachmentRemoved( avi->mAttachment, lastItem );
  }
  qDeleteAll( items );
}

void AttachmentView::editCurrentAttachment()
{
  QList<QTreeWidgetItem *> items = selectedItems();
  if ( items.isEmpty() ) {
    return;
  }
  // Update the view to reflect that we're only editing one item.
  if ( items.size() > 1 ) {
    setCurrentItem( items[ 0 ] );
  }

  AttachmentViewItem *item = static_cast< AttachmentViewItem * >( currentItem() );
  QPointer<KNComposer::AttachmentPropertiesDlg> dlg = new KNComposer::AttachmentPropertiesDlg( item->mAttachment, this );
  if ( dlg->exec() == QDialog::Accepted && dlg ) {
    item->emitDataChanged(); // notify the changes
  }
  delete dlg;
}



const QList<KNAttachment::Ptr> AttachmentView::attachments()
{
  QList<KNAttachment::Ptr> al;
  KNAttachment::Ptr a;
  QTreeWidgetItemIterator it( this, QTreeWidgetItemIterator::All );
  while ( *it ) {
    a = static_cast< AttachmentViewItem * >( *it )->mAttachment;
    al.append( a );
    ++it;
  }
  return al;
}



void AttachmentView::keyPressEvent( QKeyEvent *event )
{
  if ( !selectedItems().isEmpty() ) {
    switch ( event->key() ) {
      case Qt::Key_Delete:
        emit deletePressed();
        break;
      case Qt::Key_Return:
      case Qt::Key_Enter:
        emit returnPressed();
        break;
    }
  }

  QTreeView::keyPressEvent( event );
}

void AttachmentView::contextMenuEvent( QContextMenuEvent* event )
{
  QTreeWidgetItem *item = itemAt( event->pos() );
  if ( item ) {
    setCurrentItem( item );
    emit contextMenuRequested( event->globalPos() );
    return;
  }

  QAbstractScrollArea::contextMenuEvent( event );
}



// -- AttachmentViewItem -----------------------------------------------

AttachmentViewItem::AttachmentViewItem( AttachmentView *parent, KNAttachment::Ptr attachment )
  : QTreeWidgetItem( parent ),
    mAttachment( attachment )
{
  Q_ASSERT( mAttachment );
}

AttachmentViewItem::~AttachmentViewItem()
{
}

QVariant AttachmentViewItem::data( int column, int role ) const
{
  if ( role == Qt::DisplayRole ) {
    switch ( column ) {
      case AttachmentView::File:
        return mAttachment->name();
      case AttachmentView::Type:
        return mAttachment->mimeType();
      case AttachmentView::Size:
        return mAttachment->contentSize();
      case AttachmentView::Description:
        return mAttachment->description();
      case AttachmentView::Encoding:
        return mAttachment->encoding();
    }
  }

  return QTreeWidgetItem::data( column, role );
}




} // namespace Composer
} // namespace KNode

