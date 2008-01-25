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

#include "kolabwizard.h"
#include "kolabconfig.h"

#include "kolabkmailchanges.h"

#include "kresources/kolab/kcal/resourcekolab.h"
#include "kresources/kolab/kabc/resourcekolab.h"
#include "kresources/kolab/knotes/resourcekolab.h"

#include <libkcal/resourcecalendar.h>
#include <kabc/resource.h>

#include <klineedit.h>
#include <klocale.h>

#include <qcheckbox.h>
#include <qhbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qwhatsthis.h>

class SetupLDAPSearchAccount : public KConfigPropagator::Change
{
  public:
    SetupLDAPSearchAccount()
      : KConfigPropagator::Change( i18n("Setup LDAP Search Account") )
    {
    }

    void apply()
    {
      const QString host = KolabConfig::self()->server();

      // Figure out the basedn
      QString basedn = host;
      // If the user gave a full email address, the domain name
      // of that overrides the server name for the ldap dn
      const QString user = KolabConfig::self()->user();
      int pos = user.find( "@" );
      if ( pos > 0 ) {
        const QString h = user.mid( pos+1 );
        if ( !h.isEmpty() )
          // The user did type in a domain on the email address. Use that
          basedn = h;
      }
      { // while we're here, write default domain
        KConfig c( "kmailrc" );
        c.setGroup( "General" );
        c.writeEntry( "Default domain", basedn );
      }

      basedn.replace(".",",dc=");
      basedn.prepend("dc=");

      // Set the changes
      KConfig c( "kabldaprc" );
      c.setGroup( "LDAP" );
      bool hasMyServer = false;
      uint selHosts = c.readNumEntry("NumSelectedHosts", 0);
      for ( uint i = 0 ; i < selHosts && !hasMyServer; ++i )
        if ( c.readEntry( QString("SelectedHost%1").arg(i) ) == host )
          hasMyServer = true;
      if ( !hasMyServer ) {
        c.writeEntry( "NumSelectedHosts", selHosts + 1 );
        c.writeEntry( QString("SelectedHost%1").arg(selHosts), host);
        c.writeEntry( QString("SelectedBase%1").arg(selHosts), basedn);
        c.writeEntry( QString("SelectedPort%1").arg(selHosts), "389");
      }
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
      KCal::ResourceKolab *r = new KCal::ResourceKolab( 0 );
      r->setResourceName( i18n("Kolab Server") );
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
      KABC::ResourceKolab *r = new KABC::ResourceKolab( 0 );
      r->setResourceName( i18n("Kolab Server") );
      m.add( r );
      m.setStandardResource( r );
      m.writeConfig();
    }

};

class CreateNotesImapResource : public KConfigPropagator::Change
{
  public:
    CreateNotesImapResource()
      : KConfigPropagator::Change( i18n("Create Notes IMAP Resource") )
    {
    }

    void apply()
    {
      KRES::Manager<ResourceNotes> m( "notes" );
      m.readConfig();
      Kolab::ResourceKolab *r = new Kolab::ResourceKolab( 0 );
      r->setResourceName( i18n("Kolab Server") );
      m.add( r );
      m.setStandardResource( r );
      m.writeConfig();
    }

};


class KolabPropagator : public KConfigPropagator
{
  public:
    KolabPropagator()
      : KConfigPropagator( KolabConfig::self(), "kolab.kcfg" )
    {
    }

  protected:
    void addKorganizerChanges( Change::List &changes )
    {
      KURL freeBusyBaseUrl;
      // usrWriteConfig() is called first, so kolab1Legacy is correct
      if ( KolabConfig::self()->kolab1Legacy() ) {
        freeBusyBaseUrl = "webdavs://" + KolabConfig::self()->server() +
                                "/freebusy/";

      ChangeConfig *c = new ChangeConfig;
      c->file = "korganizerrc";
      c->group = "FreeBusy";

      c->name = "FreeBusyPublishUrl";

      QString user = KolabConfig::self()->user();
      // We now use the full email address in the freebusy URL
      //int pos = user.find( "@" );
      //if ( pos > 0 ) user = user.left( pos );

      KURL publishURL = freeBusyBaseUrl;
      publishURL.addPath( user + ".ifb" ); // this encodes the '@' in the username
      c->value = publishURL.url();

      changes.append( c );

      } else {
          // Kolab2: only need FreeBusyRetrieveUrl
          // "Uploading" is done by triggering a server-side script with an HTTP GET
          // (done by kmail)
          freeBusyBaseUrl = "https://" + KolabConfig::self()->server() +
                            "/freebusy/";
      }

      ChangeConfig *c = new ChangeConfig;
      c->file = "korganizerrc";
      c->group = "FreeBusy";
      c->name = "FreeBusyRetrieveUrl";
      c->value = freeBusyBaseUrl.url();
      changes.append( c );

      // Use full email address for retrieval of free/busy lists
      c = new ChangeConfig;
      c->file = "korganizerrc";
      c->group = "FreeBusy";
      c->name = "FreeBusyFullDomainRetrieval";
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
        if ( (*it)->type() == "imap" ) break;
      }
      if ( it == m.end() ) {
        changes.append( new CreateCalendarImapResource );
        changes.append( new CreateContactImapResource );
        changes.append( new CreateNotesImapResource );
      }
    }
};

