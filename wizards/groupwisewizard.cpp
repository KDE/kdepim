/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "groupwisewizard.h"

#include "groupwiseconfig.h"
#include "kmailchanges.h"

#include "kresources/groupwise/kabc_groupwiseprefs.h"
#include "kresources/groupwise/kabc_resourcegroupwise.h"
#include "kresources/groupwise/kcal_groupwiseprefsbase.h"
#include "kresources/groupwise/kcal_resourcegroupwise.h"

#include <kcal/resourcecalendar.h>
#include <kpimutils/email.h>

#include <klineedit.h>
#include <klocale.h>

#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QSpinBox>
#include <QGroupBox>

QString serverUrl()
{
  QString url;
  if ( GroupwiseConfig::self()->useHttps() ) url = "https";
  else url = "http";
  url += "://" + GroupwiseConfig::self()->host() + ':' +
    QString::number( GroupwiseConfig::self()->port() ) + GroupwiseConfig::self()->path();
  return url;
}

class CreateGroupwiseKcalResource : public KConfigPropagator::Change
{
  public:
    CreateGroupwiseKcalResource()
      : KConfigPropagator::Change( i18n("Create GroupWise Calendar Resource") )
    {
    }

    void apply()
    {
      KCal::CalendarResourceManager m( "calendar" );
      m.readConfig();

      KCal::ResourceGroupwise *r = new KCal::ResourceGroupwise();

      r->setResourceName( i18n("GroupWise") );
      r->prefs()->setUrl( serverUrl() );
      r->prefs()->setUser( GroupwiseConfig::self()->user() );
      r->prefs()->setPassword( GroupwiseConfig::self()->password() );
      r->setSavePolicy( KCal::ResourceCached::SaveDelayed );
      r->setReloadPolicy( KCal::ResourceCached::ReloadInterval );
      r->setReloadInterval( 20 );
      m.add( r );

      m.writeConfig();

      GroupwiseConfig::self()->setKcalResource( r->identifier() );
    }
};

class UpdateGroupwiseKcalResource : public KConfigPropagator::Change
{
  public:
    UpdateGroupwiseKcalResource()
      : KConfigPropagator::Change( i18n("Update GroupWise Calendar Resource") )
    {
    }

    void apply()
    {
      KCal::CalendarResourceManager m( "calendar" );
      m.readConfig();

      KCal::CalendarResourceManager::Iterator it;
      for ( it = m.begin(); it != m.end(); ++it ) {
        if ( (*it)->identifier() == GroupwiseConfig::kcalResource() ) {
          KCal::ResourceGroupwise *r = static_cast<KCal::ResourceGroupwise *>( *it );
          r->prefs()->setUrl( serverUrl() );
          r->prefs()->setUser( GroupwiseConfig::self()->user() );
          r->prefs()->setPassword( GroupwiseConfig::self()->password() );
          // TODO: don't change existing policy
          r->setSavePolicy( KCal::ResourceCached::SaveDelayed );
          r->setReloadPolicy( KCal::ResourceCached::ReloadInterval );
          r->setReloadInterval( 20 );
        }
      }
      m.writeConfig();
    }
};

class CreateGroupwiseKabcResource : public KConfigPropagator::Change
{
  public:
    CreateGroupwiseKabcResource()
      : KConfigPropagator::Change( i18n("Create GroupWise Address Book Resource") )
    {
    }

    void apply()
    {
      KRES::Manager<KABC::Resource> m( "contact" );
      m.readConfig();

      QString url = serverUrl();
      QString user( GroupwiseConfig::self()->user() );
      QString password( GroupwiseConfig::self()->password() );

      KABC::ResourceGroupwise *r = new KABC::ResourceGroupwise( url, user,
                                                                password,
                                                                QStringList(),
                                                                QString() );
      r->setResourceName( i18n("GroupWise") );
      m.add( r );
      m.writeConfig();

      GroupwiseConfig::self()->setKabcResource( r->identifier() );
    }
};

class UpdateGroupwiseKabcResource : public KConfigPropagator::Change
{
  public:
    UpdateGroupwiseKabcResource()
      : KConfigPropagator::Change( i18n("Update GroupWise Address Book Resource") )
    {
    }

    void apply()
    {
      KRES::Manager<KABC::Resource> m( "contact" );
      m.readConfig();

      KRES::Manager<KABC::Resource>::Iterator it;
      for ( it = m.begin(); it != m.end(); ++it ) {
        if ( (*it)->identifier() == GroupwiseConfig::kabcResource() ) {
          KABC::ResourceGroupwise *r = static_cast<KABC::ResourceGroupwise *>( *it );
          r->prefs()->setUrl( serverUrl() );
          r->prefs()->setUser( GroupwiseConfig::self()->user() );
          r->prefs()->setPassword( GroupwiseConfig::self()->password() );
        }
      }
      m.writeConfig();
    }
};


class GroupwisePropagator : public KConfigPropagator
{
  public:
    GroupwisePropagator()
      : KConfigPropagator( GroupwiseConfig::self(), "groupwise.kcfg" )
    {
    }

