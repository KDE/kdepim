#include <qlayout.h>
#include <qframe.h>
#include <qvbox.h>
#include <qcheckbox.h>

#include <kapplication.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klistview.h>

#include "ldapoptionswidget.h"
#include "kabprefs.h"

#include "prefsdialog.h"

PrefsDialog::PrefsDialog( QWidget *parent )
  : KDialogBase( IconList, i18n( "Preferences" ), Apply | Ok | Cancel, Ok,
                 parent, 0, false, true )
{
  setupPages();

  readConfig();
}

void PrefsDialog::setupPages()
{
  ////////////////////////////////////////
  // Views
  QFrame *page = addPage( i18n("Views"), i18n("Views"),
                          KGlobal::iconLoader()->loadIcon( "view_remove", KIcon::Desktop ) );

  QVBoxLayout *topLayout = new QVBoxLayout( page, 0, 0 );

  mViewPage = new ViewPage( page );
  topLayout->addWidget( mViewPage );

  //////////////////////////////////
  // LDAP
  page = addPage( i18n("LDAP"), i18n("LDAP"),
                  KGlobal::iconLoader()->loadIcon( "find", KIcon::Desktop ) );

  topLayout = new QVBoxLayout( page, 0, 0 );

  mLdapWidget = new LDAPOptionsWidget( page );
  topLayout->addWidget( mLdapWidget );

  ////////////////////////////////////////
  // Addressees
  page = addPage( i18n("Addressees"), i18n("Addressees"),
                  KGlobal::iconLoader()->loadIcon( "vcard", KIcon::Desktop ) );

  topLayout = new QVBoxLayout( page, 0, 0 );
  mAddresseePage = new AddresseePage( page );
  topLayout->addWidget( mAddresseePage );
}

void PrefsDialog::readConfig()
{
  mAddresseePage->restoreSettings();
  mLdapWidget->restoreSettings();
  mViewPage->restoreSettings();
}

void PrefsDialog::writeConfig()
{
  mAddresseePage->saveSettings();
  mLdapWidget->saveSettings();
  mViewPage->saveSettings();
  KABPrefs::instance()->writeConfig();
}

void PrefsDialog::slotApply()
{
  writeConfig();

  emit configChanged();
}

void PrefsDialog::slotOk()
{
  slotApply();
  accept();
}

AddresseePage::AddresseePage( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QVBoxLayout *layout = new QVBoxLayout( this, KDialog::spacingHint(), KDialog::marginHint() );
  mNameParsing = new QCheckBox( i18n( "Automatic name parsing for new addressees" ), this );
  layout->addWidget( mNameParsing );
  layout->addStretch( 1 );
}

void AddresseePage::restoreSettings()
{
  mNameParsing->setChecked( KABPrefs::instance()->mAutomaticNameParsing );
}

void AddresseePage::saveSettings()
{
  KABPrefs::instance()->mAutomaticNameParsing = mNameParsing->isChecked();
}

ViewPage::ViewPage( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QVBoxLayout *layout = new QVBoxLayout( this, KDialog::spacingHint(), KDialog::marginHint() );
  mViewsSingleClickBox = new QCheckBox( i18n( "Honor KDE single click" ), this );
  layout->addWidget( mViewsSingleClickBox );
  layout->addStretch( 1 );
}

void ViewPage::restoreSettings()
{
  mViewsSingleClickBox->setChecked( KABPrefs::instance()->mHonorSingleClick );
}

void ViewPage::saveSettings()
{
  KABPrefs::instance()->mHonorSingleClick = mViewsSingleClickBox->isChecked();
}

#include "prefsdialog.moc"
