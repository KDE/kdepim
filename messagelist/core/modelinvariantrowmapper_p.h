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

#ifndef __MESSAGELIST_CORE_MODELINVARIANTROWMAPPER_P_H__
#define __MESSAGELIST_CORE_MODELINVARIANTROWMAPPER_P_H__

#include "core/modelinvariantrowmapper.h"

#include <QTimer>
#include <QTime>

#include <KDebug>

namespace MessageList
{

namespace Core
{

class ModelInvariantRowMapperPrivate
{
public:
  explicit ModelInvariantRowMapperPrivate( ModelInvariantRowMapper *owner ) : q( owner ) { }

  /**
   * Internal. Don't look a this :)
   */
  void updateModelInvariantIndex( int modelIndexRow, ModelInvariantIndex * invariantToFill );

  /**
   * Internal. Don't look a this :)
   */
  ModelInvariantIndex * modelIndexRowToModelInvariantIndexInternal( int modelIndexRow, bool updateIfNeeded );

  /**
   * Internal: Removes the first RowShift from the list.
   */
  void killFirstRowShift();

  /**
   * This is called from the ModelInvariantIndex destructor.
   * You don't need to care.
   */
  void indexDead( ModelInvariantIndex * index );

  /**
   * Internal: Performs a lazy update step.
   */
  void slotPerformLazyUpdate();

  ModelInvariantRowMapper * const q;

  QList< RowShift * > * mRowShiftList;  ///< The ordered list of RowShifts, most recent at the end
  QHash< int, ModelInvariantIndex * > * mCurrentInvariantHash; ///< The up-to-date invariants
  uint mCurrentShiftSerial;             ///< Current model change serial: FIXME: it explodes at 2^32 :D
  uint mRemovedShiftCount;              ///< The number of shifts that we have completely processed
  int mLazyUpdateChunkInterval;         ///< Msecs: how much time we spend inside a lazy update chunk
  int mLazyUpdateIdleInterval;          ///< Msecs: how much time we idle between lazy update chunks
  QTimer * mUpdateTimer;                ///< Background lazy update timer
};

} // Core

} // MessageList

#endif //!__MESSAGELIST_CORE_MODELINVARIANTROWMAPPER_P_H__

