/*
    configuredialog_p.cpp

    This file is part of kgpgcertmanager
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

#include "configuredialog_p.h"
#include <qlayout.h>
#include "directoryservicesconfigurationdialogimpl.h"
#include <cryptplugfactory.h>
#include <kleo/cryptoconfig.h>
#include <kmessagebox.h>
#include <klocale.h>

static const char s_dirserv_componentName[] = "dirmngr";
static const char s_dirserv_groupName[] = "LDAP";
static const char s_dirserv_entryName[] = "LDAP Server";

DirectoryServicesConfigurationPage::DirectoryServicesConfigurationPage( QWidget * parent, const char * name )
    : KCModule( parent, name )
{
  QVBoxLayout* lay = new QVBoxLayout( this );
  mWidget = new DirectoryServicesConfigurationDialogImpl( this );
  lay->addWidget( mWidget );
  mConfig = Kleo::CryptPlugFactory::instance()->config();
  connect( mWidget, SIGNAL( changed() ), this, SLOT( changed() ) );
}

// Helper method for load/save/defaults. Implements runtime checks on the configuration option.
Kleo::CryptoConfigEntry* DirectoryServicesConfigurationPage::configEntry() {
    Kleo::CryptoConfigEntry* entry = mConfig->entry( s_dirserv_componentName, s_dirserv_groupName, s_dirserv_entryName );
    if ( !entry ) {
        KMessageBox::error( this, i18n( "Backend error: gpgconf doesn't seem to know the entry for %1/%2/%3" ).arg( s_dirserv_componentName, s_dirserv_groupName, s_dirserv_entryName ) );
        return 0;
    }
    if( entry->argType() != Kleo::CryptoConfigEntry::ArgType_LDAPURL || !entry->isList() ) {
        KMessageBox::error( this, i18n( "Backend error: gpgconf has wrong type for %1/%2/%3: %4 %5" ).arg( s_dirserv_componentName, s_dirserv_groupName, s_dirserv_entryName ).arg( entry->argType() ).arg( entry->isList() ) );
        return 0;
    }
    return entry;
}

void DirectoryServicesConfigurationPage::load()
{
    Kleo::CryptoConfigEntry* entry = configEntry();
    if ( entry ) {
        KURL::List urls = entry->urlValueList();
        mWidget->setInitialServices( urls );
    }
}

void DirectoryServicesConfigurationPage::save()
{
    Kleo::CryptoConfigEntry* entry = configEntry();
    if ( entry ) {
        KURL::List urls = mWidget->urlList();
        entry->setURLValueList( urls );
        mConfig->sync( true );
    }
}

void DirectoryServicesConfigurationPage::defaults()
{
    // ## We can't use  entry->resetToDefault() here. It would reset to default before the user has a chance to click OK or Cancel.
#if 0
    Kleo::CryptoConfigEntry* entry = configEntry();
    if ( entry ) {
        entry->resetToDefault();
      mConfig->sync( true );
      mConfig->clear();
      load();
    }
#endif
    mWidget->clear(); // the default is an empty list.
}

extern "C"
{
  KCModule *create_kgpgcertmanager_config_dirserv( QWidget *parent, const char * )
  {
    DirectoryServicesConfigurationPage *page =
      new DirectoryServicesConfigurationPage( parent, "kgpgcertmanager_config_dirserv" );
    return page;
  }
}

#include "configuredialog_p.moc"
