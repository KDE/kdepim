/*
    knserverinfo.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include "knserverinfo.h"
#include "utilities.h"

#include <kmessagebox.h>
#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>
#include <kwallet.h>
using namespace KWallet;


KNServerInfo::KNServerInfo()
  : t_ype(STnntp), i_d(-1), p_ort(119), h_old(300),
    t_imeout(60), n_eedsLogon(false)
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
    conf->deleteEntry("pass"); //first time run, don't store it in the config file

    if (Wallet::folderDoesNotExist(Wallet::NetworkWallet(), "knode") ||
        Wallet::keyDoesNotExist(Wallet::NetworkWallet(), "knode", QString::number(i_d)))
      return;

    Wallet* wallet = openWallet();
    if (!wallet || wallet->readPassword(QString::number(i_d), p_ass)) {
      //Save the pass in wallet as this might be the first time it's used
      if (wallet)
        wallet->writePassword(QString::number(i_d), p_ass);
    }
  }
}

Wallet* KNServerInfo::openWallet()
{
  //I use this because I don't want to bother user with error messages when
  //he/she didn't even enable wallet.
  if (!Wallet::isEnabled())
	return 0;
  QString networkWallet = Wallet::NetworkWallet();
  Wallet* wallet = Wallet::openWallet(networkWallet);
  if (!wallet) {
    KMessageBox::error(0, i18n("The wallet could not be opened. "
                               "This error is most probably caused "
                               "by providing a wrong password."));
    return 0;
  }

  if (wallet && !wallet->hasFolder("knode"))
    wallet->createFolder("knode");
  wallet->setFolder("knode");
  return wallet;
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
    //open wallet for storing only if the user actually does have the password
    if (n_eedsLogon) {
      Wallet* wallet = openWallet();
      if (!wallet || wallet->writePassword(QString::number(i_d), p_ass)) {
          KMessageBox::information(0, i18n("KWallet is not running. It is strongly recommend to use "
                                           "KWallet for managing your password"),
                                   i18n("KWallet is Not Running."), "KWalletWarning" );
          conf->writeEntry("pass", KNHelper::encryptStr(p_ass));
      }
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
