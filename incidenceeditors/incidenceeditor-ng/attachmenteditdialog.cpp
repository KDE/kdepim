/*
  This file is part of KOrganizer.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
  Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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
*/

#include "attachmenteditdialog.h"

#include <KDE/KCal/Attachment>
#include <KDE/KIO/NetAccess>

#include "attachmenticonview.h" // UGLY
#include "ui_attachmenteditdialog.h"

using namespace IncidenceEditorsNG;

AttachmentEditDialog::AttachmentEditDialog( AttachmentIconItem *item,
                                            QWidget *parent,
                                            bool modal )
  : KDialog( parent )
#ifdef KDEPIM_ENTERPRISE_BUILD
  , mAttachment( new KCal::Attachment( '\0' ) ) //use the non-uri constructor
                                                // as we want inline by default
#else
   , mAttachment( new KCal::Attachment( QString() ) )
#endif
  , mItem( item )
  , mMimeType( KMimeType::mimeType( item->mimeType() ) )
  , mUi( new Ui::AttachmentEditDialog )
{ 
  QWidget *page = new QWidget(this);
  mUi->setupUi( page );
  mUi->mLabelEdit->setText( item->label().isEmpty() ? item->uri() : item->label() );
  mUi->mIcon->setPixmap( item->icon() );
  mUi->mInlineCheck->setChecked( item->isBinary() );

  QString typecomment = item->mimeType().isEmpty() ?
                        i18nc( "@label unknown mimetype", "Unknown" ) :
                        mMimeType->comment();
  mUi->mTypeLabel->setText( typecomment );
  
  setMainWidget( page );
  setModal( modal );

  if ( item->attachment()->isUri() || !item->attachment()->data() ) {
    mUi->mStackedWidget->setCurrentIndex( 0 );
    mUi->mURLRequester->setUrl( item->uri() );
    urlChanged( item->uri() );
  } else {
    mUi->mStackedWidget->setCurrentIndex( 1 );
    mUi->mSizeLabel->setText( QString::fromLatin1( "%1 (%2)" ).
                                 arg( KIO::convertSize( item->attachment()->size() ) ).
                                 arg( KGlobal::locale()->formatNumber(
                                        item->attachment()->size(), 0 ) ) );
  }
  
  connect( mUi->mURLRequester, SIGNAL(urlSelected(const KUrl &)),
           SLOT(urlChanged(const KUrl &)) );
  connect( mUi->mURLRequester, SIGNAL( textChanged( const QString& ) ),
           SLOT( urlChanged( const QString& ) ) );
}

void AttachmentEditDialog::accept()
{
  slotApply();
  KDialog::accept();
}

void AttachmentEditDialog::slotApply()
{
  if ( mUi->mLabelEdit->text().isEmpty() ) {
    if ( mUi->mURLRequester->url().isLocalFile() ) {
      mItem->setLabel( mUi->mURLRequester->url().fileName() );
    } else {
      mItem->setLabel( mUi->mURLRequester->url().url() );
    }
  } else {
    mItem->setLabel( mUi->mLabelEdit->text() );
  }
  if ( mItem->label().isEmpty() ) {
    mItem->setLabel( i18nc( "@label", "New attachment" ) );
  }
  mItem->setMimeType( mMimeType->name() );
  if ( mUi->mStackedWidget->currentIndex() == 0 ) {
    if ( mUi->mInlineCheck->isChecked() ) {
      QString tmpFile;
      if ( KIO::NetAccess::download( mUi->mURLRequester->url(), tmpFile, this ) ) {
        QFile f( tmpFile );
        if ( !f.open( QIODevice::ReadOnly ) ) {
          return;
        }
        QByteArray data = f.readAll();
        f.close();
        mItem->setData( data );
      }
      KIO::NetAccess::removeTempFile( tmpFile );
    } else {
      mItem->setUri( mUi->mURLRequester->url().url() );
    }
  }
}

void AttachmentEditDialog::urlChanged( const QString &url )
{
  enableButtonOk( !url.isEmpty() );
}

void AttachmentEditDialog::urlChanged( const KUrl &url )
{
  mMimeType = KMimeType::findByUrl( url );
  mUi->mTypeLabel->setText( mMimeType->comment() );
  mUi->mIcon->setPixmap( AttachmentIconItem::icon( mMimeType, url.path() ) );
}

