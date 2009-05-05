/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Daniel Molkentin <molkentin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "scalixwizard.h"
#include "scalixconfig.h"

#include "scalixkmailchanges.h"

#include "libkdepim/kconfigpropagator.h"

#include "kresources/scalix/kcal/resourcescalix.h"
#include "kresources/scalix/kabc/resourcescalix.h"

#include <kcal/resourcecalendar.h>
#include <kabc/resource.h>

#include <kcombobox.h>
#include <kdbusservicestarter.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstringhandler.h>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <qapplication.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwhatsthis.h>

#include <unistd.h>

class SetupLDAPSearchAccount : public KConfigPropagator::Change
{
  public:
    SetupLDAPSearchAccount()
      : KConfigPropagator::Change( i18n("Setup LDAP Search Account") )
    {
    }

    void apply()
    {
      const QString host = ScalixConfig::self()->server();

      QString basedn( "o=Scalix" );

      { // while we're here, write default domain
        KConfig c( "kmailrc" );
        KConfigGroup g = c.group( "General" );
        g.writeEntry( "Default domain", basedn );
      }

      // Set the changes
      KConfig c( "kabldaprc" );
      KConfigGroup g = c.group( "LDAP" );
      bool hasMyServer = false;
      uint selHosts = g.readEntry("NumSelectedHosts", 0);
      for ( uint i = 0 ; i < selHosts && !hasMyServer; ++i )
        if ( g.readEntry( QString("SelectedHost%1").arg(i), QString() ) == host )
          hasMyServer = true;
      if ( !hasMyServer ) {
        g.writeEntry( "NumSelectedHosts", selHosts + 1 );
        g.writeEntry( QString("SelectedHost%1").arg(selHosts), host);
        g.writeEntry( QString("SelectedBase%1").arg(selHosts), basedn);
        g.writeEntry( QString("SelectedPort%1").arg(selHosts), "389");
      }
    }

};

class SetupScalixAdmin : public KConfigPropagator::Change
{
  public:
    SetupScalixAdmin()
      : KConfigPropagator::Change( i18n( "Setup ScalixAdmin Account" ) )
    {
    }

    void apply()
    {
      KConfig c( "scalixadminrc" );
      KConfigGroup g = c.group( "Account" );

      g.writeEntry( "user", ScalixConfig::self()->user() );
      g.writeEntry( "pass", KStringHandler::obscure( ScalixConfig::self()->password() ) );
      g.writeEntry( "host", ScalixConfig::self()->server() );
      if ( ScalixConfig::self()->security() == ScalixConfig::None )
        g.writeEntry( "port", 143 );
      else
        g.writeEntry( "port", 993 );

      switch ( ScalixConfig::self()->security() ) {
        case ScalixConfig::None:
          g.writeEntry( "use-ssl", "false" );
          g.writeEntry( "use-tls", "false" );
          break;
        case ScalixConfig::TLS:
          g.writeEntry( "use-ssl", "false" );
          g.writeEntry( "use-tls", "true" );
          break;
        case ScalixConfig::SSL:
          g.writeEntry( "use-ssl", "true" );
          g.writeEntry( "use-tls", "false" );
          break;
      }
      switch ( ScalixConfig::self()->authentication() ) {
        case ScalixConfig::Password:
          g.writeEntry( "auth", "*" );
          break;
        case ScalixConfig::NTLM_SPA:
          g.writeEntry( "auth", "NTLM" );
          break;
        case ScalixConfig::GSSAPI:
          g.writeEntry( "auth", "GSSAPI" );
          break;
        case ScalixConfig::DIGEST_MD5:
          g.writeEntry( "auth", "DIGEST-MD5" );
          break;
        case ScalixConfig::CRAM_MD5:
          g.writeEntry( "auth", "CRAM-MD5" );
          break;
      }

      g = c.group( "LDAP" );

      g.writeEntry( "host", ScalixConfig::self()->server() );
      g.writeEntry( "port", "389" );
      g.writeEntry( "base", "o=Scalix" );
      g.writeEntry( "bindDn", "" );
      g.writeEntry( "password", "" );
    }
};

class CreateCalendarImapResource : public KConfigPropagator::Change
{
  public:
    CreateCalendarImapResource()
      : KConfigPropagator::Change( i18n("Create Calendar IMAP Resource") )
    {
    }

    void apply()
    {
      KCal::CalendarResourceManager m( "calendar" );
      m.readConfig();
      KCal::ResourceScalix *r = new KCal::ResourceScalix();
      r->setResourceName( i18n("Scalix Server") );
      m.add( r );
      m.setStandardResource( r );
      m.writeConfig();
    }
};

