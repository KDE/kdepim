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
#include <klocale.h>

#include "stdprogress.h"

using namespace KSync;

Progress StdProgress::connection() {
    return Progress( Progress::Connection, i18n("A connection was opened.") );
}
Progress StdProgress::connected() {
    return Progress( Progress::Connected, i18n("A connection was established.") );
}
Progress StdProgress::authenticated() {
    return Progress( Progress::Authenticated,  i18n("Successfully authenticated.") );
}
Progress StdProgress::syncing(const QString& str) {
    return Progress( Progress::Syncing, i18n("Currently synchronizing %1").arg(str) );
}
Progress StdProgress::downloading(const QString& str) {
    return Progress( Progress::Downloading, i18n("Currently downloading %1").arg(str) );
}
Progress StdProgress::uploading(const QString& str) {
    return Progress( Progress::Uploading, i18n("Currently uploading %1").arg(str) );
}
Progress StdProgress::converting(const QString& str) {
    return Progress( Progress::Converting,  i18n("Converting %1 to native format").arg(str) );
}
Progress StdProgress::reconverting( const QString& str) {
    return Progress( Progress::Reconverting, i18n("Reconverting %1 to remote format").arg(str) );
}
Progress StdProgress::done() {
    return Progress( Progress::Done, i18n("Done.") );
}