    ~GroupwisePropagator()
    {
      GroupwiseConfig::self()->writeConfig();
    }

  protected:
    class Writer : public CreateDisconnectedImapAccount::CustomWriter
    {
      public:
	void writeFolder( KConfig &, int ) {}
	void writeIds( int accountId, int transportId )
	{
          GroupwiseConfig::setKMailAccountId( accountId );
	  GroupwiseConfig::setKMailTransportId( transportId );
	}
    };

    void addCustomChanges( Change::List &changes )
    {
      ChangeConfig *c = new ChangeConfig;
      c->file = "korganizerrc";
      c->group = "FreeBusy";
      c->name = "FreeBusyRetrieveUrl";
      c->value = "groupwise://" + GroupwiseConfig::self()->host() + GroupwiseConfig::self()->path() +
        "/freebusy/";
      changes.append( c );

      KCal::CalendarResourceManager m1( "calendar" );
      m1.readConfig();
      KCal::CalendarResourceManager::Iterator it;
      for ( it = m1.begin(); it != m1.end(); ++it ) {
        if ( (*it)->type() == "groupwise" ) break;
      }
      if ( it == m1.end() ) {
        changes.append( new CreateGroupwiseKcalResource );
      } else {
        if ( (*it)->identifier() == GroupwiseConfig::kcalResource() ) {
          KCal::GroupwisePrefsBase *prefs =
            static_cast<KCal::ResourceGroupwise *>( *it )->prefs();
          if ( prefs->url() != serverUrl() ||
               prefs->port() != GroupwiseConfig::self()->port() ||
               prefs->user() != GroupwiseConfig::user() ||
               prefs->password() != GroupwiseConfig::password() ) {
            changes.append( new UpdateGroupwiseKcalResource );
          }
        }
      }

      KRES::Manager<KABC::Resource> m2( "contact" );
      m2.readConfig();
      KRES::Manager<KABC::Resource>::Iterator it2;
      for ( it2 = m2.begin(); it2 != m2.end(); ++it2 ) {
        if ( (*it2)->type() == "groupwise" ) break;
      }
      if ( it2 == m2.end() ) {
        changes.append( new CreateGroupwiseKabcResource );
      } else {
        if ( (*it2)->identifier() == GroupwiseConfig::kabcResource() ) {
          KABC::GroupwisePrefs *prefs = static_cast<KABC::ResourceGroupwise *>( *it2 )->prefs();
          if ( prefs->url() != serverUrl() ||
               prefs->user() != GroupwiseConfig::user() ||
               prefs->password() != GroupwiseConfig::password() ) {
            changes.append( new UpdateGroupwiseKabcResource );
          }
        }
      }

      if ( GroupwiseConfig::createEmailAccount() ) {
        CreateDisconnectedImapAccount *ca =
          new CreateDisconnectedImapAccount( i18n("GroupWise") );

        ca->setServer( GroupwiseConfig::host() );
        ca->setUser( GroupwiseConfig::user() );
        ca->setPassword( GroupwiseConfig::password() );
        ca->setRealName( GroupwiseConfig::fullName() );
        QString email = GroupwiseConfig::email();
        if ( !email.isEmpty() ) ca->setEmail( email );
        ca->enableSavePassword( true );
        ca->enableSieve( false );
        ca->setEncryption( CreateDisconnectedImapAccount::SSL );
        ca->setAuthentication( CreateDisconnectedImapAccount::PLAIN );
        ca->setAuthenticationSend( CreateDisconnectedImapAccount::PLAIN );
        ca->setSmtpPort( 25 );

        ca->setExistingAccountId( GroupwiseConfig::kMailAccountId() );
        ca->setExistingTransportId( GroupwiseConfig::kMailTransportId() );

        ca->setCustomWriter( new Writer );

        changes.append( ca );
      }
    }
};

