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

#include <kdebug.h>

#include "syncui.h"

#include "syncalgorithm.h"

using namespace KSync;

SyncEntry *SyncAlgorithm::deconflict( SyncEntry *syncEntry, SyncEntry *target )
{
  SyncEntry* entry = syncEntry; // default to this
  if ( mUI ) entry = mUI->deconflict( syncEntry, target );

  return entry;
}

bool SyncAlgorithm::confirmDelete( SyncEntry *syncEntry, SyncEntry *target )
{
  bool ret = true;
  if ( mUI ) ret = mUI->confirmDelete( syncEntry, target );

  return ret;
}

void SyncAlgorithm::informBothDeleted( SyncEntry* entry, SyncEntry* target )
{
  if ( mUI ) mUI->informBothDeleted( entry, target );
}
