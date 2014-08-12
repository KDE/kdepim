/******************************************************************************
 *
 *  Copyright (c) 2010 Leo Franchi <lfranchi@kde.org>
 *  Copyright (c) 2010 KDAB
 *
 *  This library is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  This library is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 *  License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to the
 *  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA, 02110-1301, USA.
 *
 *****************************************************************************/

#include "mdnstateattribute.h"

#include <attributefactory.h>

#include <QtCore/QByteArray>

using namespace MessageCore;

/**
 *  @internal
 */
class MDNStateAttribute::Private
{
public:
    MDNSentState dataToState( const QByteArray &data )
    {
        MDNSentState state = MDNStateUnknown;

        switch ( data.at( 0 ) ) {
        case 'N': state = MDNNone; break;
        case 'I': state = MDNIgnore; break;
        case 'R': state = MDNDisplayed; break;
        case 'D': state = MDNDeleted; break;
        case 'F': state = MDNDispatched; break;
        case 'P': state = MDNProcessed; break;
        case 'X': state = MDNDenied; break;
        case 'E': state = MDNFailed; break;
        case 'U': state = MDNStateUnknown; break;
        default: state = MDNStateUnknown; break;
        }

        return state;
    }

    QByteArray stateToData( const MDNSentState &state )
    {
        QByteArray data = "U"; // Unknown

        switch ( state ) {
        case MDNNone:         data = "N"; break;
        case MDNIgnore:       data = "I"; break;
        case MDNDisplayed:    data = "R"; break;
        case MDNDeleted:      data = "D"; break;
        case MDNDispatched:   data = "F"; break;
        case MDNProcessed:    data = "P"; break;
        case MDNDenied:       data = "X"; break;
        case MDNFailed:       data = "E"; break;
        case MDNStateUnknown: data = "U"; break;
        }

        return data;
    }
    
    QByteArray mSentState;
};

MDNStateAttribute::MDNStateAttribute( const MDNSentState &state )
    : d( new Private )
{
    d->mSentState = d->stateToData( state );
}

MDNStateAttribute::MDNStateAttribute( const QByteArray &stateData )
    : d( new Private )
{
    d->mSentState = stateData;
}

MDNStateAttribute::~MDNStateAttribute()
{
    delete d;
}

MDNStateAttribute* MDNStateAttribute::clone() const
{
    return new MDNStateAttribute( d->mSentState );
}

QByteArray MDNStateAttribute::type() const
{
    static const QByteArray sType( "MDNStateAttribute" );
    return sType;
}

QByteArray MDNStateAttribute::serialized() const
{
    return d->mSentState;
}

void MDNStateAttribute::deserialize( const QByteArray &data )
{
    d->mSentState = data;
}

void MDNStateAttribute::setMDNState( const MDNSentState &state )
{
    d->mSentState = d->stateToData( state );
}

MDNStateAttribute::MDNSentState MDNStateAttribute::mdnState() const
{
    return d->dataToState( d->mSentState );
}

// Register the attribute when the library is loaded.
namespace {

bool dummyMDNStateAttribute()
{
    using namespace MessageCore;
    Akonadi::AttributeFactory::registerAttribute<MDNStateAttribute>();
    return true;
}

const bool registeredMDNStateAttribute = dummyMDNStateAttribute();

}
