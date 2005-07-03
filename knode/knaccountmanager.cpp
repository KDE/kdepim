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

#include <stdlib.h>

#include <qdir.h>

#include <kdebug.h>
#include <ksimpleconfig.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kwallet.h>

#include "kngroupmanager.h"
#include "knnntpaccount.h"
#include "knglobals.h"
#include "knconfigmanager.h"
#include "utilities.h"
#include "knaccountmanager.h"
#include "knfoldermanager.h"

KWallet::Wallet* KNAccountManager::mWallet = 0;
bool KNAccountManager::mWalletOpenFailed = false;

KNAccountManager::KNAccountManager(KNGroupManager *gm, QObject * parent, const char * name)
  : QObject(parent, name), gManager(gm), c_urrentAccount(0),
  mAsyncOpening( false )
{
  accList=new QPtrList<KNNntpAccount>;
  accList->setAutoDelete(true);
  s_mtp=new KNServerInfo();
  s_mtp->setType(KNServerInfo::STsmtp);
  s_mtp->setId(0);
  KConfig *conf=knGlobals.config();
  conf->setGroup("MAILSERVER");
  s_mtp->readConf(conf);

  loadAccounts();
}


KNAccountManager::~KNAccountManager()
{
  delete accList;
  delete s_mtp;
  delete mWallet;
  mWallet = 0;
}


void KNAccountManager::prepareShutdown()
{
  for(KNNntpAccount *a=accList->first(); a; a=accList->next())
    a->saveInfo();
}


void KNAccountManager::loadAccounts()
{
  QString dir(locateLocal("data","knode/"));
  if (dir.isNull()) {
    KNHelper::displayInternalFileError();
    return;
  }
  QDir d(dir);
  KNNntpAccount *a;
  QStringList entries(d.entryList("nntp.*", QDir::Dirs));

  QStringList::Iterator it;
  for(it = entries.begin(); it != entries.end(); ++it) {
    a=new KNNntpAccount();
    if (a->readInfo(dir+(*it)+"/info")) {
      accList->append(a);
      gManager->loadGroups(a);
      emit accountAdded(a);
    } else {
      delete a;
      kdError(5003) << "Unable to load account " << (*it) << "!" << endl;
    }
  }
}


KNNntpAccount* KNAccountManager::account(int i)
{
  KNNntpAccount *ret=0;
  if(i>0)
  {
    for(KNNntpAccount *a=accList->first(); a; a=accList->next()) {
      if(a->id()==i) {
        ret=a;
        break;
      }
    }
  }
  return ret;
}


void KNAccountManager::setCurrentAccount(KNNntpAccount *a)
{
  c_urrentAccount=a;
}


// a is new account allocated and configured by the caller
bool KNAccountManager::newAccount(KNNntpAccount *a)
{
  // find a unused id for the new account...
  QString dir(locateLocal("data","knode/"));
  if (dir.isNull()) {
    delete a;
    KNHelper::displayInternalFileError();
    return false;
  }
  QDir d(dir);
  QStringList entries(d.entryList("nntp.*", QDir::Dirs));

  int id = 1;
  while (entries.findIndex(QString("nntp.%1").arg(id))!=-1)
    ++id;

  a->setId(id);

  dir = locateLocal("data",QString("knode/nntp.%1/").arg(a->id()));
  if (!dir.isNull()) {
    accList->append(a);
    emit(accountAdded(a));
    return true;
  } else {
    delete a;
    KMessageBox::error(knGlobals.topWidget, i18n("Cannot create a folder for this account."));
    return false;
  }
}


