/*
* This file is part of Akonadi
*
* Copyright (c) 2010 Bertjan Broeksema <b.broeksema@home.nl>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301  USA
*/

#include "mainview.h"

#include <QtDeclarative/QDeclarativeEngine>

#include <KCal/Todo>
#include <KDebug>
#include <KGlobal>
#include <KStandardDirs>

#include <akonadi/kcal/incidencemimetypevisitor.h>
#include <akonadi/entitytreemodel.h>
#include <Akonadi/ItemFetchScope>

#include "incidenceview.h"
#include "tasklistproxy.h"

using namespace Akonadi;

MainView::MainView( QWidget *parent ) : KDeclarativeMainView( "tasks", new TaskListProxy, parent )
{
  addMimeType( IncidenceMimeTypeVisitor::todoMimeType() );
  itemFetchScope().fetchFullPayload();
}

void MainView::newTask()
{
  IncidenceView *editor = new IncidenceView;
  Item item;
  item.setMimeType( Akonadi::IncidenceMimeTypeVisitor::todoMimeType() );
  KCal::Todo::Ptr todo( new KCal::Todo );

  // make it due one day from now
  todo->setDtStart( KDateTime::currentLocalDateTime() );
  todo->setDtDue( KDateTime::currentLocalDateTime().addDays( 1 ) );

  item.setPayload<KCal::Todo::Ptr>( todo );
  editor->load( item );
  editor->show();
}

void MainView::setPercentComplete(int row, int percentComplete)
{
  const QModelIndex idx = itemModel()->index(row, 0);
  itemModel()->setData(idx, percentComplete, TaskListProxy::PercentComplete);
}

void MainView::editIncidence( const Akonadi::Item &item )
{
  IncidenceView *editor = new IncidenceView;
  editor->load( item );
  editor->show();
}

