/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>

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
#include "tasksactionmanager.h"

#include <calendarsupport/calendar.h>

#include <Akonadi/EntityTreeModel>
#include <Akonadi/Item>

#include <KCalCore/Todo>

#include <KAction>
#include <KActionCollection>
#include <KLocalizedString>

#include <QtGui/QItemSelectionModel>

using namespace CalendarSupport;

TasksActionManager::TasksActionManager( KActionCollection *actionCollection, QObject *parent)
  : QObject( parent )
  , mActionCollection( actionCollection )
  , mCalendar( 0 )
  , mItemSelectionModel( 0 )
{
  initActions();
}

void TasksActionManager::setCalendar( Calendar *calendar )
{
  // Doesn't take ownership. The calendar is used in updateActions.
  mCalendar = calendar;
}

void TasksActionManager::setItemSelectionModel( QItemSelectionModel *itemSelectionModel )
{
  if ( mItemSelectionModel )
    disconnect( mItemSelectionModel, SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ) );

  mItemSelectionModel = itemSelectionModel;
  if ( mItemSelectionModel ) {
    connect( mItemSelectionModel, SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ),
             SLOT( updateActions() ) );
  }

  updateActions();
}

void TasksActionManager::updateActions()
{
  mActionCollection->action( QLatin1String( "add_new_subtask" ) )->setEnabled( false );
  mActionCollection->action( QLatin1String( "make_subtask_independent" ) )->setEnabled( false );
  mActionCollection->action( QLatin1String( "make_all_subtasks_independent" ) )->setEnabled( false );

  const QModelIndexList list = mItemSelectionModel->selectedRows();
  if ( list.size() != 1 )
    return;

  const QModelIndex idx = list.first();
  Akonadi::Item item = idx.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();

  if ( !item.isValid() )
    return;

  if ( !item.hasPayload<KCalCore::Todo::Ptr>() )
    return;

  if ( mCalendar->findChildren( item ).size() >= 1 )
    mActionCollection->action( QLatin1String( "make_all_subtasks_independent" ) )->setEnabled( true );

  mActionCollection->action( QLatin1String( "add_new_subtask" ) )->setEnabled( true );

  // Only enable the make_subtask_independent action for todos that have a parent.
  KCalCore::Todo::Ptr todo = item.payload<KCalCore::Todo::Ptr>();
  const bool enable = !todo->relatedTo( KCalCore::Todo::RelTypeParent ).isEmpty();
  mActionCollection->action( QLatin1String( "make_subtask_independent" ) )->setEnabled( enable );
}

void TasksActionManager::initActions()
{
  KAction *action = mActionCollection->addAction( QLatin1String( "add_new_task" ) );
  action->setText( i18n( "New Task" ) );

  action = mActionCollection->addAction( QLatin1String( "import_tasks" ) );
  action->setText( i18n( "Import Tasks" ) );

  action = mActionCollection->addAction( QLatin1String( "export_tasks" ) );
  action->setText( i18n( "Export Tasks" ) );

  action = mActionCollection->addAction( QLatin1String( "add_new_subtask" ) );
  action->setText( i18n( "New Sub Task" ) );

  action = mActionCollection->addAction( QLatin1String( "make_subtask_independent" ) );
  action->setText( i18n( "Make Sub Task Independent" ) );

  action = mActionCollection->addAction( QLatin1String( "make_all_subtasks_independent" ) );
  action->setText( i18n( "Make All Sub Tasks Independent" ) );

  action = mActionCollection->addAction( QLatin1String( "purge_completed_tasks" ) );
  action->setText( i18n( "Purge completed tasks" ) );

  action = mActionCollection->addAction( QLatin1String( "save_all_attachments" ) );
  action->setText( i18n( "Save All" ) );

}

#include "tasksactionmanager.moc"
