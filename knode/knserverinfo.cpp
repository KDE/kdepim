/*
    knserverinfo.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2004 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/


#include <kmessagebox.h>
#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>
#include <kstaticdeleter.h>
#include <kwallet.h>
using namespace KWallet;

#include "knglobals.h"
#include "knserverinfo.h"
#include "knaccountmanager.h"
#include "utilities.h"

#include <qwidget.h>

KNServerInfo::KNServerInfo() :
  t_ype(STnntp), i_d(-1), p_ort(119), h_old(300),
  t_imeout(60), n_eedsLogon(false), p_assDirty(false),
  mPassLoaded( false )
{
}



KNServerInfo::~KNServerInfo()
{
}



void KNServerInfo::readConf(KConfig *conf)
{
  s_erver=conf->readEntry("server", "localhost");

  if(t_ype==STnntp)
    p_ort=conf->readNumEntry("port", 119);
  else
    p_ort=conf->readNumEntry("port", 25);

  h_old=conf->readNumEntry("holdTime", 300);

  if(h_old < 0) h_old=0;

  t_imeout=conf->readNumEntry("timeout", 60);

  if(t_imeout < 15) t_imeout=15;

  if(t_ype==STnntp) {
    i_d=conf->readNumEntry("id", -1);
    n_eedsLogon=conf->readBoolEntry("needsLogon",false);
    u_ser=conf->readEntry("user");
    p_ass = KNHelper::decryptStr(conf->readEntry("pass"));

    // migration to KWallet
    if (Wallet::isEnabled() && !p_ass.isEmpty()) {
      conf->deleteEntry( "pass" );
      p_assDirty = true;
    }

    // if the wallet is open, no need to delay the password loading
    if (Wallet::isOpen( Wallet::NetworkWallet() ))
      readPassword();
  }
}


void KNServerInfo::saveConf(KConfig *conf)
{
  conf->writeEntry("server", s_erver);
  if ( p_ort == 0 ) p_ort = 119;
  conf->writeEntry("port", p_ort);
  conf->writeEntry("holdTime", h_old);
  conf->writeEntry("timeout", t_imeout);
  if (t_ype==STnntp) {
    conf->writeEntry("id", i_d);
    conf->writeEntry("needsLogon", n_eedsLogon);
    conf->writeEntry("user", u_ser);
    // open wallet for storing only if the user actually changed the password
    if (n_eedsLogon && p_assDirty) {
      Wallet *wallet = KNServerInfo::wallet();
      if (!wallet || wallet->writePassword(QString::number(i_d), p_ass)) {
        if ( KMessageBox::warningYesNo( 0,
             i18n("KWallet is not available. It is strongly recommended to use "
                  "KWallet for managing your passwords.\n"
                  "However, KNode can store the password in its configuration "
                  "file instead. The password is stored in an obfuscated format, "
                  "but should not be considered secure from decryption efforts "
                  "if access to the configuration file is obtained.\n"
                  "Do you want to store the password for server '%1' in the "
                  "configuration file?").arg( server() ),
             i18n("KWallet Not Available"),
             KGuiItem( i18n("Store Password") ),
             KGuiItem( i18n("Do Not Store Password") ) )
             == KMessageBox::Yes ) {
          conf->writeEntry( "pass", KNHelper::encryptStr( p_ass ) );
        }
      }
      p_assDirty = false;
    }
  }
}



bool KNServerInfo::operator==(const KNServerInfo &s)
{
  return (  (t_ype==s.t_ype)  &&
            (s_erver==s.s_erver) &&
            (p_ort==s.p_ort) &&
            (h_old==s.h_old) &&
            (t_imeout==s.t_imeout) &&
            (n_eedsLogon==s.n_eedsLogon) &&
            (u_ser==s.u_ser) &&
            (p_ass==s.p_ass)            );
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


Wallet* KNServerInfo::mWallet = 0;

Wallet* KNServerInfo::wallet()
{
  static bool walletOpenFailed = false;
  if (mWallet && mWallet->isOpen())
    return mWallet;

  if (!Wallet::isEnabled() || walletOpenFailed)
    return 0;

  delete mWallet;
  static KStaticDeleter<Wallet> sd;
  if (knGlobals.top)
    sd.setObject( mWallet, Wallet::openWallet(Wallet::NetworkWallet(),
                  knGlobals.topWidget->topLevelWidget()->winId()) );
  else
    sd.setObject( mWallet, Wallet::openWallet(Wallet::NetworkWallet()) );

  if (!mWallet) {
    walletOpenFailed = true;
    return 0;
  }

  if (mWallet && !mWallet->hasFolder("knode"))
    mWallet->createFolder("knode");
  mWallet->setFolder("knode");
  return mWallet;
}


void KNServerInfo::readPassword()
{
  // no need to load a password if the account doesn't require auth
  if (!n_eedsLogon)
    return;
  mPassLoaded = true;

  // check wether there is a chance to find our password at all
  if (Wallet::folderDoesNotExist(Wallet::NetworkWallet(), "knode") ||
      Wallet::keyDoesNotExist(Wallet::NetworkWallet(), "knode", QString::number(i_d)))
    return;

  // finally try to open the wallet and read the password
  if (wallet())
    wallet()->readPassword(QString::number(i_d), p_ass);
}

// kate: space-indent on; indent-width 2;
