
#ifndef KSYNC_UNKNOWN_SYNCEE_H
#define KSYNC_UNKNOWN_SYNCEE_H

// $Id$

/**
 * $Log$
 * Revision 1.1  2002/07/15 19:20:52  zecke
 * addressbooksyncee fixes in regard of the modificationState
 *
 * Ported KUnknownSyncEntry to the new API and namespace
 *
 */

#include <qdatetime.h>
#include <qcstring.h>

#include "syncer.h"

namespace KSync {

    /**
     * UnknownSyncEntry is if a Konnector was requested
     * to download a file and is unable to convert.
     * Basicly the UnknownSyncEntry either holds the
     * fileName and the tempName or
     * the ByteArray
     */
    class UnknownSyncEntry : public SyncEntry {
    public:
        /**
         * A Konnector can be asked to download a file on sync.
         * Either it can download the file to temporary place or
         * it can copy the file to a bytearray.
         * DownLoadMode defines the mode
         */
        enum DownLoadMode { Tempfile = 0, ByteArray };
        typedef QPtrList<UnknownSyncEntry> PtrList;

        /**
         * c'tor
         * @param array the ByteArray
         * @param path the path where the file was downloaded from
         */
        UnknownSyncEntry( const QByteArray& array, const QString& path );

        /**
         * c'tor
         * @param fileName the place where the temp file is stored
         * @param path The path where the files comes from
         */
        UnknownSyncEntry( const QString& fileName, const QString& path );

        /**
         * c'tor
         */
        UnknownSyncEntry( const UnknownSyncEntry& entry );

        ~UnknownSyncEntry();

        /**
         * the bytearray
         */
        QByteArray array()const;

        /**
         * path to the original place
         */
        QString path()const;

        /**
         * the fileName of the temp file
         */
        QString fileName()const;

        /**
         * the mode of the SyncEntry
         */
        int mode()const;

        /**
         * if it's possible we can keep a ::stat here
         */
        QDateTime lastAccess()const;

        /**
         * set the last access
         */
        void setLastAccess(const QDateTime& time);

        QString name();
        QString id();
        QString timestamp();
        QString type()const;
        bool equals( SyncEntry* entry );
        SyncEntry* clone();

    private:
        class UnknownSyncEntryPrivate;
        UnknownSyncEntryPrivate* d;
        int mMode;
        bool mHasAccess : 1;
        QByteArray mArray;
        QString mPath;
        QString mFileName;
        QDateTime mTime;

    };

    class UnknownSyncee : public Syncee {
    public:
        UnknownSyncee();
        ~UnknownSyncee();

        SyncEntry* firstEntry();
        SyncEntry* nextEntry();
        QString type()const;
        void addEntry( SyncEntry*  );
        void removeEntry( SyncEntry* );
        bool read();
        bool write();
        SyncEntry::PtrList added();
        SyncEntry::PtrList modified();
        SyncEntry::PtrList removed();
        Syncee* clone();
    private:
        class UnknownSynceePrivate;
        UnknownSynceePrivate* d;
        /** voidi returns an empty PtrList */
        SyncEntry::PtrList voidi();
        UnknownSyncEntry::PtrList mList;

    };
};

#endif
