/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <freyther@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
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

        virtual bool canFilter( Syncee* ) = 0;
        /**
         * both methods may return 0 if they're
         * not able to convert!
         */
        virtual Syncee* reconvert( Syncee* ) = 0;
        virtual Syncee* convert( Syncee* ) = 0;
    };
}

#endif
