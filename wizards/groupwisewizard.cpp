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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "groupwisewizard.h"
#include "groupwiseconfig.h"

#include "kresources/groupwise/kabc_groupwiseprefs.h"
#include "kresources/groupwise/kabc_resourcegroupwise.h"
#include "kresources/groupwise/kcal_groupwiseprefs.h"
#include "kresources/groupwise/kcal_resourcegroupwise.h"

#include <libkcal/resourcecalendar.h>

#include <klineedit.h>
#include <klocale.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qspinbox.h>

class CreateGroupwiseKcalResource : public KConfigPropagator::Change
{
  public:
    CreateGroupwiseKcalResource()
      : KConfigPropagator::Change( i18n("Create Groupwise Calendar Resource") )
    {
    }

    void apply()
    {
      kdDebug() << "CREATE KCAL GROUPWISE" << endl;
    
      KCal::CalendarResourceManager m( "calendar" );
      m.readConfig();

      kdDebug() << "TICK" << endl;

      KCal::ResourceGroupwise *r = new KCal::ResourceGroupwise();
      r->setResourceName( i18n("Groupwise") );
      r->prefs()->setHost( GroupwiseConfig::self()->host() );
      r->prefs()->setPort( GroupwiseConfig::self()->port() );
      r->prefs()->setUser( GroupwiseConfig::self()->user() );
      r->prefs()->setPassword( GroupwiseConfig::self()->password() );
      r->setSavePolicy( KCal::ResourceCached::SaveDelayed );
      r->setReloadPolicy( KCal::ResourceCached::ReloadInterval );
      r->setReloadInterval( 20 );
      m.add( r );

      kdDebug() << "TICK." << endl;

      m.writeConfig();

      kdDebug() << "TICK.." << endl;
      
      GroupwiseConfig::self()->setKcalResource( r->identifier() );
    }
};

class UpdateGroupwiseKcalResource : public KConfigPropagator::Change
{
  public:
    UpdateGroupwiseKcalResource()
      : KConfigPropagator::Change( i18n("Update Groupwise Calendar Resource") )
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
          r->prefs()->setHost( GroupwiseConfig::self()->host() );
          r->prefs()->setPort( GroupwiseConfig::self()->port() );
          r->prefs()->setUser( GroupwiseConfig::self()->user() );
          r->prefs()->setPassword( GroupwiseConfig::self()->password() );
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
      : KConfigPropagator::Change( i18n("Create Groupwise Addressbook Resource") )
    {
    }

    void apply()
    {
      KRES::Manager<KABC::Resource> m( "contact" );
      m.readConfig();

      KURL url;
      url.setProtocol( "http" );
      url.setHost( GroupwiseConfig::self()->host() );
      url.setPort( GroupwiseConfig::self()->port() );
      url.setPath( "/soap/" );
      QString user( GroupwiseConfig::self()->user() );
      QString password( GroupwiseConfig::self()->password() );

      KABC::ResourceGroupwise *r = new KABC::ResourceGroupwise( url.url(), user,
                                                                password,
                                                                QStringList(),
                                                                QString::null );
      r->setResourceName( i18n("Groupwise") );
      m.add( r );
      m.writeConfig();

      GroupwiseConfig::self()->setKabcResource( r->identifier() );
    }
};

class UpdateGroupwiseKabcResource : public KConfigPropagator::Change
{
  public:
    UpdateGroupwiseKabcResource()
      : KConfigPropagator::Change( i18n("Update groupwise Addressbook Resource") )
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
          r->prefs()->setHost( GroupwiseConfig::self()->host() );
          r->prefs()->setPort( GroupwiseConfig::self()->port() );
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
    void addCustomChanges( Change::List &changes )
    {
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
          KCal::GroupwisePrefs *prefs = static_cast<KCal::ResourceGroupwise *>( *it )->prefs();
          if ( prefs->host() != GroupwiseConfig::self()->host() ||
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
          if ( prefs->host() != GroupwiseConfig::self()->host() ||
               prefs->port() != GroupwiseConfig::self()->port() ||
               prefs->user() != GroupwiseConfig::user() ||
               prefs->password() != GroupwiseConfig::password() ) {
            changes.append( new UpdateGroupwiseKabcResource );
          }
        }
      }
    }
};

GroupwiseWizard::GroupwiseWizard() : KConfigWizard( new GroupwisePropagator )
{
  QFrame *page = createWizardPage( i18n("Novell Groupwise") );

  QGridLayout *topLayout = new QGridLayout( page );
  topLayout->setSpacing( spacingHint() );

  QLabel *label = new QLabel( i18n("Server name:"), page );
  topLayout->addWidget( label, 0, 0 );
  mServerEdit = new KLineEdit( page );
  topLayout->addWidget( mServerEdit, 0, 1 );

  label = new QLabel( i18n("Port:"), page );
  topLayout->addWidget( label, 1, 0 );
  mPortEdit = new QSpinBox( 1, 65536, 1, page );
  topLayout->addWidget( mPortEdit, 1, 1 );

  label = new QLabel( i18n("User name:"), page );
  topLayout->addWidget( label, 2, 0 );
  mUserEdit = new KLineEdit( page );
  topLayout->addWidget( mUserEdit, 2, 1 );

  label = new QLabel( i18n("Password:"), page );
  topLayout->addWidget( label, 3, 0 );
  mPasswordEdit = new KLineEdit( page );
  mPasswordEdit->setEchoMode( KLineEdit::Password );
  topLayout->addWidget( mPasswordEdit, 3, 1 );

  mSavePasswordCheck = new QCheckBox( i18n("Save password"), page );
  topLayout->addMultiCellWidget( mSavePasswordCheck, 4, 4, 0, 1 );

  mSecureCheck = new QCheckBox( i18n("Encrypt communication with server"),
                                page );
  topLayout->addMultiCellWidget( mSecureCheck, 5, 5, 0, 1 );

  topLayout->setRowStretch( 6, 1 );

  setupRulesPage();
  setupChangesPage();

  resize( 400, 300 );
}

GroupwiseWizard::~GroupwiseWizard()
{
}

void GroupwiseWizard::usrReadConfig()
{
  mServerEdit->setText( GroupwiseConfig::self()->host() );
  mPortEdit->setValue( GroupwiseConfig::self()->port() );
  mUserEdit->setText( GroupwiseConfig::self()->user() );
  mPasswordEdit->setText( GroupwiseConfig::self()->password() );
  mSavePasswordCheck->setChecked( GroupwiseConfig::self()->savePassword() );
  mSecureCheck->setChecked( GroupwiseConfig::self()->useHttps() );
}

void GroupwiseWizard::usrWriteConfig()
{
  GroupwiseConfig::self()->setHost( mServerEdit->text() );
  GroupwiseConfig::self()->setPort( mPortEdit->value() );
  GroupwiseConfig::self()->setUser( mUserEdit->text() );
  GroupwiseConfig::self()->setPassword( mPasswordEdit->text() );
  GroupwiseConfig::self()->setSavePassword( mSavePasswordCheck->isChecked() );
  GroupwiseConfig::self()->setUseHttps( mSecureCheck->isChecked() );
}
