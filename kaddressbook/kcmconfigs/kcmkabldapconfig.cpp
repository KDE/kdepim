#include <qcheckbox.h>
#include <qframe.h>
#include <qlayout.h>

#include <kaboutdata.h>
#include <klocale.h>

#include "ldapoptionswidget.h"

#include "kcmkabldapconfig.h"

extern "C"
{
  KCModule *create_kabldapconfig( QWidget *parent, const char * ) {
    return new KCMKabLdapConfig( parent, "kcmkabldapconfig" );
  }
}

KCMKabLdapConfig::KCMKabLdapConfig( QWidget *parent, const char *name )
  : KCModule( parent, name )
{
  QVBoxLayout *layout = new QVBoxLayout( this );
  mConfigWidget = new LDAPOptionsWidget( this );
  layout->addWidget( mConfigWidget );

  connect( mConfigWidget, SIGNAL( changed( bool ) ), SIGNAL( changed( bool ) ) );

  load();
}

void KCMKabLdapConfig::load()
{
  mConfigWidget->restoreSettings();
}

void KCMKabLdapConfig::save()
{
  mConfigWidget->saveSettings();
}

void KCMKabLdapConfig::defaults()
{
  mConfigWidget->defaults();
}

const KAboutData* KCMKabLdapConfig::aboutData() const
{
  KAboutData *about = new KAboutData( I18N_NOOP( "kcmkabldapconfig" ),
                                      I18N_NOOP( "KAB LDAP Configure Dialog" ),
                                      0, 0, KAboutData::License_GPL,
                                      I18N_NOOP( "(c), 2003 Tobias Koenig" ) );

  about->addAuthor( "Tobias Koenig", 0, "tokoe@kde.org" );

  return about;
}

#include "kcmkabldapconfig.moc"
