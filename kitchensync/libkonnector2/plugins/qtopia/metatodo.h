/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2003 Holger Freyther <freyther@kde.org>

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
#ifndef OPIE_META_TODO_H
#define OPIE_META_TODO_H

#include <todosyncee.h>

#include "md5metatemplate.h"

namespace OpieHelper {
    class MetaTodo : public MD5Template<KSync::TodoSyncee, KSync::TodoSyncEntry> {
    public:
        MetaTodo();
        ~MetaTodo();

        QString string( KSync::TodoSyncEntry* );

    };
}

#endif
