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

#include "filteractionredirect.h"

#include "../mailkernel.h"
#include "../mailutil.h"

#include <messagecomposer/messagefactory.h>
#include <messagecomposer/messagesender.h>
#include <messagecore/messagehelpers.h>

#include <KDE/KLocale>

using namespace MailCommon;

FilterAction* FilterActionRedirect::newAction()
{
  return new FilterActionRedirect;
}

FilterActionRedirect::FilterActionRedirect( QObject *parent )
  : FilterActionWithAddress( "redirect", i18n( "Redirect To" ), parent )
{
}

FilterAction::ReturnCode FilterActionRedirect::process( ItemContext &context ) const
{
  if ( mParameter.isEmpty() )
    return ErrorButGoOn;

  KMime::Message::Ptr msg = MessageCore::Util::message( context.item() );

  MessageComposer::MessageFactory factory( msg, context.item().id() );
  factory.setFolderIdentity( Util::folderIdentity( context.item() ) );
  factory.setIdentityManager( KernelIf->identityManager() );

  KMime::Message::Ptr rmsg = factory.createRedirect( mParameter );
  if ( !rmsg )
    return ErrorButGoOn;

  sendMDN( context.item(), KMime::MDN::Dispatched );

  if ( !KernelIf->msgSender()->send( rmsg, MessageSender::SendLater ) ) {
    kDebug() << "FilterAction: could not redirect message (sending failed)";
    return ErrorButGoOn; // error: couldn't send
  }

  return GoOn;
}

SearchRule::RequiredPart FilterActionRedirect::requiredPart() const
{
  return SearchRule::CompleteMessage;
}
