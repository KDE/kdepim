/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "actionpartservice.h"

#include <kdebug.h>
#include <ktrader.h>

using namespace KSync;

bool ActionPartService::mAvailablePartsRead = false;
ActionPartService::List ActionPartService::mAvailableParts;

const ActionPartService::List &ActionPartService::availableParts()
{
  if ( !mAvailablePartsRead ) {
    KTrader::OfferList offers = KTrader::self()->query(
        QString::fromLatin1("KitchenSync/ActionPart"), QString::null );

    KTrader::OfferList::ConstIterator it;
    for ( it = offers.begin(); it != offers.end(); ++it ) {
      kdDebug() << "DESKTOP: " << (*it)->desktopEntryName() << endl;
      ActionPartService ser( *it );
      mAvailableParts.append( ser );
    }

    mAvailablePartsRead = true;
  }
  
  return mAvailableParts;
}

ActionPartService ActionPartService::partForId( const QString &id )
{
  availableParts();
  
  ActionPartService::List::ConstIterator it;
  for( it = mAvailableParts.begin(); it != mAvailableParts.end(); ++it ) {
    kdDebug() << "id: " << (*it).id() << endl;

    if ( (*it).id() == id ) return *it;
  }
  
  kdDebug() << "ActionPartService: No part for name '" << id << "'" << endl;

  return ActionPartService();
}

ActionPartService::ActionPartService()
{
}

ActionPartService::ActionPartService( const KService::Ptr &service )
  : m_id( service->desktopEntryName() ), m_name( service->name() ),
    m_comment( service->comment() ),
    m_iconName( service->icon() ), m_libName( service->library() )
{
  kdDebug() << "xx: " << m_id << endl;
}

ActionPartService::~ActionPartService()
{
}

QString ActionPartService::name() const
{
  return m_name;
}

QString ActionPartService::id() const
{
  return m_id;
}

QString ActionPartService::comment() const
{
  return m_comment;
}

QString ActionPartService::libraryName() const
{
  return m_libName;
}

QString ActionPartService::iconName() const
{
  return m_iconName;
}

void ActionPartService::setId( const QString &id )
{
  m_id = id;
}

void ActionPartService::setName( const QString &name )
{
  m_name = name;
}

void ActionPartService::setComment( const QString &comment )
{
  m_comment = comment;
}

void ActionPartService::setLibraryName( const QString &libName )
{
  m_libName = libName;
}

void ActionPartService::setIconName( const QString &icon )
{
  m_iconName = icon;
}

ActionPartService &ActionPartService::operator=( const ActionPartService &man1 )
{
  m_name = man1.m_name;
  m_comment = man1.m_comment;
  m_iconName = man1.m_iconName;
  m_libName = man1.m_libName;
  return *this;
}

bool ActionPartService::operator== ( const ActionPartService &par2 )
{
  return name() == par2.name();
}

bool ActionPartService::operator== ( const ActionPartService &par2 ) const
{
  return name() == par2.name();
}
