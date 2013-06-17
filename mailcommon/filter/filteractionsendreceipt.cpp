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

#include "filteractionsendreceipt.h"

#include "kernel/mailkernel.h"
#include "util/mailutil.h"

#include "messagecomposer/helper/messagefactory.h"
#include "messagecomposer/sender/messagesender.h"

#include <KDE/KLocale>

using namespace MailCommon;

FilterActionSendReceipt::FilterActionSendReceipt( QObject *parent )
  : FilterActionWithNone( "confirm delivery", i18n( "Confirm Delivery" ), parent )
{
}

FilterAction::ReturnCode FilterActionSendReceipt::process( ItemContext &context ) const
{
  const KMime::Message::Ptr msg = context.item().payload<KMime::Message::Ptr>();

  MessageComposer::MessageFactory factory( msg, context.item().id() );
  factory.setFolderIdentity( Util::folderIdentity( context.item() ) );
  factory.setIdentityManager( KernelIf->identityManager() );

  const KMime::Message::Ptr receipt = factory.createDeliveryReceipt();
  if ( !receipt )
    return ErrorButGoOn;

  // Queue message. This is a) so that the user can check
  // the receipt before sending and b) for speed reasons.
  KernelIf->msgSender()->send( receipt, MessageComposer::MessageSender::SendLater );

  return GoOn;
}

SearchRule::RequiredPart FilterActionSendReceipt::requiredPart() const
{
  return SearchRule::CompleteMessage;
}


FilterAction* FilterActionSendReceipt::newAction()
{
  return new FilterActionSendReceipt;
}

#include "filteractionsendreceipt.moc"