class CreateContactImapResource : public KConfigPropagator::Change
{
  public:
    CreateContactImapResource()
      : KConfigPropagator::Change( i18n("Create Contact IMAP Resource") )
    {
    }

    void apply()
    {
      KRES::Manager<KABC::Resource> m( "contact" );
      m.readConfig();
      KABC::ResourceScalix *r = new KABC::ResourceScalix();
      r->setResourceName( i18n("Scalix Server") );
      m.add( r );
      m.setStandardResource( r );
      m.writeConfig();
    }

};

class SynchronizeScalixAccount : public KConfigPropagator::Change
{
  public:
    SynchronizeScalixAccount()
      : KConfigPropagator::Change( i18n("Synchronize Scalix Account") )
    {
    }

    void apply()
    {
      QMessageBox *msg = new QMessageBox( qApp->mainWidget() );
      msg->setText( i18n("Preparing initial synchronization with Scalix server...") );
      msg->show();
      qApp->processEvents();
      sleep( 1 );
      qApp->processEvents();

      QString error;
      QString dbusService;
      int result = KDBusServiceStarter::self()->
        findServiceFor( "DBUS/ResourceBackend/IMAP", QString(),
                        &error, &dbusService );

      if ( result != 0 ) {
        KMessageBox::error( 0, i18n( "Unable to start KMail to trigger initial synchronization with Scalix server" ) );
        delete msg;
        return;
      }

      QDBusInterface ref(dbusService, "/KMail" , "org.kde.kmail.kmail", QDBusConnection::sessionBus());

      // loop until dcop iface is set up correctly
      QStringList list;
      while ( list.isEmpty() ) {
        QDBusReply<QStringList> ret = ref.call( "accounts" );
        list = ret;
      }

      ref.call( "checkAccount", i18n( "Scalix Server" ) );

      // ugly hack, but kmail needs a second before accepting the second dcop call
      sleep( 5 );
      ref.call( "checkAccount", i18n( "Scalix Server" ) );

      delete msg;
    }
};


class ScalixPropagator : public KConfigPropagator
{
  public:
    ScalixPropagator()
      : KConfigPropagator( ScalixConfig::self(), "scalix.kcfg" )
    {
    }

  protected:
    void addKorganizerChanges( Change::List &changes )
    {
      KUrl freeBusyBaseUrl = "scalix://" + ScalixConfig::self()->server() + "/freebusy/";
      freeBusyBaseUrl.setUser( ScalixConfig::self()->user() );

      ChangeConfig *c = new ChangeConfig;
      c->file = "korganizerrc";
      c->group = "FreeBusy";
      c->name = "FreeBusyRetrieveUrl";
      c->value = freeBusyBaseUrl.url() + ScalixConfig::self()->eMail();
      changes.append( c );

      c = new ChangeConfig;
      c->file = "korganizerrc";
      c->group = "FreeBusy";
      c->name = "FreeBusyRetrieveUser";
      c->value = ScalixConfig::self()->user();
      changes.append( c );

      c = new ChangeConfig;
      c->file = "korganizerrc";
      c->group = "FreeBusy";
      c->name = "FreeBusyRetrievePassword";
      c->value = ScalixConfig::self()->password();
      changes.append( c );

      c = new ChangeConfig;
      c->file = "korganizerrc";
      c->group = "FreeBusy";
      c->name = "FreeBusyPublishUrl";
      c->value = freeBusyBaseUrl.url() + "Calendar/" + ScalixConfig::self()->eMail();
      changes.append( c );

      c = new ChangeConfig;
      c->file = "korganizerrc";
      c->group = "FreeBusy";
      c->name = "FreeBusyPublishUser";
      c->value = ScalixConfig::self()->user();
      changes.append( c );

      c = new ChangeConfig;
      c->file = "korganizerrc";
      c->group = "FreeBusy";
      c->name = "FreeBusyPublishPassword";
      c->value = ScalixConfig::self()->password();
      changes.append( c );

      // Use full email address for retrieval of free/busy lists
      c = new ChangeConfig;
      c->file = "korganizerrc";
      c->group = "FreeBusy";
      c->name = "FreeBusyFullDomainRetrieval";
      c->value = "true";
      changes.append( c );

      // Disable hostname checking
      c = new ChangeConfig;
      c->file = "korganizerrc";
      c->group = "FreeBusy";
      c->name = "FreeBusyCheckHostname";
      c->value = "false";
      changes.append( c );

      // Enable automatic retrieval
      c = new ChangeConfig;
      c->file = "korganizerrc";
      c->group = "FreeBusy";
      c->name = "FreeBusyRetrieveAuto";
      c->value = "true";
      changes.append( c );

      c = new ChangeConfig;
      c->file = "korganizerrc";
      c->group = "Group Scheduling";
      c->name = "Use Groupware Communication";
      c->value = "true";
      changes.append( c );

      // Use identity "from control center", i.e. from emaildefaults
      c = new ChangeConfig;
      c->file = "korganizerrc";
      c->group = "Personal Settings";
      c->name = "Use Control Center Email";
      c->value = "true";
      changes.append( c );
    }

