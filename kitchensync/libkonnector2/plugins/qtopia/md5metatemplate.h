#ifndef OPIE_HELPER_META_TEMPLATE_NEW_H
#define OPIE_HELPER_META_TEMPLATE_NEW_H

#include <kmdcodec.h>

#include <syncer.h>
#include <idhelper.h>


#include "md5map.h"

namespace OpieHelper {

    template <class Syncee = KSync::Syncee, class Entry = KSync::SyncEntry>
    class MD5Template {
    public:
        MD5Template();
        virtual ~MD5Template();

        void doMeta( Syncee* newEntries, const MD5Map& );
        void saveMeta( Syncee*,  MD5Map& );

    protected:
        virtual QString string( Entry* ) = 0;

    private:
        QString md5sum( const QString& );
    };

    template<class Syncee, class Entry>
    MD5Template<Syncee, Entry>::MD5Template() {
    }
    template<class Syncee, class Entry>
    MD5Template<Syncee, Entry>::~MD5Template() {
    }
    template<class Syncee, class Entry>
    void MD5Template<Syncee, Entry>::doMeta( Syncee* newEntries,  const MD5Map& map) {
        bool found;
        Entry* entryNew;
        /**
         * Now we'll search for some meta info
         * go through all entries
         * check if they exist
         * if exist check if modified
         * else it was added
         */
        for ( entryNew = (Entry*)newEntries->firstEntry();
              entryNew != 0l;
              entryNew = (Entry*)newEntries->nextEntry() ) {
            found = false;

            /*
             * check if the MD5Map contains
             * the UID
             * if the md5 sums are not equal
             * set the modified state
             * ADDED set Added state
             */
            if ( map.contains( entryNew->id() ) ) {
                found = true;
                QString str = map.md5sum( entryNew->id() );
                QString newStr = string( entryNew );

                if ( str != md5sum( newStr )  ) {
                    entryNew->setState( KSync::SyncEntry::Modified );
                }
            }
            if (!found ) {
                entryNew->setState( KSync::SyncEntry::Added );
            }
        }
        /*
         * Now find the deleted records
         */
        MD5Map::Iterator it;
        MD5Map::Map ma = map.map();
        for ( it = ma.begin(); it != ma.end(); ++it ) {
            entryNew = (Entry*)newEntries->findEntry( it.key() );
            /**
             * if we've a UID
             * but we can not find it
             * in the Syncee
             * it was removed
             */
            if (!entryNew) {
                entryNew = new Entry();
                entryNew->setId( it.key() );

                /* add entry first and then to setState */
                newEntries->addEntry( entryNew );
                entryNew->setState( KSync::SyncEntry::Removed );
            }
        }

    }
    template<class Syncee, class Entry>
    void MD5Template<Syncee, Entry>::saveMeta( Syncee* syncee, MD5Map& map) {
        map.clear();
        for ( Entry* entry = (Entry*)syncee->firstEntry();
              entry != 0l; entry = (Entry*)syncee->nextEntry() ) {

            /* only save meta for not deleted SyncEntries! */
            if ( entry->state() != KSync::SyncEntry::Removed ) {
                map.insert( entry->id(), md5sum( string( entry ) ) );
            }
        }
    }
    template<class Syncee, class Entry>
    QString MD5Template<Syncee, Entry>::md5sum( const QString& base ) {
        KMD5 sum(base.local8Bit() );
        QString str = QString::fromLatin1( sum.hexDigest().data() );

        return str;
    }
};


#endif
