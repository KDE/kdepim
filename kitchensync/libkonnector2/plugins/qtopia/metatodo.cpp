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

#include <libkcal/todo.h>

#include "metatodo.h"

using namespace OpieHelper;

MetaTodo::MetaTodo()
    : MD5Template<KSync::TodoSyncee, KSync::TodoSyncEntry>() {
}
MetaTodo::~MetaTodo() {
}
QString MetaTodo::string( KSync::TodoSyncEntry* entry ) {
    QString str;
    KCal::Todo* todo = entry->todo();

    str += todo->categories().join(";");
    str += QString::number( todo->isCompleted() );
    str += QString::number( todo->percentComplete() );
    str += todo->summary();
    if ( todo->hasDueDate() ) {
        str += todo->dtDue().toString("dd.MM.yyyy");
    }
    str += QString::number( todo->priority() );
    str += todo->description();
    kdDebug(5227) << "Meta String is " << str << "Todo is " << todo->isCompleted() << QString::number( todo->isCompleted() ) << endl;
    return str;
}
