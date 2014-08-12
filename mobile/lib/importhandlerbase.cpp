/*
    Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "importhandlerbase.h"

#include <AkonadiWidgets/collectiondialog.h>
#include <AkonadiCore/entitytreemodel.h>
#include <AkonadiCore/itemcreatejob.h>
#include <kfiledialog.h>
#include <QProgressDialog>
#include <QUrl>
#include <QtCore/QPointer>

ImportHandlerBase::ImportHandlerBase( QObject *parent )
  : QObject( parent ),
    mImportProgressDialog( 0 ),
    mSelectionModel( 0 )
{
}

ImportHandlerBase::~ImportHandlerBase()
{
}

void ImportHandlerBase::setSelectionModel( QItemSelectionModel *model )
{
  mSelectionModel = model;
}

void ImportHandlerBase::exec()
{
  const QStringList fileNames = KFileDialog::getOpenFileNames( QUrl(), fileDialogNameFilter(),
                                                               0, fileDialogTitle() );

  if ( fileNames.count() == 0 ) {
    deleteLater();
    return;
  }

  bool ok = false;
  const Akonadi::Item::List items = createItems( fileNames, &ok );
  if ( !ok || items.isEmpty() ) {
    deleteLater();
    return;
  }

  QPointer<Akonadi::CollectionDialog> dlg = new Akonadi::CollectionDialog();
  dlg->setMimeTypeFilter( mimeTypes() );
  dlg->setAccessRightsFilter( Akonadi::Collection::CanCreateItem );
  dlg->setCaption( collectionDialogTitle() );
  dlg->setDescription( collectionDialogText() );

  // preselect the currently selected folder
  if ( mSelectionModel ) {
    const QModelIndexList indexes = mSelectionModel->selectedRows();
    if ( !indexes.isEmpty() ) {
      const QModelIndex collectionIndex = indexes.first();
      const Akonadi::Collection collection = collectionIndex.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
      if ( collection.isValid() )
        dlg->setDefaultCollection( collection );
    }
  }

  if ( !dlg->exec() || !dlg ) {
    delete dlg;
    deleteLater();
    return;
  }

  const Akonadi::Collection collection = dlg->selectedCollection();
  delete dlg;

  if ( !mImportProgressDialog ) {
    mImportProgressDialog = new QProgressDialog( 0);
    mImportProgressDialog->setWindowTitle(importDialogTitle() );
    mImportProgressDialog->setLabelText( importDialogText( items.count(), collection.name() ) );
    mImportProgressDialog->setCancelButton(0);
    mImportProgressDialog->setAutoClose( true );
    mImportProgressDialog->setRange( 1, items.count() );
  }

  mImportProgressDialog->show();

  foreach ( const Akonadi::Item &item, items ) {
    Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob( item, collection );
    connect( job, SIGNAL(result(KJob*)), SLOT(slotImportJobDone(KJob*)) );
  }
}

void ImportHandlerBase::slotImportJobDone( KJob* )
{
  if ( !mImportProgressDialog )
    return;


  mImportProgressDialog->setValue( mImportProgressDialog->value() + 1 );

  // cleanup on last step
  if ( mImportProgressDialog->value() == mImportProgressDialog->maximum() ) {
    mImportProgressDialog->deleteLater();
    mImportProgressDialog = 0;
    deleteLater();
  }
}

