
#ifndef KSYNC_SYNC_ALGO_H
#define KSYNC_SYNC_ALGO_H

#include <syncalgorithm.h>

namespace KSync {

    class PIMSyncAlg : public SyncAlgorithm {
    public:
        PIMSyncAlg( SyncUi* ui );
        ~PIMSyncAlg();
        virtual void syncToTarget(Syncee* syncee,
                                  Syncee* target,
                                  bool override = false );
    private:
        void syncFirst(Syncee* syncee,
                       Syncee* target,
                       bool over );
        void syncMeta(Syncee* syncee,
                      Syncee* target,
                      bool over );
    };
};

#endif
