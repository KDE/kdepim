/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Tobias Koenig <tokoe@kdab.com>

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

#include "attachmenteditor.h"

#include <kactioncollection.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <messagecomposer/attachment/attachmentcontrollerbase.h>
#include <messagecomposer/attachment/attachmentmodel.h>

#include <QtCore/QAbstractItemModel>
#include <QAction>
#include <QItemSelectionModel>

AttachmentEditor::AttachmentEditor( KActionCollection *actionCollection, MessageComposer::AttachmentModel *model,
                                    MessageComposer::AttachmentControllerBase *controller, QObject *parent )
  : QObject( parent ),
    mModel( model ),
    mSelectionModel( new QItemSelectionModel( mModel ) ),
    mAttachmentController( controller )
{
  mAddAction = actionCollection->action( QLatin1String("attach") );
  mAddAction->setText( i18n( "Add Attachment" ) );
  mDeleteAction = actionCollection->action( QLatin1String("remove") );
  mDeleteAction->setText( i18n( "Remove Attachment" ) );

  mSignAction = new QAction( this );
  mSignAction->setText( i18n( "Sign" ) );
  mSignAction->setCheckable( true );
  connect( mSignAction, SIGNAL(triggered(bool)), SLOT(signAttachment(bool)) );

  mEncryptAction = new QAction( this );
  mEncryptAction->setText( i18n( "Encrypt" ) );
  mEncryptAction->setCheckable( true );
  connect( mEncryptAction, SIGNAL(triggered(bool)), SLOT(encryptAttachment(bool)) );

  actionCollection->addAction( QLatin1String("toggle_attachment_signed"), mSignAction );
  actionCollection->addAction( QLatin1String("toggle_attachment_encrypted"), mEncryptAction );

  connect( mSelectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
           this, SLOT(selectionChanged()) );

  selectionChanged();
}

void AttachmentEditor::setRowSelected( int row )
{
  Q_ASSERT( row >= 0 && row < mModel->rowCount() );

  mSelectionModel->select( mModel->index( row, 0, QModelIndex() ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
}

void AttachmentEditor::selectionChanged()
{
  mAddAction->setEnabled( true );

  if ( mSelectionModel->hasSelection() ) {
    mDeleteAction->setEnabled( true );
    mSignAction->setEnabled( true );
    mEncryptAction->setEnabled( true );

    const QModelIndex signIndex = mModel->index( mSelectionModel->selectedIndexes().first().row(), MessageComposer::AttachmentModel::SignColumn );
    const QModelIndex encryptIndex = mModel->index( mSelectionModel->selectedIndexes().first().row(), MessageComposer::AttachmentModel::EncryptColumn );

    mSignAction->setChecked( signIndex.data( Qt::CheckStateRole ).toBool() );
    mEncryptAction->setChecked( encryptIndex.data( Qt::CheckStateRole ).toBool() );
  } else {
    mDeleteAction->setEnabled( false );
    mSignAction->setEnabled( false );
    mEncryptAction->setEnabled( false );
  }

  const QModelIndexList selectedRows = mSelectionModel->selectedRows();
  MessageCore::AttachmentPart::List selectedParts;
  foreach( const QModelIndex &index, selectedRows ) {
    const MessageCore::AttachmentPart::Ptr part = index.data( MessageComposer::AttachmentModel::AttachmentPartRole ).value<MessageCore::AttachmentPart::Ptr>();
    selectedParts.append( part );
  }

  mAttachmentController->setSelectedParts( selectedParts );
}

void AttachmentEditor::signAttachment( bool value )
{
  if ( !mSelectionModel->hasSelection() )
    return;

  const QModelIndex index = mModel->index( mSelectionModel->selectedIndexes().first().row(), MessageComposer::AttachmentModel::SignColumn );
  mModel->setData( index, value, Qt::CheckStateRole );
}

void AttachmentEditor::encryptAttachment( bool value )
{
  if ( !mSelectionModel->hasSelection() )
    return;

  const QModelIndex index = mModel->index( mSelectionModel->selectedIndexes().first().row(), MessageComposer::AttachmentModel::EncryptColumn );
  mModel->setData( index, value, Qt::CheckStateRole );
}

