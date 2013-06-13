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

#include "filteractionwithaddress.h"

#include <messagecore/widgets/emailaddressrequester.h>

using namespace MailCommon;

FilterActionWithAddress::FilterActionWithAddress( const char *name, const QString &label, QObject *parent )
  : FilterActionWithString( name, label, parent )
{
}

QWidget* FilterActionWithAddress::createParamWidget( QWidget *parent ) const
{
  MessageCore::EmailAddressRequester *requester = new MessageCore::EmailAddressRequester( parent );
  requester->setText( mParameter );

  connect( requester, SIGNAL(textChanged()), this, SIGNAL(filterActionModified()) );

  return requester;
}

void FilterActionWithAddress::applyParamWidgetValue( QWidget *paramWidget )
{
  mParameter = qobject_cast<MessageCore::EmailAddressRequester*>( paramWidget )->text();
}

void FilterActionWithAddress::setParamWidgetValue( QWidget *paramWidget ) const
{
  qobject_cast<MessageCore::EmailAddressRequester*>( paramWidget )->setText( mParameter );
}

void FilterActionWithAddress::clearParamWidget( QWidget *paramWidget ) const
{
  qobject_cast<MessageCore::EmailAddressRequester*>( paramWidget )->clear();
}

#include "filteractionwithaddress.moc"
