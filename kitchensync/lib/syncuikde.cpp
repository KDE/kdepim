// $Id$

#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>

#include "syncer.h"

#include "syncuikde.h"

using namespace KSync;

SyncUiKde::SyncUiKde(QWidget *parent) :
  mParent(parent)
{
}

SyncUiKde::~SyncUiKde()
{
}

SyncEntry *SyncUiKde::deconflict(SyncEntry *syncEntry,SyncEntry *targetEntry)
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
