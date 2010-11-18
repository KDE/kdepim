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

#include "snippetsmanager.h"

#include "snippetdialog_p.h"
#include "snippetsmodel_p.h"

#include <klocale.h>
#include <kmessagebox.h>

#include <QtGui/QAction>
#include <QtGui/QItemSelectionModel>

using namespace MailCommon;

class SnippetsManager::Private
{
  public:
    Private( SnippetsManager *qq )
      : q( qq )
    {
    }

    QModelIndex currentGroupIndex() const;

    void selectionChanged();

    void addSnippet();
    void editSnippet();
    void deleteSnippet();

    void addSnippetGroup();
    void editSnippetGroup();
    void deleteSnippetGroup();

    SnippetsManager *q;
    SnippetsModel *mModel;
    QItemSelectionModel *mSelectionModel;
    KActionCollection *mActionCollection;

    QAction *mAddSnippetAction;
    QAction *mEditSnippetAction;
    QAction *mDeleteSnippetAction;
    QAction *mAddSnippetGroupAction;
    QAction *mEditSnippetGroupAction;
    QAction *mDeleteSnippetGroupAction;
};

QModelIndex SnippetsManager::Private::currentGroupIndex() const
{
  if ( mSelectionModel->selectedIndexes().isEmpty() )
    return QModelIndex();

  const QModelIndex index = mSelectionModel->selectedIndexes().first();
  if ( index.data( SnippetsModel::IsGroupRole ).toBool() )
    return index;
  else
    return mModel->parent( index );
}

void SnippetsManager::Private::selectionChanged()
{
  const bool itemSelected = !mSelectionModel->selectedIndexes().isEmpty();

  if ( itemSelected ) {
    const QModelIndex index = mSelectionModel->selectedIndexes().first();
    const bool isGroup = index.data( SnippetsModel::IsGroupRole ).toBool();
    if ( isGroup ) {
      mEditSnippetAction->setEnabled( false );
      mDeleteSnippetAction->setEnabled( false );
      mEditSnippetGroupAction->setEnabled( true );
      mDeleteSnippetGroupAction->setEnabled( true );
    } else {
      mEditSnippetAction->setEnabled( true );
      mDeleteSnippetAction->setEnabled( true );
      mEditSnippetGroupAction->setEnabled( false );
      mDeleteSnippetGroupAction->setEnabled( false );
    }
  } else {
    mEditSnippetAction->setEnabled( false );
    mDeleteSnippetAction->setEnabled( false );
    mEditSnippetGroupAction->setEnabled( false );
    mDeleteSnippetGroupAction->setEnabled( false );
  }
}

void SnippetsManager::Private::addSnippet()
{
  const bool noGroupAvailable = (mModel->rowCount() == 0);

  if ( noGroupAvailable ) {
    // create a 'General' snippet group
    if ( !mModel->insertRow( mModel->rowCount(), QModelIndex() ) )
      return;

    const QModelIndex groupIndex = mModel->index( mModel->rowCount() - 1, 0, QModelIndex() );
    mModel->setData( groupIndex, i18n( "General" ), SnippetsModel::NameRole );

    mSelectionModel->select( groupIndex, QItemSelectionModel::ClearAndSelect );
  }

  SnippetDialog dlg( mActionCollection, false );
  dlg.setWindowTitle( i18nc( "@title:window", "Add Snippet" ) );
  dlg.setGroupModel( mModel );

  if ( !dlg.exec() )
    return;

  const QModelIndex groupIndex = dlg.groupIndex();

  if ( !mModel->insertRow( mModel->rowCount( groupIndex ), groupIndex ) )
    return;

  const QModelIndex index = mModel->index( mModel->rowCount( groupIndex ) - 1, 0, groupIndex );
  mModel->setData( index, dlg.name(), SnippetsModel::NameRole );
  mModel->setData( index, dlg.text(), SnippetsModel::TextRole );
  mModel->setData( index, dlg.keySequence().toString(), SnippetsModel::KeySequenceRole );
}

void SnippetsManager::Private::editSnippet()
{
  const QModelIndex oldGroupIndex = currentGroupIndex();

  QModelIndex index = mSelectionModel->selectedIndexes().first();

  SnippetDialog dlg( mActionCollection, false );
  dlg.setWindowTitle( i18nc( "@title:window", "Edit Snippet" ) );
  dlg.setGroupModel( mModel );
  dlg.setGroupIndex( oldGroupIndex );
  dlg.setName( index.data( SnippetsModel::NameRole ).toString() );
  dlg.setText( index.data( SnippetsModel::TextRole ).toString() );
  dlg.setKeySequence( QKeySequence::fromString( index.data( SnippetsModel::KeySequenceRole ).toString() ) );

  if ( !dlg.exec() )
    return;

  const QModelIndex newGroupIndex = dlg.groupIndex();

  if ( oldGroupIndex != newGroupIndex ) {
    mModel->removeRow( index.row(), oldGroupIndex );
    mModel->insertRow( mModel->rowCount( newGroupIndex ), newGroupIndex );

    index = mModel->index( mModel->rowCount( newGroupIndex ) - 1, 0, newGroupIndex );
  }

  mModel->setData( index, dlg.name(), SnippetsModel::NameRole );
  mModel->setData( index, dlg.text(), SnippetsModel::TextRole );
  mModel->setData( index, dlg.keySequence().toString(), SnippetsModel::KeySequenceRole );
}

