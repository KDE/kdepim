/*
* This file is part of Akonadi
*
* Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
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

#ifndef MAINVIEW_H
#define MAINVIEW_H

#include "kdeclarativemainview.h"

#include <AkonadiCore/Entity>
#include <Akonadi/Calendar/ETMCalendar>

#include <calendarviews/eventview.h>
#include <calendarviews/prefs.h>

class KJob;
class TasksActionManager;
class ConfigWidget;

namespace Akonadi {
class StandardCalendarActionManager;
class IncidenceChanger;
}

namespace CalendarSupport {
class CalendarUtils;
}

class MainView : public KDeclarativeMainView
{
  Q_OBJECT
  public:
    explicit MainView( QWidget *parent = 0 );
    ~MainView();

    void setConfigWidget( ConfigWidget *configWidget );

  public slots:
    void newTask();
    void newSubTask();
    void makeTaskIndependent();
    void makeAllSubtasksIndependent();
    void purgeCompletedTasks();
    void setPercentComplete( int row, int percentComplete );
    void editIncidence();
    void editIncidence( const Akonadi::Item &item );
    void saveAllAttachments();

  private slots:
    void finishEdit( QObject *editor );
    void fetchForSaveAllAttachmentsDone( KJob* job );
    void processActionFail( const Akonadi::Item &item, const QString &msg );
    void processActionFinish( const Akonadi::Item &item );
    void archiveOldEntries();
    void updateActionTexts();
    void configureCategories();

  protected:
    virtual void doDelayedInit();
    virtual QAbstractItemModel* createItemModelContext( QDeclarativeContext *context, QAbstractItemModel *model );
    virtual void setupStandardActionManager( QItemSelectionModel *collectionSelectionModel,
                                             QItemSelectionModel *itemSelectionModel );

    virtual void setupAgentActionManager( QItemSelectionModel *selectionModel );

    virtual QAbstractProxyModel* createItemFilterModel() const;
    virtual ImportHandlerBase* importHandler() const;
    virtual ExportHandlerBase* exportHandler() const;

    Akonadi::Item currentItem() const;

  private:
    CalendarSupport::CalendarUtils *mCalendarUtils;
    QHash<QObject*, Akonadi::Entity::Id> mOpenItemEditors;
    Akonadi::StandardCalendarActionManager *mStandardActionManager;
    TasksActionManager *mTasksActionManager;
    EventViews::PrefsPtr mCalendarPrefs;
    Akonadi::ETMCalendar::Ptr mCalendar;
    Akonadi::IncidenceChanger *mChanger;
};

#endif // MAINVIEW_H