    virtual void addCustomChanges( Change::List &changes )
    {
      addKorganizerChanges( changes );

      // KMail cruft has been outsourced to kolabkmailchanges.cpp
      createKMailChanges( changes );

      changes.append( new SetupLDAPSearchAccount );

      KCal::CalendarResourceManager m( "calendar" );
      m.readConfig();
      KCal::CalendarResourceManager::Iterator it;
      for ( it = m.begin(); it != m.end(); ++it ) {
        if ( (*it)->type() == "scalix" ) break;
      }
      if ( it == m.end() ) {
        changes.append( new CreateCalendarImapResource );
        changes.append( new CreateContactImapResource );
      }

      changes.append( new SetupScalixAdmin );

      changes.append( new SynchronizeScalixAccount );
    }
};

ScalixWizard::ScalixWizard() : KConfigWizard( new ScalixPropagator )
{
  QWidget *page = createWizardPage( i18n("Scalix Server") );

  QGridLayout *topLayout = new QGridLayout( page );
  topLayout->setSpacing( spacingHint() );

  QLabel *label = new QLabel( i18n( "Full name:" ), page );
  topLayout->addWidget( label, 0, 0 );
  mRealNameEdit = new KLineEdit( page );
  topLayout->addWidget( mRealNameEdit, 0, 1 );
  label->setBuddy( mRealNameEdit );
  mRealNameEdit->setWhatsThis( i18n( "Your full name. Example: <i>Joe User</i>" ) );

  label = new QLabel( i18n( "Email address:" ), page );
  topLayout->addWidget( label, 1, 0 );
  mEMailEdit = new KLineEdit( page );
  topLayout->addWidget( mEMailEdit, 1, 1 );
  label->setBuddy( mEMailEdit );
  mEMailEdit->setWhatsThis( i18n( "Your email address on the Scalix Server. "
                                  "Example: <i>name@crossplatform.com</i>" ) );

  label = new QLabel( i18n( "Server:" ), page );
  topLayout->addWidget( label, 2, 0 );
  mServerEdit = new KLineEdit( page );
  topLayout->addWidget( mServerEdit, 2, 1 );
  label->setBuddy( mServerEdit );
  mServerEdit->setWhatsThis( i18n( "The name or IP of the Scalix Server. "
                                   "Example: <i>scalix.domain.com</i>" ) );

  label = new QLabel( i18n("Username:"), page );
  topLayout->addWidget( label, 3, 0 );
  mUserEdit = new KLineEdit( page );
  topLayout->addWidget( mUserEdit, 3, 1 );
  label->setBuddy( mUserEdit );
  mUserEdit->setWhatsThis( i18n( "The user respectively login name. "
                                 "Example: <i>joe</i>" ) );

  label = new QLabel( i18n("Password:"), page );
  topLayout->addWidget( label, 4, 0 );
  mPasswordEdit = new KLineEdit( page );
  mPasswordEdit->setEchoMode( KLineEdit::Password );
  topLayout->addWidget( mPasswordEdit, 4, 1 );
  label->setBuddy( mPasswordEdit );
  mPasswordEdit->setWhatsThis( i18n( "The password to your login." ) );

  mSavePasswordCheck = new QCheckBox( i18n("Save password"), page );
  topLayout->addWidget( mSavePasswordCheck, 5, 0, 1, 2 );
  mSavePasswordCheck->setWhatsThis( i18n( "Shall the password be saved in KWallet?" ) );

  label = new QLabel( i18n( "Use secure connection:" ), page );
  topLayout->addWidget( label, 6, 0 );
  mSecurity = new KComboBox( page );
  mSecurity->addItem( i18n( "No Encryption" ) );
  mSecurity->addItem( i18n( "TLS Encryption" ) );
  mSecurity->addItem( i18n( "SSL Encryption" ) );
  topLayout->addWidget( mSecurity, 6, 1 );
  label->setBuddy( mSecurity );
  mSecurity->setWhatsThis( i18n( "Choose the encryption type that is supported by your server." ) );

  label = new QLabel( i18n( "Authentication type:" ), page );
  topLayout->addWidget( label, 7, 0 );
  mAuthentication = new KComboBox( page );
  mAuthentication->addItem( i18n( "Password" ) );
  mAuthentication->addItem( i18n( "NTLM / SPA" ) );
  mAuthentication->addItem( i18n( "GSSAPI" ) );
  mAuthentication->addItem( i18n( "DIGEST-MD5" ) );
  mAuthentication->addItem( i18n( "CRAM-MD5" ) );
  topLayout->addWidget( mAuthentication, 7, 1 );
  label->setBuddy( mAuthentication );
  mAuthentication->setWhatsThis( i18n( "Choose the authentication type that is supported by your server." ) );

  topLayout->setRowStretch( 8, 1 );

  //DF: I don't see the point in showing the user those pages.
  //They are very 'internal' and of no use to anyone other than developers.
  //(This is even more true for the rules page. The changes page is sort of OK)

  setupRulesPage();
  setupChangesPage();

  setInitialSize( QSize( 600, 300 ) );
}

