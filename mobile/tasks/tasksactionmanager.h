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
#ifndef TASKSACTIONMANAGER_H
#define TASKSACTIONMANAGER_H

#include <Akonadi/Calendar/ETMCalendar>
#include <QtCore/QObject>

class KActionCollection;
class QItemSelectionModel;

class TasksActionManager : public QObject
{
  Q_OBJECT
public:
  explicit TasksActionManager( KActionCollection *actionCollection, QObject *parent = 0 );

  void setCalendar( const Akonadi::ETMCalendar::Ptr &calendar );
  void setItemSelectionModel( QItemSelectionModel *itemSelectionModel );

public slots:
  void updateActions();

private:
  void initActions(); // Initializes the tasks application specific actions.

private:
  KActionCollection         *mActionCollection;
  Akonadi::ETMCalendar::Ptr mCalendar;
  QItemSelectionModel       *mItemSelectionModel;
};

#endif // TASKACTIONMANAGER_H
