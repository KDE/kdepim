#include <qcheckbox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlayout.h>

#include <kaboutdata.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kjanuswidget.h>
#include <klistview.h>
#include <klocale.h>

#include "ldapoptionswidget.h"
#include "kabprefs.h"

#include "kcmkabconfig.h"

extern "C"
{
  KCModule *create_kabconfig( QWidget *parent, const char * ) {
    return new KCMKabConfig( parent, "kcmkabconfig" );
  }
}

KCMKabConfig::KCMKabConfig( QWidget *parent, const char *name )
  : KCModule( parent, name )
{
  QVBoxLayout *layout = new QVBoxLayout( this );
  mConfigWidget = new ConfigureWidget( this, "mConfigWidget" );
  layout->addWidget( mConfigWidget );

  connect( mConfigWidget, SIGNAL( changed( bool ) ), SIGNAL( changed( bool ) ) );

  load();
}

void KCMKabConfig::load()
{
  mConfigWidget->restoreSettings();
}

void KCMKabConfig::save()
{
  kdDebug() << "KCMKabConfig::save()" << endl;
  mConfigWidget->saveSettings();
}

void KCMKabConfig::defaults()
{
  mConfigWidget->defaults();
}

const KAboutData* KCMKabConfig::aboutData() const
{
  KAboutData *about = new KAboutData( I18N_NOOP( "kcmkabconfig" ),
                                      I18N_NOOP( "KAddressBook Configure Dialog" ),
                                      0, 0, KAboutData::License_GPL,
                                      I18N_NOOP( "(c), 2003 Tobias Koenig" ) );

  about->addAuthor( "Tobias Koenig", 0, "tokoe@kde.org" );

  return about;
}


ConfigureWidget::ConfigureWidget( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QVBoxLayout *topLayout = new QVBoxLayout( this, KDialog::marginHint(),
                                            KDialog::spacingHint() );

  QGroupBox *groupBox = new QGroupBox( 0, Qt::Vertical, i18n( "General" ), this );
  QVBoxLayout *boxLayout = new QVBoxLayout( groupBox->layout() );
  boxLayout->setAlignment( Qt::AlignTop );

  mViewsSingleClickBox = new QCheckBox( i18n( "Honor KDE single click" ), groupBox, "msingle" );
  boxLayout->addWidget( mViewsSingleClickBox );

  mNameParsing = new QCheckBox( i18n( "Automatic name parsing for new addressees" ), groupBox, "mparse" );
  boxLayout->addWidget( mNameParsing );

  topLayout->addWidget( groupBox );

  connect( mNameParsing, SIGNAL( toggled( bool ) ), this, SLOT( modified() ) );
  connect( mViewsSingleClickBox, SIGNAL( toggled( bool ) ), this, SLOT( modified() ) );
}

void ConfigureWidget::restoreSettings()
{
  bool blocked = signalsBlocked();
  blockSignals( true );

  mNameParsing->setChecked( KABPrefs::instance()->mAutomaticNameParsing );
  mViewsSingleClickBox->setChecked( KABPrefs::instance()->mHonorSingleClick );

  blockSignals( blocked );

  emit changed( false );
}

void ConfigureWidget::saveSettings()
{
  kdDebug() << "ConfigureWidget::save()" << endl;

  KABPrefs::instance()->mAutomaticNameParsing = mNameParsing->isChecked();
  KABPrefs::instance()->mHonorSingleClick = mViewsSingleClickBox->isChecked();

  KABPrefs::instance()->writeConfig();

  emit changed( false );
}

void ConfigureWidget::defaults()
{
  mNameParsing->setChecked( true );
  mViewsSingleClickBox->setChecked( false );

  emit changed( true );
}

void ConfigureWidget::modified()
{
  emit changed( true );
}

#include "kcmkabconfig.moc"
