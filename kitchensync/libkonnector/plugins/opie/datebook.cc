
#include <qbitarray.h>
#include <qbuffer.h>
#include <qdom.h>
#include <qfile.h>
#include <qtextstream.h>

#include <kdebug.h>

#include <kalendarsyncentry.h>


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
                    KonnectorUIDHelper* helper,
                    bool meta )
    : Base( edit,  helper,  meta )
{


}
DateBook::~DateBook()
{

}
QPtrList<KCal::Event> DateBook::toKDE( const QString& fileName )
{
    kdDebug() << "To KDE " << endl;
    QPtrList<KCal::Event> m_list;

    QFile file( fileName );
    if ( file.open( IO_ReadOnly ) ) {
        kdDebug() << "file open" << endl;
        QDomDocument doc("mydocument");
        if ( doc.setContent( &file ) ) {
            kdDebug() << "setContent" << endl;
            KCal::Event *event;
            QDomElement docElem = doc.documentElement();
            kdDebug() << "TagName docElem " << docElem.tagName() << endl;
            QDomNode n = docElem.firstChild();
            QString dummy;
            int Int;
            bool ok;
            while (!n.isNull() ) {
                QDomElement el = n.toElement();
                if (!el.isNull() ) {
                    kdDebug() << "e " << el.tagName() << endl;
                    kdDebug() << "e.isNull not" << endl;
                    if ( el.tagName() == "events") {
                        QDomNode no = el.firstChild();
                        while (!no.isNull() ) {
                            QDomElement e = no.toElement();
                            if (!e.isNull() ) {
                                if (e.tagName() == "event") {
                                    kdDebug() << "inside event" << endl;
                                    event = new KCal::Event();
                                    QStringList list = QStringList::split(";",  e.attribute("Categories") );
                                    QStringList categories;
                                    for ( uint i = 0; i < list.count(); i++ ) {
                                        kdDebug() << list[i]<< " categories " << m_edit->categoryById( list[i],  "Calendar") << endl;
                                        categories.append(m_edit->categoryById(list[i], "Calendar") );
                                    }
                                    if (!categories.isEmpty() ) {
                                        event->setCategories( categories );
                                    }
                                    //event->setDescription(e.attribute("Description") );
                                    event->setSummary( e.attribute("description") );
                                    event->setUid( kdeId( "event",  e.attribute("uid") ) );
                                    event->setDescription( e.attribute("note") );
                                    event->setLocation( e.attribute("location") );
                                    // time
                                    bool ok;
                                    QString start = e.attribute("start");
                                    kdDebug() << "Start " << fromUTC( (time_t) start.toLong() ).toString() << endl;
                                    event->setDtStart( fromUTC( (time_t) start.toLong() ) );
                                    QString end = e.attribute("end");
                                    kdDebug() << "End " << fromUTC( (time_t) end.toLong() ).toString() << endl;
                                    event->setDtEnd( fromUTC( (time_t) end.toLong() ) );
                                    if ( e.attribute("type") == "AllDay" ) {
                                        event->setFloats( true );
                                    }else{
                                        event->setFloats( false );
                                    }
                                    KCal::Alarm *al = new KCal::Alarm( event );
                                    al->setText( event->description() );
                                    al->setOffset( e.attribute("alarm").toInt() * -60 );
                                    al->setAudioFile( e.attribute("sound") );
                                    event->addAlarm( al );

                                    // Recurrence damn I feared to do that
                                    QString type = e.attribute("rtype");
                                    int freq = e.attribute("rfreq").toInt();
                                    bool hasEnd = e.attribute("rhasenddate");

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
                                        int rposition = e.attribute("rposition").toInt();
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
                                    m_list.append( event );
                                }
                            }
                            no = no.nextSibling();
                        }
                    }
                    n = n.nextSibling();
                } // n.isNULL
            }
        }
    }
    return m_list;
}
QByteArray DateBook::fromKDE( KAlendarSyncEntry* entry )
{
    m_kde2opie.clear();
    QValueList<Kontainer> newIds = entry->ids( "event");
    for ( QValueList<Kontainer>::ConstIterator idIt = newIds.begin(); idIt != newIds.end(); ++idIt ) {
        m_helper->addId("event",  (*idIt).first(),  (*idIt).second() );
    }
    QByteArray array;
    QBuffer buffer( array );
    if ( buffer.open(IO_WriteOnly ) ) {
        QTextStream stream( &buffer );
        stream.setEncoding( QTextStream::UnicodeUTF8 );
        stream <<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
        stream << "<!DOCTYPE DATEBOOK><DATEBOOK>" << endl;
        QPtrList<KCal::Event> list = entry->calendar()->getAllEvents();
        KCal::Event *event;
        for ( event = list.first(); event != 0; event = list.next() ) {
            stream << event2string( event ) << endl;
        }
        stream << "<events>" << endl;
        stream << "</events>" << endl;
        stream << "</DATEBOOK>" << endl;

    }
    m_helper->replaceIds( "event",  m_kde2opie );
    return array;
}
QString DateBook::event2string( KCal::Event *event )
{
    QString str;
    str.append( "<event ");
    str.append( "description=\"" +event->summary()  + "\" ");
    str.append( "location=\"" + event->location() + "\" ");
    str.append( "categories=\"" +  categoriesToNumber( event->categories() ) + "\" ");
    str.append( "uid=\"" +  konnectorId("event",  event->uid() ) + "\" ");
    str.append( "start=\"" + QString::number( toUTC( event->dtStart() ) ) + "\" ");
    str.append( "end=\"" +  QString::number(  toUTC( event->dtEnd() ) ) + "\" ");
    str.append( "note=\"" + event->description() + "\" "); //use escapeString copied from TT
    if ( event->doesFloat() )
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
            str.append( "rweekdays=\"" + QString::number(static_cast<int> (day) ) + "\" ");
            break;
        }
        case KCal::Recurrence::rMonthlyPos :{
            str.append( "rposition=\"" + QString::number( week( event->dtStart().date() ) )  + "\" ");
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
        int sec = al->offset().asSeconds();
        sec = sec > 0 ? sec % 60 : sec % -60; // as minutes now
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
    str.append( " />" );
    return str;
}
