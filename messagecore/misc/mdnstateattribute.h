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

#ifndef MESSAGECORE_MDNSTATEATTRIBUTE_H
#define MESSAGECORE_MDNSTATEATTRIBUTE_H

#include "messagecore_export.h"

#include <akonadi/attribute.h>

namespace MessageCore {

/**
 * @short An Attribute that keeps track of the MDN state of a mail message.
 *
 * Once a mail that contains a Message Disposition Notification is processed,
 * the outcome of the user action will be stored in this attribute.
 *
 * @author Leo Franchi <lfranchi@kde.org>
 * @see Akonadi::Attribute
 * @since 4.6
 */
class MESSAGECORE_EXPORT MDNStateAttribute : public Akonadi::Attribute
{
public:
    /**
     * Describes the "MDN sent" state.
     */
    enum MDNSentState
    {
        MDNStateUnknown, ///< The state is unknown.
        MDNNone,         ///< No MDN has been set.
        MDNIgnore,       ///< Ignore sending a MDN.
        MDNDisplayed,    ///< The message has been displayed by the UA to someone reading the recipient's mailbox.
        MDNDeleted,      ///< The message has been deleted.
        MDNDispatched,   ///< The message has been sent somewhere in some manner without necessarily having been previously displayed to the user.
        MDNProcessed,    ///< The message has been processed in some manner without being displayed to the user.
        MDNDenied,       ///< The recipient does not wish the sender to be informed of the message's disposition.
        MDNFailed        ///< A failure occurred that prevented the proper generation of an MDN.
    };

    /**
     * Creates a new MDN state attribute.
     *
     * @param state The state the attribute will have.
     */
    explicit MDNStateAttribute( const MDNSentState &state = MDNStateUnknown );

    /**
     * Creates a new MDN state attribute.
     *
     * @param state The string representation of the state the attribute will have.
     */
    explicit MDNStateAttribute( const QByteArray &state );
    
    /**
     * Destroys the MDN state attribute.
     */
    ~MDNStateAttribute();
    
    /**
     * Reimplemented from Attribute
     */
    QByteArray type() const;
    
    /**
     * Reimplemented from Attribute
     */
    MDNStateAttribute* clone() const;
    
    /**
     * Reimplemented from Attribute
     */
    QByteArray serialized() const;
    
    /**
     * Reimplemented from Attribute
     */
    void deserialize( const QByteArray &data );

    /**
     * Sets the MDN @p state.
     */
    void setMDNState( const MDNSentState &state );

    /**
     * Returns the MDN state.
     */
    MDNStateAttribute::MDNSentState mdnState() const;
    
private:
    //@cond PRIVATE
    class Private;
    Private* const d;
    //@endcond
};

}

#endif
