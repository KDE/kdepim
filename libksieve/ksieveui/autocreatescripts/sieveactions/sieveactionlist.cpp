/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "sieveactionlist.h"
#include "sieveaction.h"
#include "sieveactiondiscard.h"
#include "sieveactionsetflags.h"
#include "sieveactionstop.h"
#include "sieveactionaddflags.h"
#include "sieveactionfileinto.h"
#include "sieveactionreject.h"
#include "sieveactionkeep.h"
#include "sieveactionredirect.h"
#include "sieveactionremoveflags.h"
#include "sieveactionnotify.h"
#include "sieveactiondeleteheader.h"
#include "sieveactionaddheader.h"
#include "sieveactionvacation.h"
#include "sieveactionenclose.h"
#include "sieveactionreplace.h"
#include "sieveactionextracttext.h"
#include "sieveactionbreak.h"
#include "sieveactionconvert.h"
#include "sieveactionsetvariable.h"
#include "sieveactionreturn.h"

QList<KSieveUi::SieveAction *> KSieveUi::SieveActionList::actionList()
{
    QList<KSieveUi::SieveAction*> list;
    list.append(new KSieveUi::SieveActionDiscard);
    list.append(new KSieveUi::SieveActionStop);
    list.append(new KSieveUi::SieveActionSetFlags);
    list.append(new KSieveUi::SieveActionAddFlags);
    list.append(new KSieveUi::SieveActionRemoveFlags);
    list.append(new KSieveUi::SieveActionFileInto);
    list.append(new KSieveUi::SieveActionReject);
    list.append(new KSieveUi::SieveActionKeep);
    list.append(new KSieveUi::SieveActionRedirect);
    list.append(new KSieveUi::SieveActionNotify);
    list.append(new KSieveUi::SieveActionDeleteHeader);
    list.append(new KSieveUi::SieveActionAddHeader);
    list.append(new KSieveUi::SieveActionVacation);
    list.append(new KSieveUi::SieveActionEnclose);
    list.append(new KSieveUi::SieveActionReplace);
    list.append(new KSieveUi::SieveActionExtractText);
    list.append(new KSieveUi::SieveActionBreak);
    list.append(new KSieveUi::SieveActionConvert);
    list.append(new KSieveUi::SieveActionSetVariable);
    list.append(new KSieveUi::SieveActionReturn);
    return list;
}
