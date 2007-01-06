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

#include "transport.h"
#include "transportmanager.h"
#include "mailtransport_defs.h"

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstringhandler.h>
#include <kwallet.h>

using namespace KPIM;
using namespace KWallet;

Transport::Transport( const QString &cfgGroup ) :
    TransportBase( cfgGroup ),
    mPasswordLoaded( false ),
    mPasswordDirty( false ),
    mStorePasswordInFile( false ),
    mNeedsWalletMigration( false ),
    mIsAdHoc( false )
{
  kDebug() << k_funcinfo << cfgGroup << endl;
  readConfig();
}

bool Transport::isValid() const
{
  return (id() > 0 || isAdHoc()) && !host().isEmpty() && port() <= 65536;
}

QString Transport::password()
{
  if ( !mPasswordLoaded && requiresAuthentication() && storePassword() && mPassword.isEmpty() )
    TransportManager::self()->loadPasswords();
  return mPassword;
}

void Transport::setPassword(const QString & passwd)
{
  mPasswordLoaded = true;
  if ( mPassword == passwd )
    return;
  mPasswordDirty = true;
  mPassword = passwd;
}

bool Transport::isComplete() const
{
  return !requiresAuthentication() || !storePassword() || mPasswordLoaded;
}

QString Transport::authenticationTypeString() const
{
  switch ( authenticationType() ) {
    case EnumAuthenticationType::LOGIN: return QLatin1String( "LOGIN" );
    case EnumAuthenticationType::PLAIN: return QLatin1String( "PLAIN" );
    case EnumAuthenticationType::CRAM_MD5: return QLatin1String( "CRAM-MD5" );
    case EnumAuthenticationType::DIGEST_MD5: return QLatin1String( "DIGEST-MD5" );
    case EnumAuthenticationType::NTLM: return QLatin1String( "NTLM" );
    case EnumAuthenticationType::GSSAPI: return QLatin1String( "GSSAPI" );
  }
  Q_ASSERT( false );
  return QString();
}

void Transport::usrReadConfig()
{
  TransportBase::usrReadConfig();

  // we have everything we need
  if ( !storePassword() || mPasswordLoaded )
    return;

  // try to find a password in the config file otherwise
  KConfigGroup group( config(), currentGroup() );
  mPassword = KStringHandler::obscure( group.readEntry( "password" ) );
  // TODO legacy password migration from KMail/KNode

  if ( !mPassword.isEmpty() ) {
    mPasswordLoaded = true;
    if ( Wallet::isEnabled() ) {
      mNeedsWalletMigration = true;
    } else {
      mStorePasswordInFile = true;
    }
  } else {
    // read password if wallet is open, defer otherwise
    if ( Wallet::isOpen( Wallet::NetworkWallet() ) )
      readPassword();
  }
}

void Transport::usrWriteConfig()
{
  if ( isAdHoc() )
    return;

  if ( requiresAuthentication() && storePassword() && mPasswordDirty ) {
    Wallet *wallet = TransportManager::self()->wallet();
    if ( !wallet || wallet->writePassword(QString::number(id()), mPassword) != 0 ) {
      // wallet saving failed, ask if we should store in the config file instead
      if ( mStorePasswordInFile || KMessageBox::warningYesNo( 0,
            i18n("KWallet is not available. It is strongly recommended to use "
                "KWallet for managing your passwords.\n"
                "However, the password can be stored in the configuration "
                "file instead. The password is stored in an obfuscated format, "
                "but should not be considered secure from decryption efforts "
                "if access to the configuration file is obtained.\n"
                "Do you want to store the password for server '%1' in the "
                "configuration file?", name() ),
            i18n("KWallet Not Available"),
            KGuiItem( i18n("Store Password") ),
            KGuiItem( i18n("Do Not Store Password") ) )
            == KMessageBox::Yes ) {
        // write to config file
        KConfigGroup group( config(), currentGroup() );
        group.writeEntry( "password", KStringHandler::obscure( mPassword ) );
        mStorePasswordInFile = true;
      }
    }
    mPasswordDirty = false;
  }

  TransportBase::usrWriteConfig();
  TransportManager::self()->emitChangesCommitted();
}

void Transport::readPassword()
{
  // no need to load a password if the account doesn't require auth
  if ( !requiresAuthentication() )
    return;
  mPasswordLoaded = true;

  // check wether there is a chance to find our password at all
  if ( Wallet::folderDoesNotExist(Wallet::NetworkWallet(), WALLET_FOLDER) ||
      Wallet::keyDoesNotExist(Wallet::NetworkWallet(), WALLET_FOLDER, QString::number(id())) )
  {
    // try migrating password from kmail
    if ( Wallet::folderDoesNotExist(Wallet::NetworkWallet(), "kmail") ||
         Wallet::keyDoesNotExist(Wallet::NetworkWallet(), "kmail", QString::fromLatin1("transport-%1").arg( id() ) ) )
      return;
    kDebug() << k_funcinfo << "migrating password from kmail wallet" << endl;
    KWallet::Wallet *wallet = TransportManager::self()->wallet();
    if ( wallet ) {
      wallet->setFolder( "kmail" );
      wallet->readPassword( QString::fromLatin1("transport-%1").arg( id() ), mPassword );
      wallet->removeEntry( QString::fromLatin1("transport-%1").arg( id() ) );
      wallet->setFolder( WALLET_FOLDER );
      mPasswordDirty = true;
      writeConfig();
    }
    return;
  }

  // finally try to open the wallet and read the password
  KWallet::Wallet *wallet = TransportManager::self()->wallet();
  if ( wallet )
    wallet->readPassword( QString::number(id()), mPassword );
}

bool Transport::needsWalletMigration() const
{
  return mNeedsWalletMigration;
}

void Transport::migrateToWallet()
{
  kDebug() << k_funcinfo << "migrating " << id() << " to wallet" << endl;
  mNeedsWalletMigration = false;
  KConfigGroup group( config(), currentGroup() );
  group.deleteEntry( "password" );
  mPasswordDirty = true;
  mStorePasswordInFile = false;
  writeConfig();
}

bool Transport::isAdHoc() const
{
  return mIsAdHoc;
}

void Transport::setAdHoc(bool b)
{
  mIsAdHoc = b;
}
