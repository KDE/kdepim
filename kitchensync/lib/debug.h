#ifndef KSYNC_DEBUG_H
#define KSYNC_DEBUG_H

#include <syncer.h>

namespace KSync {

    /**
     * MetaDebug is using kDebug()
     * internally
     * and outputs all
     * MetaInformations of a complete Syncee
     */
    class MetaDebug {
    public:
        MetaDebug(int area );
        ~MetaDebug();

        /**
         * Operator for syncee
         */
        MetaDebug &operator<< ( Syncee* );
    private:
        int m_area;
    };

    /**
     * prints the content of a syncee
     */
    class SynceeDebug {
    public:
        SynceeDebug(int area );
        ~SynceeDebug();

        SynceeDebug& operator<<( Syncee* );
    private:
        int m_area;
    };

    MetaDebug   metaDebug(int area = 0 );
    SynceeDebug synceeDebug(int area = 0 );
}


#endif
