/*
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>

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
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <krandom.h>
#include <kstaticdeleter.h>
#include <kurl.h>
#include <kwallet.h>

#include <QApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
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

  QDBusConnection::sessionBus().registerObject( DBUS_OBJECT_PATH, this,
      QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportScriptableSignals );

  QDBusConnection::sessionBus().connect( QString(), QString(), DBUS_INTERFACE_NAME, "changesCommitted",
                                         this, SLOT(slotTransportsChanged()) );

  mIsMainInstance = QDBusConnection::sessionBus().registerService( DBUS_SERVICE_NAME );
  connect( QDBusConnection::sessionBus().interface(), SIGNAL(serviceOwnerChanged(QString,QString,QString)),
           SLOT(dbusServiceOwnerChanged(QString,QString,QString)) );
}

TransportManager::~TransportManager()
{
  delete mConfig;
  qDeleteAll( mTransports );
}

TransportManager* TransportManager::self()
{
  if ( !mInstance ) {
    sTransportManagerDeleter.setObject( mInstance, new TransportManager() );
    mInstance->readConfig();
  }
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

void TransportManager::schedule(TransportJob * job)
{
  connect( job, SIGNAL(result(KJob*)), SLOT(jobResult(KJob*)) );

  // check if the job is waiting for the wallet
  if ( !job->transport()->isComplete() ) {
    kDebug() << k_funcinfo << "job waits for wallet: " << job << endl;
    mWalletQueue << job;
    loadPasswordsAsync();
    return;
  }

  job->start();
}

TransportJob* TransportManager::createTransportJob( int transportId )
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

TransportJob* TransportManager::createTransportJob(const QString & transport)
{
  bool ok = false;
  Transport *t = 0;

  int transportId = transport.toInt( &ok );
  if ( ok )
    t = transportById( transportId );

  if ( !t )
    t = transportByName( transport, false );

  if ( t )
    return createTransportJob( t->id() );

  KUrl url( transport );
  if ( !url.isValid() )
    return 0;

  t = new Transport( "adhoc" );
  t->setDefaults();
  t->setName( transport );
  t->setAdHoc( true );

  if ( url.protocol() == SMTP_PROTOCOL || url.protocol() == SMTPS_PROTOCOL ) {
    t->setType( Transport::EnumType::SMTP );
    t->setHost( url.host() );
    if ( url.protocol() == SMTPS_PROTOCOL ) {
      t->setEncryption( Transport::EnumEncryption::SSL );
      t->setPort( SMTPS_PORT );
    }
    if ( url.hasPort() )
      t->setPort( url.port() );
    if ( url.hasUser() ) {
      t->setRequiresAuthentication( true );
      t->setUserName( url.user() );
    }
  }

  else if ( url.protocol() == "file" ) {
    t->setType( Transport::EnumType::Sendmail );
    t->setHost( url.path( KUrl::RemoveTrailingSlash ) );
  }

  else {
    delete t;
    return 0;
  }

  switch ( t->type() ) {
    case Transport::EnumType::SMTP:
      return new SmtpJob( t, this );
    case Transport::EnumType::Sendmail:
      return new SendmailJob( t, this );
  }

  delete t;
  Q_ASSERT( false );
  return 0;
}

bool TransportManager::isEmpty() const
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
  QList<Transport*> oldTransports = mTransports;
  mTransports.clear();

  QRegExp re( "^Transport (.+)$" );
  QStringList groups = mConfig->groupList().filter( re );
  foreach ( QString s, groups ) {
    re.indexIn( s );
    Transport *t = 0;

    // see if we happen to have that one already
    foreach ( Transport *old, oldTransports ) {
      if ( old->currentGroup() == "Transport " + re.cap( 1 ) ) {
        kDebug() << k_funcinfo << "reloading existing transport: " << s << endl;
        t = old;
        t->readConfig();
        oldTransports.removeAll( old );
        break;
      }
    }

    if ( !t )
      t = new Transport( re.cap( 1 ) );
    if ( t->id() <= 0 ) {
      t->setId( createId() );
      t->writeConfig();
    }
    mTransports.append( t );
  }

  qDeleteAll( oldTransports );
  oldTransports.clear();

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
  migrateToWallet();
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

  // flush the wallet queue
  foreach ( TransportJob *job, mWalletQueue ) {
    job->start();
  }
  mWalletQueue.clear();

  emit passwordsChanged();
}

void TransportManager::loadPasswordsAsync()
{
  kDebug() << k_funcinfo << endl;

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
  kDebug() << k_funcinfo << endl;
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

void TransportManager::migrateToWallet()
{
  // check if we tried this already
  static bool firstRun = true;
  if ( !firstRun )
    return;
  firstRun = false;

  // check if we are the main instance
  if ( !mIsMainInstance )
    return;

  // check if migration is needed
  QStringList names;
  foreach ( Transport *t, mTransports )
    if ( t->needsWalletMigration() )
      names << t->name();
  if ( names.isEmpty() )
    return;

  // ask user if he wants to migrate
  int result = KMessageBox::questionYesNoList( 0,
    i18n("The following mail transports store passwords in the configuration file instead in KWallet.\n"
         "It is recommended to use KWallet for password storage for security reasons.\n"
         "Do you want to migrate your passwords to KWallet?"),
    names );
  if ( result != KMessageBox::Yes )
    return;

  // perform migration
  foreach ( Transport *t, mTransports )
    if ( t->needsWalletMigration() )
      t->migrateToWallet();
}

void TransportManager::dbusServiceOwnerChanged(const QString & service, const QString & oldOwner, const QString & newOwner)
{
  Q_UNUSED( oldOwner );
  if ( service == DBUS_SERVICE_NAME && newOwner.isEmpty() )
    QDBusConnection::sessionBus().registerService( DBUS_SERVICE_NAME );
}

void TransportManager::jobResult(KJob * job)
{
  mWalletQueue.removeAll( static_cast<TransportJob*>( job ) );
}

#include "transportmanager.moc"
