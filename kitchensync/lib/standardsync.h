// the actual implementation is from Cornelius Schumacher for now.


// $Id$

#ifndef KSYNC_STANDARD_SYNC_H
#define KSYNC_STANDARD_SYNC_H

#include "syncalgorithm.h"

namespace KSync {
    class StandardSync : public SyncAlgorithm {
    public:
        StandardSync(SyncUi* ui) : SyncAlgorithm( ui ) {};

        virtual ~StandardSync() {
        };

        virtual void syncToTarget(Syncee* syncee,  Syncee* target,  bool override = false );
    };
};

#endif
