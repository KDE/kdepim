/*
    This file is part of libkcal.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>
    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
    
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
    
    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "folderconfig.h"

#include "folderlister.h"
#include "groupwaredataadaptor.h"
#include "folderlistview.h"

#include <klistview.h>
#include <klocale.h>
#include <kdebug.h>
#include <kdialog.h>

#include <qlayout.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qhbox.h>
#include <qlabel.h>

using namespace KPIM;

FolderConfig::FolderConfig( QWidget *parent )
  : QWidget( parent ), mFolderLister( 0 )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->addSpacing( KDialog::spacingHint() );

  QGroupBox *topBox = new QGroupBox( 1, Horizontal, i18n("Folder Selection"),
    this );
  topLayout->addWidget( topBox );
  
  QPushButton *button = new QPushButton( i18n("Update Folder List"), topBox );
  connect( button, SIGNAL( clicked() ), SIGNAL( updateFoldersClicked() ) );
  
  mFolderList = new FolderListView( topBox );

/*  QHBox *writeBox = new QHBox( topBox );

  new QLabel( i18n("Write to:"), writeBox );
  
  mWriteCombo = new QComboBox( writeBox );*/
}

FolderConfig::~FolderConfig()
{
}

void FolderConfig::setFolderLister( FolderLister *f )
{
  mFolderLister = f;
  
  QValueList<FolderListView::Property> types;
  QValueList<FolderLister::ContentType> suptypes( mFolderLister->supportedTypes() );
  if ( suptypes.contains( FolderLister::Event ) ) types << FolderListView::Event;
  if ( suptypes.contains( FolderLister::Todo ) ) types << FolderListView::Todo;
  if ( suptypes.contains( FolderLister::Journal ) ) types << FolderListView::Journal;
  if ( suptypes.contains( FolderLister::Contact ) ) types << FolderListView::Contact;
  if ( suptypes.contains( FolderLister::All ) ) types << FolderListView::All;
  if ( suptypes.contains( FolderLister::Unknown ) ) types << FolderListView::Unknown;

  mFolderList->setEnabledTypes( types );
  connect( mFolderLister, SIGNAL( foldersRead() ), SLOT( updateFolderList() ) );
}

void FolderConfig::retrieveFolderList( const KURL &url )
{
  kdDebug(7000) << "FolderConfig::retrieveFolderList()" << endl;
  if ( !mOldFolderListerURL.isEmpty() ) return;

  if ( mFolderLister->adaptor() ) {
    mOldFolderListerURL = mFolderLister->adaptor()->baseURL();
    mFolderLister->adaptor()->setBaseURL( url );
  }
  mFolderLister->retrieveFolders( url );
}

void FolderConfig::updateFolderList()
{
  mFolderList->clear();

  QStringList write;

  if ( !mOldFolderListerURL.isEmpty() && mFolderLister->adaptor() ) {
    mFolderLister->adaptor()->setBaseURL( mOldFolderListerURL );
    mOldFolderListerURL = KURL();
  }

  FolderLister::Entry::List folders = mFolderLister->folders();
  FolderLister::Entry::List::ConstIterator it;
  for( it = folders.begin(); it != folders.end(); ++it ) {
    FolderListItem *item = new FolderListItem( mFolderList, (*it) );
    if ( mFolderLister->writeDestinationId( FolderLister::Event ) == (*it).id ) {
      item->setDefault( FolderListView::Event );
    }
    if ( mFolderLister->writeDestinationId( FolderLister::Todo ) == (*it).id ) {
      item->setDefault( FolderListView::Todo );
    }
    if ( mFolderLister->writeDestinationId( FolderLister::Journal ) == (*it).id ) {
      item->setDefault( FolderListView::Journal );
    }
    if ( mFolderLister->writeDestinationId( FolderLister::Contact ) == (*it).id ) {
      item->setDefault( FolderListView::Contact );
    }
    if ( mFolderLister->writeDestinationId( FolderLister::All ) == (*it).id ) {
      item->setDefault( FolderListView::All );
    }
    if ( mFolderLister->writeDestinationId( FolderLister::Unknown ) == (*it).id ) {
      item->setDefault( FolderListView::Unknown );
    }
  }
}

void FolderConfig::saveSettings()
{
  QPtrList<QListViewItem> lst;

  FolderLister::Entry::List folders;

  QListViewItemIterator it( mFolderList );
  while ( it.current() ) {
    FolderListItem *item = dynamic_cast<FolderListItem *>( it.current() );
    if ( item ) {
      FolderLister::Entry folder = item->folder();
      folder.active = item->isOn();
      folders.append( folder );
      if ( item->isDefault( FolderListView::Event ) ) {
        mFolderLister->setWriteDestinationId( FolderLister::Event, folder.id );
      }
      if ( item->isDefault( FolderListView::Todo ) ) {
        mFolderLister->setWriteDestinationId( FolderLister::Todo, folder.id );
      }
      if ( item->isDefault( FolderListView::Journal ) ) {
        mFolderLister->setWriteDestinationId( FolderLister::Journal, folder.id );
      }
      if ( item->isDefault( FolderListView::Contact ) ) {
        mFolderLister->setWriteDestinationId( FolderLister::Contact, folder.id );
      }
      if ( item->isDefault( FolderListView::All ) ) {
        mFolderLister->setWriteDestinationId( FolderLister::All, folder.id );
      }
      if ( item->isDefault( FolderListView::Unknown ) ) {
        mFolderLister->setWriteDestinationId( FolderLister::Unknown, folder.id );
      }
    }
    ++it;
  }

  mFolderLister->setFolders( folders );
}

#include "folderconfig.moc"
