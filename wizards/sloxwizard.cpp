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

#include "sloxwizard.h"
#include "sloxconfig.h"

#include "libkdepim/kconfigpropagator.h"

#include "kresources/slox/kabcsloxprefs.h"
#include "kresources/slox/kabcresourceslox.h"
#include "kresources/slox/kcalsloxprefs.h"
#include "kresources/slox/kcalresourceslox.h"

#include <kcal/resourcecalendar.h>

#include <klineedit.h>
#include <klocale.h>

#include <QLayout>
#include <QCheckBox>
#include <QLabel>


QString sloxUrl()
{
  QString url;

  if ( SloxConfig::self()->useHttps() ) url = "https://";
  else url = "http://";

  url += SloxConfig::self()->server();

  return url;
}

class CreateSloxKcalResource : public KConfigPropagator::Change
{
  public:
    CreateSloxKcalResource()
      : KConfigPropagator::Change( i18n("Create SLOX Calendar Resource") )
    {
    }

    void apply()
    {
      KCal::CalendarResourceManager m( "calendar" );
      m.readConfig();

      KUrl url( sloxUrl() );

      KCalResourceSlox *r = new KCalResourceSlox( url );
      r->setResourceName( i18n("Openexchange Server") );
      r->prefs()->setUser( SloxConfig::self()->user() );
      r->prefs()->setPassword( SloxConfig::self()->password() );
      r->setSavePolicy( KCal::ResourceCached::SaveDelayed );
      r->setReloadPolicy( KCal::ResourceCached::ReloadInterval );
      r->setReloadInterval( 20 );
      m.add( r );
      m.writeConfig();

      SloxConfig::self()->setKcalResource( r->identifier() );
    }
};

class UpdateSloxKcalResource : public KConfigPropagator::Change
{
  public:
    UpdateSloxKcalResource()
      : KConfigPropagator::Change( i18n("Update SLOX Calendar Resource") )
    {
    }

    void apply()
    {
      KCal::CalendarResourceManager m( "calendar" );
      m.readConfig();

      KUrl url( sloxUrl() );

      KCal::CalendarResourceManager::Iterator it;
      for ( it = m.begin(); it != m.end(); ++it ) {
        if ( (*it)->identifier() == SloxConfig::kcalResource() ) {
          KCalResourceSlox *r = static_cast<KCalResourceSlox *>( *it );
          r->prefs()->setUrl( url.url() );
          r->prefs()->setUser( SloxConfig::self()->user() );
          r->prefs()->setPassword( SloxConfig::self()->password() );
          r->setSavePolicy( KCal::ResourceCached::SaveDelayed );
          r->setReloadPolicy( KCal::ResourceCached::ReloadInterval );
          r->setReloadInterval( 20 );
        }
      }
      m.writeConfig();
    }
};

class CreateSloxKabcResource : public KConfigPropagator::Change
{
  public:
    CreateSloxKabcResource()
      : KConfigPropagator::Change( i18n("Create SLOX Address Book Resource") )
    {
    }

    void apply()
    {
      KRES::Manager<KABC::Resource> m( "contact" );
      m.readConfig();

      KUrl url( sloxUrl() );
      QString user( SloxConfig::self()->user() );
      QString password( SloxConfig::self()->password() );

      KABC::ResourceSlox *r = new KABC::ResourceSlox( url, user, password );
      r->setResourceName( i18n("Openexchange Server") );
      m.add( r );
      m.writeConfig();

      SloxConfig::self()->setKabcResource( r->identifier() );
    }
};

class UpdateSloxKabcResource : public KConfigPropagator::Change
{
  public:
    UpdateSloxKabcResource()
      : KConfigPropagator::Change( i18n("Update SLOX Address Book Resource") )
    {
    }

    void apply()
    {
      KRES::Manager<KABC::Resource> m( "contact" );
      m.readConfig();

      KUrl url( sloxUrl() );

      KRES::Manager<KABC::Resource>::Iterator it;
      for ( it = m.begin(); it != m.end(); ++it ) {
        if ( (*it)->identifier() == SloxConfig::kabcResource() ) {
          KABC::ResourceSlox *r = static_cast<KABC::ResourceSlox *>( *it );
          r->prefs()->setUrl( url.url() );
          r->prefs()->setUser( SloxConfig::self()->user() );
          r->prefs()->setPassword( SloxConfig::self()->password() );
        }
      }
      m.writeConfig();
    }
};


class SloxPropagator : public KConfigPropagator
{
  public:
    SloxPropagator()
      : KConfigPropagator( SloxConfig::self(), "slox.kcfg" )
    {
    }

