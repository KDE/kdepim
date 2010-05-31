/*=========================================================================
| KABCDAV
|--------------------------------------------------------------------------
| (c) 2010  Timothy Pearson
|
| This project is released under the GNU General Public License.
| Please see the file COPYING for more details.
|--------------------------------------------------------------------------
| CardDAV resource preferences class.
 ========================================================================*/

/*=========================================================================
| INCLUDES
 ========================================================================*/

#include "preferences.h"

#include <kwallet.h>
#include <qstring.h>
#include <qurl.h>
#include <kdebug.h>

/*=========================================================================
| NAMESPACES
 ========================================================================*/

using namespace KABC;
using namespace KWallet;

/*=========================================================================
| CONSTANTS
 ========================================================================*/

const QString CardDavPrefs::NO_PASSWORD = "";
const QString CardDavPrefs::WALLET_FOLDER = "CardDAV resource";
const QString CardDavPrefs::WALLET_PWD_SUFFIX = ":carddav_password";

/*=========================================================================
| METHODS
 ========================================================================*/

bool CardDavPrefs::setWalletFolder(const QString& folder) {
    bool ret = true;

    if (!mNoWallet && NULL != mWallet) {
        if (!mWallet->hasFolder(folder)) {
            if (!mWallet->createFolder(folder)) {
                ret = false;
                kdWarning() << "can't create the wallet folder for CardDAV passwords";
            }
        }
        if (!mWallet->setFolder(folder)) {
            ret = false;
            kdWarning() << "can't set the wallet folder for CardDAV passwords";
        }
    } else {
        // the wallet is inaccessible or not configured
        ret = false;
    }

    return ret;
}

Wallet* CardDavPrefs::getWallet() {
    Wallet* ret = NULL;

    if (!mNoWallet) {
        // the wallet is not marked as inaccessible

        if (NULL == mWallet) {
            kdDebug() << "creating wallet for " + mPrefix;

            mWallet = Wallet::openWallet(Wallet::NetworkWallet(), 0);
            if (NULL == mWallet) {
                mNoWallet = true; // can't open the wallet, mark it inaccessible
                kdWarning() << "can't create a wallet for CardDAV passwords";
            } else {
                if (setWalletFolder(WALLET_FOLDER)) {
                    // reserved
                } else {
                    // can't set the wallet folder, remove the wallet and mark it inaccessible
                    kdWarning() << "can't set the walet folder for CardDAV passwords";
                    removeWallet(true);
                }
            }
        }

        ret = mWallet;
    }

    return ret;
}

void CardDavPrefs::removeWallet(bool noWallet) {
    delete mWallet;
    mWallet = NULL;
    mNoWallet = noWallet;
}

void CardDavPrefs::addPrefix(const QString& prefix) {
    KConfigSkeletonItem::List itemList = items();
    KConfigSkeletonItem::List::Iterator it;

    for ( it = itemList.begin(); it != itemList.end(); ++it ) {
        (*it)->setGroup( prefix + ':' + (*it)->group() );
    }
}

bool CardDavPrefs::writePasswordToWallet(const QString& password) {

    Wallet* w = getWallet();

    bool ret = false;
    if (NULL != w) {
        int rc = w->writePassword(mPrefix + WALLET_PWD_SUFFIX, password);
        if (0 != rc) {
            kdWarning() << "CardDAV: can't write password to the wallet";
        } else {
            ret = true;
        }
    }

    return ret;
}

bool CardDavPrefs::readPasswordFromWallet(QString& password) {
    Wallet* w = getWallet();

    bool ret = false;
    if (NULL != w) {
        QString p;
        int rc = w->readPassword(mPrefix + WALLET_PWD_SUFFIX, p);
        if (0 == rc) {
            //CardDavPrefsSkel::setPassword(p);
            password = p;
            ret = true;
        } else {
            kdWarning() << "CardDAV: can't read password from the wallet";
            password = NO_PASSWORD;
        }
    }

    return ret;
}

bool CardDavPrefs::removePasswordFromWallet() {

    Wallet* w = getWallet();

    bool ret = false;
    if (NULL != w) {
        int rc = w->removeEntry(mPrefix + WALLET_PWD_SUFFIX);
        if (0 == rc) {
            ret = true;
        } else {
            kdWarning() << "CardDAV: can't remove password from the wallet";
        }
    }

    return ret;
}

void CardDavPrefs::setPassword(const QString& p) {

    mPassword = p;

    if (rememberPassword()) {
        writePasswordToWallet(p);
    }
}

QString CardDavPrefs::password() {
    if (NO_PASSWORD == mPassword) {
        readPasswordFromWallet(mPassword);
    }
    return mPassword;
}

QString CardDavPrefs::getusername() {
    return username();
}

void CardDavPrefs::setRememberPassword(bool v) {
    kdDebug() << "remember: " << v;

    CardDavPrefsSkel::setRememberPassword(v);

    if (!v) {
        // we should not remember password. If there is one already stored, it must be removed.
        kdDebug() << "removing password from wallet";
        removePasswordFromWallet();
    }
}

void CardDavPrefs::writeConfig() {
    CardDavPrefsSkel::writeConfig();
}

void CardDavPrefs::readConfig() {

    CardDavPrefsSkel::readConfig();

    // the password is not in config file, try to restore it from the wallet.
    /*if (rememberPassword()) {
        readPasswordFromWallet();
    }*/
}

QString CardDavPrefs::getFullUrl() {

    QUrl t(url());
    QString safeURL;
    int firstAt;

    t.setUser(username());
    t.setPassword(password());

    safeURL = t.toString();

    firstAt = safeURL.find("@") + 1;
    while (safeURL.find("@", firstAt) != -1) {
        safeURL.replace(safeURL.find("@", firstAt), 1, "%40");
    }

    // Unencode the username, as Zimbra stupidly rejects the %40
    safeURL.replace("%40", "@");

    // Encode any spaces, as libcarddav stupidly fails otherwise
    safeURL.replace(" ", "%20");

    return safeURL;
}

// EOF ========================================================================

