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
#include <QList>

namespace KWallet {
  class Wallet;
}

class KNGroupManager;
class KNNntpAccount;


/** Account manager.
 */
class KNAccountManager : public QObject
{
  Q_OBJECT

  public:
    /** Create a new account manager.
     * @param gm The group manager.
     * @param parent The parent object.
     */
    KNAccountManager( KNGroupManager *gm, QObject * parent = 0 );
    /** Delete this account manager and all managed accounts.
     */
    ~KNAccountManager();

    /// List of accounts
    typedef QList<KNNntpAccount*> List;

    /** Save all accounts. */
    void prepareShutdown();

    /** Sets the current account.
     * @param a The current account.
     */
    void setCurrentAccount( KNNntpAccount *a );

    /** Add a new account.
     *  @param a A new account allocated and configured by the caller.
     */
    bool newAccount( KNNntpAccount *a );
    /** Remove an existing account.
     *  @param a The account to remove, if @p a is 0, the current account will
     *  be removed.
     */
    bool removeAccount( KNNntpAccount *a = 0 );
    /** Show the properties dialog for the given account.
     * @param a The account to edit, uses the current if @p a is 0.
     */
    void editProperties( KNNntpAccount *a = 0 );
    void accountRenamed(KNNntpAccount *a=0);

    /** Returns true if there is a current account. */
    bool hasCurrentAccount() const { return c_urrentAccount != 0; }
    /** Returns the current account. */
    KNNntpAccount* currentAccount() const       { return c_urrentAccount; }
    /** Returns the account with the given id. */
    KNNntpAccount* account( int id );
    /** Returns the list of all accounts. */
    List accounts() const { return mAccounts; }
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
    List mAccounts;
    static KWallet::Wallet *mWallet;
    static bool mWalletOpenFailed;
    bool mAsyncOpening;

};

#endif
