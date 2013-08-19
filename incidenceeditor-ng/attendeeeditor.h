/*
  Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
  Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#ifndef INCIDENCEEDITOR_ATTENDEEEDITOR_H
#define INCIDENCEEDITOR_ATTENDEEEDITOR_H

#include "attendeeline.h"
#include "attendeedata.h"

#include <libkdepim/multiplyingline/multiplyinglineeditor.h>

namespace IncidenceEditorNG {

class INCIDENCEEDITORS_NG_EXPORT AttendeeLineFactory : public KPIM::MultiplyingLineFactory
{
  Q_OBJECT
  public:
    explicit AttendeeLineFactory( QObject *parent ) : KPIM::MultiplyingLineFactory( parent )
    {
    }

    virtual KPIM::MultiplyingLine *newLine( QWidget *parent )
    {
      return new AttendeeLine( parent );
    }
};

class INCIDENCEEDITORS_NG_EXPORT AttendeeEditor : public KPIM::MultiplyingLineEditor
{
  Q_OBJECT
  public:
    explicit AttendeeEditor( QWidget *parent = 0 );

    AttendeeData::List attendees() const;

    void addAttendee( const KCalCore::Attendee::Ptr &attendee );
    void removeAttendee( const AttendeeData::Ptr &attendee );

    void setActions( AttendeeLine::AttendeeActions actions );

  signals:
    void countChanged( int );
    void changed( const KCalCore::Attendee::Ptr &oldAttendee,
                  const KCalCore::Attendee::Ptr &newAttendee );
    void editingFinished( KPIM::MultiplyingLine * );

  protected slots:
    void slotLineAdded( KPIM::MultiplyingLine * );
    void slotCalculateTotal();
};

}

#endif