ScalixWizard::~ScalixWizard()
{
}

QString ScalixWizard::validate()
{
  if ( mRealNameEdit->text().isEmpty() ||
       mEMailEdit->text().isEmpty() ||
       mServerEdit->text().isEmpty() ||
       mUserEdit->text().isEmpty() ||
       mPasswordEdit->text().isEmpty() )
    return i18n( "Please fill in all fields." );

  return QString();
}

void ScalixWizard::usrReadConfig()
{
  mRealNameEdit->setText( ScalixConfig::self()->realName() );
  mEMailEdit->setText( ScalixConfig::self()->eMail() );
  mServerEdit->setText( ScalixConfig::self()->server() );
  mUserEdit->setText( ScalixConfig::self()->user() );
  mPasswordEdit->setText( ScalixConfig::self()->password() );
  mSavePasswordCheck->setChecked( ScalixConfig::self()->savePassword() );

  switch ( ScalixConfig::self()->security() ) {
    default:
    case ScalixConfig::None:
      mSecurity->setCurrentIndex( 0 );
      break;
    case ScalixConfig::TLS:
      mSecurity->setCurrentIndex( 1 );
      break;
    case ScalixConfig::SSL:
      mSecurity->setCurrentIndex( 2 );
      break;
  }

  switch ( ScalixConfig::self()->authentication() ) {
    default:
    case ScalixConfig::Password:
      mAuthentication->setCurrentIndex( 0 );
      break;
    case ScalixConfig::NTLM_SPA:
      mAuthentication->setCurrentIndex( 1 );
      break;
    case ScalixConfig::GSSAPI:
      mAuthentication->setCurrentIndex( 2 );
      break;
    case ScalixConfig::DIGEST_MD5:
      mAuthentication->setCurrentIndex( 3 );
      break;
    case ScalixConfig::CRAM_MD5:
      mAuthentication->setCurrentIndex( 4 );
      break;
  }
}

void ScalixWizard::usrWriteConfig()
{
  ScalixConfig::self()->setRealName( mRealNameEdit->text() );
  ScalixConfig::self()->setEMail( mEMailEdit->text() );
  ScalixConfig::self()->setServer( mServerEdit->text() );
  ScalixConfig::self()->setUser( mUserEdit->text() );
  ScalixConfig::self()->setPassword( mPasswordEdit->text() );
  ScalixConfig::self()->setSavePassword( mSavePasswordCheck->isChecked() );

  switch ( mSecurity->currentIndex() ) {
    default:
    case 0:
      ScalixConfig::self()->setSecurity( ScalixConfig::None );
      break;
    case 1:
      ScalixConfig::self()->setSecurity( ScalixConfig::TLS );
      break;
    case 2:
      ScalixConfig::self()->setSecurity( ScalixConfig::SSL );
      break;
  }

  switch ( mAuthentication->currentIndex() ) {
    default:
    case 0:
      ScalixConfig::self()->setAuthentication( ScalixConfig::Password );
      break;
    case 1:
      ScalixConfig::self()->setAuthentication( ScalixConfig::NTLM_SPA );
      break;
    case 2:
      ScalixConfig::self()->setAuthentication( ScalixConfig::GSSAPI );
      break;
    case 3:
      ScalixConfig::self()->setAuthentication( ScalixConfig::DIGEST_MD5 );
      break;
    case 4:
      ScalixConfig::self()->setAuthentication( ScalixConfig::CRAM_MD5 );
      break;
  }
}
