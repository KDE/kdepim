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

#include "knaccountmanager.h"

#include "knconfigmanager.h"
#include "knfoldermanager.h"
#include "knglobals.h"
#include "kngroupmanager.h"
#include "utilities.h"

#include <QDir>
#include <kdebug.h>
#include <kconfig.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kwallet.h>



KWallet::Wallet* KNAccountManager::mWallet = 0;
bool KNAccountManager::mWalletOpenFailed = false;

KNAccountManager::KNAccountManager( KNGroupManager *gm, QObject * parent )
  : QObject( parent ), gManager( gm ),
  mAsyncOpening( false )
{
  loadAccounts();
}


KNAccountManager::~KNAccountManager()
{
  mAccounts.clear();
  delete mWallet;
  mWallet = 0;
}


void KNAccountManager::prepareShutdown()
{
  for ( KNNntpAccount::List::Iterator it = mAccounts.begin(); it != mAccounts.end(); ++it )
    (*it)->writeConfig();
}


void KNAccountManager::loadAccounts()
{
  QString dir( KStandardDirs::locateLocal( "data", "knode/" ) );
  if (dir.isNull()) {
    KNHelper::displayInternalFileError();
    return;
  }
  QDir d(dir);
  KNNntpAccount::Ptr a;
  QStringList entries(d.entryList(QStringList("nntp.*"), QDir::Dirs));

  QStringList::Iterator it;
  for(it = entries.begin(); it != entries.end(); ++it) {
    a = KNNntpAccount::Ptr( new KNNntpAccount() );
    if (a->readInfo(dir+(*it) + "/info")) {
      mAccounts.append(a);
      gManager->loadGroups(a);
      emit accountAdded(a);
    } else {
      kError(5003) <<"Unable to load account" << (*it) <<"!";
    }
  }
}


KNNntpAccount::Ptr KNAccountManager::account( int id )
{
  if ( id <= 0 )
    return KNNntpAccount::Ptr();
  for ( KNNntpAccount::List::ConstIterator it = mAccounts.constBegin(); it != mAccounts.constEnd(); ++it )
    if ( (*it)->id() == id )
      return *it;
  return KNNntpAccount::Ptr();
}


void KNAccountManager::setCurrentAccount( KNNntpAccount::Ptr a )
{
  c_urrentAccount = a;
}


// a is new account allocated and configured by the caller
bool KNAccountManager::newAccount( KNNntpAccount::Ptr a )
{
  // find a unused id for the new account...
  QString dir( KStandardDirs::locateLocal( "data", "knode/" ) );
  if (dir.isNull()) {
    KNHelper::displayInternalFileError();
    return false;
  }
  QDir d(dir);
  QStringList entries = d.entryList( QStringList( "nntp.*" ), QDir::Dirs );

  int id = 1;
  while (entries.indexOf(QString("nntp.%1").arg(id))!=-1)
    ++id;

  a->setId(id);

  dir = KStandardDirs::locateLocal( "data", QString( "knode/nntp.%1/" ).arg( a->id() ) );
  if (!dir.isNull()) {
    mAccounts.append(a);
    emit(accountAdded(a));
    return true;
  } else {
    KMessageBox::error(knGlobals.topWidget, i18n("Cannot create a folder for this account."));
    return false;
  }
}


// a==0: remove current account
bool KNAccountManager::removeAccount( KNNntpAccount::Ptr a )
{
  if(!a) a=c_urrentAccount;
  if(!a) return false;

  KNGroup::List lst;
  if ( knGlobals.folderManager()->unsentForAccount( a->id() ) > 0 ) {
    KMessageBox::sorry( knGlobals.topWidget,
      i18n("This account cannot be deleted since there are some unsent messages for it.") );
  }
  else if ( KMessageBox::warningContinueCancel ( knGlobals.topWidget,
            i18n("Do you really want to delete this account?"), "", KGuiItem( i18n("&Delete"), "edit-delete") )
            ==KMessageBox::Continue ) {
    lst = gManager->groupsOfAccount( a );
    for ( KNGroup::List::Iterator it = lst.begin(); it != lst.end(); ++it ) {
      if ( (*it)->isLocked() ) {
        KMessageBox::sorry( knGlobals.topWidget, i18n("At least one group of this account is currently in use.\n"
            "The account cannot be deleted at the moment.") );
        return false;
      }
    }
    for ( KNGroup::List::Iterator it = lst.begin(); it != lst.end(); ++it )
      gManager->unsubscribeGroup( (*it) );

    QDir dir(a->path());
    if (dir.exists()) {
      QFileInfoList list = dir.entryInfoList();  // get list of matching files and delete all
      QFileInfo it;
      Q_FOREACH( it, list ) {
        dir.remove(it.fileName());
      }
      dir.cdUp();                                       // directory should now be empty, deleting it
      dir.rmdir(QString("nntp.%1/").arg(a->id()));
    }

    if( c_urrentAccount == a ) {
      setCurrentAccount( KNNntpAccount::Ptr() );
    }

    emit(accountRemoved(a));
    mAccounts.removeAll( a );  // finally delete a
    return true;
  }

  return false;
}


void KNAccountManager::editProperties( KNNntpAccount::Ptr a )
{
  if(!a) a=c_urrentAccount;
  if(!a) return;

  a->editProperties(knGlobals.topWidget);
  emit(accountModified(a));
}


void KNAccountManager::accountRenamed( KNNntpAccount::Ptr a )
{
  if(!a) a=c_urrentAccount;
  if(!a) return;

  emit(accountModified(a));
}


KNNntpAccount::Ptr KNAccountManager::first() const
{
  if ( mAccounts.isEmpty() )
    return KNNntpAccount::Ptr();
  return mAccounts.first();
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
  for ( KNNntpAccount::List::Iterator it = mAccounts.begin(); it != mAccounts.end(); ++it )
    (*it)->readPassword();
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
    mWallet = Wallet::openWallet( Wallet::NetworkWallet(), 0 );

  if ( !mWallet ) {
    mWalletOpenFailed = true;
    return 0;
  }

  prepareWallet();
  return mWallet;
}


void KNAccountManager::prepareWallet()
{
  if ( !mWallet )
    return;
  if ( !mWallet->hasFolder("knode") )
    mWallet->createFolder( "knode" );
  mWallet->setFolder( "knode" );
}


void KNAccountManager::slotWalletOpened( bool success )
{
  mAsyncOpening = false;
  if ( !success ) {
    mWalletOpenFailed = true;
    delete mWallet;
    mWallet = 0;
  } else {
    prepareWallet();
  }
  loadPasswords();
}

//--------------------------------

