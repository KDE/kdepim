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

#include <qdom.h>
#include <qfile.h>

#include <klocale.h>

#include <calendarsyncee.h>
#include <libkcal/calendarlocal.h>

#include "device.h"
#include "datebook.h"

using namespace OpieHelper;

namespace {
// from TT
int week ( const QDate &start ) {
    int stop = start.day();
    int sentinel = start.dayOfWeek();
    int dayOfWeek = QDate( start.year(),  start.month(),  1 ).dayOfWeek();
    int week = 1;
    for ( int i = 1; i < stop; i++ ) {
        if ( dayOfWeek++ == sentinel )
            week++;
        if ( dayOfWeek > 7 )
            dayOfWeek = 0;
    }
    return week;
}

}

DateBook::DateBook( CategoryEdit* edit,
                    KSync::KonnectorUIDHelper* helper,
                    const QString& tz,
                    Device *dev )
    : Base( edit,  helper,  tz, dev )
{
}
DateBook::~DateBook(){
}
/**
 * Converts an Opie Event to a KDE one
 */
KCal::Event* DateBook::toEvent( QDomElement e, ExtraMap& extraMap, const QStringList& lst) {
    KCal::Event* event = new KCal::Event();

    /* Category block */
    {
    QStringList list = QStringList::split(";",  e.attribute("categories") );
    QStringList categories;

    QString cat;
    for ( uint i = 0; i < list.count(); i++ ) {
        cat = m_edit->categoryById(list[i], "Calendar");
	/* only add if name not empty and was not added before */
        if (!cat.isEmpty() && !categories.contains(cat) )
            categories.append(cat );
    }
    if (!categories.isEmpty() ) {
        event->setCategories( categories );
    }

    }

    event->setSummary( e.attribute("description") );
    event->setUid( kdeId( "EventSyncEntry",  e.attribute("uid") ) );
    event->setDescription( e.attribute("note") );
    event->setLocation( e.attribute("location") );
    // time

    QString start = e.attribute("start");
    event->setDtStart( fromUTC( (time_t) start.toLong() ) );

    QString end = e.attribute("end");
    event->setDtEnd( fromUTC( (time_t) end.toLong() ) );

    // type
    if ( e.attribute("type") == "AllDay" ) {
        event->setFloats( true );
    }else{
        event->setFloats( false );
    }

    // FIXME alarm


    // Recurrence damn I feared to do that
    QString type = e.attribute("rtype");
    int freq = e.attribute("rfreq").toInt();
    bool hasEnd = e.attribute("rhasenddate").toInt();

    KCal::Recurrence *rec = event->recurrence();
    start = e.attribute("created");
    rec->setRecurStart( fromUTC( (time_t) start.toLong() ) );

    if ( type == "Daily" ) {
        if ( hasEnd ) {
            start = e.attribute("enddt");
            rec->setDaily(freq,  fromUTC( (time_t) start.toLong() ).date() );
        }else{
            rec->setDaily( freq,  -1 );
        }
        // weekly
    }else if ( type == "Weekly") {
        int days = e.attribute("rweekdays").toInt();
        QBitArray bits( 7 );
        bits.fill( false );
        if ( Monday & days )
            bits.setBit( 0 );
        if ( Tuesday & days )
            bits.setBit( 1 );
        if ( Wednesday & days )
            bits.setBit( 2 );
        if ( Thursday & days )
            bits.setBit( 3 );
        if ( Friday & days )
            bits.setBit( 4 );
        if ( Saturday & days )
            bits.setBit( 5 );
        if ( Sunday & days )
            bits.setBit( 6 );

        if ( hasEnd ) {
            start = e.attribute("enddt");
            rec->setWeekly( freq,  bits, fromUTC( (time_t) start.toLong() ).date() );
        }else{
            rec->setWeekly( freq,  bits,  -1 );
        }
    // monthly
    }else if ( type == "MonthlyDay" ) {
        // monthly day the  1st Saturday of the month
        int rposition = e.attribute("rposition").toInt();
        if ( hasEnd ) {
            start = e.attribute("enddt");
            rec->setMonthly( KCal::Recurrence::rMonthlyPos,
                             freq,fromUTC( (time_t) start.toLong() ).date() );
        }else{
            rec->setMonthly( KCal::Recurrence::rMonthlyPos,
                             freq,  -1 );
        }
        QBitArray array( 7);
        array.fill( false );
        QDate date = event->dtStart().date();
        array.setBit( date.dayOfWeek() - 1 );
        rec->addMonthlyPos( rposition, array );

    }else if ( type == "MonthlyDate" ) {
        // int rposition = e.attribute("rposition").toInt();
        if ( hasEnd ) {
            start = e.attribute("enddt");
            rec->setMonthly( KCal::Recurrence::rMonthlyDay,
                             freq,fromUTC( (time_t) start.toLong() ).date() );
        }else{
            rec->setMonthly( KCal::Recurrence::rMonthlyDay,
                             freq,  -1 );
        }
        QDate date = event->dtStart().date();
        rec->addMonthlyDay( date.day() );

    }else if ( type == "Yearly" ) {
        if (hasEnd ) {
            start = e.attribute("enddt");
            rec->setYearly( KCal::Recurrence::rYearlyDay,
                            freq,
                            fromUTC( (time_t) start.toLong() ).date() );
        }else{
            rec->setYearly( KCal::Recurrence::rYearlyDay,
                            freq, -1 );
        }
        rec->addYearlyNum( event->dtStart().date().dayOfYear() );
    }

    // now save the attributes for later use
    extraMap.add("datebook", e.attribute("uid"), e.attributes(), lst );

    return event;
}

