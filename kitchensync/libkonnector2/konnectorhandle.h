#ifndef KSYNC_KONNECTORHANDLE_H
#define KSYNC_KONNECTORHANDLE_H

#include <qobject.h>
#include <qstring.h>

namespace KSync {

/**
 * A KonnectorHandle is a convenience class
 * to deal with a single konnector
 * instead of the KonnectorManager directly
 */
class KonnectorHandle : public QObject
{
    KonnectorHandle( Konnector *, QObject * );
    ~KonnectorHandle();

    /* yet to implement */

};

}

#endif
