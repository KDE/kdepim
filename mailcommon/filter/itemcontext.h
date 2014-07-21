/*
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef MAILCOMMON_ITEMCONTEXT_H
#define MAILCOMMON_ITEMCONTEXT_H

#include "mailcommon_export.h"
#include "../search/searchpattern.h"

#include <Collection>
#include <Item>

namespace MailCommon {

/**
 * @short A helper class for the filtering process
 *
 * The item context is used to pass the item together with meta data
 * through the filter chain.
 * This allows to 'record' all actions that shall be taken and execute them
 * at the end of the filter chain.
 */
class MAILCOMMON_EXPORT ItemContext
{
public:
    /**
     * Creates an item context for the given @p item.
     * @p requestedPart the part requested for the item (Envelope, Header or CompleteMessage)
     */
    ItemContext( const Akonadi::Item &item, bool needsFullPayload );

    /**
     * Returns the item of the context.
     */
    Akonadi::Item &item();

    /**
     * Sets the target collection the item should be moved to.
     */
    void setMoveTargetCollection( const Akonadi::Collection &collection );

    /**
     * Returns the target collection the item should be moved to, or an invalid
     * collection if the item should not be moved at all.
     */
    Akonadi::Collection moveTargetCollection() const;

    /**
     * Marks that the item's payload has been changed and needs to be written back.
     */
    void setNeedsPayloadStore();

    /**
     * Returns whether the item's payload needs to be written back.
     */
    bool needsPayloadStore() const;

    /**
     * Marks that the item's flags has been changed and needs to be written back.
     */
    void setNeedsFlagStore();

    /**
     * Returns whether the item's flags needs to be written back.
     */
    bool needsFlagStore() const;

    /** Returns true if the full payload was requested for the item or not.
     * Full payload is needed to change the headers or the body */
    bool needsFullPayload() const;

    void setDeleteItem();
    bool deleteItem() const;

private:
    Akonadi::Item mItem;
    Akonadi::Collection mMoveTargetCollection;
    bool mNeedsPayloadStore;
    bool mNeedsFlagStore;
    bool mDeleteItem;
    bool mNeedsFullPayload;
};

}

#endif