GroupwiseWizard::GroupwiseWizard() : KConfigWizard( new GroupwisePropagator )
{
  QWidget *page = createWizardPage( i18n("Novell GroupWise") );

  QGridLayout *topLayout = new QGridLayout;
  topLayout->setSpacing( spacingHint() );
  page->setLayout(topLayout);

  QLabel *label = new QLabel( i18n("Server name:"));
  topLayout->addWidget( label, 0, 0 );
  mServerEdit = new KLineEdit;
  topLayout->addWidget( mServerEdit, 0, 1 );

  label = new QLabel( i18n("Path to SOAP interface:"));
  topLayout->addWidget( label, 1, 0 );
  mPathEdit = new KLineEdit;
  topLayout->addWidget( mPathEdit, 1, 1 );

  label = new QLabel( i18n("Port:"));
  topLayout->addWidget( label, 2, 0 );
  mPortEdit = new QSpinBox( 1, 65536, 1, this );
  topLayout->addWidget( mPortEdit, 2, 1 );

  label = new QLabel( i18n("User name:") );
  topLayout->addWidget( label, 3, 0 );
  mUserEdit = new KLineEdit;
  topLayout->addWidget( mUserEdit, 3, 1 );

  label = new QLabel( i18n("Password:"));
  topLayout->addWidget( label, 4, 0 );
  mPasswordEdit = new KLineEdit;
  mPasswordEdit->setEchoMode( KLineEdit::Password );
  topLayout->addWidget( mPasswordEdit, 4, 1 );

  mSavePasswordCheck = new QCheckBox( i18n("Save password"));
  topLayout->addWidget( mSavePasswordCheck, 5, 0, 1, 2 );

  mSecureCheck = new QCheckBox( i18n("Encrypt communication with server"));
  topLayout->addWidget( mSecureCheck, 5, 0, 2, 2 );

  topLayout->setRowStretch( 6, 1 );


  mEmailPage = createWizardPage( i18n("Mail") );

  topLayout = new QGridLayout;
  topLayout->setSpacing( spacingHint() );
  mEmailPage->setLayout(topLayout);

  mEmailBox = new QGroupBox( i18n("Create Mail Account"));
  mEmailBox->setCheckable( true );
  topLayout->addWidget( mEmailBox, 0, 0 );

  QGridLayout *accountLayout= new QGridLayout;
  accountLayout->setSpacing( spacingHint() );

  label = new QLabel( i18n("Email address:"));
  accountLayout->addWidget( label, 0, 0 );

  mEmailEdit = new KLineEdit;
  accountLayout->addWidget( mEmailEdit, 0, 1 );

  label = new QLabel( i18n("Full name:"));
  accountLayout->addWidget( label, 1, 0 );

  mFullNameEdit = new KLineEdit;
  accountLayout->addWidget( mFullNameEdit, 1, 1 );

  accountLayout->setRowStretch( 2, 1 );

  mEmailBox->setLayout(accountLayout);

  connect( this, SIGNAL( currentPageChanged( KPageWidgetItem *, KPageWidgetItem * ) ),
    SLOT( slotAboutToShowPage( KPageWidgetItem *, KPageWidgetItem * ) ) );


  setupRulesPage();
  setupChangesPage();

  resize( 600, 400 );
}

GroupwiseWizard::~GroupwiseWizard()
{
}

QString GroupwiseWizard::validate()
{
  if( mServerEdit->text().isEmpty() ||
      mPathEdit->text().isEmpty() ||
      mPortEdit->text().isEmpty() ||
      mUserEdit->text().isEmpty() ||
      mPasswordEdit->text().isEmpty() )
    return i18n( "Please fill in all fields." );

  if ( mEmailBox->isChecked() ) {
    if( !KPIMUtils::isValidSimpleAddress( mEmailEdit->text() ) )
      return i18n("Invalid email address entered.");
    if( mFullNameEdit->text().isEmpty() )
      return i18n( "Please fill in all fields." );
  }

  return QString();
}

void GroupwiseWizard::usrReadConfig()
{
  mServerEdit->setText( GroupwiseConfig::self()->host() );
  mPathEdit->setText( GroupwiseConfig::self()->path() );
  mPortEdit->setValue( GroupwiseConfig::self()->port() );
  mUserEdit->setText( GroupwiseConfig::self()->user() );
  mPasswordEdit->setText( GroupwiseConfig::self()->password() );
  mSavePasswordCheck->setChecked( GroupwiseConfig::self()->savePassword() );
  mSecureCheck->setChecked( GroupwiseConfig::self()->useHttps() );
  mEmailEdit->setText( GroupwiseConfig::self()->email() );
  mFullNameEdit->setText( GroupwiseConfig::fullName() );
  mEmailBox->setChecked( GroupwiseConfig::createEmailAccount() );
}

void GroupwiseWizard::usrWriteConfig()
{
  GroupwiseConfig::self()->setHost( mServerEdit->text() );
  GroupwiseConfig::self()->setPath( mPathEdit->text() );
  GroupwiseConfig::self()->setPort( mPortEdit->value() );
  GroupwiseConfig::self()->setUser( mUserEdit->text() );
  GroupwiseConfig::self()->setPassword( mPasswordEdit->text() );
  GroupwiseConfig::self()->setSavePassword( mSavePasswordCheck->isChecked() );
  GroupwiseConfig::self()->setUseHttps( mSecureCheck->isChecked() );
  GroupwiseConfig::setEmail( mEmailEdit->text() );
  GroupwiseConfig::setFullName( mFullNameEdit->text() );
  GroupwiseConfig::setCreateEmailAccount( mEmailBox->isChecked() );
}

void GroupwiseWizard::slotAboutToShowPage( KPageWidgetItem * page, KPageWidgetItem * )
{
  if ( page->widget() == mEmailPage ) {
    if ( mEmailEdit->text().isEmpty() ) {
      QString host = mServerEdit->text();
      int pos = host.lastIndexOf( "." );
      if ( pos > 0 ) pos = host.lastIndexOf( ".", pos - 1 );
      if ( pos > 0 ) host = host.mid( pos + 1 );
      QString email = mUserEdit->text() + '@' + host;
      mEmailEdit->setText( email );
    }
  }
}

#include "groupwisewizard.moc"
