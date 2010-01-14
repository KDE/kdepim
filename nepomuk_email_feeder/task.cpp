/*
    Copyright (c) 2009 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "task.h"

#include <gpgme++/context.h>
#include <gpgme++/key.h>
#include <gpgme++/vfsmountresult.h>

#include <kdebug.h>
#include <KStandardDirs>

#include <Soprano/BackendSetting>
#include <Soprano/Client/DBusClient>
#include <Soprano/Client/DBusModel>
#include <Soprano/Model>
#include <Soprano/PluginManager>
#include <Soprano/QueryResultIterator>

#include <QDir>
#include <QFile>

#include <boost/scoped_ptr.hpp>
#include <Nepomuk/ResourceManager>

Task::Task(QObject* parent) :
  QObject(parent),
  m_ctx( 0 ),
  m_cryptoModel( 0 )
{
}

Task::~Task()
{
  resetModel();
  unmountCryptoContainer();
}

bool Task::createCryptoContainer(const QByteArray& keyId)
{
  kDebug() << keyId;
  if ( keyId.isEmpty() )
    return false;

  const QString path = containerPathFromKeyId( keyId );
  if ( QFile::exists( path ) ) {
    kDebug() << "container exists already, nothing to do";
    return true;
  }

  std::vector<GpgME::Key> keys;
  {
    // TODO: we should get the real key from OTP maybe, which probably can handle CMS as well, not just PGP
    boost::scoped_ptr<GpgME::Context> ctx( GpgME::Context::createForProtocol( GpgME::OpenPGP ) );
    if ( !ctx )
      return false;
    GpgME::Error error;
    const GpgME::Key key = ctx->key( keyId.constData(), error );
    kDebug() << error.asString();
    if ( key.isNull() || key.isInvalid() )
      return false;
    keys.push_back( key );
  }

  delete m_ctx;
  m_ctx = GpgME::Context::createForEngine( GpgME::G13Engine ).release();
  if ( !m_ctx )
    return false;
  const GpgME::Error error = m_ctx->createVFS( path.toLocal8Bit(), keys );
  kDebug() << path << error.asString();
  return !error;
}

bool Task::mountCryptoContainer( const QByteArray& keyId )
{
  kDebug() << keyId;

  const QString cryptoContainer = containerPathFromKeyId( keyId );
  const QString mountDir = repositoryPathFromKeyId( keyId );
  KStandardDirs::makeDir( mountDir );

  // ### HACK temporary hack until G13 reports that as an error correctly
  // G13 currently fails silently if the mount target contains anything already, so check for that
  const QDir dir( mountDir );
  if ( !dir.entryList( QDir::AllEntries | QDir::NoDotAndDotDot ).isEmpty() ) {
    kWarning() << "Mount dir is not empty, can't mount there!" << dir.entryList( QDir::AllEntries | QDir::NoDotAndDotDot );
    return false;
  }

  if ( !m_ctx )
    m_ctx = GpgME::Context::createForEngine( GpgME::G13Engine ).release();
  if ( !m_ctx ) {
    kWarning() << "Unable to aquire GpgME context for G13";
    return false;
  }

  GpgME::VfsMountResult res = m_ctx->mountVFS( cryptoContainer.toLocal8Bit(), mountDir.toLocal8Bit() );
  kDebug() << res.mountDir() << res.error().asString() << res.error();

#ifdef Q_OS_UNIX
  // ### HACK temporary hack until the mount race is fixed in G13
  sleep( 1 );
#else
  // TODO check if similar hack needed for Windows as well
#endif

  kWarning( res.error() ) << res.error();
  return !res.error();
}

void Task::unmountCryptoContainer()
{
  kDebug( m_ctx != 0 ) << "Unmounting crypto container";
  delete m_ctx;
  m_ctx = 0;
}

QList< QByteArray > Task::listCryptoContainers()
{
  const QString containerBasePath = KStandardDirs::locateLocal( "data", "nepomuk/encrypted-repository" );
  const QDir dir( containerBasePath );
  QList<QByteArray> keys;
  foreach ( QString container, dir.entryList( QStringList() << QLatin1String( "*.g13" ), QDir::Files | QDir::Readable ) ) {
    container.chop( 4 );
    keys.append( container.toLatin1() );
  }
  return keys;
}

QString Task::containerPathFromKeyId(const QByteArray& keyId)
{
  Q_ASSERT( !keyId.isEmpty() );
  QString containerPath = KStandardDirs::locateLocal( "data", "nepomuk/encrypted-repository" );
  KStandardDirs::makeDir( containerPath, 0700 );
  containerPath += QDir::separator();
  containerPath += QString::fromLatin1( keyId );
  containerPath += QLatin1String(".g13");
  return containerPath;
}

QString Task::repositoryPathFromKeyId(const QByteArray& keyId)
{
  Q_ASSERT( !keyId.isEmpty() );
  return KStandardDirs::locateLocal( "data", "nepomuk/repository/" ) + QLatin1String( keyId );
}


Soprano::Model* Task::cryptoModel( const QByteArray &keyId )
{
  if ( m_cryptoModel )
    return m_cryptoModel;

  Soprano::Client::DBusClient dbusClient( "org.kde.nepomuk.services.nepomukstorage" );
  m_cryptoModel = dbusClient.createModel( keyId );
  if ( !m_cryptoModel ) {
    //  try again, the hard way, we are apparently using a Nepomuk server that lost support for multiple repoisitories :-/
    const Soprano::Backend* backend = Soprano::PluginManager::instance()->discoverBackendByName( "virtuosobackend" );
    Soprano::BackendSettings settings;
    settings.append( Soprano::BackendSetting( Soprano::BackendOptionStorageDir, repositoryPathFromKeyId( keyId ) ) );
    m_cryptoModel = backend->createModel( settings );
  }
  if ( m_cryptoModel ) {
    Nepomuk::ResourceManager::instance()->setOverrideMainModel( m_cryptoModel );
    return m_cryptoModel;
  }

  kWarning() << "Could not obtain cryto index model" << dbusClient.lastError();
  return 0;
}

void Task::resetModel()
{
  if ( !m_cryptoModel )
    return;

  Nepomuk::ResourceManager::instance()->setOverrideMainModel( 0 );
  delete m_cryptoModel;
  m_cryptoModel = 0;
#ifdef Q_OS_UNIX
  // ### HACK FIXME if we unmount the crypto container before virtuoso exits it might write to the normal filesystem
  kDebug() << "waiting for virtuoso to shut down";
  sleep( 5 );
#endif
}

#include "task.moc"
