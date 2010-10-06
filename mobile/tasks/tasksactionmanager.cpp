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

#include <KAction>
#include <KActionCollection>
#include <KLocalizedString>

#include <QtGui/QItemSelectionModel>

TasksActionManager::TasksActionManager( KActionCollection *actionCollection, QObject *parent)
  : QObject( parent )
  , mActionCollection( actionCollection )
  , mItemSelectionModel( 0 )
{
  initActions();
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

}

void TasksActionManager::initActions()
{
  KAction *action = mActionCollection->addAction( QLatin1String( "add_new_task" ) );
  action->setText( i18n( "New Task" ) );

  action = mActionCollection->addAction( QLatin1String( "import_tasks" ) );
  action->setText( i18n( "Import Tasks" ) );

  action = mActionCollection->addAction( QLatin1String( "export_tasks" ) );
  action->setText( i18n( "Export Tasks" ) );
}

#include "tasksactionmanager.moc"
