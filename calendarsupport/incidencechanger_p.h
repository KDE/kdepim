/*
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Author: Sergio Martins, <sergio.martins@kdab.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#ifndef CALENDARSUPPORT_INCIDENCECHANGER_P_H
#define CALENDARSUPPORT_INCIDENCECHANGER_P_H

#include "incidencechanger.h"
#include "next/invitationhandler.h"

#include <Akonadi/Item>

#include <KCalCore/Incidence>

#include <QObject>

namespace CalendarSupport {

class IncidenceChanger::Private : public QObject
{
  Q_OBJECT
  public:

    struct Change {
      KCalCore::Incidence::Ptr oldInc;
      Akonadi::Item newItem;
      IncidenceChanger::WhatChanged action;
      QWidget *parent;
      uint atomicOperationId;
    };

    struct AddInfo {
      QWidget *parent;
      uint atomicOperationId;
    };

    Private( Akonadi::Entity::Id defaultCollectionId, Calendar *calendar ) :
      mDefaultCollectionId( defaultCollectionId ),
      mDestinationPolicy( IncidenceChanger::ASK_DESTINATION ),
      mCalendar( calendar )
    {
    }

    ~Private()
    {
    }
    void queueChange( Change * );
    bool performChange( Change * );

    /*
     * Called when deleting an incidence, queued changes are canceled.
     */
    void cancelChanges( Akonadi::Item::Id id );

    bool myAttendeeStatusChanged( const KCalCore::Incidence::Ptr &newInc,
                                  const KCalCore::Incidence::Ptr &oldInc );

    Akonadi::Entity::Id mDefaultCollectionId;

    DestinationPolicy mDestinationPolicy;

    // Changes waiting for a job on the same item.id() to end
    QHash<Akonadi::Item::Id,Change*> mQueuedChanges;

    // Current changes with modify jobs still running
    QHash<Akonadi::Item::Id, Change*> mCurrentChanges;

    QHash<Akonadi::Item::Id, int> mLatestRevisionByItemId;

    QList<Akonadi::Item::Id> mDeletedItemIds;

    Calendar *mCalendar;

    // Index my atomic operation id
    QHash<uint, InvitationHandler::SendStatus> mOperationStatus;

    QHash<KJob*,AddInfo> mAddInfoForJob;

  public slots:
    void performNextChange( Akonadi::Item::Id );

  private slots:
    void changeIncidenceFinished( KJob *job );

  signals:
    void incidenceChangeFinished( const Akonadi::Item &oldinc,
                                  const Akonadi::Item &newInc,
                                  CalendarSupport::IncidenceChanger::WhatChanged,
                                  bool );

};

}

#endif
