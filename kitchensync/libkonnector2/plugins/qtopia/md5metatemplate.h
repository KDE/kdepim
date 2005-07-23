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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef OPIE_HELPER_META_TEMPLATE_NEW_H
#define OPIE_HELPER_META_TEMPLATE_NEW_H

#include <synchistory.h>

#include <konnector.h>
#include <idhelper.h>


namespace OpieHelper {

template <class Syncee = KSync::Syncee, class Entry = KSync::SyncEntry>
class MD5Template : public KSync::SyncHistory<Syncee,Entry> {
public:
    MD5Template( Syncee*, const QString& file );
    ~MD5Template();

protected:
    virtual QString entryToString( Entry* ) = 0;
    virtual QString string( Entry* );
};


template<class Syncee, class Entry>
MD5Template<Syncee, Entry>::MD5Template( Syncee* syncee, const QString& file )
    : KSync::SyncHistory<Syncee, Entry>( syncee, file )
{}

template<class Syncee, class Entry>
MD5Template<Syncee, Entry>::~MD5Template()
{}

template<class Syncee, class Entry>
QString MD5Template<Syncee, Entry>::string( Entry* ent) {
    QString str = entryToString( ent );
    return KSync::Konnector::generateMD5Sum( str );
}
}


#endif
