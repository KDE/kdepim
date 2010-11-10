/*
* Copyright (c) 2010 Volker Krause <vkrause@kde.org>
* Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
* Copyright (c) 2010 Andras Mantia <andras@kdab.com>
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

#include <Akonadi/Entity>
#include <KCalCore/ScheduleMessage>

#include <calendarviews/eventviews/eventview.h>
#include <calendarviews/eventviews/prefs.h>

namespace CalendarSupport {
class Calendar;
class IncidenceChanger;
}

namespace KPIMIdentities {
class IdentityManager;
}

class CalendarInterface;
class KJob;
class QDate;
class ConfigWidget;

class MainView : public KDeclarativeMainView
{
  Q_OBJECT

  public:
    explicit MainView( QWidget* parent = 0 );

    ~MainView();

    void setConfigWidget( ConfigWidget *configWidget );

  public Q_SLOTS:
    void showRegularCalendar();

    void setCurrentEventItemId( qint64 id );

    void newEvent();
    void newTodo();
    void editIncidence( const Akonadi::Item &item, const QDate &date );
    void uploadFreeBusy();
    void mailFreeBusy();
    void sendAsICalendar();
    void publishItemInformation();
    void sendInvitation();
    void sendStatusUpdate();
    void sendCancellation();
    void requestUpdate();
    void requestChange();
    void saveAllAttachments();
    void archiveOldEntries();
    void changeCalendarColor();

  protected Q_SLOTS:
    void delayedInit();
    void qmlLoadingStateChanged( QDeclarativeView::Status status );

  private Q_SLOTS:
    void finishEdit( QObject *editor );
    void fetchForSendICalDone( KJob *job );
    void fetchForPublishItemDone( KJob *job );
    void fetchForiTIPMethodDone( KJob *job );
    void fetchForSaveAllAttachmentsDone( KJob *job );

  protected:
    virtual void setupStandardActionManager( QItemSelectionModel *collectionSelectionModel,
                                             QItemSelectionModel *itemSelectionModel );

    virtual void setupAgentActionManager( QItemSelectionModel *selectionModel );

    virtual QAbstractProxyModel* createItemFilterModel() const;
    virtual ImportHandlerBase* importHandler() const;
    virtual ExportHandlerBase* exportHandler() const;
    virtual GuiStateManager* createGuiStateManager() const;
    virtual bool useFilterLineEditInCurrentState() const;

    void scheduleiTIPMethod( KCalCore::iTIPMethod method );

  private:
    CalendarSupport::Calendar *m_calendar;
    CalendarInterface* m_calendarIface;
    QHash<QObject*, Akonadi::Entity::Id> m_openItemEditors;
    KPIMIdentities::IdentityManager* m_identityManager;
    CalendarSupport::IncidenceChanger *m_changer;
    EventViews::PrefsPtr m_calendarPrefs;
};

#endif // MAINVIEW_H
