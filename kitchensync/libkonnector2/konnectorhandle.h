#ifndef KSYNC_KONNECTOR_HANDLE_H
#define KSYNC_KONNECTOR_HANDLE_H

#include <qobject.h>
#include <qstring.h>

namespace KSync {
    typedef QString UDI;
    /**
     * A KonnectorHandle is a convient class
     * to deal with a single konnector
     * instead of the KonnectorManager directly
     */
    class KonnectorHandle : public QObject {
        KonnectorHandle( const UDI&, QObject* );
        ~KonnectorHandle();

        /* yet to implement */

    };
}

#endif
