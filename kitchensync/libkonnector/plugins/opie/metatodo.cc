
#include <kdebug.h>

#include "metatodo.h"

using namespace OpieHelper;

namespace {
    // returns true if modified
    bool test(KCal::Todo*,  KCal::Todo*);

};

MetaTodoReturn::MetaTodoReturn()
{

}
MetaTodoReturn::~MetaTodoReturn()
{

}
MetaTodoReturn::MetaTodoReturn( const MetaTodoReturn& old )
{
    (*this ) = old;
}
QPtrList<KCal::Todo> MetaTodoReturn::added()
{
    return m_add;
}
QPtrList<KCal::Todo> MetaTodoReturn::removed()
{
    return m_del;
}
QPtrList<KCal::Todo> MetaTodoReturn::modified()
{
    return m_mod;
}

MetaTodoReturn &MetaTodoReturn::operator=( const MetaTodoReturn& ole )
{
    m_add = ole.m_add;
    m_del = ole.m_del;
    m_mod = ole.m_mod;

    return *this;
}

MetaTodo::MetaTodo()
{

}
MetaTodo::~MetaTodo()
{

}
// let's make some meta data
// if in new and not in old  -> Added
// if not in new but in old  -> removed
// if in new and in old and different -> modified
MetaTodoReturn MetaTodo::doMeta( QPtrList<KCal::Todo> &news,
                                 QPtrList<KCal::Todo> &old )
{
    QPtrList<KCal::Todo> add;
    QPtrList<KCal::Todo> rem;
    QPtrList<KCal::Todo> mod;
    KCal::Todo *New;
    KCal::Todo *ole;
    bool found;
    for ( New = news.first(); New != 0; New = news.next() ) {
        found= false;
        for ( ole = old.first(); ole != 0; ole = old.next() ) {
            if ( New->uid() == ole->uid() ) {
                found = true;
                if ( test( New,  ole ) ) {
                    mod.append( (KCal::Todo*) (New->clone() ) );
                    kdDebug() << "Mod append" << endl;
                }
                break;
            }
        }
        if ( !found ) { // new entry
            KCal::Todo *todo = static_cast<KCal::Todo*> (New->clone() );
            add.append( todo );
        }
    }
    // find old entries
    for ( ole = old.first(); ole != 0; ole = old.next() ) {
        found = false;
        for ( New = news.first(); New != 0; New = news.next() ) {
            if (New->uid() == ole->uid() ) {
                found = true;
                break;
            }
        }
        if ( !found ) {
            KCal::Todo *todo = static_cast<KCal::Todo*> (ole->clone() );
            rem.append( todo );
        }
    }
    MetaTodoReturn ret;
    ret.m_add = add;
    ret.m_del = rem;
    ret.m_mod = mod;
    return ret;
}

namespace{
    // FIXME try bool first, then int, then QString, QStringList
    // if modified return true which should increase speed as well
    bool test( KCal::Todo* fi,  KCal::Todo* se)
    {
        // kdDebug() << "Test " << endl;
        bool mod = false;
        if (fi->categories() != se->categories() ) {
            //  kdDebug() << "Categoryies not match" << endl;
            //kdDebug() << "New " << fi->categories().join(";") << endl;
            //kdDebug() << "Old " << se->categories().join(";") << endl;
            return true;
        }
        if (fi->isCompleted() != se->isCompleted() ) {
            //kdDebug() << "Completed mismatch " << endl;
            //kdDebug() << "New " << fi->isCompleted() << endl;
            //kdDebug() << "Old " << se->isCompleted() << endl;
            return true;
        }
        if (fi->hasDueDate() && se->hasDueDate() ) {
            //kdDebug() << "DueDate match" << endl;
                if (fi->dtDue().date() != se->dtDue().date() ) {
                    //kdDebug() << "Due Date mis match" << endl;
                    //kdDebug() << "New " << fi->dtDue().toString() << endl;
                    //kdDebug() << "Old " << se->dtDue().toString() << endl;
                    return true;
                }
        }else{
            if ( fi->hasDueDate() != se->hasDueDate() )
                return true;
        }
        if ( fi->priority() != se->priority() ) {
            //kdDebug() << "Priority mis match" << endl;
            //kdDebug() << "New " << fi->priority() << endl;
            //kdDebug() << "Old " << se->priority() << endl;
            return true;
        }
        if ( fi->description() != se->description() ) {
            //kdDebug(5202) << "Description " << endl;
            //kdDebug(5202) << "New " << fi->description() << endl;
            //kdDebug(5202) << "Old " << se->description() << endl;
            return true;
        }
        //kdDebug() << " False" << endl;
        return false;
    }
}
