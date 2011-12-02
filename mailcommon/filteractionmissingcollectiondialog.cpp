/* -*- mode: C++; c-file-style: "gnu" -*-
  This file is part of KMail, the KDE mail client.
  Copyright (c) 2011 Montel Laurent <montel@kde.org>

  KMail is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  KMail is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "filteractionmissingcollectiondialog.h"
#include "mailkernel.h"
#include "mailutil.h"
#include <Akonadi/EntityMimeTypeFilterModel>
#include "folderrequester.h"

#include <KLocale>

#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>

FilterActionMissingCollectionDialog::FilterActionMissingCollectionDialog( const Akonadi::Collection::List& list, const QString& filtername, QWidget *parent )
  : KDialog( parent ), mListwidget( 0 )
{
  setModal( true );
  setCaption( i18n( "Select Folder" ) );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );
  showButtonSeparator( true );
  QVBoxLayout* lay = new QVBoxLayout( mainWidget() );
  if ( !list.isEmpty() ) {
    QLabel *lab = new QLabel( i18n( "We found some folders which can used for this filter:" ) );
    lay->addWidget( lab );
    mListwidget = new QListWidget( this );
    lay->addWidget( mListwidget );
    const int numberOfItems( list.count() );
    for ( int i = 0; i< numberOfItems; ++i ) {
      Akonadi::Collection col = list.at( i );
      QListWidgetItem *item = new QListWidgetItem( MailCommon::Util::fullCollectionPath( col ) );
      item->setData( FilterActionMissingCollectionDialog::IdentifyCollection, col.id() );
      mListwidget->addItem(  item );
    }
    connect( mListwidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), SLOT(slotCurrentItemChanged()));
  }

  QLabel *label = new QLabel( this );
  label->setText( i18n( "Folder is missing. Select a folder for filter \"%1\" please", filtername ) );
  lay->addWidget( label );
  mFolderRequester = new MailCommon::FolderRequester( this );
  lay->addWidget( mFolderRequester );
}

FilterActionMissingCollectionDialog::~FilterActionMissingCollectionDialog()
{
}

void FilterActionMissingCollectionDialog::slotCurrentItemChanged()
{
  QListWidgetItem * currentItem = mListwidget->currentItem();
  if ( currentItem ) {
    const Akonadi::Collection::Id id =  currentItem->data( FilterActionMissingCollectionDialog::IdentifyCollection ).toLongLong();
    mFolderRequester->setCollection(Akonadi::Collection( id ));
  }
}

Akonadi::Collection FilterActionMissingCollectionDialog::selectedCollection() const
{
  return mFolderRequester->collection();
}

void FilterActionMissingCollectionDialog::getPotentialFolders(  const QAbstractItemModel *model, const QModelIndex& parentIndex, const QString& lastElement, Akonadi::Collection::List& list )
{
  const int rowCount = model->rowCount( parentIndex );
  for ( int row = 0; row < rowCount; ++row ) {
    const QModelIndex index = model->index( row, 0, parentIndex );
    if ( model->rowCount( index ) > 0 ) {
      getPotentialFolders( model, index, lastElement, list );
    } else {
      if ( model->data( index ).toString() == lastElement )
        list << model->data( index, Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
    }
  }
}

Akonadi::Collection::List FilterActionMissingCollectionDialog::potentialCorrectFolders( const QString& path )
{
  Akonadi::Collection::List lst;
  const QString realPath = MailCommon::Util::realFolderPath( path );
  if ( realPath.isEmpty() )
    return lst;
  const int lastSlash = realPath.lastIndexOf( QLatin1Char( '/' ) );
  QString lastElement;
  if ( lastSlash == -1 )
    lastElement = realPath;
  else
    lastElement = realPath.right( realPath.length() - lastSlash - 1 );

  if ( KernelIf->collectionModel() ) {
    FilterActionMissingCollectionDialog::getPotentialFolders( KernelIf->collectionModel(),  QModelIndex(), lastElement,lst ) ;
    const int numberOfItems( lst.count() );
    for ( int i = 0; i < numberOfItems; ++i ) {
      if ( MailCommon::Util::fullCollectionPath( lst.at( i ) ) == realPath ) {
        return  Akonadi::Collection::List()<< lst.at( i );
      }
    }
  }
  return lst;
}

#include "filteractionmissingcollectiondialog.moc"

