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

class MainView : public KDeclarativeMainView
{
  Q_OBJECT

  public:
    explicit MainView( QWidget* parent = 0 );

    ~MainView();

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

  protected Q_SLOTS:
    void delayedInit();
    void connectQMLSlots( QDeclarativeView::Status status );

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

    virtual QAbstractProxyModel* itemFilterModel() const;
    virtual ImportHandlerBase* importHandler() const;
    virtual ExportHandlerBase* exportHandler() const;
    virtual GuiStateManager* createGuiStateManager() const;

    void scheduleiTIPMethod( KCalCore::iTIPMethod method );

  private:
    CalendarSupport::Calendar *m_calendar;
    CalendarInterface* m_calendarIface;
    QHash<QObject*, Akonadi::Entity::Id> m_openItemEditors;
    KPIMIdentities::IdentityManager* m_identityManager;
    CalendarSupport::IncidenceChanger *m_changer;
};

#endif // MAINVIEW_H
