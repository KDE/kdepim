
#include <kdebug.h>

#include "metatodo.h"

using namespace OpieHelper;

namespace {
    // returns true if modified
    bool testOle(KCal::Todo*,  KCal::Todo*);

};


MetaTodo::MetaTodo()
{

}
MetaTodo::~MetaTodo()
{

}
bool MetaTodo::test( KSync::TodoSyncEntry* newE, KSync::TodoSyncEntry* old ) {
    return testOle( newE->todo(),  old->todo() );
}

namespace{
    // FIXME try bool first, then int, then QString, QStringList
    // if modified return true which should increase speed as well
    bool testOle( KCal::Todo* fi,  KCal::Todo* se)
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
