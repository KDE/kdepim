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
#include <kapabilities.h>

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
    Private() : socket( 0 ) {}

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
  connect( d->socket, SIGNAL( sync( SynceeList ) ),
           SLOT( slotSync( SynceeList ) ) );
  connect( d->socket, SIGNAL( error( const Error & ) ),
           SLOT( slotError( const Error & ) ) );
  connect( d->socket, SIGNAL( prog( const Progress & ) ),
           SLOT( slotProg( const Progress & ) ) );
  connect( this, SIGNAL(storagePathChanged(const QString&)),
           d->socket, SLOT(setStoragePath(const QString&)) );

  d->socket->setDestIP( mDestinationIP );
  d->socket->setUser( mUserName );
  d->socket->setPassword( mPassword );
  d->socket->setModel( mModel, mModelName );

  d->socket->startUp();
}

QtopiaKonnector::~QtopiaKonnector()
{
  delete d;
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

Kapabilities QtopiaKonnector::capabilities()
{
  Kapabilities caps;
  caps.setSupportMetaSyncing( true );
  caps.setSupportsPushSync( true );
  caps.setNeedsConnection( true );
  caps.setSupportsListDir( true );
  caps.setNeedsIPs( true );
  caps.setNeedsSrcIP( false );
  caps.setNeedsDestIP( true );
  caps.setAutoHandle( false );
  caps.setNeedAuthentication( true );

  QValueList<QPair<QString, QString> > user;
  user.append(qMakePair(QString::fromLatin1("root"), QString::fromLatin1("rootme") ) );
  caps.setUserProposals( user );

  QStringList ips;
  ips << "1.1.1.1";
  caps.setIpProposals( ips );

  // Model Stuff
  QStringList models;
  models << "Opie and Qtopia 1.6" << "Sharp Zaurus ROM";
  caps.setModels( models );
  caps.setNeedsModelName( true );

  return caps;
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
  d->socket->hangUP();
  return true;
}

QString QtopiaKonnector::metaId() const
{
  return d->socket->metaId();
}

QIconSet QtopiaKonnector::iconSet() const
{
  kdDebug(5225) << "iconSet" << endl;
  QPixmap logo;
  logo.load( locate( "appdata", "pics/opie.png" ) );
  return QIconSet( logo );
}

QString QtopiaKonnector::iconName() const
{
  return QString::fromLatin1("opie.png");
}

bool QtopiaKonnector::writeSyncees()
{
  kdDebug(5201) << " writing it now " << endl;
  d->socket->write( mSynceeList );
  return true;
}

/* private slots for communication here */
void QtopiaKonnector::slotSync( SynceeList list )
{
  mSynceeList = list;
  emit synceesRead( this );
}

void QtopiaKonnector::slotError( const Error& err )
{
  error( err );
}

void QtopiaKonnector::slotProg( const Progress& prog )
{
  progress( prog );
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
