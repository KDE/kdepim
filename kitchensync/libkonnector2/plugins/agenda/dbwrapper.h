#ifndef VR3_DBWRAPPER_H
#define VR3_DBWRAPPER_H

#include <qstring.h>

namespace Vr3 {
    /** the wrapper around the db
     *
     * see kdelibs/kabc/plugins/evolution for another DB3 wrapper
     * this one should look similar!
     */
    class DBWrapper {
    public:
        DBWrapper();
        ~DBWrapper();

        bool open( const QString& fileName );
        void close();

    };
}

#endif
