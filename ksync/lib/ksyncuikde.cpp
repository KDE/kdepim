/*
    This file is part of ksync.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#include "ksyncer.h"

#include "ksyncuikde.h"

KSyncUiKde::KSyncUiKde(QWidget *parent) :
  mParent(parent)
{
}

KSyncUiKde::~KSyncUiKde()
{
}

KSyncEntry *KSyncUiKde::deconflict(KSyncEntry *syncEntry,KSyncEntry *targetEntry)
{
  QString text = i18n("Which entry do you want to take precedence?\n");
  text += i18n("Entry 1: '%1'\n").arg(syncEntry->name());
  text += i18n("Entry 2: '%1'\n").arg(targetEntry->name());

  int result = KMessageBox::warningYesNoCancel(mParent,text,
      i18n("Resolve conflict"),i18n("Entry 1"),i18n("Entry 2"));

  if (result == KMessageBox::Yes) {
    return syncEntry;
  } else if (result == KMessageBox::No) {
    return targetEntry;
  }
  
  return 0;
}
