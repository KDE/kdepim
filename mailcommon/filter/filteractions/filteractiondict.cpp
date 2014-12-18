/*
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "filteractiondict.h"

#include "filteractionaddheader.h"
#include "filteractionaddtag.h"
#include "filteractionaddtoaddressbook.h"
#include "filteractionbeep.h"
#include "filteractioncopy.h"
#include "filteractiondelete.h"
#include "filteractionexec.h"
#include "filteractionforward.h"
#include "filteractionmove.h"
#include "filteractionpipethrough.h"
#include "filteractionplaysound.h"
#include "filteractionredirect.h"
#include "filteractionremoveheader.h"
#include "filteractionreplyto.h"
#include "filteractionrewriteheader.h"
#include "filteractionsendfakedisposition.h"
#include "filteractionsendreceipt.h"
#include "filteractionsetidentity.h"
#include "filteractionsetstatus.h"
#include "filteractionsettransport.h"
#include "filteractionunsetstatus.h"

using namespace MailCommon;

//=============================================================================
//
//   Filter  Action  Dictionary
//
//=============================================================================
FilterActionDict::~FilterActionDict()
{
    qDeleteAll(mList);
}

void FilterActionDict::init()
{
    insert(FilterActionMove::newAction);
    insert(FilterActionCopy::newAction);
    insert(FilterActionSetIdentity::newAction);
    insert(FilterActionSetStatus::newAction);
    insert(FilterActionAddTag::newAction);
    insert(FilterActionSendFakeDisposition::newAction);
    insert(FilterActionSetTransport::newAction);
    insert(FilterActionReplyTo::newAction);
    insert(FilterActionForward::newAction);
    insert(FilterActionRedirect::newAction);
    insert(FilterActionSendReceipt::newAction);
    insert(FilterActionExec::newAction);
    insert(FilterActionPipeThrough::newAction);
    insert(FilterActionRemoveHeader::newAction);
    insert(FilterActionAddHeader::newAction);
    insert(FilterActionRewriteHeader::newAction);
    insert(FilterActionPlaySound::newAction);
    insert(FilterActionAddToAddressBook::newAction);
    insert(FilterActionDelete::newAction);
    insert(FilterActionBeep::newAction);
    insert(FilterActionUnsetStatus::newAction);
    // Register custom filter actions below this line.
}

// The int in the QDict constructor (41) must be a prime
// and should be greater than the double number of FilterAction types
FilterActionDict::FilterActionDict()
    : QMultiHash<QString, FilterActionDesc *>()
{
    init();
}

void FilterActionDict::insert(FilterActionNewFunc aNewFunc)
{
    FilterAction *action = aNewFunc();
    FilterActionDesc *desc = new FilterActionDesc;
    desc->name = action->name();
    desc->label = action->label();
    desc->create = aNewFunc;

    QMultiHash<QString, FilterActionDesc *>::insert(desc->name, desc);
    QMultiHash<QString, FilterActionDesc *>::insert(desc->label, desc);
    mList.append(desc);

    delete action;
}

const QList<FilterActionDesc *> &FilterActionDict::list() const
{
    return mList;
}

