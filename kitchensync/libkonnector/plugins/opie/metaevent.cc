
#include <kdebug.h>

#include "metaevent.h"

using namespace OpieHelper;


namespace {

    bool test( KCal::Event*,  KCal::Event* );
};

MetaEventReturn::MetaEventReturn()
{

}
MetaEventReturn::MetaEventReturn( const MetaEventReturn& ole )
{
    (*this) = ole;
}
MetaEventReturn::~MetaEventReturn()
{

}
QPtrList<KCal::Event> MetaEventReturn::added()
{
    return m_add;
}
QPtrList<KCal::Event> MetaEventReturn::modified()
{
    return m_mod;
}
QPtrList<KCal::Event> MetaEventReturn::removed()
{
    return m_del;
}
MetaEventReturn &MetaEventReturn::operator=( const MetaEventReturn &ole )
{
    m_add = ole.m_add;
    m_del = ole.m_del;
    m_mod = ole.m_mod;
    return *this;
}

MetaEvent::MetaEvent()
{

}
MetaEvent::~MetaEvent()
{

}
// same as in MetaTodo
// search new, modifief, removed
MetaEventReturn MetaEvent::doMeta( QPtrList<KCal::Event> &newE,
                                   QPtrList<KCal::Event> &old )
{
    QPtrList<KCal::Event> add;
    QPtrList<KCal::Event> rem;
    QPtrList<KCal::Event> mod;
    KCal::Event *New;
    KCal::Event *ole;
    bool found = false;
    for ( New = newE.first(); New != 0; New = newE.next() ) {
        found = false;
        for ( ole = old.first(); ole != 0; ole = old.next() ) {
            if ( New->uid() == ole->uid() ) {
                found = true;
                if ( test( New,  ole ) )
                    mod.append( (KCal::Event*) (New->clone() ) );
                break;
            }
        }
        if (!found ) {
            KCal::Event *event = static_cast<KCal::Event*> (New->clone() );
            add.append( event );
        }
    }
    for ( ole = old.first(); ole != 0; ole = old.next() ) {
        found = false;
        for ( New = newE.first(); New != 0; New = newE.next() ) {
            if ( New->uid() == ole->uid() ) {
                found = true;
                break;
            }
        }
        if (!found )
            rem.append( (KCal::Event*)(ole->clone() ) );
    }
    MetaEventReturn ret;
    ret.m_add = add;
    ret.m_del = rem;
    ret.m_mod = mod;
    return ret;
}

namespace {
    bool test( KCal::Event* fi,  KCal::Event* se)
    {
        bool ret = false;
        kdDebug() << "summary  " << fi->summary() << endl;
        if (fi->summary() != se->summary() ) {
            kdDebug() << "Summary mismatch" << endl;
            kdDebug() << "New " << fi->summary() << endl;
            kdDebug() << "Old " << se->summary() << endl;
            return true;
        }
        if (fi->location() != se->location() ) {
            kdDebug() << "Location mismatch" << endl;
            kdDebug() << "New " << fi->location() << endl;
            kdDebug() << "Old " << se->location() << endl;
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
        if ( fi->description() != se->description() ) {
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