bool DateBook::toKDE( const QString& fileName, ExtraMap& extraMap, KSync::CalendarSyncee *syncee )
{
    QFile file( fileName );
    if ( !file.open( IO_ReadOnly ) ) {
        return false;
    }
    QDomDocument doc("mydocument");
    if ( !doc.setContent( &file ) ) {
        return false;
    }

    QDomElement docElem = doc.documentElement();
    QDomNode n = docElem.firstChild();
    QString dummy;
    QStringList attr = supportedAttributes();
    while (!n.isNull() ) {
        QDomElement el = n.toElement();
        if (!el.isNull() ) {

            if ( el.tagName() == "events") {

                QDomNode no = el.firstChild();
                while (!no.isNull() ) {
                    QDomElement e = no.toElement();

                    if (!e.isNull() ) {
                        if (e.tagName() == "event") {
                            KCal::Event* event = toEvent( e, extraMap, attr );
                            if (event != 0 ) {
                                KSync::CalendarSyncEntry* entry;
                                entry = new KSync::CalendarSyncEntry( event, syncee );
                                syncee->addEntry( entry );
                            }
                        }
                    }
                    no = no.nextSibling();
                }
            }
            n = n.nextSibling();
        }// n.isNULL
    }

    return true;
}

KTempFile* DateBook::fromKDE( KSync::CalendarSyncee* syncee, ExtraMap& extraMap )
{
    m_kde2opie.clear();
    Kontainer::ValueList newIds = syncee->ids( "EventSyncEntry");
    Kontainer::ValueList::ConstIterator idIt;
    for ( idIt = newIds.begin(); idIt != newIds.end(); ++idIt ) {
        m_helper->addId("EventSyncEntry",  (*idIt).first,  (*idIt).second );
    }
    KTempFile* tempFile = file();
    if ( tempFile->textStream() ) {
        QTextStream *stream = tempFile->textStream();
        stream->setEncoding( QTextStream::UnicodeUTF8 );
        *stream <<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
        *stream << "<!DOCTYPE DATEBOOK><DATEBOOK>" << endl;
        KSync::CalendarSyncEntry *entry;
        KCal::Event *event;
        *stream << "<events>" << endl;
        for ( entry = (KSync::CalendarSyncEntry*) syncee->firstEntry();
              entry != 0;
              entry = (KSync::CalendarSyncEntry*) syncee->nextEntry() )
        {
            /*  do not safe deleted records */
            if ( entry->wasRemoved() )
              continue;
            event = dynamic_cast<KCal::Event*>( entry->incidence() );
            if ( !event )
              continue;

            *stream << event2string( event, extraMap ) << endl;
        }
        *stream << "</events>" << endl;
        *stream << "</DATEBOOK>" << endl;

    }
    if (m_helper )
        m_helper->replaceIds( "EventSyncEntry",  m_kde2opie );

    tempFile->close();
    return tempFile;
}
QString DateBook::event2string( KCal::Event *event, ExtraMap& map )
{
    QString uid = konnectorId("EventSyncEntry", event->uid() );
    bool doesFloat = event->doesFloat();
    QString str;
    str.append( "<event ");
    str.append( "description=\"" +escape( event->summary() ) + "\" ");

    str.append( appendText( "location=\"" + escape( event->location() ) + "\" ",
                            "location=\"\" ") );
    str.append( appendText( "categories=\"" +
                            categoriesToNumber( event->categories() ) + "\" ",
                            "categories=\"\" ") );

    str.append( "uid=\"" + uid  + "\" ");
    str.append( "start=\"" +startDate( event->dtStart(), doesFloat ) + "\" ");
    str.append( "end=\"" +  endDate( event->dtEnd(), doesFloat) + "\" ");

    str.append( appendText("note=\"" + escape( event->description() ) + "\" ",
                           "note=\"\" ")); //use escapeString copied from TT
    if ( doesFloat )
        str.append( "type=\"AllDay\" ");
    // recurrence
    KCal::Recurrence *rec = event->recurrence();
    if ( rec->doesRecur() ) {
        QString type;
        switch( rec->doesRecur() ) {
        case KCal::Recurrence::rDaily :{
            type = "Daily";
            break;
        }
        case KCal::Recurrence::rWeekly :{
            type = "Weekly";
            signed char day = 0;
            QBitArray array = rec->days();
            if ( array.testBit(0 ) ) day |= Monday;
            if ( array.testBit(1 ) ) day |= Tuesday;
            if ( array.testBit(2 ) ) day |= Wednesday;
            if ( array.testBit(3 ) ) day |= Thursday;
            if ( array.testBit(4 ) ) day |= Friday;
            if ( array.testBit(5 ) ) day |= Saturday;
            if ( array.testBit(6 ) ) day |= Sunday;

            /* need to be > 0  we set the repeat day to the day where the event takes place*/
            if ( day < 0 ) {
                switch( event->dtStart().date().dayOfWeek() ) {
                case 1: // Monday
                    day = Monday;
                    break;
                case 2: // Tuesday
                    day = Tuesday;
                    break;
                case 3: // Wednesday
                    day = Wednesday;
                    break;
                case 4: //  Thursday
                    day = Thursday;
                    break;
                case 5: // Friday
                    day = Friday;
                    break;
                case 6: // Staurday
                    day = Saturday;
                    break;
                default:// should never happen
                case 7: // Sunday
                    day = Sunday;
                    break;
                }

            }
            str.append( "rweekdays=\"" + QString::number(static_cast<int> (day) ) + "\" ");
            break;
        }
        case KCal::Recurrence::rMonthlyPos :{
            int rpos = week( event->dtStart().date() );
            if ( rpos != 0 )
                str.append( "rposition=\"" + QString::number(rpos)  + "\" ");
            type = "MonthlyDay";
            break;
        }
        case KCal::Recurrence::rMonthlyDay :{
            type = "MonthlyDate";

            break;
        }
        case KCal::Recurrence::rYearlyMonth: // fall through
        case KCal::Recurrence::rYearlyPos: // fall through Might be wrong though
        case KCal::Recurrence::rYearlyDay :{
            type = "Yearly";
            break;
        }
        case KCal::Recurrence::rNone : // fall through
        default :
            type = QString::null;
            break;
        }
        if (!type.isEmpty() ) {
            str.append( "rtype=\"" + type + "\" ");
            str.append( "rfreq=\"" + QString::number( rec->frequency() ) + "\" ");
            if ( rec->duration() == -1 || rec->duration() != 0 )
                str.append( "rhasenddate=\"0\" ");
            else if ( rec->duration() == 0 ) {
                str.append( "rhasenddate=\"1\" ");
                str.append( "enddt=\"" + QString::number( toUTC(rec->endDate() ) ) + "\" ");
            }
            str.append( "created=\"" + QString::number( toUTC(rec->recurStart() ) ) + "\" ");
        }
    }
    // FIXME alarm
    str += map.toString( "datebook", uid );
    str.append( " />" );
    return str;
}
/*
 * A list of attributes we handle
 */
