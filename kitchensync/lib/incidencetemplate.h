
#ifndef KSYNC_INCEDENCE_TEMPLATE_H
#define KSYNC_INCEDENCE_TEMPLATE_H

#include "syncer.h"
#include <incidence.h>

namespace KSync {
    template <class Entry = KCal::Incidence>
    class IncidenceTemplate : public SyncEntry {
    public:
        typedef QPtrList<Entry> PtrList;
        IncidenceTemplate(Entry* entry)
            : SyncEntry(), mIncidence( entry ) {

        };
        IncidenceTemplate( const IncidenceTemplate& temp )
            : SyncEntry( temp ){
            mIncidence = (Entry*)temp.mIncidence->clone();
        }
        ~IncidenceTemplate() {
            delete mIncidence;
        }
        QString type() const { return mIncidence->type() + "SyncEntry"; }
        QString name() { return mIncidence->summary(); }
        QString id() { return mIncidence->uid(); }
        void setId(const QString& id) { mIncidence->setUid( id ); }
        QString timestamp() { return mIncidence->lastModified().toString(); }
        Entry* incidence() { return mIncidence; };
        bool equals( SyncEntry* entry) {
            IncidenceTemplate* inEntry = dynamic_cast<IncidenceTemplate*> (entry );
            if (!inEntry )
                return false;
            if (mIncidence->uid() != inEntry->incidence()->uid() ) return false;
            if (mIncidence->lastModified() != inEntry->incidence()->lastModified() )
                return false;

            return true;
        }
        SyncEntry* clone() {
            return new IncidenceTemplate<Entry>( *this );
        }
    private:
        Entry* mIncidence;

    };

};


#endif
