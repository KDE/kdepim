/*
  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.

  NOTE: May, 2010. Extracted this code from
        kdepim/incidenceeditors/editorattachments.{h,cpp}
*/

#include <config-enterprise.h>

#include "attachmenticonview.h"

#include <KIconLoader>
#include <KTemporaryFile>

#include <QDrag>
#include <QKeyEvent>
#include <QMimeData>

using namespace IncidenceEditorNG;

AttachmentIconItem::AttachmentIconItem( const KCalCore::Attachment::Ptr &att, QListWidget *parent )
  : QListWidgetItem( parent )
{
  if ( att ) {
    mAttachment = KCalCore::Attachment::Ptr( new KCalCore::Attachment( *att.data() ) );
    mAttachment->setLabel( att->label() );
  } else {
    // for the enteprise, inline attachments are the default
#ifdef KDEPIM_ENTERPRISE_BUILD
    mAttachment =
      KCalCore::Attachment::Ptr(
        new KCalCore::Attachment( QByteArray() ) ); // use the non-uri constructor
                                                    // as we want inline by default
#else
    mAttachment = KCalCore::Attachment::Ptr( new KCalCore::Attachment( QString() ) );
#endif
  }
  readAttachment();
  setFlags( flags() | Qt::ItemIsDragEnabled );
}

AttachmentIconItem::~AttachmentIconItem()
{
}

KCalCore::Attachment::Ptr AttachmentIconItem::attachment() const
{
  return mAttachment;
}

const QString AttachmentIconItem::uri() const
{
  return mAttachment->uri();
}

const QString AttachmentIconItem::savedUri() const
{
  return mSaveUri;
}

void AttachmentIconItem::setUri( const QString &uri )
{
  mSaveUri = uri;
  mAttachment->setUri( mSaveUri );
  readAttachment();
}

void AttachmentIconItem::setData( const QByteArray &data )
{
  mAttachment->setDecodedData( data );
  readAttachment();
}

const QString AttachmentIconItem::mimeType() const
{
  return mAttachment->mimeType();
}

void AttachmentIconItem::setMimeType( const QString &mime )
{
  mAttachment->setMimeType( mime );
  readAttachment();
}

const QString AttachmentIconItem::label() const
{
  return mAttachment->label();
}

void AttachmentIconItem::setLabel( const QString &description )
{
  if ( mAttachment->label() == description ) {
    return;
  }
  mAttachment->setLabel( description );
  readAttachment();
}

bool AttachmentIconItem::isBinary() const
{
  return mAttachment->isBinary();
}

QPixmap AttachmentIconItem::icon() const
{
  return icon( KMimeType::mimeType( mAttachment->mimeType() ),
                mAttachment->uri(), mAttachment->isBinary() );
}

QPixmap AttachmentIconItem::icon( KMimeType::Ptr mimeType,
                                  const QString &uri,
                                  bool binary )
{
//QT5
#if 0
  QString iconStr = mimeType->iconName( uri );
  QStringList overlays;
  if ( !uri.isEmpty() && !binary ) {
    overlays << "emblem-link";
  }

  return KIconLoader::global()->loadIcon( iconStr, KIconLoader::Desktop, 0,
                                          KIconLoader::DefaultState,
                                          overlays );
#else
return QPixmap();
#endif
}

void AttachmentIconItem::readAttachment()
{
  setText( mAttachment->label() );
  setFlags( flags() | Qt::ItemIsEditable );

  if ( mAttachment->mimeType().isEmpty() || !( KMimeType::mimeType( mAttachment->mimeType() ) ) ) {
    KMimeType::Ptr mimeType;
    if ( mAttachment->isUri() ) {
      mimeType = KMimeType::findByUrl( mAttachment->uri() );
    } else {
      mimeType = KMimeType::findByContent( mAttachment->decodedData() );
    }
    mAttachment->setMimeType( mimeType->name() );
  }

  setIcon( icon() );
}

