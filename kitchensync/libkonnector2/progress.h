/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>

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
#ifndef KSYNC_PROGRESS_H
#define KSYNC_PROGRESS_H

#include <qstring.h>

#include "notify.h"

namespace KSync {
    /* base class for error and progress? -zecke */
    class Progress : public Notify{
    public:
        enum ProgressCodes {
            Connection, Connected, Authenticated,
            Syncing, Downloading, Uploading, Converting,
            Reconverting, Done,  Undefined
        };
        Progress(int code, const QString& text);
        Progress( const QString& text = QString::null );

        bool operator==( const Progress& );

    private:
        class Private;
        Private *d;
    };
}


#endif