void SnippetsManager::Private::deleteSnippet()
{
  const QModelIndex index = mSelectionModel->selectedIndexes().first();

  const QString snippetName = index.data( SnippetsModel::NameRole ).toString();

  if ( KMessageBox::warningContinueCancel( 0, i18nc( "@info",
                  "Do you really want to remove snippet \"%1\"?<nl/>"
                  "<warning>There is no way to undo the removal.</warning>", snippetName ),
                  QString(),
                  KStandardGuiItem::remove() ) == KMessageBox::Cancel ) {
    return;
  }

  mModel->removeRow( index.row(), currentGroupIndex() );
}

void SnippetsManager::Private::addSnippetGroup()
{
  SnippetDialog dlg( mActionCollection, true );
  dlg.setWindowTitle( i18nc( "@title:window", "Add Group" ) );

  if ( !dlg.exec() )
    return;

  if ( !mModel->insertRow( mModel->rowCount(), QModelIndex() ) )
    return;

  const QModelIndex groupIndex = mModel->index( mModel->rowCount() - 1, 0, QModelIndex() );
  mModel->setData( groupIndex, dlg.name(), SnippetsModel::NameRole );
}

void SnippetsManager::Private::editSnippetGroup()
{
  const QModelIndex groupIndex = currentGroupIndex();
  if ( !groupIndex.isValid() )
    return;

  SnippetDialog dlg( mActionCollection, true );
  dlg.setWindowTitle( i18nc( "@title:window", "Edit Group" ) );
  dlg.setName( groupIndex.data( SnippetsModel::NameRole ).toString() );

  if ( !dlg.exec() )
    return;

  mModel->setData( groupIndex, dlg.name(), SnippetsModel::NameRole );
}

void SnippetsManager::Private::deleteSnippetGroup()
{
  const QModelIndex groupIndex = currentGroupIndex();
  if ( !groupIndex.isValid() )
    return;

  const QString groupName = groupIndex.data( SnippetsModel::NameRole ).toString();

  if ( mModel->rowCount( groupIndex ) > 0 ) {
    if ( KMessageBox::warningContinueCancel( 0, i18nc( "@info",
                    "Do you really want to remove group \"%1\" along with all its snippets?<nl/>"
                    "<warning>There is no way to undo the removal.</warning>", groupName ),
                    QString(),
                    KStandardGuiItem::remove() ) == KMessageBox::Cancel ) {
      return;
    }
  } else {
    if ( KMessageBox::warningContinueCancel( 0, i18nc( "@info",
                    "Do you really want to remove group \"%1\"?", groupName ),
                    QString(),
                    KStandardGuiItem::remove() ) == KMessageBox::Cancel ) {
      return;
    }
  }

  mModel->removeRow( groupIndex.row(), QModelIndex() );
}

SnippetsManager::SnippetsManager( KActionCollection *actionCollection, QObject *parent )
  : QObject( parent ), d( new Private( this ) )
{
  d->mModel = new SnippetsModel( this );
  d->mSelectionModel = new QItemSelectionModel( d->mModel );
  d->mActionCollection = actionCollection;

  d->mAddSnippetAction = new QAction( i18n( "Add Snippet..." ), this );
  d->mEditSnippetAction = new QAction( i18n( "Edit Snippet..." ), this );
  d->mDeleteSnippetAction = new QAction( i18n( "Remove Snippet" ), this );

  d->mAddSnippetGroupAction = new QAction( i18n( "Add Group..." ), this );
  d->mEditSnippetGroupAction = new QAction( i18n( "Rename Group..." ), this );
  d->mDeleteSnippetGroupAction = new QAction( i18n( "Remove Group" ), this );

  connect( d->mSelectionModel, SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
           this, SLOT( selectionChanged() ) );

  connect( d->mAddSnippetAction, SIGNAL( triggered( bool ) ), SLOT( addSnippet() ) );
  connect( d->mEditSnippetAction, SIGNAL( triggered( bool ) ), SLOT( editSnippet() ) );
  connect( d->mDeleteSnippetAction, SIGNAL( triggered( bool ) ), SLOT( deleteSnippet() ) );

  connect( d->mAddSnippetGroupAction, SIGNAL( triggered( bool ) ), SLOT( addSnippetGroup() ) );
  connect( d->mEditSnippetGroupAction, SIGNAL( triggered( bool ) ), SLOT( editSnippetGroup() ) );
  connect( d->mDeleteSnippetGroupAction, SIGNAL( triggered( bool ) ), SLOT( deleteSnippetGroup() ) );

  d->selectionChanged();
}

SnippetsManager::~SnippetsManager()
{
  delete d;
}

QAbstractItemModel *SnippetsManager::model() const
{
  return d->mModel;
}

QItemSelectionModel *SnippetsManager::selectionModel() const
{
  return d->mSelectionModel;
}

QAction *SnippetsManager::addSnippetAction() const
{
  return d->mAddSnippetAction;
}

QAction *SnippetsManager::editSnippetAction() const
{
  return d->mEditSnippetAction;
}

QAction *SnippetsManager::deleteSnippetAction() const
{
  return d->mDeleteSnippetAction;
}

QAction *SnippetsManager::addSnippetGroupAction() const
{
  return d->mAddSnippetGroupAction;
}

QAction *SnippetsManager::editSnippetGroupAction() const
{
  return d->mEditSnippetGroupAction;
}

QAction *SnippetsManager::deleteSnippetGroupAction() const
{
  return d->mDeleteSnippetGroupAction;
}

#include "snippetsmanager.moc"
