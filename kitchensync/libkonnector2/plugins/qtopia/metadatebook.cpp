/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2003 Holger Freyther <freyther@kde.org>

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

#include <libkcal/event.h>
#include <libkcal/recurrence.h>

#include "metadatebook.h"

using namespace OpieHelper;

MetaDatebook::MetaDatebook()
    : MD5Template<KSync::EventSyncee, KSync::EventSyncEntry>()
{
}
MetaDatebook::~MetaDatebook() {
}
QString MetaDatebook::string( KSync::EventSyncEntry* ev) {
    QString string;
    KCal::Event* event = ev->incidence();

    string  = event->categories().join(";");
    string += event->summary();
    string += event->description();
    string += event->location();
    string += event->dtStart().toString("dd.MM.yyyy hh:mm:ss");
    string += event->dtEnd().toString("dd.MM.yyyy hh:mm:ss");
    string += QString::number( event->doesFloat() );

    /* Recurrance */
    KCal::Recurrence* rec = event->recurrence();
    if ( rec->doesRecur() ) {
        switch( rec->doesRecur() ) {
        case KCal::Recurrence::rDaily:
            string += "Daily";
            break;
        case KCal::Recurrence::rWeekly:{
            string += "Weekly";
            string += days( rec->days() );
            break;
        }
        case KCal::Recurrence::rMonthlyPos:
            string += "MonthlyDay";
            break;
        case KCal::Recurrence::rMonthlyDay:
            string += "MonthlyDate";
            break;
        case KCal::Recurrence::rYearlyDay: // see datebook.cpp
        case KCal::Recurrence::rYearlyMonth:
        case KCal::Recurrence::rYearlyPos:
            string += "Yearly";
            break;
        case KCal::Recurrence::rNone:
        default:
            break;
        }
        string += QString::number( rec->frequency() );

        /* test duration */
        string += QString::number( rec->duration() );
        if ( rec->duration() == 0 ) {
            string += rec->endDate().toString("dd.MM.yyyy");
        }
        string += rec->recurStart().date().toString("dd.MM.yyyy hh:mm:ss");
    }
    /* Alarms here */
/*    KCal::Alarm* al = event->alarms().first();
    if ( al != 0 ) {
        int sec = al->offset().asSeconds();
        string += QString::number( sec );
        string += al->audioFile();
    }
*/
    return string;
}
QString MetaDatebook::days( const QBitArray& ar ) {
    QString str;
    if ( ar.testBit(0 ) ) str += "Mo";
    if ( ar.testBit(1 ) ) str += "Di";
    if ( ar.testBit(2 ) ) str += "Mi";
    if ( ar.testBit(3 ) ) str += "Do";
    if ( ar.testBit(4 ) ) str += "Fr";
    if ( ar.testBit(5 ) ) str += "Sa";
    if ( ar.testBit(6 ) ) str += "So";

    return str;
}
