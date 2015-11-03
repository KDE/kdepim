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

#include "itemcontext.h"

using namespace MailCommon;

ItemContext::ItemContext(const Akonadi::Item &item, bool needsFullPayload)
    : mItem(item), mItemContextAction(needsFullPayload ? FullPayload : None)
{
}

Akonadi::Item &ItemContext::item()
{
    return mItem;
}

void ItemContext::setMoveTargetCollection(const Akonadi::Collection &collection)
{
    mMoveTargetCollection = collection;
}

Akonadi::Collection ItemContext::moveTargetCollection() const
{
    return mMoveTargetCollection;
}

void ItemContext::setNeedsPayloadStore()
{
    mItemContextAction |= PlayloadStore;
}

bool ItemContext::needsPayloadStore() const
{
    return (mItemContextAction & PlayloadStore);
}

void ItemContext::setNeedsFlagStore()
{
    mItemContextAction |= FlagStore;
}

bool ItemContext::needsFlagStore() const
{
    return (mItemContextAction & FlagStore);
}

void ItemContext::setDeleteItem()
{
    mItemContextAction |= DeleteItem;
}

bool ItemContext::deleteItem() const
{
    return (mItemContextAction & DeleteItem);
}

bool ItemContext::needsFullPayload() const
{
    return (mItemContextAction & FullPayload);
}
