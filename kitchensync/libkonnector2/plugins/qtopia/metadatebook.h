#ifndef META_DATEBOOK_H
#define META_DATEBOOK_H

#include <qbitarray.h>

#include <eventsyncee.h>

#include "md5metatemplate.h"

namespace OpieHelper {
    class MetaDatebook : public MD5Template<KSync::EventSyncee, KSync::EventSyncEntry> {
    public:
        MetaDatebook();
        ~MetaDatebook();

        QString string( KSync::EventSyncEntry* );


    private:
        QString days( const QBitArray& );

    };
};

#endif
