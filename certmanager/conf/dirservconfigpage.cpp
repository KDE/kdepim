/*
    dirservconfigpage.cpp

    This file is part of kleopatra
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2, as published by the Free Software Foundation.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

#include "dirservconfigpage.h"
#include "directoryserviceswidget.h"

#include <kleo/cryptobackendfactory.h>

#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kconfig.h>
#include <knuminput.h>
#include <kdialog.h>

#include <qhbox.h>
#include <qlabel.h>
#include <qdatetimeedit.h>
#include <qcheckbox.h>
#include <qlayout.h>

#include <kdepimmacros.h>

// For sync'ing kabldaprc
class KABSynchronizer
{
public:
  KABSynchronizer()
    : mConfig( "kabldaprc" ) {
    mConfig.setGroup( "LDAP" );
  }

  KURL::List readCurrentList() const {

    KURL::List lst;
    // stolen from kabc/ldapclient.cpp
    const uint numHosts = mConfig.readUnsignedNumEntry( "NumSelectedHosts" );
    for ( uint j = 0; j < numHosts; j++ ) {
      const QString num = QString::number( j );

      KURL url;
      url.setProtocol( "ldap" );
      url.setPath( "/" ); // workaround KURL parsing bug
      const QString host = mConfig.readEntry( QString( "SelectedHost" ) + num ).stripWhiteSpace();
      url.setHost( host );

      const int port = mConfig.readUnsignedNumEntry( QString( "SelectedPort" ) + num );
      if ( port != 0 )
        url.setPort( port );

      const QString base = mConfig.readEntry( QString( "SelectedBase" ) + num ).stripWhiteSpace();
      url.setQuery( base );

      const QString bindDN = mConfig.readEntry( QString( "SelectedBind" ) + num ).stripWhiteSpace();
      url.setUser( bindDN );

      const QString pwdBindDN = mConfig.readEntry( QString( "SelectedPwdBind" ) + num ).stripWhiteSpace();
      url.setPass( pwdBindDN );
      lst.append( url );
    }
    return lst;
  }

  void writeList( const KURL::List& lst ) {

    mConfig.writeEntry( "NumSelectedHosts", lst.count() );

    KURL::List::const_iterator it = lst.begin();
    KURL::List::const_iterator end = lst.end();
    unsigned j = 0;
    for( ; it != end; ++it, ++j ) {
      const QString num = QString::number( j );
      KURL url = *it;

      Q_ASSERT( url.protocol() == "ldap" );
      mConfig.writeEntry( QString( "SelectedHost" ) + num, url.host() );
      mConfig.writeEntry( QString( "SelectedPort" ) + num, url.port() );

      // KURL automatically encoded the query (e.g. for spaces inside it),
      // so decode it before writing it out
      const QString base = KURL::decode_string( url.query().mid(1) );
      mConfig.writeEntry( QString( "SelectedBase" ) + num, base );
      mConfig.writeEntry( QString( "SelectedBind" ) + num, url.user() );
      mConfig.writeEntry( QString( "SelectedPwdBind" ) + num, url.pass() );
    }
    mConfig.sync();
  }

private:
  KConfig mConfig;
};

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

DirectoryServicesConfigurationPage::DirectoryServicesConfigurationPage( QWidget * parent, const char * name )
    : KCModule( parent, name )
{
  mConfig = Kleo::CryptoBackendFactory::instance()->config();
  QVBoxLayout* lay = new QVBoxLayout( this, 0, KDialog::spacingHint() );
  Kleo::CryptoConfigEntry* entry = configEntry( s_dirserv_componentName, s_dirserv_groupName, s_dirserv_entryName,
                                                Kleo::CryptoConfigEntry::ArgType_LDAPURL, true );
  mWidget = new Kleo::DirectoryServicesWidget( entry, this );
  lay->addWidget( mWidget );
  connect( mWidget, SIGNAL( changed() ), this, SLOT( slotChanged() ) );

  // LDAP timeout
  QHBox* box = new QHBox( this );
  box->setSpacing( KDialog::spacingHint() );
  lay->addWidget( box );
  QLabel* label = new QLabel( i18n( "LDAP &timeout (minutes:seconds)" ), box );
  mTimeout = new QTimeEdit( box );
  mTimeout->setDisplay( QTimeEdit::Minutes | QTimeEdit::Seconds );
  connect( mTimeout, SIGNAL( valueChanged( const QTime& ) ), this, SLOT( slotChanged() ) );
  label->setBuddy( mTimeout );
  QWidget* stretch = new QWidget( box );
  box->setStretchFactor( stretch, 2 );

  // Max number of items returned by queries
  box = new QHBox( this );
  box->setSpacing( KDialog::spacingHint() );
  lay->addWidget( box );
  mMaxItems = new KIntNumInput( box );
  mMaxItems->setLabel( i18n( "&Maximum number of items returned by query" ), Qt::AlignLeft | Qt::AlignVCenter );
  mMaxItems->setMinValue( 0 );
  connect( mMaxItems, SIGNAL( valueChanged(int) ), this, SLOT( slotChanged() ) );
  stretch = new QWidget( box );
  box->setStretchFactor( stretch, 2 );

#ifdef NOT_USEFUL_CURRENTLY
  mAddNewServersCB = new QCheckBox( i18n( "Automatically add &new servers discovered in CRL distribution points" ), this );
  connect( mAddNewServersCB, SIGNAL( clicked() ), this, SLOT( slotChanged() ) );
  lay->addWidget( mAddNewServersCB );
#endif

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
    //kdDebug() << "timeout:" << mTimeoutConfigEntry->uintValue() << "  -> " << time << endl;
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

  // Also write the LDAP URLs to kabldaprc so that they are used by kaddressbook
  KABSynchronizer sync;
  const KURL::List toAdd = mWidget->urlList();
  KURL::List currentList = sync.readCurrentList();

  KURL::List::const_iterator it = toAdd.begin();
  KURL::List::const_iterator end = toAdd.end();
  for( ; it != end; ++it ) {
    // check if the URL is already in currentList
    if ( currentList.find( *it ) == currentList.end() )
      // if not, add it
      currentList.append( *it );
  }
  sync.writeList( currentList );
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
  KDE_EXPORT KCModule *create_kleopatra_config_dirserv( QWidget *parent, const char * )
  {
    DirectoryServicesConfigurationPage *page =
      new DirectoryServicesConfigurationPage( parent, "kleopatra_config_dirserv" );
    return page;
  }
}

// kdelibs-3.2 didn't have the changed signal in KCModule...
void DirectoryServicesConfigurationPage::slotChanged()
{
  emit changed(true);
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
        KMessageBox::error( this, i18n( "Backend error: gpgconf does not seem to know the entry for %1/%2/%3" ).arg( componentName, groupName, entryName ) );
        return 0;
    }
    if( entry->argType() != argType || entry->isList() != isList ) {
        KMessageBox::error( this, i18n( "Backend error: gpgconf has wrong type for %1/%2/%3: %4 %5" ).arg( componentName, groupName, entryName ).arg( entry->argType() ).arg( entry->isList() ) );
        return 0;
    }
    return entry;
}

#include "dirservconfigpage.moc"
