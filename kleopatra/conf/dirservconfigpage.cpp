/* -*- mode: c++; c-basic-offset:4 -*-
    conf/dirservconfigpage.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2004,2008 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config-kleopatra.h>

#include "dirservconfigpage.h"
#include "libkleo/ui/directoryserviceswidget.h"
#include "libkleo/kleo/cryptobackendfactory.h"

#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kconfig.h>
#include <knuminput.h>
#include <kdialog.h>
#include <kcomponentdata.h> 

#include <khbox.h>
#include <QLabel>
#include <qdatetimeedit.h>
#include <QCheckBox>
#include <QLayout>

#include <kdemacros.h>

#if 0 // disabled, since it is apparently confusing
// For sync'ing kabldaprc
class KABSynchronizer
{
public:
  KABSynchronizer()
    : mConfig( "kabldaprc" ) {
    mConfig.setGroup( "LDAP" );
  }

  KUrl::List readCurrentList() const {

    KUrl::List lst;
    // stolen from kabc/ldapclient.cpp
    const uint numHosts = mConfig.readEntry( "NumSelectedHosts" );
    for ( uint j = 0; j < numHosts; j++ ) {
      const QString num = QString::number( j );

      KUrl url;
      url.setProtocol( "ldap" );
      url.setPath( "/" ); // workaround KUrl parsing bug
      const QString host = mConfig.readEntry( QString( "SelectedHost" ) + num ).trimmed();
      url.setHost( host );

      const int port = mConfig.readEntry( QString( "SelectedPort" ) + num );
      if ( port != 0 )
        url.setPort( port );

      const QString base = mConfig.readEntry( QString( "SelectedBase" ) + num ).trimmed();
      url.setQuery( base );

      const QString bindDN = mConfig.readEntry( QString( "SelectedBind" ) + num ).trimmed();
      url.setUser( bindDN );

      const QString pwdBindDN = mConfig.readEntry( QString( "SelectedPwdBind" ) + num ).trimmed();
      url.setPass( pwdBindDN );
      lst.append( url );
    }
    return lst;
  }

  void writeList( const KUrl::List& lst ) {

    mConfig.writeEntry( "NumSelectedHosts", lst.count() );

    KUrl::List::const_iterator it = lst.begin();
    KUrl::List::const_iterator end = lst.end();
    unsigned j = 0;
    for( ; it != end; ++it, ++j ) {
      const QString num = QString::number( j );
      KUrl url = *it;

      Q_ASSERT( url.protocol() == "ldap" );
      mConfig.writeEntry( QString( "SelectedHost" ) + num, url.host() );
      mConfig.writeEntry( QString( "SelectedPort" ) + num, url.port() );

      // KUrl automatically encoded the query (e.g. for spaces inside it),
      // so decode it before writing it out
      const QString base = KUrl::decode_string( url.query().mid(1) );
      mConfig.writeEntry( QString( "SelectedBase" ) + num, base );
      mConfig.writeEntry( QString( "SelectedBind" ) + num, url.user() );
      mConfig.writeEntry( QString( "SelectedPwdBind" ) + num, url.pass() );
    }
    mConfig.sync();
  }

private:
  KConfig mConfig;
};

#endif

static const char s_dirserv_componentName[] = "dirmngr";
static const char s_dirserv_groupName[] = "LDAP";
static const char s_dirserv_entryName[] = "LDAP Server";

static const char s_timeout_componentName[] = "dirmngr";
static const char s_timeout_groupName[] = "LDAP";
static const char s_timeout_entryName[] = "ldaptimeout";

static const char s_maxitems_componentName[] = "dirmngr";
static const char s_maxitems_groupName[] = "LDAP";
static const char s_maxitems_entryName[] = "max-replies";

static const char s_addnewservers_componentName[] = "dirmngr";
static const char s_addnewservers_groupName[] = "LDAP";
static const char s_addnewservers_entryName[] = "add-servers";

DirectoryServicesConfigurationPage::DirectoryServicesConfigurationPage( const KComponentData &instance, QWidget *parent, const QVariantList &args )
    : KCModule( instance, parent, args )
{
  mConfig = Kleo::CryptoBackendFactory::instance()->config();
  QGridLayout * glay = new QGridLayout( this );
  glay->setSpacing( KDialog::spacingHint() );
  glay->setMargin( 0 );

  int row = 0;
  Kleo::CryptoConfigEntry* entry = configEntry( s_dirserv_componentName, s_dirserv_groupName, s_dirserv_entryName,
                                                Kleo::CryptoConfigEntry::ArgType_LDAPURL, true );
  mWidget = new Kleo::DirectoryServicesWidget( entry, this );
  if ( QLayout * l = mWidget->layout() ) {
      l->setSpacing( KDialog::spacingHint() );
      l->setMargin( 0 );
  }
  glay->addWidget( mWidget, row, 0, 1, 3 );
  connect( mWidget, SIGNAL(changed()), this, SLOT(changed()) );

  // LDAP timeout
  ++row;
  QLabel* label = new QLabel( i18n( "LDAP &timeout (minutes:seconds):" ), this );
  mTimeout = new QTimeEdit( this );
  mTimeout->setDisplayFormat( "mm:ss" );
  connect( mTimeout, SIGNAL(timeChanged(QTime)), this, SLOT(changed()) );
  label->setBuddy( mTimeout );
  glay->addWidget( label, row, 0 );
  glay->addWidget( mTimeout, row, 1 );

  // Max number of items returned by queries
  ++row;
  label = new QLabel( i18n( "&Maximum number of items returned by query:" ), this );
  mMaxItems = new KIntNumInput( this );
  mMaxItems->setMinimum( 0 );
  label->setBuddy( mMaxItems );
  connect( mMaxItems, SIGNAL(valueChanged(int)), this, SLOT(changed()) );
  glay->addWidget( label, row, 0 );
  glay->addWidget( mMaxItems, row, 1 );

#ifdef NOT_USEFUL_CURRENTLY
  ++row
  mAddNewServersCB = new QCheckBox( i18n( "Automatically add &new servers discovered in CRL distribution points" ), this );
  connect( mAddNewServersCB, SIGNAL(clicked()), this, SLOT(changed()) );
  glay->addWidget( mAddNewServersCB, row, 0, 1, 3 );
#endif

  glay->setRowStretch( ++row, 1 );
  glay->setColumnStretch( 2, 1 );

#ifndef HAVE_UNBROKEN_KCMULTIDIALOG
  load();
#endif
}

void DirectoryServicesConfigurationPage::load()
{
  mWidget->load();

  mTimeoutConfigEntry = configEntry( s_timeout_componentName, s_timeout_groupName, s_timeout_entryName, Kleo::CryptoConfigEntry::ArgType_UInt, false );
  if ( mTimeoutConfigEntry ) {
    QTime time = QTime().addSecs( mTimeoutConfigEntry->uintValue() );
    //kDebug() <<"timeout:" << mTimeoutConfigEntry->uintValue() <<"  ->" << time;
    mTimeout->setTime( time );
  }

  mMaxItemsConfigEntry = configEntry( s_maxitems_componentName, s_maxitems_groupName, s_maxitems_entryName, Kleo::CryptoConfigEntry::ArgType_UInt, false );
  if ( mMaxItemsConfigEntry ) {
    mMaxItems->blockSignals( true ); // KNumInput emits valueChanged from setValue!
    mMaxItems->setValue( mMaxItemsConfigEntry->uintValue() );
    mMaxItems->blockSignals( false );
  }

#ifdef NOT_USEFUL_CURRENTLY
  mAddNewServersConfigEntry = configEntry( s_addnewservers_componentName, s_addnewservers_groupName, s_addnewservers_entryName, Kleo::CryptoConfigEntry::ArgType_None, false );
  if ( mAddNewServersConfigEntry ) {
    mAddNewServersCB->setChecked( mAddNewServersConfigEntry->boolValue() );
  }
#endif
}

void DirectoryServicesConfigurationPage::save()
{
  mWidget->save();

  QTime time( mTimeout->time() );
  unsigned int timeout = time.minute() * 60 + time.second();
  if ( mTimeoutConfigEntry && mTimeoutConfigEntry->uintValue() != timeout )
    mTimeoutConfigEntry->setUIntValue( timeout );
  if ( mMaxItemsConfigEntry && mMaxItemsConfigEntry->uintValue() != (uint)mMaxItems->value() )
    mMaxItemsConfigEntry->setUIntValue( mMaxItems->value() );
#ifdef NOT_USEFUL_CURRENTLY
  if ( mAddNewServersConfigEntry && mAddNewServersConfigEntry->boolValue() != mAddNewServersCB->isChecked() )
    mAddNewServersConfigEntry->setBoolValue( mAddNewServersCB->isChecked() );
#endif

  mConfig->sync( true );

#if 0
  // Also write the LDAP URLs to kabldaprc so that they are used by kaddressbook
  KABSynchronizer sync;
  const KUrl::List toAdd = mWidget->urlList();
  KUrl::List currentList = sync.readCurrentList();

  KUrl::List::const_iterator it = toAdd.begin();
  KUrl::List::const_iterator end = toAdd.end();
  for( ; it != end; ++it ) {
    // check if the URL is already in currentList
    if ( currentList.find( *it ) == currentList.end() )
      // if not, add it
      currentList.append( *it );
  }
  sync.writeList( currentList );
#endif
}

void DirectoryServicesConfigurationPage::defaults()
{
  mWidget->defaults();
  if ( mTimeoutConfigEntry )
    mTimeoutConfigEntry->resetToDefault();
  if ( mMaxItemsConfigEntry )
    mMaxItemsConfigEntry->resetToDefault();
#ifdef NOT_USEFUL_CURRENTLY
  if ( mAddNewServersConfigEntry )
    mAddNewServersConfigEntry->resetToDefault();
#endif
  load();
}

extern "C"
{
  KDE_EXPORT KCModule *create_kleopatra_config_dirserv( QWidget *parent=0, const QVariantList &args=QVariantList() )
  {
    DirectoryServicesConfigurationPage *page =
      new DirectoryServicesConfigurationPage( KComponentData( "kleopatra" ), parent, args );
    page->setObjectName( "kleopatra_config_dirserv" );
    return page;
  }
}

// Find config entry for ldap servers. Implements runtime checks on the configuration option.
Kleo::CryptoConfigEntry* DirectoryServicesConfigurationPage::configEntry( const char* componentName,
                                                                          const char* groupName,
                                                                          const char* entryName,
                                                                          Kleo::CryptoConfigEntry::ArgType argType,
                                                                          bool isList )
{
    Kleo::CryptoConfigEntry* entry = mConfig->entry( componentName, groupName, entryName );
    if ( !entry ) {
        KMessageBox::error( this, i18n( "Backend error: gpgconf does not seem to know the entry for %1/%2/%3", componentName, groupName, entryName ) );
        return 0;
    }
    if( entry->argType() != argType || entry->isList() != isList ) {
        KMessageBox::error( this, i18n( "Backend error: gpgconf has wrong type for %1/%2/%3: %4 %5", componentName, groupName, entryName, entry->argType(), entry->isList() ) );
        return 0;
    }
    return entry;
}

#include "dirservconfigpage.moc"