AttachmentIconView::AttachmentIconView( QWidget *parent )
  : QListWidget( parent )
{
  setMovement( Static );
  setAcceptDrops( true );
  setSelectionMode( ExtendedSelection );
  setSelectionRectVisible( false );
  setIconSize( QSize( KIconLoader::SizeLarge, KIconLoader::SizeLarge ) );
  setFlow( LeftToRight );
  setWrapping( true );
#ifndef QT_NO_DRAGANDDROP
  setDragDropMode( DragDrop );
  setDragEnabled( true );
#endif
  setEditTriggers( EditKeyPressed );
  setContextMenuPolicy( Qt::CustomContextMenu );
}

KUrl AttachmentIconView::tempFileForAttachment( const KCalCore::Attachment::Ptr &attachment ) const
{
  if ( mTempFiles.contains( attachment ) ) {
    return mTempFiles.value( attachment );
  }
  KTemporaryFile *file = new KTemporaryFile();
  file->setParent( const_cast<AttachmentIconView*>( this ) );

  QStringList patterns = KMimeType::mimeType( attachment->mimeType() )->patterns();

  if ( !patterns.empty() ) {
    file->setSuffix( QString( patterns.first() ).remove( '*' ) );
  }
  file->setAutoRemove( true );
  file->open();
  // read-only not to give the idea that it could be written to
  file->setPermissions( QFile::ReadUser );
  file->write( QByteArray::fromBase64( attachment->data() ) );
  mTempFiles.insert( attachment, file->fileName() );
  file->close();
  return mTempFiles.value( attachment );
}

QMimeData *AttachmentIconView::mimeData( const QList< QListWidgetItem*> items ) const
{
  // create a list of the URL:s that we want to drag
  KUrl::List urls;
  QStringList labels;
  foreach ( QListWidgetItem *it, items ) {
    if ( it->isSelected() ) {
      AttachmentIconItem *item = static_cast<AttachmentIconItem *>( it );
      if ( item->isBinary() ) {
        urls.append( tempFileForAttachment( item->attachment() ) );
      } else {
        urls.append( item->uri() );
      }
      labels.append( KUrl::toPercentEncoding( item->label() ) );
    }
  }
  if ( selectionMode() == NoSelection ) {
    AttachmentIconItem *item = static_cast<AttachmentIconItem *>( currentItem() );
    if ( item ) {
      urls.append( item->uri() );
      labels.append( KUrl::toPercentEncoding( item->label() ) );
    }
  }

  QMap<QString, QString> metadata;
  metadata["labels"] = labels.join( ":" );

  QMimeData *mimeData = new QMimeData;
  urls.populateMimeData( mimeData, metadata );
  return mimeData;
}

QMimeData *AttachmentIconView::mimeData() const
{
  return mimeData( selectedItems() );
}

void AttachmentIconView::startDrag( Qt::DropActions supportedActions )
{
  Q_UNUSED( supportedActions );
#ifndef QT_NO_DRAGANDDROP
  QPixmap pixmap;
  if ( selectedItems().size() > 1 ) {
    pixmap = KIconLoader::global()->loadIcon( "mail-attachment", KIconLoader::Desktop );
  }
  if ( pixmap.isNull() ) {
    pixmap = static_cast<AttachmentIconItem *>( currentItem() )->icon();
  }

  const QPoint hotspot( pixmap.width() / 2, pixmap.height() / 2 );

  QDrag *drag = new QDrag( this );
  drag->setMimeData( mimeData() );

  drag->setPixmap( pixmap );
  drag->setHotSpot( hotspot );
  drag->exec( Qt::CopyAction );
#endif
}

void AttachmentIconView::keyPressEvent( QKeyEvent *event )
{
  if ( ( event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter ) &&
       currentItem() && state() != EditingState ) {
    emit itemDoubleClicked( currentItem() ); // ugly, but itemActivated() also includes single click
    return;
  }
  QListWidget::keyPressEvent( event );
}

