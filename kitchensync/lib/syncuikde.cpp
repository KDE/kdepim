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
#include <kmessagebox.h>
#include <klocale.h>

#include "syncer.h"
#include "syncee.h"

#include "syncuikde.h"

using namespace KSync;

SyncUiKde::SyncUiKde(QWidget *parent, bool confirm, bool inform) :
  mParent(parent), m_confirm( confirm ), m_inform( inform )
{
}

SyncUiKde::~SyncUiKde()
{
}

SyncEntry *SyncUiKde::deconflict(SyncEntry *syncEntry,SyncEntry *targetEntry)
{
    if ( syncEntry->wasModified() && targetEntry->wasModified() )
        return changedChanged( syncEntry, targetEntry );
    else if ( syncEntry->wasRemoved() && targetEntry->wasModified() )
        return deletedChanged( syncEntry, targetEntry );

    /* fallback */
    QString text = i18n("Which entry do you want to take precedence?\n");
    text += i18n("Entry 1: '%1'\n").arg(syncEntry->name());
    text += i18n("Entry 2: '%1'\n").arg(targetEntry->name());

    int result = KMessageBox::questionYesNo(mParent,text,
                                            i18n("Resolve conflict"),i18n("Entry 1"),i18n("Entry 2"));

    if (result == KMessageBox::Yes) {
        return syncEntry;
    } else if (result == KMessageBox::No) {
        return targetEntry;
    }

  return 0;
}

bool SyncUiKde::confirmDelete( SyncEntry* entry, SyncEntry* target )
{
    if (!m_confirm ) return true;

    QString text = i18n("%1 was deleted on %2. Do you want to delete it?").arg( target->name() ).arg( entry->syncee()->source() );

    int res =KMessageBox::questionYesNo(mParent, text, i18n("Delete?") );
    if ( res == KMessageBox::Yes ) return true;
    else return false;

    return true;
}

/**
 * deleted on one side...
 */
SyncEntry* SyncUiKde::deletedChanged( SyncEntry* syncEntry, SyncEntry* target )
{
    QString text = i18n("%1 was deleted on %2 and changed on %3").arg( target->name() ).arg( syncEntry->syncee()->source() ).arg( target->syncee()->source() );
    int res = KMessageBox::questionYesNo(mParent, text, i18n("Delete or modify?"),
                               i18n("Delete"), i18n("Modify") );
    if ( res == KMessageBox::Yes )
        return syncEntry;
    else if ( res == KMessageBox::No )
        return target;

    return 0;

}

SyncEntry* SyncUiKde::changedChanged( SyncEntry* syncEntry, SyncEntry* target )
{
    QString text = i18n("%1 was changed on both sources. Which one do you want to take?").arg(syncEntry->name() );

    int res = KMessageBox::questionYesNo(mParent, text, i18n("Modified two entries"),
                                       syncEntry->syncee()->source(),
                                       target->syncee()->source() );

    if ( res == KMessageBox::Yes )
        return syncEntry;
    else if ( res == KMessageBox::No )
        return target;

    return 0;
}

void SyncUiKde::informBothDeleted( SyncEntry* syncEntry, SyncEntry* target )
{
    if (m_inform)
        KMessageBox::information(mParent, i18n("The entry with the id %1 was deleted on %2 and %3").arg( syncEntry->id() ).arg( syncEntry->syncee()->source() ).arg( target->syncee()->source() ) );
}
