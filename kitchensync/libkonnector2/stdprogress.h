#ifndef KSYNC_STD_PROGRESS_H
#define KSYNC_STD_PROGRESS_H

#include "progress.h"

namespace KSync {
    struct StdProgress {
        static Progress connection();
        static Progress connected();
        static Progress authenticated();
        static Progress syncing(const QString&);
        static Progress downloading(const QString& );
        static Progress uploading(const QString&);
        static Progress converting(const QString&);
        static Progress reconverting(const QString&);
        static Progress done();
    };
}


#endif
