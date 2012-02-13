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

#include "filteractionsetstatus.h"

#include <KDE/Akonadi/KMime/MessageStatus>
#include <KDE/KLocale>

using namespace MailCommon;

FilterAction* FilterActionSetStatus::newAction()
{
  return new FilterActionSetStatus;
}

FilterActionSetStatus::FilterActionSetStatus( QObject *parent )
  : FilterActionStatus( "set status", i18n( "Mark As" ), parent )
{
}



FilterAction::ReturnCode FilterActionSetStatus::process( ItemContext &context ) const
{
  const int index = mParameterList.indexOf( mParameter );
  if ( index < 1 )
    return ErrorButGoOn;

  Akonadi::MessageStatus status;
  status.setStatusFromFlags( context.item().flags() );

  const Akonadi::MessageStatus newStatus = FilterActionStatus::stati[ index - 1 ];
  if ( newStatus == Akonadi::MessageStatus::statusUnread() )
    status.setRead( false );
  else
    status.set( newStatus );

  context.item().setFlags( status.statusFlags() );
  context.setNeedsFlagStore();

  return GoOn;
}


