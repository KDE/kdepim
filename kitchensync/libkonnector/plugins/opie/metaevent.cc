
#include <kdebug.h>

#include <eventsyncee.h>

#include "metaevent.h"

using namespace OpieHelper;


namespace {

    bool testOle( KCal::Event*,  KCal::Event* );

};


MetaEvent::MetaEvent()
{

}
MetaEvent::~MetaEvent()
{

}
bool MetaEvent::test ( KSync::EventSyncEntry* newE,  KSync::EventSyncEntry* old ) {
    return testOle( newE->incidence(),  old->incidence() );
}

namespace {
    bool testOle( KCal::Event* fi,  KCal::Event* se)
    {
        bool ret = false;
        kdDebug() << "summary  " << fi->summary() << endl;
        if ( (!fi->summary().isEmpty()  && !se->summary().isEmpty() ) && fi->summary() != se->summary() ) {
            kdDebug() << "Summary mismatch" << endl;
            kdDebug() << "New " << fi->summary() << endl;
            kdDebug() << "Old " << se->summary() << endl;
            return true;
        }
        if ( (!fi->location().isEmpty() && !se->location().isEmpty() ) && fi->location() != se->location() ) {
            kdDebug() << "Location mismatch" << endl;
            kdDebug() << "New " << fi->location() << "empty " << fi->location().isEmpty() << endl;
            kdDebug() << "Old " << se->location() << "empty " << se->location().isEmpty() << endl;
            return true;
        }
        if (fi->categories() != se->categories() ) {
            kdDebug() << "Categories mismatch" << endl;
            kdDebug() << "New " << fi->categories().join(";") << endl;
            kdDebug() << "Old " << se->categories().join(";") << endl;
            return true;
        }
        if ( fi->dtStart() != se->dtStart() ) {
            kdDebug() << "Date Start " << endl;
            kdDebug() << "New " << fi->dtStart().toString() << endl;
            kdDebug() << "Old " << se->dtStart().toString() << endl;
            return true;
        }
        if ( fi->dtEnd() != se->dtEnd() ) {
            kdDebug() << "Date End " << endl;
            kdDebug() << "New " << fi->dtEnd().toString() << endl;
            kdDebug() << "Old " << se->dtEnd().toString() << endl;
            return true;
        }
        if ( (!fi->description().isEmpty() && !se->description().isEmpty()  ) && fi->description() != se->description() ) {
            kdDebug() << "Description mismatch " << endl;
            kdDebug() << "New " << fi->description() << endl;
            kdDebug() << "Old " << se->description() << endl;
            return true;
        }
        if ( fi->doesFloat() != se->doesFloat() ) {
            kdDebug() << "Float " << endl;
            kdDebug() << "New " << fi->doesFloat() << endl;
            kdDebug() << "Old " << se->doesFloat() << endl;
            return true;
        }
        KCal::Recurrence *fiRec = fi->recurrence();
        KCal::Recurrence *seRec = se->recurrence();
        if ( fiRec->doesRecur() == seRec->doesRecur() ) {
            if ( fiRec->doesRecur() ) {
                kdDebug() << "DoesRecur ==" << endl;
                kdDebug() << "New " << fiRec->doesRecur() << endl;;
                int fiRecur = fiRec->doesRecur();
                int seRecur = seRec->doesRecur();
                if ( fiRecur != seRecur )
                    return true;
                //else
                if ( fiRecur == KCal::Recurrence::rWeekly ) {
                    if (fiRec->days() != seRec->days() ) {
                        kdDebug() << "Days mismatched " << endl;
                        return true;
                    }
                }else if ( fiRecur == KCal::Recurrence::rMonthlyPos ) {
                    if (fi->dtStart() != se->dtStart() ) {
                        kdDebug() << "Date mismatched" << endl;
                        kdDebug() << "New " << fi->dtStart().toString() << endl;
                        kdDebug() << "Old " << se->dtStart().toString() << endl;
                        return true;
                    }
                }
                if ( fiRec->duration() != seRec->duration() ) { // FIXME duration
                    kdDebug() << "Duration mismatch" << endl;
                    kdDebug() << "New " << fiRec->duration() << endl;
                    kdDebug() << "Old " << seRec->duration() << endl;
                    return true;
                }
                if ( fiRec->duration() != -1 || fiRec->duration() == 0 ) {
                    if ( fiRec->endDate() != seRec->endDate() ) {
                        kdDebug() << "End Date mismatched" << endl;
                        kdDebug() << "New " << fiRec->endDate().toString() << endl;
                        kdDebug() << "Old " << seRec->endDate().toString() << endl;
                        return true;
                    }
                }
                if ( fiRec->frequency() != seRec->frequency() ) {
                    kdDebug() << "Frequency " << endl;
                    kdDebug() << "New " << fiRec->frequency() << endl;
                    kdDebug() << "Old " << seRec->frequency() << endl;
                    return true;
                }
                if ( fiRec->recurStart() != seRec->recurStart() ) {
                    kdDebug() << "Recur start " << endl;
                    kdDebug() << "New " << fiRec->recurStart().toString() << endl;
                    kdDebug() << "Old " << seRec->recurStart().toString() << endl;
                    return true;
                }
            }else{
                kdDebug() << "Does not recur" << endl;
            }
        }else{
            kdDebug() << "Else "<< endl;
            return true;
        }
        kdDebug() << "Return  " << ret << endl;
        return ret;
    };

};
