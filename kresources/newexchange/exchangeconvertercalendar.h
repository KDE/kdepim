/*
    This file is part of the exchange resource.
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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KCal_EXCHANGECONVERTERCONTACT_H
#define KCal_EXCHANGECONVERTERCONTACT_H

#include <tqstring.h>
#include <tqdom.h>

#include <libkcal/incidence.h>
#include <libkcal/icalformat.h>

namespace KCal {
class ICalFormat;

class ExchangeConverterCalendar
{
  public:

    ExchangeConverterCalendar();
    
    void setTimeZone( const TQString &id );
    
    static void createRequestAppointment( TQDomDocument &doc, TQDomElement &root );
    static void createRequestTask( TQDomDocument &doc, TQDomElement &root );
    static void createRequestJournal( TQDomDocument &doc, TQDomElement &root );
    
    TQDomDocument createWebDAV( Incidence *incidence );
    
    Incidence::List parseWebDAV( const TQDomDocument& davdata );
    bool readIncidence( const TQDomElement &node, Incidence *incidence );
    
  protected:
    static void createRequestIncidence( TQDomDocument &doc, TQDomElement &root );
    bool readEvent( const TQDomElement &node, Event *event );
    bool readTodo( const TQDomElement &node, Todo *todo );
    bool readJournal( const TQDomElement &node, Journal *journal );
    bool readTZ( const TQDomElement &node, Incidence *incidence );

    KCal::ICalFormat mFormat;
    class createWebDAVVisitor;
};

}

#endif
