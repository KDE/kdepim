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
#include <libqopensync/member.h>
#include <libqopensync/result.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "syncprocess.h"
#include "syncprocessmanager.h"

using namespace QSync;

SyncProcess::SyncProcess( const QSync::Group &group )
  : QObject( 0 )
{
  setObjectName( "SyncProcess" );
  mGroup = group;
  mEngine = new QSync::Engine( mGroup );

  Result result = mEngine->initialize();
  if ( result.isError() ) {
    kDebug(5200) <<"SyncProcess::SyncProcess:" << result.message();
  }
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

QString SyncProcess::memberStatus( const QSync::Member &member ) const
{
  Q_UNUSED( member );
  return i18n( "Ready" );
}

QSync::Result SyncProcess::addMember( const QSync::Plugin &plugin )
{
  QSync::Member member = mGroup.addMember( plugin );
  QSync::Result result = member.instance();

  if ( !result.isError() ) {
    mGroup.save();
  }

  return result;
}

void SyncProcess::removeMember( const QSync::Member &member )
{
  member.cleanup();
  mGroup.removeMember( member );
  mGroup.save();
}


void SyncProcess::reinitEngine()
{
  mEngine->finalize();
  delete mEngine;
  mEngine = new QSync::Engine( mGroup );
  Result result = mEngine->initialize();
  if ( result.isError() ) {
    kDebug(5200) <<"SyncProcess::reinitEngine:" << result.message();
    KMessageBox::error( 0, i18n( "Error initializing Synchronization Engine for group \"%1\":\n %2",
                                 mGroup.name(), result.message() ) );
  }

  applyObjectTypeFilter();

  emit engineChanged( mEngine );
}

void SyncProcess::applyObjectTypeFilter()
{
//   const QSync::Conversion conversion = SyncProcessManager::self()->environment()->conversion();
//   const QStringList objectTypes = conversion.objectTypes();

  const QStringList objectTypes;
  const QStringList activeObjectTypes = mGroup.config().activeObjectTypes();

  for ( int i = 0; i < objectTypes.count(); ++i ) {
    if ( activeObjectTypes.contains( objectTypes[ i ] ) ) {
      kDebug(5200) <<"Enabled object type:" <<  objectTypes[ i ];
      /*
       * This is not required. Also this lead to filtering problems when sync with "file-sync".
       * Uncomment this line again when OpenSync is fixed!
       *
       * mGroup.setObjectTypeEnabled( objectTypes[ i ], true );
       */
    } else {
      kDebug(5200) <<"Disabled object type:" <<  objectTypes[ i ];
      mGroup.setObjectTypeEnabled( objectTypes[ i ], false );
    }
  }
}

#include "syncprocess.moc"