KolabWizard::KolabWizard() : KConfigWizard( new KolabPropagator )
{
  QFrame *page = createWizardPage( i18n("Kolab Server") );

  QGridLayout *topLayout = new QGridLayout( page );
  topLayout->setSpacing( spacingHint() );

  QLabel *label = new QLabel( i18n("Server name:"), page );
  topLayout->addWidget( label, 0, 0 );
  mServerEdit = new KLineEdit( page );
  topLayout->addWidget( mServerEdit, 0, 1 );

  label = new QLabel( i18n("Email address:"), page );
  topLayout->addWidget( label, 1, 0 );
  mUserEdit = new KLineEdit( page );
  topLayout->addWidget( mUserEdit, 1, 1 );
  QWhatsThis::add(mUserEdit, i18n("Your email address on the Kolab Server. "
                        "Format: <i>name@example.net</i>"));

  label = new QLabel( i18n("Real name:"), page );
  topLayout->addWidget( label, 2, 0 );
  mRealNameEdit = new KLineEdit( page );
  topLayout->addWidget( mRealNameEdit, 2, 1 );

  label = new QLabel( i18n("Password:"), page );
  topLayout->addWidget( label, 3, 0 );
  mPasswordEdit = new KLineEdit( page );
  mPasswordEdit->setEchoMode( KLineEdit::Password );
  topLayout->addWidget( mPasswordEdit, 3, 1 );

  mSavePasswordCheck = new QCheckBox( i18n("Save password"), page );
  topLayout->addMultiCellWidget( mSavePasswordCheck, 4, 4, 0, 1 );

  topLayout->setRowStretch( 4, 1 );

  mUseOnlineForNonGroupwareCheck = new QCheckBox( i18n("Use an online IMAP account for non-groupware folders"), page );
  topLayout->addMultiCellWidget( mUseOnlineForNonGroupwareCheck, 5, 5, 0, 1 );
  topLayout->setRowStretch( 5, 1 );

  QButtonGroup *bg = new QHButtonGroup(i18n("Server Version"), page );
  QWhatsThis::add( bg, i18n("Choose the version of the Kolab Server you are using.") );
  mKolab1 = new QRadioButton( i18n ( "Kolab 1" ), bg );
  mKolab2 = new QRadioButton( i18n ( "Kolab 2" ), bg );
  topLayout->addMultiCellWidget( bg, 6, 6, 0, 1 );

  setInitialSize( QSize( 600, 300 ) );
}

KolabWizard::~KolabWizard()
{
}

QString KolabWizard::validate()
{
  if( mServerEdit->text().isEmpty() ||
      mUserEdit->text().isEmpty() ||
      mRealNameEdit->text().isEmpty() ||
      mPasswordEdit->text().isEmpty() )
    return i18n( "Please fill in all fields." );
  return QString::null;
}

void KolabWizard::usrReadConfig()
{
  mServerEdit->setText( KolabConfig::self()->server() );
  mUserEdit->setText( KolabConfig::self()->user() );
  mRealNameEdit->setText( KolabConfig::self()->realName() );
  mPasswordEdit->setText( KolabConfig::self()->password() );
  mSavePasswordCheck->setChecked( KolabConfig::self()->savePassword() );
  mKolab1->setChecked( KolabConfig::self()->kolab1Legacy() );
  mKolab2->setChecked( !KolabConfig::self()->kolab1Legacy() );
  mUseOnlineForNonGroupwareCheck->setChecked( KolabConfig::self()->useOnlineForNonGroupware() );
}

void KolabWizard::usrWriteConfig()
{
  KolabConfig::self()->setServer( mServerEdit->text() );
  KolabConfig::self()->setUser( mUserEdit->text() );
  KolabConfig::self()->setRealName( mRealNameEdit->text() );
  KolabConfig::self()->setPassword( mPasswordEdit->text() );
  KolabConfig::self()->setSavePassword( mSavePasswordCheck->isChecked() );
  KolabConfig::self()->setKolab1Legacy( mKolab1->isChecked() );
  KolabConfig::self()->setUseOnlineForNonGroupware( mUseOnlineForNonGroupwareCheck->isChecked() );
}
