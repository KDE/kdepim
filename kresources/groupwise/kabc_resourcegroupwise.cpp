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
  : Resource( config )
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
  : Resource( 0 )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  mPrefs->setUrl( url.url() );
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
  mServer = new GroupwiseServer( mPrefs->url(), mPrefs->user(),
                                 mPrefs->password(), this );

  connect( mServer, SIGNAL( readAddressBooksFinished( const KABC::Addressee::List& ) ),
           this, SLOT( loadFinished( const KABC::Addressee::List& ) ) );
}

ResourceGroupwise::~ResourceGroupwise()
{
  kdDebug() << "KABC::~ResourceGroupwise()" << endl;

  delete mPrefs;

  kdDebug() << "KABC::~ResourceGroupwise() done" << endl;
}

void ResourceGroupwise::readConfig( const KConfig * )
{
  mPrefs->readConfig();
}

void ResourceGroupwise::writeConfig( KConfig *config )
{
  kdDebug() << "ResourceGroupwise::writeConfig() " << endl;
  kdDebug() << mPrefs->url() << endl;

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
//  mServer->logout();
}

bool ResourceGroupwise::load()
{
  mAddrMap.clear();

  mServer->readAddressBooks( mPrefs->readAddressBooks() );

  return true;
}

bool ResourceGroupwise::asyncLoad()
{
  mAddrMap.clear();

  mServer->readAddressBooks( mPrefs->readAddressBooks() );

  return true;
}

bool ResourceGroupwise::save( Ticket* )
{
  return false; // readonly
}

bool ResourceGroupwise::asyncSave( Ticket* )
{
  return false; // readonly
}

void ResourceGroupwise::insertAddressee( const Addressee& addr )
{
  KABC::Addressee addressee = addr;

  if ( mServer->insertAddressee( mPrefs->writeAddressBook(), addressee ) )
    mAddrMap.insert( addressee.uid(), addressee );
}

void ResourceGroupwise::removeAddressee( const Addressee& addr )
{
  if ( mServer->removeAddressee( addr ) )
    mAddrMap.remove( addr.uid() );
}

void ResourceGroupwise::loadFinished( const KABC::Addressee::List &addresses )
{
  KABC::Addressee::List::ConstIterator it;
  for ( it = addresses.begin(); it != addresses.end(); ++it ) {
    KABC::Addressee addr = (*it);
    addr.setChanged( false );
    addr.setResource( this );
    mAddrMap.insert( addr.uid(), addr );
  }

  emit loadingFinished( this );
}

#include "kabc_resourcegroupwise.moc"
