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
        case KCal::Recurrence::rYearlyDay:
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
