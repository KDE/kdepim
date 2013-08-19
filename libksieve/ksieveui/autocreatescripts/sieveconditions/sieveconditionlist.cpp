/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "sieveconditionlist.h"
#include "sievecondition.h"
#include "sieveconditionheader.h"
#include "sieveconditionaddress.h"
#include "sieveconditionsize.h"
#include "sieveconditionenvelope.h"
#include "sieveconditionexists.h"
#include "sieveconditiontrue.h"
#include "sieveconditionfalse.h"
//RFC5173 (extension)
#include "sieveconditionbody.h"
//rfc5260
#include "sieveconditiondate.h"
#include "sieveconditioncurrentdate.h"

#include "sieveconditionmailboxexists.h"

#include "sieveconditionspamtest.h"
#include "sieveconditionspamtestplus.h"
#include "sieveconditionvirustest.h"
#include "sieveconditionihave.h"
#include "sieveconditionenvironment.h"

#include "sieveconditionhasflag.h"
#include "sieveconditionmetadata.h"
#include "sieveconditionconvert.h"
#include "sieveconditionmetadataexists.h"
#include "sieveconditionservermetadata.h"
#include "sieveconditionservermetadataexists.h"

QList<KSieveUi::SieveCondition *> KSieveUi::SieveConditionList::conditionList()
{
    QList<KSieveUi::SieveCondition*> list;
    list.append(new KSieveUi::SieveConditionHeader);
    list.append(new KSieveUi::SieveConditionAddress);
    list.append(new KSieveUi::SieveConditionSize);
    list.append(new KSieveUi::SieveConditionEnvelope);
    list.append(new KSieveUi::SieveConditionExists);
    list.append(new KSieveUi::SieveConditionTrue);
    list.append(new KSieveUi::SieveConditionFalse);
    list.append(new KSieveUi::SieveConditionBody);
    list.append(new KSieveUi::SieveConditionDate);
    list.append(new KSieveUi::SieveConditionCurrentDate);
    list.append(new KSieveUi::SieveConditionMailboxExists);
    list.append(new KSieveUi::SieveConditionSpamTest);
    list.append(new KSieveUi::SieveConditionSpamTestPlus);
    list.append(new KSieveUi::SieveConditionVirusTest);
    list.append(new KSieveUi::SieveConditionIhave);
    list.append(new KSieveUi::SieveConditionEnvironment);
    list.append(new KSieveUi::SieveConditionHasFlag);
    list.append(new KSieveUi::SieveConditionMetaData);
    list.append(new KSieveUi::SieveConditionConvert);
    list.append(new KSieveUi::SieveConditionMetaDataExists);
    list.append(new KSieveUi::SieveConditionServerMetaData);
    list.append(new KSieveUi::SieveConditionServerMetaDataExists);
    return list;
}

