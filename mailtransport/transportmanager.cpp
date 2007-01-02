/*
    Copyright (c) 2006 Volker Krause <vkrause@kde.org>

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

#include "mailtransport_defs.h"
#include "transport.h"
#include "transportmanager.h"
#include "smtpjob.h"
#include "sendmailjob.h"

#include <kconfig.h>
#include <kconfigbase.h>
#include <kdebug.h>
#include <krandom.h>
#include <kstaticdeleter.h>
#include <kwallet.h>

#include <QApplication>
#include <QDBusConnection>
#include <QRegExp>
#include <QStringList>

using namespace KPIM;
using namespace KWallet;

TransportManager* TransportManager::mInstance = 0;
static KStaticDeleter<TransportManager> sTransportManagerDeleter;

TransportManager::TransportManager() :
    QObject(),
    mMyOwnChange( false ),
    mWallet( 0 ),
    mWalletOpenFailed( false ),
    mWalletAsyncOpen( false ),
    mDefaultTransportId( -1 )
{
  mConfig = new KConfig( "mailtransports" );
  readConfig();

  QDBusConnection::sessionBus().registerObject( DBUS_OBJECT_PATH, this,
      QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportScriptableSignals );

  QDBusConnection::sessionBus().connect( QString(), QString(), DBUS_INTERFACE_NAME, "changesCommitted",
                                         this, SLOT(slotTransportsChanged()) );
}

TransportManager::~TransportManager()
{
  delete mConfig;
  qDeleteAll( mTransports );
}

TransportManager* TransportManager::self()
{
  if ( !mInstance )
    sTransportManagerDeleter.setObject( mInstance, new TransportManager() );
  return mInstance;
}

Transport* TransportManager::transportById(int id, bool def) const
{
  foreach ( Transport* t, mTransports )
    if ( t->id() == id )
      return t;

  if ( def || id == 0 )
    return transportById( mDefaultTransportId, false );
  return 0;
}

Transport* TransportManager::transportByName(const QString & name, bool def) const
{
  foreach ( Transport* t, mTransports )
    if ( t->name() == name )
      return t;
  if ( def )
    return transportById( 0, false );
  return 0;
}

QList< Transport * > TransportManager::transports() const
{
  return mTransports;
}

Transport* TransportManager::createTransport() const
{
  int id = createId();
  Transport *t = new Transport( QString::number( id ) );
  t->setId( id );
  return t;
}

void TransportManager::addTransport(Transport * transport)
{
  Q_ASSERT( !mTransports.contains( transport ) );
  mTransports.append( transport );
  validateDefault();
  emitChangesCommitted();
}

TransportJob* TransportManager::createTransportJob(int transportId)
{
  Transport *t = transportById( transportId, false );
  if ( !t )
    return 0;
  switch ( t->type() ) {
    case Transport::EnumType::SMTP:
      return new SmtpJob( t, this );
    case Transport::EnumType::Sendmail:
      return new SendmailJob( t, this );
  }
  Q_ASSERT( false );
  return 0;
}

bool KPIM::TransportManager::isEmpty() const
{
  return mTransports.isEmpty();
}

QList<int> TransportManager::transportIds() const
{
  QList<int> rv;
  foreach ( Transport *t, mTransports )
    rv << t->id();
  return rv;
}

QStringList TransportManager::transportNames() const
{
  QStringList rv;
  foreach ( Transport *t, mTransports )
    rv << t->name();
  return rv;
}

QString TransportManager::defaultTransportName() const
{
  Transport* t = transportById( mDefaultTransportId, false );
  if ( t )
    return t->name();
  return QString();
}

int TransportManager::defaultTransportId() const
{
  return mDefaultTransportId;
}

void TransportManager::setDefaultTransport(int id)
{
  if ( id == mDefaultTransportId || !transportById( id, false ) )
    return;
  mDefaultTransportId = id;
  writeConfig();
}

void TransportManager::removeTransport(int id)
{
  Transport *t = transportById( id, false );
  if ( !t )
    return;
  mTransports.removeAll( t );
  validateDefault();
  QString group = t->currentGroup();
  delete t;
  mConfig->deleteGroup( group );
  writeConfig();
}

void TransportManager::readConfig()
{
  qDeleteAll( mTransports );
  mTransports.clear();

  QRegExp re( "^Transport (.+)$" );
  QStringList groups = mConfig->groupList().filter( re );
  foreach ( QString s, groups ) {
    re.indexIn( s );
    Transport* t = new Transport( re.cap( 1 ) );
    if ( t->id() <= 0 ) {
      t->setId( createId() );
      t->writeConfig();
    }
    mTransports.append( t );
  }

  // read default transport
  KConfigGroup group( mConfig, "General" );
  mDefaultTransportId = group.readEntry( "default-transport", 0 );
  if ( mDefaultTransportId == 0 ) {
    // migrated default transport contains the name instead
    QString name = group.readEntry( "default-transport", QString() );
    if ( !name.isEmpty() ) {
      Transport *t = transportByName( name, false );
      if ( t ) {
        mDefaultTransportId = t->id();
        writeConfig();
      }
    }
  }
  validateDefault();
}

void TransportManager::writeConfig()
{
  KConfigGroup group( mConfig, "General" );
  group.writeEntry( "default-transport", mDefaultTransportId );
  mConfig->sync();
  emitChangesCommitted();
}

void TransportManager::emitChangesCommitted()
{
  mMyOwnChange = true; // prevent us from reading our changes again
  emit transportsChanged();
  emit changesCommitted();
}

void TransportManager::slotTransportsChanged()
{
  if ( mMyOwnChange ) {
    mMyOwnChange = false;
    return;
  }

  kDebug() << k_funcinfo << endl;
  mConfig->reparseConfiguration();
  // FIXME: this deletes existing transport objects!
  readConfig();
  emit transportsChanged();
}

int TransportManager::createId() const
{
  QList<int> usedIds;
  foreach ( Transport *t, mTransports )
    usedIds << t->id();
  usedIds << 0; // 0 is default for unknown
  int newId;
  do {
      newId = KRandom::random();
  } while ( usedIds.contains( newId )  );
  return newId;
}

KWallet::Wallet * TransportManager::wallet()
{
  if ( mWallet && mWallet->isOpen() )
    return mWallet;

  if ( !Wallet::isEnabled() || mWalletOpenFailed )
    return 0;

  WId window = 0;
  if ( qApp->activeWindow() )
    window = qApp->activeWindow()->winId();
  else if ( qApp->mainWidget() )
    window = qApp->mainWidget()->topLevelWidget()->winId();

  delete mWallet;
  mWallet = Wallet::openWallet( Wallet::NetworkWallet(), window );

  if ( !mWallet ) {
    mWalletOpenFailed = true;
    return 0;
  }

  prepareWallet();
  return mWallet;
}

void TransportManager::prepareWallet()
{
  if ( !mWallet )
    return;
  if ( !mWallet->hasFolder( WALLET_FOLDER ) )
    mWallet->createFolder( WALLET_FOLDER );
  mWallet->setFolder( WALLET_FOLDER );
}

void TransportManager::loadPasswords()
{
  foreach ( Transport *t, mTransports )
    t->readPassword();
  emit passwordsChanged();
}

void TransportManager::loadPasswordsAsync()
{
  // check if there is anything to do at all
  bool found = false;
  foreach ( Transport *t, mTransports ) {
    if ( !t->isComplete() ) {
      found = true;
      break;
    }
  }
  if ( !found )
    return;

  // async wallet opening
  if ( !mWallet && !mWalletOpenFailed ) {
    WId window = 0;
    if ( qApp->activeWindow() )
      window = qApp->activeWindow()->winId();
    else if ( qApp->mainWidget() )
      window = qApp->mainWidget()->topLevelWidget()->winId();
    mWallet = Wallet::openWallet( Wallet::NetworkWallet(), window, Wallet::Asynchronous );
    if ( mWallet ) {
      connect( mWallet, SIGNAL(walletOpened(bool)), SLOT(slotWalletOpened(bool)) );
      mWalletAsyncOpen = true;
    }
    else {
      mWalletOpenFailed = true;
      loadPasswords();
    }
    return;
  }
  if ( mWallet && !mWalletAsyncOpen )
    loadPasswords();
}

void TransportManager::slotWalletOpened( bool success )
{
  mWalletAsyncOpen = false;
  if ( !success ) {
    mWalletOpenFailed = true;
    delete mWallet;
    mWallet = 0;
  } else {
    prepareWallet();
  }
  loadPasswords();
}

void TransportManager::validateDefault()
{
  if ( !transportById( mDefaultTransportId, false ) ) {
    if ( isEmpty() ) {
      mDefaultTransportId = -1;
    } else {
      mDefaultTransportId = mTransports.first()->id();
      writeConfig();
    }
  }
}

#include "transportmanager.moc"
