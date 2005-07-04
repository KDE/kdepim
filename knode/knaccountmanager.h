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

#ifndef KNACCOUNTMANAGER_H
#define KNACCOUNTMANAGER_H

#include <qglobal.h>
#include <qvaluelist.h>

namespace KWallet {
  class Wallet;
}

class KNGroupManager;
class KNNntpAccount;
class KNServerInfo;


class KNAccountManager : public QObject
{
  Q_OBJECT

  public:
    KNAccountManager(KNGroupManager *gm, QObject * parent=0, const char * name=0);
    ~KNAccountManager();

    void prepareShutdown();

    void setCurrentAccount(KNNntpAccount *a);

    bool newAccount(KNNntpAccount *a);       // a is new account allocated and configured by the caller
    bool removeAccount(KNNntpAccount *a=0);  // a==0: remove current account
    void editProperties(KNNntpAccount *a=0);
    void accountRenamed(KNNntpAccount *a=0);

    bool hasCurrentAccount() const             { return (c_urrentAccount!=0); }
    KNNntpAccount* currentAccount() const       { return c_urrentAccount; }
    KNServerInfo* smtp() const                 { return s_mtp; }
    /** Returns the account with the given id. */
    KNNntpAccount* account( int id );
    QValueList<KNNntpAccount*>::Iterator begin() { return mAccounts.begin(); }
    QValueList<KNNntpAccount*>::Iterator end()  { return mAccounts.end(); }
    /** Returns the first account (used as fallback sometimes). */
    KNNntpAccount* first() const;

    /** Loads the passwords of all accounts, allows on-demand wallet opening */
    void loadPasswords();
    /** Loads passwords of all accounts asynchronous */
    void loadPasswordsAsync();

    /** Returns a pointer to an open wallet if available, 0 otherwise */
    static KWallet::Wallet* wallet();

  protected:
    void loadAccounts();
    KNGroupManager *gManager;
    KNNntpAccount *c_urrentAccount;
    KNServerInfo *s_mtp;

  signals:
    void accountAdded(KNNntpAccount *a);
    void accountRemoved(KNNntpAccount *a);   // don't do anything with a, it will be deleted soon
    void accountModified(KNNntpAccount *a);
    /** Emitted if passwords have been loaded from the wallet */
    void passwordsChanged();

  private slots:
    void slotWalletOpened( bool success );

  private:
    /** set/create wallet-folder */
    static void prepareWallet();

  private:
    QValueList<KNNntpAccount*> mAccounts;
    static KWallet::Wallet *mWallet;
    static bool mWalletOpenFailed;
    bool mAsyncOpening;

};

#endif
