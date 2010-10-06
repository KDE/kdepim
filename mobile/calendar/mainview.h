/*
* This file is part of Akonadi
*
* Copyright (c) 2010 Volker Krause <vkrause@kde.org>
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

class KJob;
namespace KPIMIdentities {
class IdentityManager;
}

class CalendarInterface;
class QDate;

namespace CalendarSupport
{
class Calendar;
}

class MainView : public KDeclarativeMainView
{
  Q_OBJECT

  public:
    explicit MainView( QWidget* parent = 0 );

    ~MainView();

  public slots:
    void showRegularCalendar();

    void setCurrentEventItemId( qint64 id );

    void newEvent();
    void newTodo();
    void editIncidence( const Akonadi::Item &item, const QDate &date );
    void uploadFreeBusy();
    void mailFreeBusy();
    void sendAsICalendar();

  protected slots:
    void delayedInit();
    void connectQMLSlots(QDeclarativeView::Status status);

  private slots:
    void finishEdit( QObject *editor );
    void fetchForSendICalDone( KJob* job );

  protected:
    virtual void setupStandardActionManager( QItemSelectionModel *collectionSelectionModel,
                                             QItemSelectionModel *itemSelectionModel );

    virtual void setupAgentActionManager( QItemSelectionModel *selectionModel );

    virtual QAbstractProxyModel* itemFilterModel() const;
    virtual ImportHandlerBase* importHandler() const;
    virtual ExportHandlerBase* exportHandler() const;

  private:
    CalendarSupport::Calendar *m_calendar;
    CalendarInterface* m_calendarIface;
    QHash<QObject*, Akonadi::Entity::Id> m_openItemEditors;
    KPIMIdentities::IdentityManager* m_identityManager;
};

#endif // MAINVIEW_H
