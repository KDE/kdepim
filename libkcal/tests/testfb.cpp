/*
    This file is part of libkcal.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include <iostream>
#include <kdebug.h>

#include "event.h"
#include "icalformat.h"
#include "freebusy.h"

using namespace KCal;

int main( int, char ** )
{
  const QString fbString =
    "BEGIN:VCALENDAR\n" 
    "PRODID:-//proko2//freebusy 1.0//EN\n"
    "METHOD:PUBLISH\n"
    "VERSION:2.0\n"
    "BEGIN:VFREEBUSY\n"
    "ORGANIZER:MAILTO:test3@kdab.net\n"
    "X-KDE-Foo:bla\n"
    "DTSTAMP:20071202T152453Z\n"
    "URL:http://mail.kdab.net/freebusy/test3%40kdab.net.ifb\n"
    "DTSTART:19700101T000000Z\n"
    "DTEND:200700101T000000Z\n"
    "COMMENT:This is a dummy vfreebusy that indicates an empty calendar\n"
    "FREEBUSY:19700101T000000Z/19700101T000000Z\n"
    "FREEBUSY;X-UID=bGlia2NhbC0xODk4MjgxNTcuMTAxMA==;X-\n"
    " SUMMARY=RW1wbG95ZWUgbWVldGluZw==;X-LOCATION=Um9vb\n"
    " SAyMTM=:20080131T170000Z/20080131T174500Z\n"
    "END:VFREEBUSY\n"
    "END:VCALENDAR\n";

  ICalFormat format;
  FreeBusy *fb = format.parseFreeBusy( fbString );
  kdDebug() << fb->busyPeriods().count() << " " << fb->dtStart() << endl;
  const PeriodList l = fb->busyPeriods();
  for ( PeriodList::ConstIterator it = l.begin(); it != l.end(); ++it )
    kdDebug() << (*it).start() << " " << (*it).end() << "+ " << (*it).summary() << ":" << (*it).location() << endl;

  typedef QMap<QCString, QString> FooMap;
  const FooMap props = fb->customProperties();
  for ( FooMap::ConstIterator it = props.begin(); it != props.end(); ++it )
    kdDebug() << it.key() << ": " << it.data() << endl;
}