    ~SloxPropagator()
    {
      SloxConfig::self()->writeConfig();
    }

  protected:
    void addCustomChanges( Change::List &changes )
    {
      KCal::CalendarResourceManager m1( "calendar" );
      m1.readConfig();
      KCal::CalendarResourceManager::Iterator it;
      for ( it = m1.begin(); it != m1.end(); ++it ) {
        if ( (*it)->type() == "slox" ) break;
      }
      if ( it == m1.end() ) {
        changes.append( new CreateSloxKcalResource );
      } else {
        if ( (*it)->identifier() == SloxConfig::kcalResource() ) {
          KCal::SloxPrefs *prefs = static_cast<KCalResourceSlox *>( *it )->prefs();
          if ( prefs->url() != sloxUrl() ||
               prefs->user() != SloxConfig::user() ||
               prefs->password() != SloxConfig::password() ) {
            changes.append( new UpdateSloxKcalResource );
          }
        }
      }

      KRES::Manager<KABC::Resource> m2( "contact" );
      m2.readConfig();
      KRES::Manager<KABC::Resource>::Iterator it2;
      for ( it2 = m2.begin(); it2 != m2.end(); ++it2 ) {
        if ( (*it2)->type() == "slox" ) break;
      }
      if ( it2 == m2.end() ) {
        changes.append( new CreateSloxKabcResource );
      } else {
        if ( (*it2)->identifier() == SloxConfig::kabcResource() ) {
          KABC::SloxPrefs *prefs = static_cast<KABC::ResourceSlox *>( *it2 )->prefs();
          if ( prefs->url() != sloxUrl() ||
               prefs->user() != SloxConfig::user() ||
               prefs->password() != SloxConfig::password() ) {
            changes.append( new UpdateSloxKabcResource );
          }
        }
      }
    }
};

SloxWizard::SloxWizard() : KConfigWizard( new SloxPropagator )
{
  QWidget *page = createWizardPage( i18n("SUSE LINUX OpenExchange Server") );

  QGridLayout *topLayout = new QGridLayout;
  topLayout->setSpacing( spacingHint() );
  page->setLayout(topLayout);

  QLabel *label = new QLabel( i18n("Server name:"));
  topLayout->addWidget( label, 0, 0 );
  mServerEdit = new KLineEdit;
  topLayout->addWidget( mServerEdit, 0, 1 );

  label = new QLabel( i18n("User name:"));
  topLayout->addWidget( label, 1, 0 );
  mUserEdit = new KLineEdit;
  topLayout->addWidget( mUserEdit, 1, 1 );

  label = new QLabel( i18n("Password:"));
  topLayout->addWidget( label, 2, 0 );
  mPasswordEdit = new KLineEdit;
  mPasswordEdit->setEchoMode( KLineEdit::Password );
  topLayout->addWidget( mPasswordEdit, 2, 1 );

  mSavePasswordCheck = new QCheckBox( i18n("Save password"));
  topLayout->addWidget( mSavePasswordCheck, 3, 0, 1, 2 );

  mSecureCheck = new QCheckBox( i18n("Encrypt communication with server"));
  topLayout->addWidget( mSecureCheck, 4, 0, 1, 2 );

  topLayout->setRowStretch( 5, 1 );

  setupRulesPage();
  setupChangesPage();

  resize( 400, 300 );
}

SloxWizard::~SloxWizard()
{
}

QString SloxWizard::validate()
{
  KUrl server( mServerEdit->text() );
  if ( !server.protocol().isEmpty() ||
      mServerEdit->text().isEmpty() ||
      mUserEdit->text().isEmpty() ||
      mPasswordEdit->text().isEmpty() )
    return i18n( "Please fill in all fields." );
  return QString();
}

void SloxWizard::usrReadConfig()
{
  mServerEdit->setText( SloxConfig::self()->server() );
  mUserEdit->setText( SloxConfig::self()->user() );
  mPasswordEdit->setText( SloxConfig::self()->password() );
  mSavePasswordCheck->setChecked( SloxConfig::self()->savePassword() );
  mSecureCheck->setChecked( SloxConfig::self()->useHttps() );
}

void SloxWizard::usrWriteConfig()
{
  SloxConfig::self()->setServer( mServerEdit->text() );
  SloxConfig::self()->setUser( mUserEdit->text() );
  SloxConfig::self()->setPassword( mPasswordEdit->text() );
  SloxConfig::self()->setSavePassword( mSavePasswordCheck->isChecked() );
  SloxConfig::self()->setUseHttps( mSecureCheck->isChecked() );
}
