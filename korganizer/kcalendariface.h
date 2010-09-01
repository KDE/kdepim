/*
    This file is a generic DCOP interface, shared between KDE applications.
    Copyright (c) 2003 David Faure <faure@kde.org>

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
#ifndef KCALENDARIFACE_H
#define KCALENDARIFACE_H

/** @file
* This file is a generic DCOP interface, shared between KDE applications.
* It handles Calendar requests.
*/

#include <dcopobject.h>
#include <tqdatetime.h>
#include <tqdatastream.h>
#include <tqstringlist.h>
// yes, this is this very header - but it tells dcopidl to include it
// in _stub.cpp and _skel.cpp files, to get the definition of the structs.
#include "kcalendariface.h"

typedef QPair<TQDateTime, TQDateTime> QDateTimePair;

/** Interface class for calendar requests. */
class KCalendarIface : public DCOPObject
{
    K_DCOP
  public:
    KCalendarIface() : DCOPObject("CalendarIface") {}

  k_dcop:

    /** This is a struct.
     *
     */
    struct ResourceRequestReply {
        bool vCalInOK;
        TQString vCalOut;
        bool vCalOutOK; bool isFree;
        TQDateTime start; TQDateTime end;
    };
    virtual KCalendarIface::ResourceRequestReply resourceRequest(
                         const TQValueList< QDateTimePair >& busy,
                         const TQCString& resource,
                         const TQString& vCalIn ) = 0;

    virtual void openEventEditor( const TQString& text ) = 0;
    virtual void openEventEditor( const TQString& summary,
                                  const TQString& description,
                                  const TQString& attachment ) = 0;
    virtual void openEventEditor( const TQString& summary,
                                  const TQString& description,
                                  const TQString& attachment,
                                  const TQStringList& attendees ) = 0;
    virtual void openEventEditor( const TQString& summary,
                                  const TQString& description,
                                  const TQString& uri,
                                  const TQString& file,
                                  const TQStringList& attendees,
                                  const TQString& attachmentMimetype ) = 0;

    virtual void openTodoEditor( const TQString& text ) = 0;
    virtual void openTodoEditor( const TQString& summary,
                                 const TQString& description,
                                 const TQString& attachment ) = 0;
    virtual void openTodoEditor( const TQString& summary,
                                 const TQString& description,
                                 const TQString& attachment,
                                 const TQStringList& attendees ) = 0;
    virtual void openTodoEditor( const TQString& summary,
                                 const TQString& description,
                                 const TQString& uri,
                                 const TQString& file,
                                 const TQStringList& attendees,
                                 const TQString& attachmentMimetype,
                                 bool isTask ) = 0;

    virtual void openJournalEditor( const TQDate& date ) = 0;
    virtual void openJournalEditor( const TQString& text,
                                    const TQDate& date ) = 0;
    virtual void openJournalEditor( const TQString& text ) = 0;
   //TODO:
   // virtual void openJournalEditor( const TQString& summary,
   //                                 const TQString& description,
   //                                 const TQString& attachment ) = 0;

    virtual void showJournalView() = 0;
    virtual void showTodoView() = 0;
    virtual void showEventView() = 0;

    virtual void goDate( const TQDate& date ) = 0;
    virtual void goDate( const TQString& date ) = 0;

    virtual void showDate( const TQDate &date ) = 0;
};

inline TQDataStream& operator<<( TQDataStream& str, const KCalendarIface::ResourceRequestReply& reply )
{
    str << reply.vCalInOK << reply.vCalOut << reply.vCalOutOK << reply.isFree << reply.start << reply.end;
    return str;
}

inline TQDataStream& operator>>( TQDataStream& str, KCalendarIface::ResourceRequestReply& reply )
{
    str >> reply.vCalInOK >> reply.vCalOut >> reply.vCalOutOK >> reply.isFree >> reply.start >> reply.end;
    return str;
}

#endif
