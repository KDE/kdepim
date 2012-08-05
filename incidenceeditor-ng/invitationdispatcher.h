/*
  Copyright (C) 2010 Bertjan Broeksema <broeksema@kde.org>
  Copyright (c) 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

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

#include "incidenceeditors-ng_export.h"
#include "editoritemmanager.h"

#include <akonadi/calendar/etmcalendar.h>
#include <QtCore/QObject>

namespace IncidenceEditorNG {

class InvitationDispatcherPrivate;

/**
  Listens to an EditorItemManager and sends out the invitations every time an
  Incidence was saved successfuly.
 */
class  INCIDENCEEDITORS_NG_EXPORT InvitationDispatcher : public QObject
{
  Q_OBJECT
  public:
    explicit InvitationDispatcher( const Akonadi::ETMCalendar::Ptr &calendar, QObject *parent = 0 );
    ~InvitationDispatcher();

    void setIsCounterProposal( bool isCounterProposal );

    /**
      Sets the manager to which this dispatcher listens for the itemSaveFinished
      signal.
     */
    void setItemManager( EditorItemManager *manager );

  private:
    InvitationDispatcherPrivate *const d_ptr;
    Q_DECLARE_PRIVATE( InvitationDispatcher )
    Q_DISABLE_COPY( InvitationDispatcher )

    Q_PRIVATE_SLOT( d_ptr, void processItemSave( IncidenceEditorNG::EditorItemManager::SaveAction ) )
    Q_PRIVATE_SLOT( d_ptr, void resetManager() )
};

}

#endif
