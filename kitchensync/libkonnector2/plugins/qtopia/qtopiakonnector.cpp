/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2003 Holger Freyther <freyther@kde.org>
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kstringhandler.h>

#include <konnectorinfo.h>

#include "qtopiaconfig.h"
#include "socket.h"

#include "qtopiakonnector.h"

using namespace KSync;

class QtopiaKonnectorFactory : public KRES::PluginFactoryBase
{
  public:
    KRES::Resource *resource( const KConfig *config )
    {
      return new QtopiaKonnector( config );
    }

    KRES::ConfigWidget *configWidget( QWidget *parent )
    {
      return new OpieHelper::QtopiaConfig( parent );
    }
};

extern "C"
{
  void *init_libqtopiakonnector()
  {
    return new QtopiaKonnectorFactory();
  }
}


class QtopiaKonnector::Private
{
  public:
    Private()
      : socket( 0 )
    {
    }

    ~Private()
    {
      delete socket;
      socket = 0;
    }

    QtopiaSocket *socket;
};

QtopiaKonnector::QtopiaKonnector( const KConfig *cfg )
    : Konnector( cfg )
{
  if ( cfg ) {
    mDestinationIP = cfg->readEntry( "DestinationIP" );
    mUserName = cfg->readEntry( "UserName" );
    mPassword = KStringHandler::obscure( cfg->readEntry( "Password" ) );
    mModel = cfg->readEntry( "Model" );
    mModelName = cfg->readEntry( "ModelName" );
  }

  d = new Private;
  d->socket = new QtopiaSocket(this, "Opie Socket" );
  d->socket->setStoragePath( storagePath() );

  /* now do some signal and slot connection */
  connect( this, SIGNAL(storagePathChanged(const QString&)),
           d->socket, SLOT(setStoragePath(const QString&)) );
  connect( d->socket, SIGNAL( sync( SynceeList ) ),
           this, SLOT( slotSync( SynceeList ) ) );

  d->socket->setDestIP( mDestinationIP );
  d->socket->setUser( mUserName );
  d->socket->setPassword( mPassword );
  d->socket->setModel( mModel, mModelName );
}

QtopiaKonnector::~QtopiaKonnector()
{
  delete d;
  d = 0;
}

void QtopiaKonnector::writeConfig( KConfig *cfg )
{
  Konnector::writeConfig( cfg );

  cfg->writeEntry( "DestinationIP", mDestinationIP );
  cfg->writeEntry( "UserName", mUserName );
  cfg->writeEntry( "Password", KStringHandler::obscure( mPassword ) );
  cfg->writeEntry( "Model", mModel );
  cfg->writeEntry( "ModelName", mModelName );
}

SynceeList QtopiaKonnector::syncees()
{
  return mSynceeList;
}

bool QtopiaKonnector::readSyncees()
{
  d->socket->setResources( resources() );
  return d->socket->startSync();
}

bool QtopiaKonnector::connectDevice()
{
  d->socket->startUp();
  return true;
}

bool QtopiaKonnector::disconnectDevice()
{
  d->socket->hangUp();
  return true;
}

QString QtopiaKonnector::metaId() const
{
  return d->socket->metaId();
}

QIconSet QtopiaKonnector::iconSet() const
{
  QPixmap logo;
  logo.load( locate( "appdata", "pics/opie.png" ) );
  return QIconSet( logo );
}

QString QtopiaKonnector::iconName() const
{
  return QString::fromLatin1("opie.png");
}

/**
 * @internal
 * @reimplementation
 */
void QtopiaKonnector::appendSyncee( KSync::Syncee* syn)
{
  mSynceeList.append( syn );
}

bool QtopiaKonnector::writeSyncees()
{
  d->socket->write( mSynceeList );

  emit synceesWritten( this );

  return true;
}

/* private slots for communication here */
void QtopiaKonnector::slotSync( SynceeList list )
{
  mSynceeList = list;
  emit synceesRead( this );
}

KonnectorInfo QtopiaKonnector::info() const
{
  return KonnectorInfo( QString::fromLatin1("Qtopia Konnector"),
                        iconSet(),
                        QString::fromLatin1("Qtopia1.5"),
                        metaId(),
                        iconName(),
                        d->socket->isConnected() );
}

#include "qtopiakonnector.moc"
