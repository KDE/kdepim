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
#include <kaddressbook/kabprefs.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmdcodec.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <libkdepim/kpimprefs.h>
#include <kio/davjob.h>

#include "webdavhandler.h"

#include "kabcresourceslox.h"

using namespace KABC;

ResourceSlox::ResourceSlox( const KConfig *config )
  : Resource( config )
{
  if ( config ) {
    init( KURL( config->readEntry( "SloxUrl" ) ),
          config->readEntry( "SloxUser" ),
          KStringHandler::obscure( config->readEntry( "SloxPassword" ) ) );
  } else {
    init( KURL(), "", "" );
  }
}

ResourceSlox::ResourceSlox( const KURL &url,
                            const QString &user, const QString &password )
  : Resource( 0 )
{
  init( url, user, password );
}

void ResourceSlox::init( const KURL &url,
                         const QString &user, const QString &password )
{
  setType( "slox" );

  mURL = url;
  mUser = user;
  mPassword = password;
}

ResourceSlox::~ResourceSlox()
{
}

void ResourceSlox::writeConfig( KConfig *config )
{
  Resource::writeConfig( config );

  config->writeEntry( "SloxUrl", mURL.url() );
  config->writeEntry( "SloxUser", mUser );
  config->writeEntry( "SloxPassword", KStringHandler::obscure( mPassword ) );
}

Ticket *ResourceSlox::requestSaveTicket()
{
  if ( !addressBook() ) {
	  kdDebug(5700) << "no addressbook" << endl;
    return 0;
  }

  return createTicket( this );
}

void ResourceSlox::releaseSaveTicket( Ticket *ticket )
{
  delete ticket;
}

bool ResourceSlox::doOpen()
{
  return true;
}

void ResourceSlox::doClose()
{
}

bool ResourceSlox::load()
{
  return true;
}

bool ResourceSlox::asyncLoad()
{
  QString url = mURL.url() + "/servlet/webdav.contacts/";

  QDomDocument doc;
  QDomElement root = WebdavHandler::addDavElement( doc, doc, "propfind" );
  QDomElement prop = WebdavHandler::addDavElement( doc, root, "prop" );
  WebdavHandler::addSloxElement( doc, prop, "lastsync", "0" );
  WebdavHandler::addSloxElement( doc, prop, "folderid" );
  WebdavHandler::addSloxElement( doc, prop, "objecttype", "all" );

//  kdDebug() << "REQUEST CONTACTS: \n" << doc.toString( 2 ) << endl;

  mDownloadJob = KIO::davPropFind( url, doc, "0" );
  connect( mDownloadJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotResult( KIO::Job * ) ) );

  return true;
}

void ResourceSlox::slotResult( KIO::Job *job )
{
  kdDebug() << "ResourceSlox::slotResult()" << endl;

  if ( job->error() ) {
    job->showErrorDialog( 0 );
  } else {
    kdDebug() << "ResourceSlox::slotResult() success" << endl;

    QDomDocument doc = mDownloadJob->response();

    QValueList<SloxItem> items = WebdavHandler::getSloxItems( doc );

    bool changed = false;

    QValueList<SloxItem>::ConstIterator it;
    for( it = items.begin(); it != items.end(); ++it ) {
      SloxItem item = *it;
      if ( item.status == SloxItem::Delete ) {
        QMap<QString,Addressee>::Iterator it;
        it = mAddrMap.find( item.uid );
        if ( it != mAddrMap.end() ) {
          mAddrMap.remove( it );
          changed = true;
        }
      } else if ( item.status == SloxItem::Create ) {
        Addressee a;
        a.setUid( item.uid );

        QDomNode n;
        for( n = item.domNode.firstChild(); !n.isNull(); n = n.nextSibling() ) {
          QDomElement e = n.toElement();
          parseContactAttribute( e, a );
        }

        a.setResource( this );
        a.setChanged( false );

        mAddrMap.replace( item.uid, a );

        changed = true;
      }
    }
  }

  mDownloadJob = 0;

  emit loadingFinished( this );
}

void ResourceSlox::parseContactAttribute( const QDomElement &e, Addressee &a )
{
  if ( e.tagName() == "lastname" ) {
    a.setFamilyName( e.text() );
  } else if ( e.tagName() == "firstname" ) {
    a.setGivenName( e.text() );
  } else if ( e.tagName() == "email" ) {
    a.insertEmail( e.text() );
  } else if ( e.tagName() == "phone" ) {
    a.insertPhoneNumber( PhoneNumber( e.text() ) );
  }
}

bool ResourceSlox::save( Ticket* )
{
  return false; // readonly
}

bool ResourceSlox::asyncSave( Ticket* )
{
  return false; // readonly
}

void ResourceSlox::insertAddressee( const Addressee& addr )
{
}

void ResourceSlox::removeAddressee( const Addressee& addr )
{
}


void ResourceSlox::setURL( const KURL &url )
{
  mURL = url;
}

KURL ResourceSlox::url() const
{
  return mURL;
}

void ResourceSlox::setUser( const QString &user )
{
  mUser = user;
}

QString ResourceSlox::user() const
{
  return mUser;
}

void ResourceSlox::setPassword( const QString &password )
{
  mPassword = password;
}

QString ResourceSlox::password() const
{
  return mPassword;
}

#include "kabcresourceslox.moc"
