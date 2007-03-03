/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <libqopensync/engine.h>
#include <libqopensync/environment.h>

#include <kdebug.h>
#include <klocale.h>

#include "syncprocess.h"
#include "syncprocessmanager.h"

using namespace QSync;

SyncProcess::SyncProcess( const QSync::Group &group )
  : QObject( 0, "SyncProcess" )
{
  mGroup = group;
  mEngine = new QSync::Engine( mGroup );

  Result result = mEngine->initialize();
  if ( result.isError() )
    kdDebug() << "SyncProcess::SyncProcess: " << result.message() << endl;
}

SyncProcess::~SyncProcess()
{
  mEngine->finalize();

  delete mEngine;
  mEngine = 0;
}

QString SyncProcess::groupStatus() const
{
  return i18n( "Ready" );
}

QString SyncProcess::memberStatus( const QSync::Member& ) const
{
  return i18n( "Ready" );
}

QSync::Result SyncProcess::addMember( const QSync::Plugin &plugin )
{
  QSync::Member member = mGroup.addMember();
  QSync::Result result = member.instance( plugin );

  if ( !result.isError() )
    mGroup.save();

  return result;
}

void SyncProcess::reinitEngine()
{
  mEngine->finalize();
  delete mEngine;
  mEngine = new QSync::Engine( mGroup );
  Result result = mEngine->initialize();
  if ( result.isError() )
    kdDebug() << "SyncProcess::reinitEngine: " << result.message() << endl;

  applyObjectTypeFilter();

  emit engineChanged( mEngine );
}

void SyncProcess::applyObjectTypeFilter()
{
  const QSync::Conversion conversion = SyncProcessManager::self()->environment()->conversion();
  const QStringList objectTypes = conversion.objectTypes();
  const QStringList activeObjectTypes = mGroup.config().activeObjectTypes();

  for ( uint i = 0; i < objectTypes.count(); ++i ) {
    if ( activeObjectTypes.contains( objectTypes[ i ] ) ) {
      kdDebug() << "Enabled object type: " <<  objectTypes[ i ] << endl;
      /*
       * This is not required. Also this lead to filtering problems when sync with "file-sync".
       * Uncomment this line again when OpenSync is fixed!
       *
       * mGroup.setObjectTypeEnabled( objectTypes[ i ], true );
       */
    } else {
      kdDebug() << "Disabled object type: " <<  objectTypes[ i ] << endl;
      mGroup.setObjectTypeEnabled( objectTypes[ i ], false );
    }
  }
}

#include "syncprocess.moc"
