/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/


#include <kmessagebox.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocale.h>
#include <kdebug.h>
#include <kwallet.h>
using namespace KWallet;

#include "knglobals.h"
#include "knserverinfo.h"
#include "knaccountmanager.h"
#include "utilities.h"

#include <kio/ioslave_defaults.h>


KNServerInfo::KNServerInfo() :
  i_d( -1 ), p_ort( DEFAULT_NNTP_PORT ),
  n_eedsLogon( false ), p_assDirty( false ),
  mPassLoaded( false ),
  mEncryption( None )
{
}



KNServerInfo::~KNServerInfo()
{
}


void KNServerInfo::readConf(KConfigGroup& conf)
{
  s_erver=conf.readEntry("server", "localhost");
  p_ort=conf.readEntry( "port", DEFAULT_NNTP_PORT );
  i_d=conf.readEntry("id", -1);

  n_eedsLogon=conf.readEntry("needsLogon",false);
  u_ser=conf.readEntry("user");
  p_ass = KNHelper::decryptStr(conf.readEntry("pass"));

  // migration to KWallet
  if ( Wallet::isEnabled() ) {
    if ( !p_ass.isEmpty() ) {
      conf.deleteEntry( "pass" );
      p_assDirty = true;
    }
  } else
    mPassLoaded = true;

  // if the wallet is open, no need to delay the password loading
  if (Wallet::isOpen( Wallet::NetworkWallet() ))
    readPassword();

  QString encStr = conf.readEntry( "encryption", "None" );
  if ( encStr.contains( "SSL", Qt::CaseInsensitive ) )
    mEncryption = SSL;
  else if ( encStr.contains( "TLS", Qt::CaseInsensitive ) )
    mEncryption = TLS;
  else
    mEncryption = None;
}


void KNServerInfo::saveConf(KConfigGroup &conf)
{
  conf.writeEntry("server", s_erver);
  if ( p_ort == 0 ) p_ort = DEFAULT_NNTP_PORT;
  conf.writeEntry("port", p_ort);
  conf.writeEntry("id", i_d);

  conf.writeEntry("needsLogon", n_eedsLogon);
  conf.writeEntry("user", u_ser);
  // open wallet for storing only if the user actually changed the password
  if (n_eedsLogon && p_assDirty) {
    Wallet *wallet = KNAccountManager::wallet();
    if (!wallet || wallet->writePassword(QString::number(i_d), p_ass)) {
      if ( KMessageBox::warningYesNo( 0,
            i18n("KWallet is not available. It is strongly recommended to use "
                "KWallet for managing your passwords.\n"
                "However, KNode can store the password in its configuration "
                "file instead. The password is stored in an obfuscated format, "
                "but should not be considered secure from decryption efforts "
                "if access to the configuration file is obtained.\n"
                "Do you want to store the password for server '%1' in the "
                "configuration file?", server() ),
            i18n("KWallet Not Available"),
            KGuiItem( i18n("Store Password") ),
            KGuiItem( i18n("Do Not Store Password") ) )
            == KMessageBox::Yes ) {
        conf.writeEntry( "pass", KNHelper::encryptStr( p_ass ) );
      }
    }
    p_assDirty = false;
  }

  switch ( mEncryption ) {
    case SSL:
      conf.writeEntry( "encryption", "SSL" );
      break;
    case TLS:
      conf.writeEntry( "encryption", "TLS" );
      break;
    default:
      conf.writeEntry( "encryption", "None" );
  }
}



bool KNServerInfo::operator==(const KNServerInfo &s)
{
  return (  (s_erver==s.s_erver) &&
            (p_ort==s.p_ort) &&
            (n_eedsLogon==s.n_eedsLogon) &&
            (u_ser==s.u_ser) &&
            (p_ass==s.p_ass) &&
            (mEncryption == s.mEncryption)
         );
}


const QString &KNServerInfo::pass()
{
  // if we need to load the password, load all of them
  if (n_eedsLogon && !mPassLoaded && p_ass.isEmpty() )
    knGlobals.accountManager()->loadPasswords();

  return p_ass;
}

void KNServerInfo::setPass(const QString &s)
{
  if (p_ass != s) {
    p_ass = s;
    p_assDirty = true;
  }
}


void KNServerInfo::readPassword()
{
  // no need to load a password if the account doesn't require auth
  if (!n_eedsLogon)
    return;
  mPassLoaded = true;

  // check whether there is a chance to find our password at all
  if (Wallet::folderDoesNotExist(Wallet::NetworkWallet(), "knode") ||
      Wallet::keyDoesNotExist(Wallet::NetworkWallet(), "knode", QString::number(i_d)))
    return;

  // finally try to open the wallet and read the password
  KWallet::Wallet *wallet = KNAccountManager::wallet();
  if ( wallet )
    wallet->readPassword( QString::number(i_d), p_ass );
}
