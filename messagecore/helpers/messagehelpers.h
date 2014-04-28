/*
 * This file is part of KDEPIM.
 * Copyright (c) 2010 Till Adam <adam@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef MESSAGECORE_MESSAGEHELPERS_H
#define MESSAGECORE_MESSAGEHELPERS_H

#include "messagecore_export.h"

#include <item.h>
#include <Akonadi/KMime/messagestatus.h>
#include <boost/shared_ptr.hpp>
#include <KMime/Message>

namespace MessageCore {

namespace Util {
/**
   * Retrieve the KMime::Message from the item, if there is one.
   * @returns A valid message pointer, or 0, is the item does not contain
   * a valid message.
   */
MESSAGECORE_EXPORT KMime::Message::Ptr message( const Akonadi::Item &item );

/**
   * Returns whether the item represents a valid KMime::Message that is not
   * in the Akonadi store (yet). This happens when operating on messages
   * attached to other mails, for example. Such items are not "valid", in
   * the akonadi sense, since jobs cannot sensibly use them, but they do
   * contain a valid message pointer.
   */
MESSAGECORE_EXPORT bool isStandaloneMessage( const Akonadi::Item &item );

/**
   * Get the message id as a string from the @p message.
   */
MESSAGECORE_EXPORT QString messageId( const KMime::Message::Ptr &message );

/**
   * Adds private headers to the given @p message that links it to the original message.
   *
   * @param message The message to add the link information to.
   * @param id The item id of the original message.
   * @param status The status (replied or forwarded) that links the message to the original message.
   */
MESSAGECORE_EXPORT void addLinkInformation( const KMime::Message::Ptr &message, Akonadi::Item::Id item, const Akonadi::MessageStatus &status );

/**
   * Reads the private headers of the given @p message to extract link information to its original message.
   *
   * @param message The message to read the link information from.
   * @param id Will contain the item id of the original message.
   * @param status Will contain the status (replied or forwarded) that linked the message to the original message.
   * @returns Whether the mail contains valid link information or not.
   */
MESSAGECORE_EXPORT bool getLinkInformation( const KMime::Message::Ptr &msg, QList<Akonadi::Item::Id> &id, QList<Akonadi::MessageStatus> &status );
}
}

#endif
