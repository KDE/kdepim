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
  setupLdapPage();

  readConfig();
}

void PrefsDialog::setupLdapPage()
{
  ////////////////////////////////////////
  // Views
  QFrame *page = addPage( i18n("Views"), i18n("Views"),
                          KGlobal::iconLoader()->loadIcon( "viewmag", KIcon::Desktop ) );

  QVBoxLayout *topLayout = new QVBoxLayout( page, spacingHint(), marginHint() );
  topLayout->setAutoAdd( true );

  mViewsSingleClickBox = new QCheckBox( i18n( "Honor KDE single click" ), page );

  new QWidget( page ); // spacer

  //////////////////////////////////
  // LDAP
  page = addPage( i18n("LDAP"), i18n("LDAP"),
                  KGlobal::iconLoader()->loadIcon( "find", KIcon::Desktop ) );

  topLayout = new QVBoxLayout( page, 0, 0 );

  mLdapWidget = new LDAPOptionsWidget( page );
  topLayout->addWidget( mLdapWidget );
}

void PrefsDialog::readConfig()
{
  mLdapWidget->restoreSettings();
  mViewsSingleClickBox->setChecked( KABPrefs::instance()->mHonorSingleClick );
}

void PrefsDialog::writeConfig()
{
  mLdapWidget->saveSettings();

  KABPrefs::instance()->mHonorSingleClick = mViewsSingleClickBox->isChecked();
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

#include "prefsdialog.moc"
