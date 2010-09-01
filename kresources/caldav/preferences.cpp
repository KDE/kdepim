/*=========================================================================
| KCalDAV
|--------------------------------------------------------------------------
| (c) 2010  Timothy Pearson
| (c) 2009  Kumaran Santhanam (initial KDE4 version)
|
| This project is released under the GNU General Public License.
| Please see the file COPYING for more details.
|--------------------------------------------------------------------------
| CalDAV resource preferences class.
 ========================================================================*/

/*=========================================================================
| INCLUDES
 ========================================================================*/

#include "preferences.h"

#include <kwallet.h>
#include <tqstring.h>
#include <tqurl.h>
#include <kdebug.h>

/*=========================================================================
| NAMESPACES
 ========================================================================*/

using namespace KCal;
using namespace KWallet;

/*=========================================================================
| CONSTANTS
 ========================================================================*/

const TQString CalDavPrefs::NO_PASSWORD = "";
const TQString CalDavPrefs::WALLET_FOLDER = "CalDAV resource";
const TQString CalDavPrefs::WALLET_PWD_SUFFIX = ":caldav_password";

/*=========================================================================
| METHODS
 ========================================================================*/

bool CalDavPrefs::setWalletFolder(const TQString& folder) {
    bool ret = true;

    if (!mNoWallet && NULL != mWallet) {
        if (!mWallet->hasFolder(folder)) {
            if (!mWallet->createFolder(folder)) {
                ret = false;
                kdWarning() << "can't create the wallet folder for CalDAV passwords";
            }
        }
        if (!mWallet->setFolder(folder)) {
            ret = false;
            kdWarning() << "can't set the wallet folder for CalDAV passwords";
        }
    } else {
        // the wallet is inaccessible or not configured
        ret = false;
    }

    return ret;
}

Wallet* CalDavPrefs::getWallet() {
    Wallet* ret = NULL;

    if (!mNoWallet) {
        // the wallet is not marked as inaccessible

        if (NULL == mWallet) {
            kdDebug() << "creating wallet for " + mPrefix;

            mWallet = Wallet::openWallet(Wallet::NetworkWallet(), 0);
            if (NULL == mWallet) {
                mNoWallet = true; // can't open the wallet, mark it inaccessible
                kdWarning() << "can't create a wallet for CalDAV passwords";
            } else {
                if (setWalletFolder(WALLET_FOLDER)) {
                    // reserved
                } else {
                    // can't set the wallet folder, remove the wallet and mark it inaccessible
                    kdWarning() << "can't set the walet folder for CalDAV passwords";
                    removeWallet(true);
                }
            }
        }

        ret = mWallet;
    }

    return ret;
}

void CalDavPrefs::removeWallet(bool noWallet) {
    delete mWallet;
    mWallet = NULL;
    mNoWallet = noWallet;
}

void CalDavPrefs::addPrefix(const TQString& prefix) {
    KConfigSkeletonItem::List itemList = items();
    KConfigSkeletonItem::List::Iterator it;

    for ( it = itemList.begin(); it != itemList.end(); ++it ) {
        (*it)->setGroup( prefix + ':' + (*it)->group() );
    }
}

bool CalDavPrefs::writePasswordToWallet(const TQString& password) {

    Wallet* w = getWallet();

    bool ret = false;
    if (NULL != w) {
        int rc = w->writePassword(mPrefix + WALLET_PWD_SUFFIX, password);
        if (0 != rc) {
            kdWarning() << "CalDAV: can't write password to the wallet";
        } else {
            ret = true;
        }
    }

    return ret;
}

bool CalDavPrefs::readPasswordFromWallet(TQString& password) {
    Wallet* w = getWallet();

    bool ret = false;
    if (NULL != w) {
        TQString p;
        int rc = w->readPassword(mPrefix + WALLET_PWD_SUFFIX, p);
        if (0 == rc) {
            //CalDavPrefsSkel::setPassword(p);
            password = p;
            ret = true;
        } else {
            kdWarning() << "CalDAV: can't read password from the wallet";
            password = NO_PASSWORD;
        }
    }

    return ret;
}

bool CalDavPrefs::removePasswordFromWallet() {

    Wallet* w = getWallet();

    bool ret = false;
    if (NULL != w) {
        int rc = w->removeEntry(mPrefix + WALLET_PWD_SUFFIX);
        if (0 == rc) {
            ret = true;
        } else {
            kdWarning() << "CalDAV: can't remove password from the wallet";
        }
    }

    return ret;
}

void CalDavPrefs::setPassword(const TQString& p) {

    mPassword = p;

    if (rememberPassword()) {
        writePasswordToWallet(p);
    }
}

TQString CalDavPrefs::password() {
    if (NO_PASSWORD == mPassword) {
        readPasswordFromWallet(mPassword);
    }
    return mPassword;
}

TQString CalDavPrefs::getusername() {
    return username();
}

void CalDavPrefs::setRememberPassword(bool v) {
    kdDebug() << "remember: " << v;

    CalDavPrefsSkel::setRememberPassword(v);

    if (!v) {
        // we should not remember password. If there is one already stored, it must be removed.
        kdDebug() << "removing password from wallet";
        removePasswordFromWallet();
    }
}

void CalDavPrefs::writeConfig() {
    CalDavPrefsSkel::writeConfig();
}

void CalDavPrefs::readConfig() {

    CalDavPrefsSkel::readConfig();

    // the password is not in config file, try to restore it from the wallet.
    /*if (rememberPassword()) {
        readPasswordFromWallet();
    }*/
}

TQString CalDavPrefs::getFullUrl() {

    TQUrl t(url());
    TQString safeURL;
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

    // Encode any spaces, as libcaldav stupidly fails otherwise
    safeURL.replace(" ", "%20");

    return safeURL;
}

TQString CalDavPrefs::getFullTasksUrl() {
    if (useSTasks() == 0)
      return getFullUrl();

    TQUrl t(tasksUrl());
    TQString safeURL;
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

    // Encode any spaces, as libcaldav stupidly fails otherwise
    safeURL.replace(" ", "%20");

    return safeURL;
}

TQString CalDavPrefs::getFullJournalsUrl() {
    if (useSJournals() == 0)
      return getFullUrl();

    TQUrl t(journalsUrl());
    TQString safeURL;
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

    // Encode any spaces, as libcaldav stupidly fails otherwise
    safeURL.replace(" ", "%20");

    return safeURL;
}

// EOF ========================================================================

