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

#include "filteractionreplyto.h"

#include <KDE/KLocale>

using namespace MailCommon;

FilterAction* FilterActionReplyTo::newAction()
{
  return new FilterActionReplyTo;
}

FilterActionReplyTo::FilterActionReplyTo( QObject *parent )
  : FilterActionWithAddress( "set Reply-To", i18n( "Set Reply-To To" ), parent )
{
  mParameter.clear();
}

FilterAction::ReturnCode FilterActionReplyTo::process( ItemContext &context ) const
{
  const KMime::Message::Ptr msg = context.item().payload<KMime::Message::Ptr>();
  KMime::Headers::Generic *header = new KMime::Headers::Generic( "Reply-To", msg.get(), mParameter, "utf-8" );
  msg->setHeader( header );
  msg->assemble();

  context.setNeedsPayloadStore();

  return GoOn;
}
