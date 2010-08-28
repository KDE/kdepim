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

/*=========================================================================
| INCLUDES
 ========================================================================*/

#include "resource.h"
#include "config.h"
#include "configwidgets.h"

#include <kcombobox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <klineedit.h>
#include <klistview.h>
#include <kurlrequester.h>

#include <tqlabel.h>
#include <tqlayout.h>
#include <tqcheckbox.h>

/*=========================================================================
| NAMESPACE
 ========================================================================*/

using namespace KCal;

/*=========================================================================
| CONSTANTS
 ========================================================================*/

/*=========================================================================
| STATIC METHODS
 ========================================================================*/

ResourceCalDav* ResourceCalDavConfig::getCalDavResource(KRES::Resource* resource) {
    ResourceCalDav *res = dynamic_cast<ResourceCalDav *>( resource );
    if (!res) {
        kdDebug() << "invalid resource type";
    }

    return res;
}

CalDavPrefs* ResourceCalDavConfig::getPrefs(ResourceCalDav* res) {
    CalDavPrefs* p = NULL;

    if (res) {
        p = res->prefs();
        if (!p) {
            kdDebug() << "CalDAV: res->prefs() returned NULL";
        }
    }

    return p;
}

/*=========================================================================
| CONSTRUCTOR / DESTRUCTOR
 ========================================================================*/

ResourceCalDavConfig::ResourceCalDavConfig( TQWidget *parent )
    : KRES::ConfigWidget( parent )
{
    setupUI();
}

/*=========================================================================
| METHODS
 ========================================================================*/

void ResourceCalDavConfig::loadSettings( KRES::Resource *resource ) {
    ResourceCalDav* res = getCalDavResource(resource);
    CalDavPrefs* p = getPrefs(res);
    if (NULL != p) {
        mUrl->setText(p->url());
        mUsername->setText(p->username());
        mRememberPassword->setChecked(p->rememberPassword());
        mPassword->setText(p->password());
        mTasksUrl->setText(p->tasksUrl());
        mUseSTasks->setChecked(p->useSTasks());

        mReloadConfig->loadSettings(res);
        mSaveConfig->loadSettings(res);
    }
}

void ResourceCalDavConfig::saveSettings( KRES::Resource *resource ) {
    ResourceCalDav* res = getCalDavResource(resource);
    if (NULL != res) {
        mReloadConfig->saveSettings(res);
        mSaveConfig->saveSettings(res);

        CalDavPrefs* p = getPrefs(res);
        if (NULL != p) {
            p->setUrl(mUrl->text());
            p->setUsername(mUsername->text());
            p->setRememberPassword(mRememberPassword->isChecked());
            p->setPassword(mPassword->text());
            p->setTasksUrl(mTasksUrl->text());
            p->setUseSTasks(mUseSTasks->isChecked());
        }
    }
}

void ResourceCalDavConfig::setupUI() {
    TQVBoxLayout *vertical = new TQVBoxLayout(this);

    TQGridLayout *mainLayout = new TQGridLayout( this );

    // URL
    TQLabel *label = new TQLabel( i18n( "URL:" ), this );
    mUrl = new TQLineEdit( this );
    mainLayout->addWidget( label, 1, 0 );
    mainLayout->addWidget( mUrl, 1, 1 );

    // Tasks URL
    TQLabel *tlabel = new TQLabel( i18n( "Tasks URL:" ), this );
    mTasksUrl = new TQLineEdit( this );
    mainLayout->addWidget( tlabel, 2, 0 );
    mainLayout->addWidget( mTasksUrl, 2, 1 );

    // Use Task URL checkbox
    mUseSTasks = new TQCheckBox( i18n("Use separate Tasks URL"), this );
    mainLayout->addWidget(mUseSTasks, 3, 0 );

    // Username
    label = new TQLabel( i18n( "Username:" ), this );
    mUsername = new TQLineEdit( this );
    mainLayout->addWidget( label, 4, 0 );
    mainLayout->addWidget( mUsername, 4, 1 );

    // Password
    label = new TQLabel( i18n( "Password:" ), this );
    mPassword = new TQLineEdit( this );
    mPassword->setEchoMode( TQLineEdit::Password );
    mainLayout->addWidget( label, 5, 0 );
    mainLayout->addWidget( mPassword, 5, 1 );

    // Remember password checkbox
    mRememberPassword = new TQCheckBox( i18n("Remember password"), this );
    mainLayout->addWidget(mRememberPassword, 6, 1);

    mTasksUrl->setEnabled(mUseSTasks->isChecked());
    connect( mUseSTasks, TQT_SIGNAL( toggled( bool ) ),
           TQT_SLOT( slotSTasksToggled( bool ) ) );

    // configs
    TQHBoxLayout* horizontal = new TQHBoxLayout(this);

    // Reload config
    mReloadConfig = new CalDavReloadConfig(this);
    horizontal->addWidget(mReloadConfig);

    // Save config
    mSaveConfig = new CalDavSaveConfig(this);
    horizontal->addWidget(mSaveConfig);

    // FIXME: This feature does not work; hide the UI elements for later use
    mRememberPassword->hide();
    label->hide();
    mPassword->hide();

    // combining layouts
    vertical->addLayout(mainLayout);
    vertical->addLayout(horizontal);
}

void ResourceCalDavConfig::slotSTasksToggled( bool enabled ) {
    mTasksUrl->setEnabled(enabled);
}

// EOF ========================================================================