QStringList DateBook::supportedAttributes(){
    QStringList lst;
    lst << "description";
    lst << "location";
    lst << "categories";
    lst << "uid";
    lst << "start";
    lst << "end";
    lst << "note";
    lst << "type";
    lst << "rweekdays";
    lst << "rposition";
    lst << "rtype";
    lst << "rfreq";
    lst << "rhasenddate";
    lst << "enddt";
    lst << "created";
    /*
     * we need to handle Recurrence Exceptions
     * alarms, timezones later
     */
    return lst;
}
/*
 * Qtopia etwartet AllDay events in einer Zeitspanne von 00:00:00
 * bis 23:59:59... but in korg bdays are from 00:00:00 - 00:00:00 (
 * no time associated )
 * He'll help Qtopia here if it's an all day event we will produce
 * a better time...
 */
QString DateBook::startDate( const QDateTime& _dt,  bool allDay ) {
    QDateTime dt = _dt;
    if (allDay )
        dt.setTime( QTime(0, 0, 0 ) );

    return QString::number( toUTC( dt ) );
}
QString DateBook::endDate( const QDateTime& _dt,  bool allDay ) {
    QDateTime dt = _dt;
    if (allDay )
        dt.setTime( QTime(23, 59, 59 ) );

    return QString::number( toUTC(dt ) );
}

