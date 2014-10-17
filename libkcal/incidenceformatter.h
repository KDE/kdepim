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

#include <qdatetime.h>
#include <qstring.h>
#include <qstringlist.h>

namespace KCal {
class Calendar;
class Incidence;
class IncidenceBase;
class ScheduleMessage;
class Attendee;

class LIBKCAL_EXPORT InvitationFormatterHelper
{
  public:
    virtual QString generateLinkURL( const QString &id ) { return id; }
    virtual QString makeLink( const QString &id, const QString &text );
    virtual QString makeBtnLink( const QString &id, const QString &text, const QString &image );
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
    static QString KDE_DEPRECATED toolTipString( IncidenceBase *incidence, bool richText = true );
    static QString toolTipStr( Calendar *calendar,
                               IncidenceBase *incidence,
                               const QDate &date=QDate(),
                               bool richText = true );
    static QString mailBodyString( IncidenceBase *incidencebase );
    static QString KDE_DEPRECATED extensiveDisplayString( IncidenceBase *incidence );
    static QString extensiveDisplayStr( Calendar *calendar,
                                        IncidenceBase *incidence,
                                        const QDate &date=QDate() );

    static QString formatICalInvitation( QString invitation, Calendar *mCalendar,
                                         InvitationFormatterHelper *helper );
    static QString KDE_DEPRECATED formatICalInvitationNoHtml( QString invitation,
                                                              Calendar *mCalendar,
                                                              InvitationFormatterHelper *helper );
    static QString KDE_DEPRECATED formatICalInvitationNoHtml( QString invitation,
                                                              Calendar *mCalendar,
                                                              InvitationFormatterHelper *helper,
                                                              const QString &sender );

    static QString formatICalInvitationNoHtml( QString invitation,
                                               Calendar *mCalendar,
                                               InvitationFormatterHelper *helper,
                                               const QString &sender,
                                               bool outlookCompareStyle );

    // Format a TNEF attachment to an HTML mail
    static QString formatTNEFInvitation( const QByteArray& tnef,
                                         Calendar *mCalendar,
                                         InvitationFormatterHelper *helper );
    // Transform a TNEF attachment to an iCal or vCard
    static QString msTNEFToVPart( const QByteArray& tnef );

    static QString recurrenceString( Incidence *incidence );

    static QString formatGroupwareLinks(InvitationFormatterHelper *helper,
                                        Incidence *existingIncidence,
                                        Incidence *inc,
                                        ScheduleMessage *msg,
                                        bool rsvpRec,
                                        Attendee *a);
    /*
      Returns a reminder string computed for the specified Incidence.
      Each item of the returning QStringList corresponds to a string
      representation of an reminder belonging to this incidence.
      @param incidence is a pointer to the Incidence.
      @param shortfmt if false, a short version of each reminder is printed;
      else a longer version of each reminder is printed.
    */
    static QStringList reminderStringList( Incidence *incidence, bool shortfmt = true );

    static QString timeToString( const QDateTime &date, bool shortfmt = true );

    static QString dateToString( const QDateTime &date, bool shortfmt = true );

    static QString dateTimeToString( const QDateTime &date,
                                     bool dateOnly = false,
                                     bool shortfmt = true );
    /**
      Returns a Calendar Resource label name for the specified Incidence.
      @param calendar is a pointer to the Calendar.
      @param incidence is a pointer to the Incidence.
    */
    static QString resourceString( Calendar *calendar, Incidence *incidence );

    /**
      Returns a duration string computed for the specified Incidence.
      Only makes sense for Events and Todos.
      @param incidence is a pointer to the Incidence.
    */
    static QString durationString( Incidence *incidence );

  private:
    static QString formatICalInvitationHelper( QString invitation,
                                               Calendar *mCalendar,
                                               InvitationFormatterHelper *helper,
                                               bool noHtmlMode,
                                               const QString &sender,
                                               bool outlookCompareStyle );
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
