#ifndef KSYNC_GENERICSYNCEE_H
#define KSYNC_GENERICSYNCEE_H

#include <qstring.h>
#include <qstringlist.h>

#include <kdebug.h>

#include "syncer.h"

/**
 * this is my first template ever
 */
namespace KSync {
    template <class Entry= SyncEntry>
    class SyncTemplate : public Syncee {
    public:
        typedef QPtrList<Entry> PtrList;
        SyncTemplate() : Syncee()  {
            mList.setAutoDelete( true );
        };
        ~SyncTemplate() { };
/*        QString type() const { return QString::fromLatin1(typeName); }*/
        /**
         * basic clone implementation
         */
        QString type() const { return QString::fromLatin1("SyncTemplate"); }
        Syncee* clone() {
            SyncTemplate* temp = new SyncTemplate<Entry>();
            temp->setSyncMode( syncMode() );
	    temp->setFirstSync( firstSync() );
            Entry* entry;
            for ( entry = mList.first(); entry != 0; entry = mList.next() ) {
                temp->addEntry( (Entry*)entry->clone() );
            }
            return temp;
        };
        bool read() { return true;}
        bool write() { return true; }

        SyncEntry* firstEntry() {
            return mList.first();
        }
        SyncEntry* nextEntry() {
            return mList.next();
        }
        SyncEntry::PtrList added() {
            return find( SyncEntry::Added );
        }
        SyncEntry::PtrList modified() {
            return find( SyncEntry::Modified );
        }
        SyncEntry::PtrList removed() {
            return find(SyncEntry::Removed );
        }
        void addEntry( SyncEntry* entry ) {
            kdDebug(5230) << "addEntry " << entry->type() << endl;
            Entry* tempEntry = dynamic_cast<Entry*> ( entry );
            if ( tempEntry == 0l ) {
                kdDebug(5230) << "could not cast" << endl;
                return;
            };
            tempEntry->setSyncee( this );
            if ( tempEntry->state() == SyncEntry::Undefined ) {
                if ( hasChanged( tempEntry ) )
                    tempEntry->setState( SyncEntry::Modified );
            }
            mList.append( tempEntry );
        }
        void removeEntry( SyncEntry* entry ) {
            Entry* tempEntry = dynamic_cast<Entry*> ( entry );
            if ( tempEntry == 0l )
                return;
            mList.remove( tempEntry );
        }

    protected:
        SyncEntry::PtrList find( int state ) {
            kdDebug(5230) << "find state " << state << endl;
            SyncEntry::PtrList found;
            Entry* entry;
            for (entry = mList.first(); entry != 0; entry = mList.next() ) {
                if ( entry->state() == state ) {
                    kdDebug(5230) << "matched state in find " << entry->state() << endl;
                    found.append( entry );
                }
            }
            return found;
        }
        PtrList mList;

    };
}


#endif
