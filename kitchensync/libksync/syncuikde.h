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
#ifndef KSYNCUIKDE_H
#define KSYNCUIKDE_H

#include "syncui.h"

namespace KSync {

class SyncEntry;

class KDE_EXPORT SyncUiKde : public SyncUi
{
  public:
    SyncUiKde( QWidget *parent, bool confirmDelete, bool inform = false );
    virtual ~SyncUiKde();

    void setConfirmDelete( bool b);

    SyncEntry *deconflict( SyncEntry *syncEntry, SyncEntry *target );
    bool confirmDelete( SyncEntry *entry, SyncEntry *target );
    void informBothDeleted( SyncEntry *syncEntry, SyncEntry *target );

  private:
    SyncEntry *deletedChanged( SyncEntry *entry, SyncEntry *target );
    SyncEntry *changedChanged( SyncEntry *entry, SyncEntry *target );

    QWidget *mParent;
    bool m_confirm : 1;
    bool m_inform  : 1;
};

}

#endif
