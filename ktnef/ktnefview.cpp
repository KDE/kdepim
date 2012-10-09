/*
  This file is part of KTnef.

  Copyright (C) 2002 Michael Goffioul <kdeprint@swing.be>
  Copyright (c) 2012 Allen Winter <winter@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software Foundation,
  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "ktnefview.h"
#include "attachpropertydialog.h"

#include <KTNEF/KTNEFAttach>

#include <KApplication>
#include <KDebug>
#include <KIconLoader>
#include <KLocale>
#include <KMimeType>

#include <QPixmap>
#include <QTimer>

class Attachment : public QTreeWidgetItem
{
  public:
    Attachment( QTreeWidget *parent, KTNEFAttach *attach );
    ~Attachment();

    KTNEFAttach *getAttachment() const
    {
      return mAttach;
    }

  private:
    KTNEFAttach *mAttach;
};

Attachment::Attachment( QTreeWidget *parent, KTNEFAttach *attach )
  : QTreeWidgetItem( parent, QStringList( attach->name() ) ), mAttach( attach )
{
  setText( 2, QString::number( mAttach->size() ) );
  if ( !mAttach->fileName().isEmpty() ) {
    setText( 0, mAttach->fileName() );
  }

  KMimeType::Ptr mimeType = KMimeType::mimeType( mAttach->mimeTag() );
  setText( 1, mimeType->comment() );

  QPixmap pix = loadRenderingPixmap( attach, kapp->palette().color( QPalette::Background ) );
  if ( !pix.isNull() ) {
    setIcon( 0, pix );
  } else {
    setIcon( 0, KIcon( mimeType->iconName() ) );
  }
}

Attachment::~Attachment()
{
}

//----------------------------------------------------------------------------//

KTNEFView::KTNEFView( QWidget *parent )
  : QTreeWidget( parent )
{
  const QStringList headerLabels =
    ( QStringList( i18nc( "@title:column file name", "File Name" ) )
      << i18nc( "@title:column file type", "File Type" )
      << i18nc( "@title:column file size", "Size" ) );
  setHeaderLabels( headerLabels );
  setSelectionMode( QAbstractItemView::ExtendedSelection );
  setDragEnabled( true );
  QTimer::singleShot( 0, this, SLOT(adjustColumnWidth()) );
}

KTNEFView::~KTNEFView()
{
}

void KTNEFView::setAttachments( const QList<KTNEFAttach *>& list )
{
  clear();
  if ( !list.isEmpty() ) {
    QList<KTNEFAttach *>::ConstIterator it;
    QList<KTNEFAttach *>::ConstIterator end(list.constEnd());
    for ( it=list.constBegin(); it != end; ++it ) {
      new Attachment( this, (*it) );
    }
  }
}

void KTNEFView::resizeEvent( QResizeEvent *e )
{
  adjustColumnWidth();
  resize( width(), height() );
  if ( e ) {
    QTreeWidget::resizeEvent( e );
  }
}

QList<KTNEFAttach *> KTNEFView::getSelection()
{
  mAttachments.clear();

  QList<QTreeWidgetItem *> list = selectedItems();
  if ( list.isEmpty() || !list.first() ) {
    return mAttachments;
  }

  QList<QTreeWidgetItem *>::iterator it;
  for ( it=list.begin(); it != list.end(); ++it ) {
    Attachment *a = static_cast<Attachment *>( *it );
    mAttachments.append( a->getAttachment() );
  }
  return mAttachments;
}

void KTNEFView::startDrag( Qt::DropActions dropAction )
{
  Q_UNUSED( dropAction );

  QTreeWidgetItemIterator it( this, QTreeWidgetItemIterator::Selected );
  QList<KTNEFAttach*> list;
  while ( *it ) {
    Attachment *a = static_cast<Attachment *>( *it );
    list << a->getAttachment();
    ++it;
  }
  if ( !list.isEmpty() ) {
    emit dragRequested( list );
  }
}

void KTNEFView::adjustColumnWidth()
{
  int w = width() / 2;
  setColumnWidth( 0, w );
  setColumnWidth( 1, w / 2 );
  setColumnWidth( 2, w / 2 );
}

#include "ktnefview.moc"
