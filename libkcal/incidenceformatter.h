/*
    This file is part of libkcal.

    Copyright (c) 2001-2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KCAL_INCIDENCEFORMATTER_H
#define KCAL_INCIDENCEFORMATTER_H

#include <qstring.h>

#include "libkcal_export.h"

namespace KCal {
class Calendar;
class Incidence;
class IncidenceBase;

class InvitationFormatterHelper
{
  public:
    virtual QString generateLinkURL( const QString &id ) { return id; }
    virtual QString makeLink( const QString &id, const QString &text );
};

/**
  This class is a helper class that provides several static methods to format an Incidence
  into different formats, like an HTML representation for KMail, a representation for tool tips,
  ir a representation for the event viewer.

  @short methods to format incidences into various formats for displaying them
*/
class LIBKCAL_EXPORT IncidenceFormatter
{
  public:
    static QString toolTipString( IncidenceBase *incidence, bool richText = true );
    static QString mailBodyString( IncidenceBase *incidencebase );
    static QString extensiveDisplayString( IncidenceBase *incidence );

    static QString formatICalInvitation( QString invitation, Calendar *mCalendar,
                                         InvitationFormatterHelper *helper );
    // Format a TNEF attachment to an HTML mail
    static QString formatTNEFInvitation( const QByteArray& tnef,
                                         Calendar *mCalendar,
                                         InvitationFormatterHelper *helper );
    // Transform a TNEF attachment to an iCal or vCard
    static QString msTNEFToVPart( const QByteArray& tnef );
  private:
    class EventViewerVisitor;
    class ScheduleMessageVisitor;
    class InvitationHeaderVisitor;
    class InvitationBodyVisitor;
    class ToolTipVisitor;
    class MailBodyVisitor;
};

}

#endif
