/*
  This file is part of KOrganizer.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  Copyright (c) 2010 Bertjan Broeksema <b.broeksema@home.nl>

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
  , mUi( new Ui::AttachmentEditDialog )
{ 
  QWidget *page = new QWidget(this);
  mUi->setupUi( page );
  mUi->mIcon->setPixmap( item->icon() );
  
  setMainWidget( page );
  setModal( modal );
  
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
  if ( mUi->mURLRequester ) {
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

