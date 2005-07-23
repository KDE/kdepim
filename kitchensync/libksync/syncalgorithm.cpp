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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kdebug.h>

#include "syncui.h"

#include "syncalgorithm.h"

using namespace KSync;

void SyncAlgorithm::setUi( SyncUi *ui )
{
  mUi = ui;
}

SyncEntry *SyncAlgorithm::deconflict( SyncEntry *syncEntry, SyncEntry *target )
{
  if ( mUi ) {
    return mUi->deconflict( syncEntry, target );
  } else {
    kdWarning() << "SyncAlgorithm: No UI set." << endl;
    return 0;
  }
}

bool SyncAlgorithm::confirmDelete( SyncEntry *syncEntry, SyncEntry *target )
{
  bool ret = true;
  if ( mUi ) ret = mUi->confirmDelete( syncEntry, target );
  else kdWarning() << "SyncAlgorithm: No UI set." << endl;

  return ret;
}

void SyncAlgorithm::informBothDeleted( SyncEntry *entry, SyncEntry *target )
{
  if ( mUi ) mUi->informBothDeleted( entry, target );
  else kdWarning() << "SyncAlgorithm: No UI set." << endl;
}
