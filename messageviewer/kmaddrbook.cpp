/* -*- mode: C++; c-file-style: "gnu" -*-
 * kmail: KDE mail client
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "kmaddrbook.h"

#include <Akonadi/Contact/ContactSearchJob>
#include <KABC/Addressee>

#include <KDebug>

using namespace MessageViewer;

QString KabcBridge::expandNickName( const QString& nickName )
{
  if ( nickName.isEmpty() )
    return QString();

  const QString lowerNickName = nickName.toLower();

  Akonadi::ContactSearchJob *job = new Akonadi::ContactSearchJob();
  job->setQuery( Akonadi::ContactSearchJob::NickName, lowerNickName );
  if ( !job->exec() )
    return QString();

  const KABC::Addressee::List contacts = job->contacts();
  foreach ( const KABC::Addressee &contact, contacts ) {
    if ( contact.nickName().toLower() == lowerNickName )
      return contact.fullEmail();
  }

  return QString();
}
