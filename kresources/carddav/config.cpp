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

using namespace KABC;

/*=========================================================================
| CONSTANTS
 ========================================================================*/

/*=========================================================================
| STATIC METHODS
 ========================================================================*/

ResourceCardDav* ResourceCardDavConfig::getCardDavResource(KRES::Resource* resource) {
    ResourceCardDav *res = dynamic_cast<ResourceCardDav *>( resource );
    if (!res) {
        kdDebug() << "invalid resource type";
    }

    return res;
}

CardDavPrefs* ResourceCardDavConfig::getPrefs(ResourceCardDav* res) {
    CardDavPrefs* p = NULL;

    if (res) {
        p = res->prefs();
        if (!p) {
            kdDebug() << "CardDAV: res->prefs() returned NULL";
        }
    }

    return p;
}

/*=========================================================================
| CONSTRUCTOR / DESTRUCTOR
 ========================================================================*/

ResourceCardDavConfig::ResourceCardDavConfig( TQWidget *parent )
    : KRES::ConfigWidget( parent )
{
    setupUI();
}

/*=========================================================================
| METHODS
 ========================================================================*/

void ResourceCardDavConfig::loadSettings( KRES::Resource *resource ) {
    ResourceCardDav* res = getCardDavResource(resource);
    CardDavPrefs* p = getPrefs(res);
    if (NULL != p) {
        mUrl->setText(p->url());
        mUsername->setText(p->username());
        mRememberPassword->setChecked(p->rememberPassword());
        mPassword->setText(p->password());
        mUseUriNotUID->setChecked(p->useURI());

        mReloadConfig->loadSettings(res);
        mSaveConfig->loadSettings(res);
    }
}

void ResourceCardDavConfig::saveSettings( KRES::Resource *resource ) {
    ResourceCardDav* res = getCardDavResource(resource);
    if (NULL != res) {
        mReloadConfig->saveSettings(res);
        mSaveConfig->saveSettings(res);

        CardDavPrefs* p = getPrefs(res);
        if (NULL != p) {
            p->setUrl(mUrl->text());
            p->setUsername(mUsername->text());
            p->setRememberPassword(mRememberPassword->isChecked());
            p->setPassword(mPassword->text());
            p->setUseURI(mUseUriNotUID->isChecked());
        }
    }
}

void ResourceCardDavConfig::setupUI() {
    TQVBoxLayout *vertical = new TQVBoxLayout(this);

    TQGridLayout *mainLayout = new TQGridLayout( this );

    // URL
    TQLabel *label = new TQLabel( i18n( "URL:" ), this );
    mUrl = new TQLineEdit( this );
    mainLayout->addWidget( label, 1, 0 );
    mainLayout->addWidget( mUrl, 1, 1 );

    // Username
    label = new TQLabel( i18n( "Username:" ), this );
    mUsername = new TQLineEdit( this );
    mainLayout->addWidget( label, 2, 0 );
    mainLayout->addWidget( mUsername, 2, 1 );

    // Password
    label = new TQLabel( i18n( "Password:" ), this );
    mPassword = new TQLineEdit( this );
    mPassword->setEchoMode( TQLineEdit::Password );
    mainLayout->addWidget( label, 3, 0 );
    mainLayout->addWidget( mPassword, 3, 1 );

    // Remember password checkbox
    mRememberPassword = new TQCheckBox( i18n("Remember password"), this );
    mainLayout->addWidget(mRememberPassword, 4, 1);

    mUseUriNotUID = new TQCheckBox( i18n( "Use URI instead of UID when modifying existing contacts" ), this );
    mainLayout->addWidget( mUseUriNotUID, 5, 1 );

    // configs
    TQHBoxLayout* horizontal = new TQHBoxLayout(this);

    // Reload config
    mReloadConfig = new CardDavReloadConfig(this);
    horizontal->addWidget(mReloadConfig);

    // Save config
    mSaveConfig = new CardDavSaveConfig(this);
    horizontal->addWidget(mSaveConfig);

    // FIXME: This feature does not work; hide the UI elements for later use
    mRememberPassword->hide();
    label->hide();
    mPassword->hide();

    // combining layouts
    vertical->addLayout(mainLayout);
    vertical->addLayout(horizontal);
}

// EOF ========================================================================
