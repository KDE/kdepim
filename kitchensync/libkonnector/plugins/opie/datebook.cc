
#include <qbitarray.h>
#include <qdom.h>
#include <qfile.h>

#include <kalendarsyncentry.h>

#include "datebook.h"

using namespace OpieHelper;

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
    QPtrList<KCal::Event> m_list;

    QFile file( fileName );
    if ( file.open( IO_ReadOnly ) ) {
        QDomDocument doc("mydocument");
        if ( doc.setContent( &file ) ) {
            KCal::Event *event;
            QDomElement docElem = doc.documentElement();
            QDomNode n = docElem.firstChild();
            QString dummy;
            int Int;
            bool ok;
            while (!n.isNull() ) {
                QDomElement e = n.toElement();
                if (!e.isNull() ) {
                    if (e.tagName() == "event") {
                        event = new KCal::Event();
                        QStringList list = QStringList::split(";",  e.attribute("Categories") );
                        QStringList categories;
                        for ( uint i = 0; i < list.count(); i++ ) {
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
                        event->setDtStart( fromUTC( (time_t) start.toLong() ) );
                        QString end = e.attribute("end");
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
                            else if ( Tuesday & days )
                                bits.setBit( 1 );
                            else if ( Wednesday & days )
                                bits.setBit( 2 );
                            else if ( Thursday & days )
                                bits.setBit( 3 );
                            else if ( Friday & days )
                                bits.setBit( 4 );
                            else if ( Saturday & days )
                                bits.setBit( 5 );
                            else if ( Sunday & days )
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
                    n = n.nextSibling();
                } // n.isNULL
            }
        }
    }
    return m_list;
}
QByteArray DateBook::fromKDE( KAlendarSyncEntry* entry )
{

}
