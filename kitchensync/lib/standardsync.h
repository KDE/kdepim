// the actual implementation is from Cornelius Schumacher for now.


#ifndef KitchenSync_StandardSync
#define KitchenSync_StandardSync

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
