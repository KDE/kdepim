#ifndef KSYNC_FILTER_H
#define KSYNC_FILTER_H

#include <qstring.h>
#include <qstringlist.h>
#include <qptrlist.h>

#include <syncer.h>

namespace KSync {

    /**
     * A Filter is a post processor for KonnectorPlugins
     * if they're demanded to retrieve a file where the
     * content is unknown the file will be packaged inside
     * a UnknownSyncee and a filter can then convert
     * to more suitable Syncee...
     * For Example a wrapper around KIO would download a KDE
     * addressbook the mimetype gets determined and the
     * Addressbook Filter gets called
     * A Filter need to filter in both ways....
     */
    struct Filter {
        typedef QPtrList<Filter> PtrList;
        virtual QString name() = 0;
        virtual QStringList mimeTypes()const = 0;

        /**
         * both methods may return 0 if they're
         * not able to convert!
         */
        virtual Syncee* reconvert( Syncee* ) = 0;
        virtual Syncee* convert( Syncee* ) = 0;
    };
}

#endif
