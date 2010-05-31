/*=========================================================================
| KCardDAV
|--------------------------------------------------------------------------
| (c) 2010  Timothy Pearson
|
| This project is released under the GNU General Public License.
| Please see the file COPYING for more details.
|--------------------------------------------------------------------------
| Configuration and properties dialog
 ========================================================================*/

#ifndef KABC_RESOURCECARDDAVCONFIG_H
#define KABC_RESOURCECARDDAVCONFIG_H

/*=========================================================================
| INCLUDES
 ========================================================================*/

#include "resource.h"

#include <kdemacros.h>
#include <kresources/configwidget.h>

class QLineEdit;
class QCheckBox;

namespace KABC {

class CardDavReloadConfig;
class CardDavSaveConfig;

/*=========================================================================
| CLASS
 ========================================================================*/

/**
 * Configuration widget for CardDAV resource.
 */
class KDE_EXPORT ResourceCardDavConfig : public KRES::ConfigWidget
{
    Q_OBJECT

public:

    ResourceCardDavConfig(QWidget *parent = 0);

public slots:

    virtual void loadSettings(KRES::Resource *resource);
    virtual void saveSettings(KRES::Resource *resource);

protected:

    virtual void setupUI();

private:

    QLineEdit *mUrl;
    QLineEdit *mUsername;
    QLineEdit *mPassword;
    QCheckBox *mRememberPassword;
    CardDavReloadConfig* mReloadConfig;
    CardDavSaveConfig* mSaveConfig;

    static ResourceCardDav* getCardDavResource(KRES::Resource* res);

    /**
     * Returns preferences of the given ResourceCardDav object.
     * @param res resource object.
     * @return if preferences object is obtained successfully, it's returned. Otherwise, NULL is returned.
     */
    static CardDavPrefs* getPrefs(ResourceCardDav* res);
};

} // namespace KABC


#endif //  KABC_RESOURCECARDDAVCONFIG_H

