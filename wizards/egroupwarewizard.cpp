/*
    This file is part of kdepim.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#include "egroupwarewizard.h"
#include "egroupwareconfig.h"

#include "kresources/egroupware/kabc_resourcexmlrpc.h"
#include "kresources/egroupware/kcal_resourcexmlrpc.h"
#include "kresources/egroupware/knotes_resourcexmlrpc.h"

#include <libkcal/resourcecalendar.h>

#include <klineedit.h>
#include <klocale.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>


static QString createURL( const QString &server, bool useSSL )
{
  KURL url;

  if ( useSSL )
    url.setProtocol( "https" );
  else
    url.setProtocol( "http" );

  url.setHost( server );
  url.setPath( "/egroupware/xmlrpc.php" );

  return url.url();
}

class CreateEGroupwareKabcResource : public KConfigPropagator::Change
{
  public:
    CreateEGroupwareKabcResource()
      : KConfigPropagator::Change( i18n("Create eGroupware Addressbook Resource") )
    {
    }

    void apply()
    {
      kdDebug() << "Create eGroupware Addressbook Resource" << endl;

      KRES::Manager<KABC::Resource> manager( "contact" );
      manager.readConfig();

      QString url = createURL( EGroupwareConfig::self()->server(), EGroupwareConfig::self()->useSSLConnection() );

      KABC::ResourceXMLRPC *resource = new KABC::ResourceXMLRPC( url, EGroupwareConfig::self()->domain(),
                                                                 EGroupwareConfig::self()->user(),
                                                                 EGroupwareConfig::self()->password() );
      resource->setResourceName( i18n( "eGroupware" ) );
      manager.add( resource );
      manager.writeConfig();
    }
};

// TODO: fix the i18n strings after freeze...
class ChangeEGroupwareKabcResource : public KConfigPropagator::Change
{
  public:
    ChangeEGroupwareKabcResource( const QString &identifier )
      : KConfigPropagator::Change( i18n("Create eGroupware Addressbook Resource") ),
        mIdentifier( identifier )
    {
    }

    void apply()
    {
      kdDebug() << "Change eGroupware Addressbook Resource" << endl;

      KRES::Manager<KABC::Resource> manager( "contact" );
      manager.readConfig();

      KRES::Manager<KABC::Resource>::Iterator it;
      for ( it = manager.begin(); it != manager.end(); ++it ) {
        if ( (*it)->identifier() == mIdentifier ) {
          KABC::ResourceXMLRPC *resource = static_cast<KABC::ResourceXMLRPC*>( *it );

          resource->setURL( createURL( EGroupwareConfig::self()->server(),
                                       EGroupwareConfig::self()->useSSLConnection() ) );
          resource->setDomain( EGroupwareConfig::self()->domain() );
          resource->setUser( EGroupwareConfig::self()->user() );
          resource->setPassword( EGroupwareConfig::self()->password() );

          manager.change( resource );
          manager.writeConfig();
          return;
        }
      }
    }

  private:
    QString mIdentifier;
};

class CreateEGroupwareKcalResource : public KConfigPropagator::Change
{
  public:
    CreateEGroupwareKcalResource()
      : KConfigPropagator::Change( i18n( "Create eGroupware Calendar Resource" ) )
    {
    }

    void apply()
    {
      kdDebug() << "Create eGroupware Calendar Resource" << endl;

      KCal::CalendarResourceManager manager( "calendar" );
      manager.readConfig();

      KCal::ResourceXMLRPC *resource = new KCal::ResourceXMLRPC();
      resource->setResourceName( i18n( "eGroupware" ) );
      resource->setURL( createURL( EGroupwareConfig::self()->server(), EGroupwareConfig::self()->useSSLConnection() ) );
      resource->setDomain( EGroupwareConfig::self()->domain() );
      resource->setUser( EGroupwareConfig::self()->user() );
      resource->setPassword( EGroupwareConfig::self()->password() );
      manager.add( resource );
      manager.writeConfig();
    }
};

class ChangeEGroupwareKcalResource : public KConfigPropagator::Change
{
  public:
    ChangeEGroupwareKcalResource( const QString &identifier )
      : KConfigPropagator::Change( i18n( "Create eGroupware Calendar Resource" ) ),
        mIdentifier( identifier )
    {
    }

    void apply()
    {
      kdDebug() << "Change eGroupware Calendar Resource" << endl;

      KCal::CalendarResourceManager manager( "calendar" );
      manager.readConfig();

      KCal::CalendarResourceManager::Iterator it;
      for ( it = manager.begin(); it != manager.end(); ++it ) {
        if ( (*it)->identifier() == mIdentifier ) {
          KCal::ResourceXMLRPC *resource = static_cast<KCal::ResourceXMLRPC*>( *it );

          resource->setURL( createURL( EGroupwareConfig::self()->server(),
                                       EGroupwareConfig::self()->useSSLConnection() ) );
          resource->setDomain( EGroupwareConfig::self()->domain() );
          resource->setUser( EGroupwareConfig::self()->user() );
          resource->setPassword( EGroupwareConfig::self()->password() );

          manager.change( resource );
          manager.writeConfig();
          return;
        }
      }
    }

  private:
    QString mIdentifier;
};

class CreateEGroupwareKnotesResource : public KConfigPropagator::Change
{
  public:
    CreateEGroupwareKnotesResource()
      : KConfigPropagator::Change( i18n("Create eGroupware Notes Resource") )
    {
    }

    void apply()
    {
      kdDebug() << "Create eGroupware notes Resource" << endl;

      KRES::Manager<ResourceNotes> manager( "notes" );
      manager.readConfig();

      QString url = createURL( EGroupwareConfig::self()->server(), EGroupwareConfig::self()->useSSLConnection() );

      KNotes::ResourceXMLRPC *resource = new KNotes::ResourceXMLRPC();
      resource->setResourceName( i18n( "eGroupware" ) );
      resource->setURL( createURL( EGroupwareConfig::self()->server(), EGroupwareConfig::self()->useSSLConnection() ) );
      resource->setDomain( EGroupwareConfig::self()->domain() );
      resource->setUser( EGroupwareConfig::self()->user() );
      resource->setPassword( EGroupwareConfig::self()->password() );
      manager.add( resource );
      manager.writeConfig();
    }
};

class ChangeEGroupwareKnotesResource : public KConfigPropagator::Change
{
  public:
    ChangeEGroupwareKnotesResource( const QString &identifier )
      : KConfigPropagator::Change( i18n("Create eGroupware Notes Resource") ),
        mIdentifier( identifier )
    {
    }

    void apply()
    {
      kdDebug() << "Change eGroupware Notes Resource" << endl;

      KRES::Manager<ResourceNotes> manager( "notes" );
      manager.readConfig();

      KRES::Manager<ResourceNotes>::Iterator it;
      for ( it = manager.begin(); it != manager.end(); ++it ) {
        if ( (*it)->identifier() == mIdentifier ) {
          KNotes::ResourceXMLRPC *resource = static_cast<KNotes::ResourceXMLRPC*>( *it );

          resource->setURL( createURL( EGroupwareConfig::self()->server(),
                                       EGroupwareConfig::self()->useSSLConnection() ) );
          resource->setDomain( EGroupwareConfig::self()->domain() );
          resource->setUser( EGroupwareConfig::self()->user() );
          resource->setPassword( EGroupwareConfig::self()->password() );

          manager.change( resource );
          manager.writeConfig();
          return;
        }
      }
    }

  private:
    QString mIdentifier;
};

class EGroupwarePropagator : public KConfigPropagator
{
  public:
    EGroupwarePropagator()
      : KConfigPropagator( EGroupwareConfig::self(), "egroupware.kcfg" )
    {
    }

  protected:
    void addCustomChanges( Change::List &changes )
    {
      KCal::CalendarResourceManager kcalManager( "calendar" );
      kcalManager.readConfig();
      KCal::CalendarResourceManager::Iterator kcalIt;
      for ( kcalIt = kcalManager.begin(); kcalIt != kcalManager.end(); ++kcalIt ) {
        if ( (*kcalIt)->type() == "xmlrpc" ) break;
      }
      if ( kcalIt == kcalManager.end() ) {
        changes.append( new CreateEGroupwareKcalResource );
      } else {
        changes.append( new ChangeEGroupwareKcalResource( (*kcalIt)->identifier() ) );
      }

      KRES::Manager<KABC::Resource> kabcManager( "contact" );
      kabcManager.readConfig();
      KRES::Manager<KABC::Resource>::Iterator kabcIt;
      for ( kabcIt = kabcManager.begin(); kabcIt != kabcManager.end(); ++kabcIt ) {
        if ( (*kabcIt)->type() == "xmlrpc" ) break;
      }
      if ( kabcIt == kabcManager.end() ) {
        changes.append( new CreateEGroupwareKabcResource );
      } else {
        changes.append( new ChangeEGroupwareKabcResource( (*kabcIt)->identifier() ) );
      }

      KRES::Manager<ResourceNotes> knotesManager( "notes" );
      knotesManager.readConfig();
      KRES::Manager<ResourceNotes>::Iterator knotesIt;
      for ( knotesIt = knotesManager.begin(); knotesIt != knotesManager.end(); ++knotesIt ) {
        if ( (*knotesIt)->type() == "xmlrpc" ) break;
      }
      if ( knotesIt == knotesManager.end() ) {
        changes.append( new CreateEGroupwareKnotesResource );
      } else {
        changes.append( new ChangeEGroupwareKnotesResource( (*knotesIt)->identifier() ) );
      }
    }
};

EGroupwareWizard::EGroupwareWizard() : KConfigWizard( new EGroupwarePropagator )
{
  QFrame *page = createWizardPage( i18n( "eGroupware Server" ) );

  QGridLayout *topLayout = new QGridLayout( page );
  topLayout->setSpacing( spacingHint() );

  QLabel *label = new QLabel( i18n( "Server name:" ), page );
  topLayout->addWidget( label, 0, 0 );
  mServerEdit = new KLineEdit( page );
  topLayout->addWidget( mServerEdit, 0, 1 );

  label = new QLabel( i18n( "Domain name:" ), page );
  topLayout->addWidget( label, 1, 0 );
  mDomainEdit = new KLineEdit( page );
  topLayout->addWidget( mDomainEdit, 1, 1 );

  label = new QLabel( i18n( "User name:" ), page );
  topLayout->addWidget( label, 2, 0 );
  mUserEdit = new KLineEdit( page );
  topLayout->addWidget( mUserEdit, 2, 1 );

  label = new QLabel( i18n( "Password:" ), page );
  topLayout->addWidget( label, 3, 0 );
  mPasswordEdit = new KLineEdit( page );
  mPasswordEdit->setEchoMode( KLineEdit::Password );
  topLayout->addWidget( mPasswordEdit, 3, 1 );

  mUseSSLConnectionCheck = new QCheckBox( i18n( "Use SSL connection" ), page );
  topLayout->addMultiCellWidget( mUseSSLConnectionCheck, 4, 4, 0, 1 );

  topLayout->setRowStretch( 5, 1 );

  setupRulesPage();
  setupChangesPage();

  resize( 400, 300 );
}

EGroupwareWizard::~EGroupwareWizard()
{
}

void EGroupwareWizard::usrReadConfig()
{
  mServerEdit->setText( EGroupwareConfig::self()->server() );
  mDomainEdit->setText( EGroupwareConfig::self()->domain() );
  mUserEdit->setText( EGroupwareConfig::self()->user() );
  mPasswordEdit->setText( EGroupwareConfig::self()->password() );
  mUseSSLConnectionCheck->setChecked( EGroupwareConfig::self()->useSSLConnection() );
}

void EGroupwareWizard::usrWriteConfig()
{
  EGroupwareConfig::self()->setServer( mServerEdit->text() );
  EGroupwareConfig::self()->setDomain( mDomainEdit->text() );
  EGroupwareConfig::self()->setUser( mUserEdit->text() );
  EGroupwareConfig::self()->setPassword( mPasswordEdit->text() );
  EGroupwareConfig::self()->setUseSSLConnection( mUseSSLConnectionCheck->isChecked() );
}
