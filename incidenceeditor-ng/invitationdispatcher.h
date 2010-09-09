/*
  Copyright (C) 2010 Bertjan Broeksema <broeksema@kde.org>
  Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#ifndef INCIDENCEEDITOR_INVITATIONDISPATCHER_H
#define INCIDENCEEDITOR_INVITATIONDISPATCHER_H

#include <QtCore/QObject>

#include "editoritemmanager.h"

namespace CalendarSupport {

class Calendar;
class InvitationDispatcherPrivate;

/**
  Listens to an EditorItemManager and sends out the invitations every time an
  Incidence was saved successfuly.
 */
class InvitationDispatcher : public QObject
{
  Q_OBJECT
  public:
    explicit InvitationDispatcher( Calendar *calendar, QObject *parent = 0 );
    ~InvitationDispatcher();

    void setIsCounterProposal( bool isCounterProposal );

    /**
      Sets the manager to which this dispatcher listens for the itemSaveFinished
      signal.
     */
    void setItemManager( CalendarSupport::EditorItemManager *manager );

  private:
    InvitationDispatcherPrivate * const d_ptr;
    Q_DECLARE_PRIVATE( InvitationDispatcher );
    Q_DISABLE_COPY( InvitationDispatcher );

    Q_PRIVATE_SLOT( d_ptr, void processItemSave( CalendarSupport::EditorItemManager::SaveAction ) );
    Q_PRIVATE_SLOT( d_ptr, void resetManager() );
};

}

#endif
