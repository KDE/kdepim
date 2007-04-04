/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "syncprocessmanager.h"

#include "syncprocess.h"

#include <libqopensync/environment.h>

#include <kstaticdeleter.h>
#include <kmessagebox.h>
#include <klocale.h>

static KStaticDeleter<SyncProcessManager> selfDeleter;

SyncProcessManager *SyncProcessManager::mSelf = 0;

SyncProcessManager *SyncProcessManager::self()
{
  if ( !mSelf ) {
    selfDeleter.setObject( mSelf, new SyncProcessManager() );
  }
  return mSelf;
}

SyncProcessManager::SyncProcessManager()
{
  mEnvironment = new QSync::Environment;
  QSync::Result result = mEnvironment->initialize();
  if ( result.isError() ) {
    KMessageBox::error( 0, i18n("Error initializing OpenSync.\n%1")
      .arg( result.message() ) );
  } else {
    init( mEnvironment );
  }
}

SyncProcessManager::~SyncProcessManager()
{
  QValueList<SyncProcess*>::Iterator it;
  for ( it = mProcesses.begin(); it != mProcesses.end(); ++it )
    delete *it;

  mProcesses.clear();

  mEnvironment->finalize();
  delete mEnvironment;
}

int SyncProcessManager::count() const
{
  return mProcesses.count();
}

SyncProcess* SyncProcessManager::at( int pos ) const
{
  if ( pos < 0 || pos >= (int)mProcesses.count() )
    return 0;

  return mProcesses[ pos ];
}

SyncProcess* SyncProcessManager::byGroup( const QSync::Group &group )
{
  QValueList<SyncProcess*>::Iterator it;
  for ( it = mProcesses.begin(); it != mProcesses.end(); ++it )
    if ( (*it)->group() == group )
      return *it;

  return 0;
}

SyncProcess* SyncProcessManager::byGroupName( const QString &name )
{
  QValueList<SyncProcess*>::Iterator it;
  for ( it = mProcesses.begin(); it != mProcesses.end(); ++it )
    if ( (*it)->group().name() == name )
      return *it;

  return 0;
}

void SyncProcessManager::addGroup( const QString &name )
{
  SyncProcess* process = byGroupName( name );
  if ( !process ) {
    QSync::Group group = mEnvironment->addGroup();
    group.setName( name );
    group.save();

    mProcesses.append( new SyncProcess( group ) );

    emit changed();
  } else
    qDebug( "Try to add duplicate" );
}

void SyncProcessManager::remove( SyncProcess *syncProcess )
{
  if ( syncProcess ) {
    mProcesses.remove( syncProcess );
    const QSync::Group group = syncProcess->group();
    delete syncProcess;

    mEnvironment->removeGroup( group );

    emit changed();
  }
}

void SyncProcessManager::init( QSync::Environment *environment )
{
  QSync::Environment::GroupIterator it( environment->groupBegin() );
  for ( ; it != environment->groupEnd(); ++it ) {
    /**
     * We check whether the group is valid before we append them
     * to mProcesses. That avoids crashes if the plugin of one of
     * the members isn't loaded (e.g. not installed).
     */
    const QSync::Group group = *it;
    int count = group.memberCount();

    bool isValid = true;
    for ( int i = 0; i < count; ++i ) {
      const QSync::Member member = group.memberAt( i );

      if ( !member.isValid() ) {
        isValid = false;
        break;
      }
    }

    if ( isValid )
      mProcesses.append( new SyncProcess( *it ) );
  }

  emit changed();
}

QSync::Result SyncProcessManager::addMember( SyncProcess *process,
                                             const QSync::Plugin &plugin )
{
  Q_ASSERT( process );

  QSync::Result result = process->addMember( plugin );
  if ( !result.isError() ) {
    process->group().save();
    emit syncProcessChanged( process );
  }

  return result;
}

#include "syncprocessmanager.moc"
