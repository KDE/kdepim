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

#include "exchangewizard.h"

#include "kresources/newexchange/kabc_resourceexchange.h"
#include "kresources/newexchange/kcal_resourceexchange.h"

#include "kresources/lib/folderconfig.h"

#include <libkcal/resourcecalendar.h>

#include <klineedit.h>
#include <klocale.h>

#include <qlayout.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qspinbox.h>

#if 0

i18n("Please select folders for addressbook:")
i18n("Please select folders for calendar, to-dos and journals:")

#endif


class CreateExchangeKcalResource : public KConfigPropagator::Change
{
  public:
    CreateExchangeKcalResource()
      : KConfigPropagator::Change( i18n("Create Exchange Calendar Resource") )
    {
    }

    void apply()
    {
      #if 0
      KCal::CalendarResourceManager m( "calendar" );
      m.readConfig();

      KURL url( exchangeUrl() );

      KCal::ResourceExchange *r = new KCal::ResourceExchange( new KConfig );// url );
      r->setResourceName( i18n("Exchange Server") );
      r->prefs()->setUser( ExchangeConfig::self()->user() );
      r->prefs()->setPassword( ExchangeConfig::self()->password() );
      r->setSavePolicy( KCal::ResourceCached::SaveDelayed );
      r->setReloadPolicy( KCal::ResourceCached::ReloadInterval );
      r->setReloadInterval( 20 );
      m.add( r );
      m.writeConfig();
      
      ExchangeConfig::self()->setKcalResource( r->identifier() );
      #endif
    }
};

class UpdateExchangeKcalResource : public KConfigPropagator::Change
{
  public:
    UpdateExchangeKcalResource()
      : KConfigPropagator::Change( i18n("Update Exchange Calendar Resource") )
    {
    }

    void apply()
    {
      #if 0
      KCal::CalendarResourceManager m( "calendar" );
      m.readConfig();

      KURL url( exchangeUrl() );

      KCal::CalendarResourceManager::Iterator it;
      for ( it = m.begin(); it != m.end(); ++it ) {
        if ( (*it)->identifier() == ExchangeConfig::kcalResource() ) {
          KCal::ResourceExchange *r = static_cast<KCal::ResourceExchange *>( *it );
          r->prefs()->setUrl( url.url() );
          r->prefs()->setUser( ExchangeConfig::self()->user() );
          r->prefs()->setPassword( ExchangeConfig::self()->password() );
          r->setSavePolicy( KCal::ResourceCached::SaveDelayed );
          r->setReloadPolicy( KCal::ResourceCached::ReloadInterval );
          r->setReloadInterval( 20 );
        }
      }
      m.writeConfig();
      #endif
    }
};

class CreateExchangeKabcResource : public KConfigPropagator::Change
{
  public:
    CreateExchangeKabcResource()
      : KConfigPropagator::Change( i18n("Create Exchange Addressbook Resource") )
    {
    }

    void apply()
    {
      #if 0
      KRES::Manager<KABC::Resource> m( "contact" );
      m.readConfig();

      KURL url( exchangeUrl() );
      QString user( ExchangeConfig::self()->user() );
      QString password( ExchangeConfig::self()->password() );

      KABC::ResourceExchange *r = new KABC::ResourceExchange( url, user, password );
      r->setResourceName( i18n("Exchange Server") );
      m.add( r );
      m.writeConfig();

      ExchangeConfig::self()->setKabcResource( r->identifier() );
      #endif
    }
};

class UpdateExchangeKabcResource : public KConfigPropagator::Change
{
  public:
    UpdateExchangeKabcResource()
      : KConfigPropagator::Change( i18n("Update Exchange Addressbook Resource") )
    {
    }

