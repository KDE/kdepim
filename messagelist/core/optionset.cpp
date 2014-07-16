/******************************************************************************
 *
 *  Copyright 2008 Szymon Tomasz Stefanek <pragma@kvirc.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *******************************************************************************/

#include "core/optionset.h"

#include <time.h> // for ::time( time_t * )

#include <QDataStream>

static const int gOptionSetInitialMarker = 0xcafe; // don't change
static const int gOptionSetFinalMarker = 0xbabe; // don't change

static const int gOptionSetCurrentVersion = 0x1001; // increase if you add new fields of change the meaning of some

static const int gOptionSetWithReadOnLyModeVersion = 0x1002;

using namespace MessageList::Core;

OptionSet::OptionSet()
    : mReadOnly( false )
{
    generateUniqueId();
}

OptionSet::OptionSet( const OptionSet &set )
    : mId( set.mId ), mName( set.mName ), mDescription( set.mDescription ), mReadOnly( set.mReadOnly )
{
}

OptionSet::OptionSet( const QString &name, const QString &description, bool readOnly )
    : mName( name ), mDescription( description ), mReadOnly( readOnly )
{
    generateUniqueId();
}

OptionSet::~OptionSet()
{
}

void OptionSet::generateUniqueId()
{
    static int nextUniqueId = 0;
    nextUniqueId++;
    mId = QString::fromLatin1( "%1-%2" ).arg( (unsigned int)::time(0) ).arg( nextUniqueId );
}

QString OptionSet::saveToString() const
{
    QByteArray raw;

    {
        QDataStream s( &raw, QIODevice::WriteOnly );

        s << gOptionSetInitialMarker;
        s << gOptionSetWithReadOnLyModeVersion;
        s << mId;
        s << mName;
        s << mDescription;
        s << mReadOnly;

        save( s );

        s << gOptionSetFinalMarker;
    }

    return QString::fromLatin1( raw.toHex() );
}

bool OptionSet::loadFromString(const QString &data )
{
    QByteArray raw = QByteArray::fromHex( data.toLatin1() );

    QDataStream s( &raw, QIODevice::ReadOnly );

    int marker;

    s >> marker;

    if ( marker != gOptionSetInitialMarker )
        return false; // invalid configuration

    int currentVersion;

    s >> currentVersion;

    if ( currentVersion > gOptionSetWithReadOnLyModeVersion)
        return false; // invalid configuration

    s >> mId;

    if ( mId.isEmpty() )
        return false; // invalid configuration

    s >> mName;

    if ( mName.isEmpty() )
        return false; // invalid configuration

    s >> mDescription;

    bool readOnly = false;
    if ( currentVersion == gOptionSetWithReadOnLyModeVersion )
        s >> readOnly;
    mReadOnly = readOnly;

    if ( !load( s ) )
        return false; // invalid configuration

    s >> marker;

    if ( marker != gOptionSetFinalMarker )
        return false; // invalid configuration

    return true;
}