// a==0: remove current account
bool KNAccountManager::removeAccount(KNNntpAccount *a)
{
  if(!a) a=c_urrentAccount;
  if(!a) return false;

  QPtrList<KNGroup> *lst;
  if(knGlobals.folderManager()->unsentForAccount(a->id()) > 0) {
    KMessageBox::sorry(knGlobals.topWidget, i18n("This account cannot be deleted since there are some unsent messages for it."));
  }
  else if(KMessageBox::warningContinueCancel(knGlobals.topWidget, i18n("Do you really want to delete this account?"),"",KGuiItem(i18n("&Delete"),"editdelete"))==KMessageBox::Continue) {
    lst=new QPtrList<KNGroup>;
    lst->setAutoDelete(false);
    gManager->getGroupsOfAccount(a, lst);
    for(KNGroup *g=lst->first(); g; g=lst->next()) {
      if(g->isLocked()) {
        KMessageBox::sorry(knGlobals.topWidget, i18n("At least one group of this account is currently in use.\nThe account cannot be deleted at the moment."));
        return false;
      }
    }
    for(KNGroup *g=lst->first(); g; g=lst->next())
      gManager->unsubscribeGroup(g);
    delete lst;

    QDir dir(a->path());
    if (dir.exists()) {
      const QFileInfoList *list = dir.entryInfoList();  // get list of matching files and delete all
      if (list) {
        QFileInfoListIterator it( *list );
        while (it.current()) {
          dir.remove(it.current()->fileName());
          ++it;
        }
      }
      dir.cdUp();                                       // directory should now be empty, deleting it
      dir.rmdir(QString("nntp.%1/").arg(a->id()));
    }

    if(c_urrentAccount==a) setCurrentAccount(0);

    emit(accountRemoved(a));
    accList->removeRef(a);      // finally delete a
    return true;
  }

  return false;
}


void KNAccountManager::editProperties(KNNntpAccount *a)
{
  if(!a) a=c_urrentAccount;
  if(!a) return;

  a->editProperties(knGlobals.topWidget);
  emit(accountModified(a));
}


void KNAccountManager::accountRenamed(KNNntpAccount *a)
{
  if(!a) a=c_urrentAccount;
  if(!a) return;

  emit(accountModified(a));
}


void KNAccountManager::loadPasswordsAsync()
{
  if ( !mWallet && !mWalletOpenFailed ) {
    if ( knGlobals.top )
      mWallet = Wallet::openWallet( Wallet::NetworkWallet(),
                                    knGlobals.topWidget->topLevelWidget()->winId(),
                                    Wallet::Asynchronous );
    else
      mWallet = Wallet::openWallet( Wallet::NetworkWallet(), 0, Wallet::Asynchronous );
    if ( mWallet ) {
      connect( mWallet, SIGNAL(walletOpened(bool)), SLOT(slotWalletOpened(bool)) );
      mAsyncOpening = true;
    }
    else {
      mWalletOpenFailed = true;
      loadPasswords();
    }
    return;
  }
  if ( mWallet && !mAsyncOpening )
    loadPasswords();
}


void KNAccountManager::loadPasswords()
{
  KNNntpAccount *a;
  for (a = accList->first(); a; a = accList->next())
    a->readPassword();
  emit passwordsChanged();
}


KWallet::Wallet* KNAccountManager::wallet()
{
  if ( mWallet && mWallet->isOpen() )
    return mWallet;

  if ( !Wallet::isEnabled() || mWalletOpenFailed )
    return 0;

  delete mWallet;
  if ( knGlobals.top )
    mWallet = Wallet::openWallet( Wallet::NetworkWallet(),
                                  knGlobals.topWidget->topLevelWidget()->winId() );
  else
    mWallet = Wallet::openWallet( Wallet::NetworkWallet() );

  if ( !mWallet ) {
    mWalletOpenFailed = true;
    return 0;
  }

  prepareWallet();
  return mWallet;
}


void KNAccountManager::prepareWallet()
{
  if ( mWallet && !mWallet->hasFolder("knode") )
    mWallet->createFolder( "knode" );
  mWallet->setFolder( "knode" );
}


void KNAccountManager::slotWalletOpened( bool success )
{
  mAsyncOpening = false;
  if ( !success ) {
    delete mWallet;
    mWallet = 0;
  } else
    prepareWallet();
  KNNntpAccount *a;
  for (a = accList->first(); a; a = accList->next())
    a->readPassword();
  emit passwordsChanged();
}

//--------------------------------

#include "knaccountmanager.moc"