    void apply()
    {
      # if 0
      KRES::Manager<KABC::Resource> m( "contact" );
      m.readConfig();

      KURL url( exchangeUrl() );

      KRES::Manager<KABC::Resource>::Iterator it;
      for ( it = m.begin(); it != m.end(); ++it ) {
        if ( (*it)->identifier() == ExchangeConfig::kabcResource() ) {
          KABC::ResourceExchange *r = static_cast<KABC::ResourceExchange *>( *it );
          r->prefs()->setUrl( url.url() );
          r->prefs()->setUser( ExchangeConfig::self()->user() );
          r->prefs()->setPassword( ExchangeConfig::self()->password() );
        }
      }
      m.writeConfig();
      #endif
    }
};


class ExchangePropagator : public KConfigPropagator
{
  public:
    ExchangePropagator()
      : KConfigPropagator( /*ExchangeConfig::self()*/ 0, "exchange.kcfg" )
    {
    }

    ~ExchangePropagator()
    {
      //ExchangeConfig::self()->writeConfig();
    }

  protected:
    void addCustomChanges( Change::List &changes )
    {
      #if 0
      KCal::CalendarResourceManager m1( "calendar" );
      m1.readConfig();
      KCal::CalendarResourceManager::Iterator it;
      for ( it = m1.begin(); it != m1.end(); ++it ) {
        if ( (*it)->type() == "exchange" ) break;
      }
      if ( it == m1.end() ) {
        changes.append( new CreateExchangeKcalResource );
      } else {
        if ( (*it)->identifier() == ExchangeConfig::kcalResource() ) {
          KCal::ExchangePrefs *prefs = static_cast<KCal::ResourceExchange *>( *it )->prefs();
          if ( prefs->url() != exchangeUrl() ||
               prefs->user() != ExchangeConfig::user() ||
               prefs->password() != ExchangeConfig::password() ) {
            changes.append( new UpdateExchangeKcalResource );
          }
        }
      }
      KRES::Manager<KABC::Resource> m2( "contact" );
      m2.readConfig();
      KRES::Manager<KABC::Resource>::Iterator it2;
      for ( it2 = m2.begin(); it2 != m2.end(); ++it2 ) {
        if ( (*it2)->type() == "exchange" ) break;
      }
      if ( it2 == m2.end() ) {
        changes.append( new CreateExchangeKabcResource );
      } else {
        if ( (*it2)->identifier() == ExchangeConfig::kabcResource() ) {
          KABC::ExchangePrefs *prefs = static_cast<KABC::ResourceExchange *>( *it2 )->prefs();
          if ( prefs->url() != exchangeUrl() ||
               prefs->user() != ExchangeConfig::user() ||
               prefs->password() != ExchangeConfig::password() ) {
            changes.append( new UpdateExchangeKabcResource );
          }
        }
      }
      #endif
    }
};

ExchangeWizard::ExchangeWizard() : KConfigWizard( new ExchangePropagator )
{
  QFrame *page = createWizardPage( i18n("Microsoft Exchange Server") );
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

#if 0
  KPIM::FolderConfig *fcfg = new KPIM::FolderConfig( page );
  topLayout->addMultiCellWidget( fcfg , 5, 5, 0, 1 );
#endif

  topLayout->setRowStretch( 5, 1 );

  setupRulesPage();
  setupChangesPage();

  resize( 400, 300 );
}

ExchangeWizard::~ExchangeWizard()
{
}

void ExchangeWizard::usrReadConfig()
{
#if 0
  mServerEdit->setText( ExchangeConfig::self()->server() );
  mUserEdit->setText( ExchangeConfig::self()->user() );
  mPasswordEdit->setText( ExchangeConfig::self()->password() );
  mSavePasswordCheck->setChecked( ExchangeConfig::self()->savePassword() );
  mSecureCheck->setChecked( ExchangeConfig::self()->useHttps() );
#endif
}

void ExchangeWizard::usrWriteConfig()
{
#if 0
  ExchangeConfig::self()->setServer( mServerEdit->text() );
  ExchangeConfig::self()->setUser( mUserEdit->text() );
  ExchangeConfig::self()->setPassword( mPasswordEdit->text() );
  ExchangeConfig::self()->setSavePassword( mSavePasswordCheck->isChecked() );
  ExchangeConfig::self()->setUseHttps( mSecureCheck->isChecked() );
#endif
}
