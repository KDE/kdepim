/*
    This file is part of KitchenSync.

    Copyright (c) 2003,2004 Holger Freyther <zecke@handhelds.org>

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

#ifndef KITCHENSYNC_KSYNC_MERGER_H
#define KITCHENSYNC_KSYNC_MERGER_H

#include "syncentry.h"

#include <qbitarray.h>
#include <qstring.h>

namespace KSync{

/**
 * \brief Small class to help in not losing attributes
 *
 * The idea of this class is the need for not losing attributes
 * between syncs where two parties do not support the same
 * amount of attributes.
 * One party can create a specialised Merger and set it on the
 * Syncee to allow merging attributes from a different SyncEntry
 * into one belonging to the Syncee of the party.
 *
 * Your specialised class just need to implement the merge function. and call
 * setSynceeType
 *
 */
class KDE_EXPORT Merger 
{
 public:
    Merger();
    virtual ~Merger();

    /**
     * Ask the interface to do the merge. This asks to merge 'other's additional
     * attributes into entry.
     * This will alter entry and 'other' should not be changed.
     * If other does not have a merger treat it as every attribute is supported.
     * You might at least want to merge all custom attributes.
     *
     * @return success or failure of merge
     */
    virtual bool merge( SyncEntry* entry, SyncEntry* other ) = 0;


    /**
     * Return which Syncee corresspondends to the actual implementation
     * of the interface. Normally the Attributes of the records heavily
     * depend on it.
     *
     * @return The Type of the Syncee.
     */
    QString synceeType()const;
protected:
    /**
     * Normally the actual implementation will already do that
     *
     * @param s The Type of the Syncee
     */
    void setSynceeType( const QString& s);

    /**
     * Convience method when implementing 'merge'
     *
     * @return  If the two entries are of the same type
     *
     * @param e1 The first  sync entry
     * @param e2 The second sync entry
     */
    bool sameType( SyncEntry* e1, SyncEntry* e2);
    bool sameType( SyncEntry* e1, SyncEntry* e2, const QString& wished );

    /**
     * Concience for getting the Merger from a different Syncee over
     * the SyncEntry.
     */
    template<class M>
    M* otherMerger( SyncEntry* );

protected: 
    QBitArray mBitArray;
    QString mString;
};

template<class M>
M* Merger::otherMerger( SyncEntry* entry ) 
{ 
  if ( !entry->syncee() || !entry->syncee()->merger() ) return 0l;

  return static_cast<M*>( entry->syncee()->merger() );
}


}
#endif
