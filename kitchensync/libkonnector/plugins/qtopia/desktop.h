
#ifndef OPIE_HELPER_DESKTOP_H
#define OPIE_HELPER_DESKTOP_H


#include <opiedesktopsyncee.h>

#include "helper.h"

namespace OpieHelper {

    class Desktop : public Base {
    public:
        Desktop( CategoryEdit* edit );
        ~Desktop();
        KSync::OpieDesktopSyncee* toSyncee( const QString& );
    };
};

#endif
