
#ifndef KSYNC_SYNC_ALGORITHM_H
#define KSYNC_SYNC_ALGORITHM_H

// $Id$

namespace KSync {

    class Syncee;
    class SyncEntry;
    class SyncUi;
    /**
     * A very simple sync interface for KitchenSync.
     * It'll be possible to install different sync
     * algorithm.
     *
     */
    class SyncAlgorithm {
    public:
        /**
         * An empty c'tor
         */
        SyncAlgorithm(SyncUi* ui=0) {mUI = ui; };

        /**
         * Empt d'tor
         */
        virtual ~SyncAlgorithm() {};

        /**
         * pure virtual method. Implement this if you want to have a different
         * sync algorithm
         * @param syncee Is one syncee
         * @param target Is the target syncee
         * @param override should overwrite or try to deconflict
         */
        virtual void syncToTarget(Syncee *syncee, Syncee *target, bool override=false ) = 0;

    protected:

        SyncEntry *deconflict( SyncEntry* syncEntry,  SyncEntry *target);
        bool confirmDelete( SyncEntry* syncEntry, SyncEntry* target );
        void informBothDeleted( SyncEntry* syncEntry, SyncEntry* target );

    private:
        class SyncAlgorithmPrivate;
        SyncAlgorithmPrivate* d;
        SyncUi *mUI;

    };

};

#endif
