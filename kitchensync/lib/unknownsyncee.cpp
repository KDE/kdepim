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

#include "unknownsyncee.h"

using namespace KSync;

UnknownSyncEntry::UnknownSyncEntry( const QByteArray &array,
                                    const QString &path )
    : SyncEntry(), mArray( array ), mPath( path )
{
    mHasAccess = false;
    mMode = ByteArray;
    mTime = QDateTime::currentDateTime();
}

UnknownSyncEntry::UnknownSyncEntry(const QString &fileName,
                                   const QString &path )
    : SyncEntry(), mPath( path ), mFileName( fileName )
{
    mHasAccess = false;
    mMode = Tempfile;
    mTime = QDateTime::currentDateTime();
}

UnknownSyncEntry::UnknownSyncEntry( const UnknownSyncEntry& entry)
    : SyncEntry( entry )
{
    mMode = entry.mMode;
    mHasAccess = entry.mHasAccess;
    mPath = entry.mPath;
    mArray = entry.mArray;
    mTime = entry.mTime;
}

UnknownSyncEntry::~UnknownSyncEntry()
{
}

QByteArray UnknownSyncEntry::array() const
{
    return mArray;
}

QString UnknownSyncEntry::path() const
{
    return mPath;
}

QString UnknownSyncEntry::fileName() const
{
    return mFileName;
}

int UnknownSyncEntry::mode() const
{
    return mMode;
}

QDateTime UnknownSyncEntry::lastAccess() const
{
    return mTime;
}

void UnknownSyncEntry::setLastAccess( const QDateTime& time )
{
    mHasAccess = true;
    mTime = time;
}

QString UnknownSyncEntry::name()
{
    return mPath;
}

QString UnknownSyncEntry::id()
{
    QString ids;
    ids = mPath;

    return ids;
}

QString UnknownSyncEntry::timestamp()
{
    if (mHasAccess )
        return mTime.toString();
    else
        return id();
}

QString UnknownSyncEntry::type() const
{
    return QString::fromLatin1("UnknownSyncEntry");
}

bool UnknownSyncEntry::equals( SyncEntry* entry )
{
    UnknownSyncEntry* unEntry = dynamic_cast<UnknownSyncEntry*> ( entry );
    if ( !unEntry )
        return false;

    if (mHasAccess == unEntry->mHasAccess &&
        mMode == unEntry->mMode &&
        mFileName == unEntry->mFileName &&
        mPath == unEntry->mPath &&
        mArray == unEntry->mArray) {

        if (mHasAccess )
            return (mTime == unEntry->mTime );
        else
            return true;
    }
    else
        return false;
}

SyncEntry* UnknownSyncEntry::clone()
{
    return new UnknownSyncEntry( *this );
}


UnknownSyncee::UnknownSyncee() : Syncee()
{
    mList.setAutoDelete( true );
}

UnknownSyncee::~UnknownSyncee()
{
}

SyncEntry *UnknownSyncee::firstEntry()
{
    return mList.first();
}

SyncEntry *UnknownSyncee::nextEntry()
{
    return mList.next();
}

QString UnknownSyncee::type() const
{
    return QString::fromLatin1("UnknownSyncee");
}

SyncEntry::PtrList UnknownSyncee::added()
{
    return voidi();
}

SyncEntry::PtrList UnknownSyncee::modified()
{
    return voidi();
}

SyncEntry::PtrList UnknownSyncee::removed()
{
    return voidi();
}

SyncEntry::PtrList UnknownSyncee::voidi()
{
    SyncEntry::PtrList list;
    return list;
}

void UnknownSyncee::addEntry( SyncEntry *entry )
{
    UnknownSyncEntry *unEntry;
    unEntry = dynamic_cast<UnknownSyncEntry *> (entry);
    if (unEntry == 0 )
        return;
    unEntry->setSyncee( this );
    mList.append( unEntry );
}

void UnknownSyncee::removeEntry( SyncEntry *entry )
{
    UnknownSyncEntry *unEntry;
    unEntry = dynamic_cast<UnknownSyncEntry *> (entry);
    if (unEntry == 0 )
        return;
    mList.remove( unEntry );
}

Syncee *UnknownSyncee::clone()
{
    UnknownSyncee *syncee;
    syncee = new UnknownSyncee();
    // always FirstMode
    UnknownSyncEntry *entry;
    for ( entry = mList.first(); entry != 0; entry = mList.next() ) {
        syncee->addEntry( entry->clone() ); // should be casted up to UnknownSyncEntry again...
    }
    return syncee;
}
