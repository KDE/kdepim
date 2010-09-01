/*
    This file is part of libkcal.

    Copyright (c) 2001-2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
    Copyright (c) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KCAL_INCIDENCEFORMATTER_H
#define KCAL_INCIDENCEFORMATTER_H

#include "libkcal_export.h"

#include <tqdatetime.h>
#include <tqstring.h>
#include <tqstringlist.h>

namespace KCal {
class Calendar;
class Incidence;
class IncidenceBase;

class LIBKCAL_EXPORT InvitationFormatterHelper
{
  public:
    virtual TQString generateLinkURL( const TQString &id ) { return id; }
    virtual TQString makeLink( const TQString &id, const TQString &text );
    virtual Calendar *calendar() const { return 0; }
};

/**
  This class is a helper class that provides several static methods to format an Incidence
  into different formats, like an HTML representation for KMail, a representation for tool tips,
  or a representation for the event viewer.

  @short methods to format incidences into various formats for displaying them
*/
class LIBKCAL_EXPORT IncidenceFormatter
{
  public:
    static TQString KDE_DEPRECATED toolTipString( IncidenceBase *incidence, bool richText = true );
    static TQString toolTipStr( Calendar *calendar,
                               IncidenceBase *incidence,
                               const TQDate &date=TQDate(),
                               bool richText = true );
    static TQString mailBodyString( IncidenceBase *incidencebase );
    static TQString KDE_DEPRECATED extensiveDisplayString( IncidenceBase *incidence );
    static TQString extensiveDisplayStr( Calendar *calendar,
                                        IncidenceBase *incidence,
                                        const TQDate &date=TQDate() );

    static TQString formatICalInvitation( TQString invitation, Calendar *mCalendar,
                                         InvitationFormatterHelper *helper );
    static TQString KDE_DEPRECATED formatICalInvitationNoHtml( TQString invitation,
                                                              Calendar *mCalendar,
                                                              InvitationFormatterHelper *helper );
    static TQString formatICalInvitationNoHtml( TQString invitation,
                                               Calendar *mCalendar,
                                               InvitationFormatterHelper *helper,
                                               const TQString &sender );

    // Format a TNEF attachment to an HTML mail
    static TQString formatTNEFInvitation( const TQByteArray& tnef,
                                         Calendar *mCalendar,
                                         InvitationFormatterHelper *helper );
    // Transform a TNEF attachment to an iCal or vCard
    static TQString msTNEFToVPart( const TQByteArray& tnef );

    static TQString recurrenceString( Incidence *incidence );

    /*
      Returns a reminder string computed for the specified Incidence.
      Each item of the returning TQStringList corresponds to a string
      representation of an reminder belonging to this incidence.
      @param incidence is a pointer to the Incidence.
      @param shortfmt if false, a short version of each reminder is printed;
      else a longer version of each reminder is printed.
    */
    static TQStringList reminderStringList( Incidence *incidence, bool shortfmt = true );

    static TQString timeToString( const TQDateTime &date, bool shortfmt = true );

    static TQString dateToString( const TQDateTime &date, bool shortfmt = true );

    static TQString dateTimeToString( const TQDateTime &date,
                                     bool dateOnly = false,
                                     bool shortfmt = true );
    /**
      Returns a Calendar Resource label name for the specified Incidence.
      @param calendar is a pointer to the Calendar.
      @param incidence is a pointer to the Incidence.
    */
    static TQString resourceString( Calendar *calendar, Incidence *incidence );

    /**
      Returns a duration string computed for the specified Incidence.
      Only makes sense for Events and Todos.
      @param incidence is a pointer to the Incidence.
    */
    static TQString durationString( Incidence *incidence );

  private:
    static TQString formatICalInvitationHelper( TQString invitation,
                                               Calendar *mCalendar,
                                               InvitationFormatterHelper *helper,
                                               bool noHtmlMode,
                                               const TQString &sender );
    class EventViewerVisitor;
    class ScheduleMessageVisitor;
    class InvitationHeaderVisitor;
    class InvitationBodyVisitor;
    class IncidenceCompareVisitor;
    class ToolTipVisitor;
    class MailBodyVisitor;
};

}

#endif
