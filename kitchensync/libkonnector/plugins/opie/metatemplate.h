
#ifndef OPIE_HELPER_META_TEMPLATE_H
#define OPIE_HELPER_META_TEMPLATE_H

#include <syncer.h>

namespace OpieHelper {

    template <class Syncee = KSync::Syncee,  class Entry = KSync::SyncEntry>
    class MetaTemplate {
    public:
        MetaTemplate() {};
        virtual ~MetaTemplate() {};
        Syncee* doMeta( Syncee* newE,
                        Syncee* old ) {
            bool found;
            Entry* entryNew;
            Entry* entryOld;
            /**
             * Now we will search for some meta info........
             * Go through all from newE and check their pendant
             * from old. If test fails it was modified.
             * If not found it was added.
             * Then we will go through old and search for removed
             * entries
             */
            for ( entryNew = (Entry*)newE->firstEntry();
                  entryNew != 0l;
                  entryNew = (Entry*)newE->nextEntry() )
            {
                found  = false; // we did not find anything
                for ( entryOld = (Entry*) old->firstEntry();
                      entryOld != 0l;
                      entryOld = (Entry*) old->nextEntry() )
                {
                    if ( entryNew->id() == entryOld->id() ) {
                        found = true;
                        // found the old one. Let's test for differences
                        if ( test( entryNew, entryOld ) )
                            entryNew->setState( KSync::SyncEntry::Modified );
                        break; // we found it so we don't need to search further
                    }

                };
                if (!found )  // it was not found. So it's new
                    entryNew->setState( KSync::SyncEntry::Added );

            }
            // now find the deleted once and clone them
            for ( entryOld = (Entry*) old->firstEntry();
                  entryOld != 0l;
                  entryOld = (Entry*) old->nextEntry() )
            {

                found = false;
                for ( entryNew = (Entry*) newE->firstEntry();
                      entryNew != 0l;
                      entryNew = (Entry*) newE->nextEntry() )
                {
                    if ( entryOld->id() == entryNew->id() ) {
                        found = true;
                        break;
                    }
                }
                if (!found ) {
                    Entry* remEntry = (Entry*) entryOld->clone();
                    remEntry->setState( KSync::SyncEntry::Removed );
                    newE->addEntry( remEntry );
                }
            }
            delete old;
            return newE;
        }
        virtual bool test( Entry* newE,  Entry* old ) = 0; /*{ return true; } */

    };
};


#endif
