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

#include "syncentry.h"
#include "syncee.h"

#include "syncui.h"

using namespace KSync;

SyncUi::SyncUi()
{
}

SyncUi::~SyncUi()
{
}

SyncEntry *SyncUi::deconflict( SyncEntry *syncEntry, SyncEntry *targetEntry )
{
  kdDebug(5231) << "deconflicting:" << endl;
  kdDebug(5231) << "  source: " << syncEntry->name() << endl;
  kdDebug(5231) << "  target: " << targetEntry->name() << endl;

  return 0;
}

bool SyncUi::confirmDelete( SyncEntry *entry, SyncEntry *target )
{
  kdDebug(5231) << "Entry with name " << target->name() << " was deleted "
                << endl;
  kdDebug(5231) << "From " << target->syncee()->source() << endl;

  return true;
}

void SyncUi::informBothDeleted( SyncEntry *entry, SyncEntry *other )
{
  kdDebug(5231) << "Entry with uid " << entry->id()
                << "was deleted on both sides " << endl;
}
