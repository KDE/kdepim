/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#ifndef STDSYNCUI_H
#define STDSYNCUI_H

#include "syncentry.h"

#include "syncui.h"

namespace KSync {

class SyncUiFirst : public SyncUi
{
  public:
    SyncUiFirst() {}
    virtual ~SyncUiFirst() {}

    virtual SyncEntry* deconflict( SyncEntry *syncEntry, SyncEntry* )
    {
      return syncEntry;
    }

    virtual bool confirmDelete( SyncEntry*, SyncEntry* )
    {
      return true;
    }

    virtual void informBothDeleted( SyncEntry*, SyncEntry* )
    {
      return;
    }
};

class SyncUiSecond : public SyncUi
{
  public:
    SyncUiSecond() {}
    virtual ~SyncUiSecond() {}

    virtual SyncEntry* deconflict( SyncEntry*, SyncEntry *syncEntry )
    {
      return syncEntry;
    }

    virtual bool confirmDelete( SyncEntry*, SyncEntry* )
    {
      return true;
    }

    virtual void informBothDeleted( SyncEntry*, SyncEntry* )
    {
      return;
    }
};

}

#endif
