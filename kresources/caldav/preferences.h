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

#ifndef KCAL_CALDAVPREFS_H
#define KCAL_CALDAVPREFS_H

#include "prefsskel.h"

#include <kwallet.h>
#include <kdebug.h>

class TQString;

namespace KCal {

/*=========================================================================
| CLASS
 ========================================================================*/

/**
 * This class provides access to ResourceCalDav preferences.
 * It inherits auto-generated CalDavPrefsSkel class to add password-handling code.
 * KWallet is used for storing passwords.
 * It also adds code to allow multiple CalDAV resources to store settings in the same
 * config file.
 */
class CalDavPrefs : public CalDavPrefsSkel {

public:

    /**
     * @param prefix Unique prefix of the resource instance (use identifier() method).
     */
    CalDavPrefs(const TQString& prefix)
        : mWallet(NULL)
        , mNoWallet(false)
        , mPrefix(prefix)
        , mPassword(NO_PASSWORD)
    {
        addPrefix(prefix);
    }

    virtual ~CalDavPrefs() {
        kdDebug() << "removing wallet";
        removeWallet();
    }

    virtual void writeConfig();
    virtual void readConfig();

    /**
     * Sets a new password. Also, if remember password flag is true,
     * remembers the password in the wallet. So, if you want the password
     * to be properly saved, call this method after ensuring the remember flag
     * is set.
     */
    void setPassword(const TQString& p);

    /**
     * Returns password. The password is taken from the wallet.
     * May return an empty string, if there is no password available.
     */
    TQString password();

    /**
     * Returns the username.
     */
    TQString getusername();

    void setRememberPassword(bool v);

    /**
     * @return A full URL to connect to CalDAV server (including username and password).
     */
    TQString getFullUrl();

    /**
     * @return A full URL to connect to CalDAV Tasks server (including username and password).
     */
    TQString getFullTasksUrl();

protected:

    /**
     * Add an unique prefix to KConfigGroup, so that different instances of the resource
     * can use the same config file.
     * @param prefix Unique prefix of the resource instance.
     */
    void addPrefix(const TQString& prefix);

    /**
     * Returns the wallet or NULL, if the wallet can't be obtained.
     */
    KWallet::Wallet* getWallet();

    /**
     * Tries to set a working folder for the wallet. If the wallet is not configured yet, does nothing.
     * @param folder the wallet working folder
     * @return true, if the folder has been set, and false otherwise.
     */
    bool setWalletFolder(const TQString& folder);

    /**
     * Removes the wallet. If @p noWallet is set, the wallet has been marked inaccessible, so that subsequent
     * getWallet calls will not try to recreate it.
     */
    void removeWallet(bool noWallet = false);

    /**
     * Wrire password to the wallet.
     * @param password password to write
     * @return true on success, false on failure
     */
    bool writePasswordToWallet(const TQString& password);

    /**
     * Extracts password from the wallet.
     * @param password a variable to save read password to.
     * @return true on success, false on failure
     */
    bool readPasswordFromWallet(TQString& password);

    /**
     * Clears password in the wallet.
     * @return true on success, false on failure
     */
    bool removePasswordFromWallet();

private:

    static const TQString NO_PASSWORD;
    static const TQString WALLET_FOLDER;
    static const TQString WALLET_PWD_SUFFIX;

    KWallet::Wallet* mWallet;
    bool mNoWallet;

    TQString mPrefix;
    TQString mPassword;
};

} // namespace KCal

#endif // KCAL_CALDAVPREFS_H


