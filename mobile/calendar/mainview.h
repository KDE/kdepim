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

#include <Akonadi/Calendar/ETMCalendar>
#include <AkonadiCore/Entity>
#include <KCalCore/ScheduleMessage>

#include <calendarviews/eventview.h>
#include <calendarviews/prefs.h>

#include <KCalCore/Incidence>

namespace Akonadi {
class StandardCalendarActionManager;
class IncidenceChanger;
class ITIPHandler;
}

namespace KIdentityManagement {
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

    static EventViews::PrefsPtr preferences();

  public Q_SLOTS:
    void showRegularCalendar();

    void setCurrentEventItemId( qint64 id );

    void newEvent();
    void newEventWithDate( const QDate &date );
    void newTodo();

    void openIncidenceEditor( const QString &summary,
                              const QString &description,
                              const QStringList &attachmentUris,
                              const QStringList &attendees,
                              const QStringList &atttachmentMimeTypes,
                              bool attachmentIsInline,
                              KCalCore::Incidence::IncidenceType type );

    void editIncidence();
    void editIncidence( const Akonadi::Item &item, const QDate &date );
    void deleteIncidence();
    void deleteIncidence( const Akonadi::Item &item );
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
    void qmlLoadingStateChanged( QDeclarativeView::Status status );

  private Q_SLOTS:
    void finishEdit( QObject *editor );
    void fetchForSendICalDone( KJob *job );
    void fetchForPublishItemDone( KJob *job );
    void fetchForiTIPMethodDone( KJob *job );
    void fetchForSaveAllAttachmentsDone( KJob *job );
    void updateActionTexts();
    void configureCategories();

  protected:
    void doDelayedInit();
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
    Akonadi::ETMCalendar::Ptr m_calendar;
    CalendarInterface* m_calendarIface;
    QHash<QObject*, Akonadi::Entity::Id> m_openItemEditors;
    KIdentityManagement::IdentityManager* m_identityManager;
    Akonadi::IncidenceChanger *m_changer;
    static EventViews::PrefsPtr m_calendarPrefs;
    Akonadi::StandardCalendarActionManager *mActionManager;
    Akonadi::ITIPHandler *mITIPHandler;
};

#endif // MAINVIEW_H
