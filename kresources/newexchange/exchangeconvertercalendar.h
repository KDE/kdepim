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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KCal_EXCHANGECONVERTERCONTACT_H
#define KCal_EXCHANGECONVERTERCONTACT_H

#include <qstring.h>
#include <qdom.h>

#include <libkcal/incidence.h>
#include <libkcal/icalformat.h>

namespace KCal {
class ICalFormat;

class ExchangeConverterCalendar
{
  public:

    ExchangeConverterCalendar();
    
    void setTimeZone( const QString &id );
    
    static void createRequestAppointment( QDomDocument &doc, QDomElement &root );
    static void createRequestTask( QDomDocument &doc, QDomElement &root );
    static void createRequestJournal( QDomDocument &doc, QDomElement &root );
    
    QDomDocument createWebDAV( Incidence *incidence );
    
    Incidence::List parseWebDAV( const QDomDocument& davdata );
    bool readIncidence( const QDomElement &node, Incidence *incidence );
    
  protected:
    static void createRequestIncidence( QDomDocument &doc, QDomElement &root );
    bool readEvent( const QDomElement &node, Event *event );
    bool readTodo( const QDomElement &node, Todo *todo );
    bool readJournal( const QDomElement &node, Journal *journal );
    bool readTZ( const QDomElement &node, Incidence *incidence );

    KCal::ICalFormat mFormat;
    class createWebDAVVisitor;
};

}

#endif
