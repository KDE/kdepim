

#ifndef KSYNC_OPIE_DESKTOP_SYNCEE
#define KSYNC_OPIE_DESKTOP_SYNCEE

// $Id$

/**
 * $Log$
 */

#include <qstringlist.h>
#include <qstring.h>

#include "syncer.h"

namespace KSync {
    /**
     * OpieDesktopSyncEntry
     * Opie and Qtopia are featuring a Documents Tab
     * All Documents are available with one click
     * Now we've to show these files in KitchenSyncApp
     * This is done through the Syncee transportation
     * and syncing layer.
     */
    class OpieDesktopSyncEntry  : public SyncEntry {
    public:
        typedef QPtrList<OpieDesktopSyncEntry> PtrList;
        OpieDesktopSyncEntry( const QStringList& category,
                              const QString& file,
                              const QString& name,
                              const QString& type,
                              const QString& size );
        ~OpieDesktopSyncEntry();
        OpieDesktopSyncEntry( const OpieDesktopSyncEntry& );

        QString name() ;
        QString file() const;
        QString fileType() const;
        QString size() const;
        QStringList category() const;

        QString type() const;
        QString id() ;
        QString timestamp();
        bool equals( SyncEntry* );
        SyncEntry* clone();

    private:
        class OpieDesktopSyncEntryPrivate;
        OpieDesktopSyncEntryPrivate* d;
        QStringList mCategory;
        QString mFile;
        QString mName;
        QString mType;
        QString mSize;
    };

    class OpieDesktopSyncee : public Syncee {
    public:
        OpieDesktopSyncee();
        ~OpieDesktopSyncee();
        QString type() const;
        Syncee* clone();
        bool read();
        bool write();
        void addEntry( SyncEntry* entry );
        void removeEntry( SyncEntry* entry);
        SyncEntry* firstEntry();
        SyncEntry* nextEntry();
        SyncEntry::PtrList added();
        SyncEntry::PtrList modified();
        SyncEntry::PtrList removed();

    private:
        OpieDesktopSyncEntry::PtrList mList;
        SyncEntry::PtrList voidi();
    };
};

#endif
