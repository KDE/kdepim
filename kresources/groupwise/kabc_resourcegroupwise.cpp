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

#include <qapplication.h>

#include <kabc/addressee.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>

#include "soap/groupwiseserver.h"
#include "kabc_groupwiseprefs.h"

#include "kabc_resourcegroupwise.h"

using namespace KABC;

ResourceGroupwise::ResourceGroupwise( const KConfig *config )
  : ResourceCached( config )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  if ( config ) {
    readConfig( config );
  }

  initGroupwise();
}

ResourceGroupwise::ResourceGroupwise( const KURL &url,
                                      const QString &user,
                                      const QString &password,
                                      const QStringList &readAddressBooks,
                                      const QString &writeAddressBook )
  : ResourceCached( 0 )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  mPrefs->setHost( url.url() );
  mPrefs->setUser( user );
  mPrefs->setPassword( password );
  mPrefs->setReadAddressBooks( readAddressBooks );
  mPrefs->setWriteAddressBook( writeAddressBook );

  initGroupwise();
}

void ResourceGroupwise::init()
{
  mPrefs = new GroupwisePrefs;

  setType( "groupwise" );
}

void ResourceGroupwise::initGroupwise()
{
  QString url;
  if ( mPrefs->useHttps() )
     url =  "https://";
  else
     url =  "http://";
  url += mPrefs->host() +":";
  url += QString::number(mPrefs->port()) + "/soap/";
  mServer = new GroupwiseServer( url, mPrefs->user(),
                                 mPrefs->password(), this );

  connect( mServer, SIGNAL( readAddressBooksFinished() ),
           SLOT( loadFinished() ) );
}

ResourceGroupwise::~ResourceGroupwise()
{
  delete mServer;
  mServer = 0;

  delete mPrefs;
  mPrefs = 0;
}

void ResourceGroupwise::readConfig( const KConfig * )
{
  mPrefs->readConfig();
}

void ResourceGroupwise::writeConfig( KConfig *config )
{
  Resource::writeConfig( config );

  mPrefs->writeConfig();
}

Ticket *ResourceGroupwise::requestSaveTicket()
{
  if ( !addressBook() ) {
    kdDebug(5700) << "no addressbook" << endl;
    return 0;
  }

  return createTicket( this );
}

void ResourceGroupwise::releaseSaveTicket( Ticket *ticket )
{
  delete ticket;
}

bool ResourceGroupwise::doOpen()
{
  return mServer->login();
}

void ResourceGroupwise::doClose()
{
  mServer->logout();
}

bool ResourceGroupwise::load()
{
  return asyncLoad();
}

bool ResourceGroupwise::asyncLoad()
{
  mAddrMap.clear();
  loadCache();

#if 1
  if ( !mServer->readAddressBooksSynchronous( mPrefs->readAddressBooks(), this ) ) {
    kdError() << "ResourceGroupwise::asyncLoad() error" << endl;
    return false;
  }

  loadFinished();
#else
  if ( !mServer->readAddressBooks( mPrefs->readAddressBooks(), this ) ) {
    kdError() << "ResourceGroupwise::asyncLoad() error" << endl;
    return false;
  }
#endif

  return true;
}

bool ResourceGroupwise::save( Ticket *ticket )
{
  return asyncSave( ticket );
}

bool ResourceGroupwise::asyncSave( Ticket* )
{
  KABC::Addressee::List::Iterator it;

  KABC::Addressee::List addedList = addedAddressees();
  for ( it = addedList.begin(); it != addedList.end(); ++it ) {
    if ( mServer->insertAddressee( mPrefs->writeAddressBook(), *it ) ) {
      clearChange( *it );
      setRemoteUid( (*it).uid(), (*it).custom( "GWRESOURCE", "UID" ) );
    }
  }

  KABC::Addressee::List changedList = changedAddressees();
  for ( it = changedList.begin(); it != changedList.end(); ++it ) {
    if ( mServer->changeAddressee( *it ) )
      clearChange( *it );
  }

  KABC::Addressee::List deletedList = deletedAddressees();
  for ( it = deletedList.begin(); it != deletedList.end(); ++it ) {
    if ( mServer->removeAddressee( *it ) )
      clearChange( *it );
  }

  saveCache();

  return true;
}

void ResourceGroupwise::loadFinished()
{
  if ( !mServer->error().isEmpty() ) {
    kdError() << "ResourceGroupwise::loadFinished(): Error: " <<
      mServer->error() << endl;
    emit loadingError( this, mServer->error() );
  } else {
    saveCache();

    emit loadingFinished( this );
  }
}

#include "kabc_resourcegroupwise.moc"
