
#include <qbitarray.h>
#include <qbuffer.h>
#include <qdom.h>
#include <qfile.h>
#include <qtextstream.h>

#include <kdebug.h>

#include <eventsyncee.h>

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

};

DateBook::DateBook( CategoryEdit* edit,
                    KSync::KonnectorUIDHelper* helper,
                    const QString& tz,
                    bool meta, Device *dev )
    : Base( edit,  helper,  tz, meta, dev )
{
}
DateBook::~DateBook(){
}
/**
 * Converts an Opie Event to a KDE one
 */
KCal::Event* DateBook::toEvent( QDomElement e) {
//    kdDebug(522) << "inside event" << endl;
    KCal::Event* event = new KCal::Event();
    QStringList list = QStringList::split(";",  e.attribute("categories") );
    QStringList categories;
    for ( uint i = 0; i < list.count(); i++ ) {
//        kdDebug(5229) << list[i]<< " categories " << m_edit->categoryById( list[i],  "Calendar") << endl;
        categories.append(m_edit->categoryById(list[i], "Calendar") );
    }
    if (!categories.isEmpty() ) {
        event->setCategories( categories );
    }
    //event->setDescription(e.attribute("Description") );
    event->setSummary( e.attribute("description") );
    event->setUid( kdeId( "EventSyncEntry",  e.attribute("uid") ) );
    event->setDescription( e.attribute("note") );
    event->setLocation( e.attribute("location") );
    // time

    QString start = e.attribute("start");
//    kdDebug(5229) << "Start " << fromUTC( (time_t) start.toLong() ).toString() << endl;
    event->setDtStart( fromUTC( (time_t) start.toLong() ) );

    QString end = e.attribute("end");
//    kdDebug(5229) << "End " << fromUTC( (time_t) end.toLong() ).toString() << endl;
    event->setDtEnd( fromUTC( (time_t) end.toLong() ) );

    // type
    if ( e.attribute("type") == "AllDay" ) {
        event->setFloats( true );
    }else{
        event->setFloats( false );
    }

    // alarm
    QString alarm = e.attribute("alarm");

    /* there is an alarm */
    if (!alarm.isEmpty() ) {
        KCal::Alarm *al = new KCal::Alarm( event );
        al->setStartOffset( alarm.toInt() * -60 );
        al->setAudioAlarm( e.attribute("sound") );
        event->addAlarm( al );

        kdDebug(5229) << "Alarm offset " << al->startOffset().asSeconds() << endl;
        kdDebug(5229) << "Alarm " << e.attribute("alarm") << " " << e.attribute("sound") << endl;
    }


    // Recurrence damn I feared to do that
    QString type = e.attribute("rtype");
    int freq = e.attribute("rfreq").toInt();
    bool hasEnd = e.attribute("rhasenddate").toInt();
    //kdDebug(5229) << "HasEndDate: " << hasEnd << endl;

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
//            kdDebug(5229) << "has end " << start << endl;
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
    return event;
}

KSync::EventSyncee* DateBook::toKDE( const QString& fileName )
{
//    kdDebug(5229) << "To KDE " << endl;
    KSync::EventSyncee* syncee = new KSync::EventSyncee();
    if( device() )
	syncee->setSupports( device()->supports( Device::Calendar ) );

    QFile file( fileName );
    if ( file.open( IO_ReadOnly ) ) {
        //kdDebug(5229) << "file open" << endl;
        QDomDocument doc("mydocument");
        if ( doc.setContent( &file ) ) {
//            kdDebug(5229) << "setContent" << endl;
            QDomElement docElem = doc.documentElement();
//            kdDebug(5229) << "TagName docElem " << docElem.tagName() << endl;
            QDomNode n = docElem.firstChild();
            QString dummy;
            while (!n.isNull() ) {
                QDomElement el = n.toElement();
                if (!el.isNull() ) {
//                    kdDebug(5229) << "e " << el.tagName() << endl;
//                    kdDebug(5229) << "e.isNull not" << endl;

                    if ( el.tagName() == "events") {

                        QDomNode no = el.firstChild();
                        while (!no.isNull() ) {
                            QDomElement e = no.toElement();

                            if (!e.isNull() ) {
                                if (e.tagName() == "event") {
                                    KCal::Event* event = toEvent( e );
                                    if (event != 0 ) {
                                        KSync::EventSyncEntry* entry;
                                        entry = new KSync::EventSyncEntry( event );
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
        }
    }
    return syncee;
}

KTempFile* DateBook::fromKDE( KSync::EventSyncee* syncee )
{
    m_kde2opie.clear();
    Kontainer::ValueList newIds = syncee->ids( "EventSyncEntry");
    Kontainer::ValueList::ConstIterator idIt;
    for ( idIt = newIds.begin(); idIt != newIds.end(); ++idIt ) {
        m_helper->addId("EventSyncEntry",  (*idIt).first(),  (*idIt).second() );
    }
    KTempFile* tempFile = file();
    if ( tempFile->textStream() ) {
        QTextStream *stream = tempFile->textStream();
        stream->setEncoding( QTextStream::UnicodeUTF8 );
        *stream <<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
        *stream << "<!DOCTYPE DATEBOOK><DATEBOOK>" << endl;
        KSync::EventSyncEntry *entry;
        KCal::Event *event;
        *stream << "<events>" << endl;
        for ( entry = (KSync::EventSyncEntry*) syncee->firstEntry();
              entry != 0;
              entry = (KSync::EventSyncEntry*) syncee->nextEntry() )
        {
            if (entry->state() == KSync::SyncEntry::Removed )
                continue;
            event = entry->incidence();
            *stream << event2string( event ) << endl;
        }
        *stream << "</events>" << endl;
        *stream << "</DATEBOOK>" << endl;

    }
    if (m_helper )
        m_helper->replaceIds( "EventSyncEntry",  m_kde2opie );

    tempFile->close();
    return tempFile;
}
QString DateBook::event2string( KCal::Event *event )
{
    bool doesFloat = event->doesFloat();
    QString str;
    str.append( "<event ");
    str.append( "description=\"" +escape( event->summary() ) + "\" ");
    str.append( "location=\"" + escape( event->location() ) + "\" ");
    str.append( "categories=\"" +  categoriesToNumber( event->categories() ) + "\" ");
    str.append( "uid=\"" +  konnectorId("EventSyncEntry", event->uid() ) + "\" ");
    str.append( "start=\"" +startDate( event->dtStart(), doesFloat ) + "\" ");
    str.append( "end=\"" +  endDate( event->dtEnd(), doesFloat) + "\" ");
    str.append( "note=\"" + escape( event->description() ) + "\" "); //use escapeString copied from TT
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
            char day = 0; // signed
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
        case KCal::Recurrence::rYearlyDay :{
            type = "Yearly";
            break;
        }
        case KCal::Recurrence::rNone : // fall through
        default :
            break;
        }
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
    // alarm
    KCal::Alarm *al = event->alarms().first();
    if ( al != 0 ) {
        int sec = al->startOffset().asSeconds();
        sec = sec > 0 ? sec % 60 : sec % -60; // as minutes now
	if( sec != 0 ){
            str.append( "alarm=\"" +QString::number( sec )  + "\" ");
	    QString sound = al->audioFile();
    	    if ( sound != "loud" && sound != "silent" ) {
        	if ( sound.isEmpty() )
                	sound = QString::fromLatin1("silent");
	        else
    	            sound = QString::fromLatin1("loud");
    	    }
    	    str.append( "sound=\"" +  sound + "\" ");
	}
    }
    str.append( " />" );
    return str;
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

