/*=========================================================================
| KCalDAV
|--------------------------------------------------------------------------
| (c) 2010  Timothy Pearson
| (c) 2009  Kumaran Santhanam (initial KDE4 version)
|
| This project is released under the GNU General Public License.
| Please see the file COPYING for more details.
|--------------------------------------------------------------------------
| Configuration and properties dialog
 ========================================================================*/

#ifndef KCAL_RESOURCECALDAVCONFIG_H
#define KCAL_RESOURCECALDAVCONFIG_H

/*=========================================================================
| INCLUDES
 ========================================================================*/

#include "resource.h"

#include <kdemacros.h>
#include <kresources/configwidget.h>

class TQLineEdit;
class TQCheckBox;

namespace KCal {

class CalDavReloadConfig;
class CalDavSaveConfig;

/*=========================================================================
| CLASS
 ========================================================================*/

/**
 * Configuration widget for CalDAV resource.
 */
class KDE_EXPORT ResourceCalDavConfig : public KRES::ConfigWidget
{
    Q_OBJECT

public:

    ResourceCalDavConfig(TQWidget *parent = 0);

public slots:

    virtual void loadSettings(KRES::Resource *resource);
    virtual void saveSettings(KRES::Resource *resource);

    void slotSTasksToggled( bool );

protected:

    virtual void setupUI();

private:

    TQLineEdit *mUrl;
    TQLineEdit *mTasksUrl;
    TQLineEdit *mUsername;
    TQLineEdit *mPassword;
    TQCheckBox *mUseSTasks;
    TQCheckBox *mRememberPassword;
    CalDavReloadConfig* mReloadConfig;
    CalDavSaveConfig* mSaveConfig;

    static ResourceCalDav* getCalDavResource(KRES::Resource* res);

    /**
     * Returns preferences of the given ResourceCalDav object.
     * @param res resource object.
     * @return if preferences object is obtained successfully, it's returned. Otherwise, NULL is returned.
     */
    static CalDavPrefs* getPrefs(ResourceCalDav* res);
};

} // namespace KCal


#endif //  KCAL_RESOURCECALDAVCONFIG_H

