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

#include <QDir>
#include <QFile>

#include <boost/scoped_ptr.hpp>

Task::Task(QObject* parent) :
  QObject(parent),
  m_ctx( 0 )
{
}

Task::~Task()
{
  unmountCryptoContainer();
}

bool Task::createCryptoContainer(const QByteArray& keyId)
{
  kDebug() << keyId;
  if ( keyId.isEmpty() )
    return false;

  const QString path = containerPathFromKeyId( keyId );
  if ( QFile::exists( path ) )
    return true;

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
  m_ctx = GpgME::Context::createForProtocol( GpgME::G13 );
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
    m_ctx = GpgME::Context::createForProtocol( GpgME::G13 );
  if ( !m_ctx )
    return false;

  GpgME::VfsMountResult res = m_ctx->mountVFS( cryptoContainer.toLocal8Bit(), mountDir.toLocal8Bit() );
  kDebug() << res.mountDir() << res.error().asString() << res.error();

#ifdef Q_OS_UNIX
  // ### HACK temporary hack until the mount race is fixed in G13
  sleep( 1 );
#else
  // TODO check if similar hack needed for Windows as well
#endif

  return !res.error();
}

void Task::unmountCryptoContainer()
{
  kDebug( m_ctx != 0 ) << "Unmounting crypto container";
  delete m_ctx;
  m_ctx = 0;
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

#include "task.moc"
