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

#include "sloxaccounts.h"

#include <libkcal/freebusyurlstore.h>

#include <kstaticdeleter.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kio/job.h>
#include <kstringhandler.h>
#include <kconfig.h>

#include <qfile.h>
#include <qdom.h>

QString SloxAccounts::mServer;
QString SloxAccounts::mDomain;

SloxAccounts *SloxAccounts::mSelf = 0;
KStaticDeleter<SloxAccounts> selfDeleter;

SloxAccounts *SloxAccounts::self()
{
  if ( !mSelf ) {
    selfDeleter.setObject( mSelf, new SloxAccounts );
  }
  return mSelf;
}

SloxAccounts::SloxAccounts()
{
  kdDebug() << "SloxAccounts()" << endl;

#if 0
  KABC::AddressBook *ab = KABC::StdAddressBook::self();
  ab->asyncLoad();
#endif

  mDownloadJob = 0;

  readAccounts();
}

SloxAccounts::~SloxAccounts()
{
  kdDebug() << "~SloxAccounts()" << endl;

  if ( mDownloadJob ) mDownloadJob->kill();
}

void SloxAccounts::setServer( const QString &server )
{
  mServer = server;

  QStringList l = QStringList::split( '.', server );

  if ( l.count() < 2 ) mDomain = server;
  else mDomain = l[ l.count() - 2 ] + "." + l[ l.count() - 1 ];
}

void SloxAccounts::insertUser( const QString &id, const KABC::Addressee &a )
{
  kdDebug() << "SloxAccount::insertUser() " << id << endl;

  mUsers.replace( id, a );

  QString email = a.preferredEmail();
  
  QString url = "http://" + mServer + "/servlet/webdav.freebusy?username=";
  url += id + "&server=" + mDomain;

  KCal::FreeBusyUrlStore::self()->writeUrl( email, url );
}

KABC::Addressee SloxAccounts::lookupUser( const QString &id )
{
  QMap<QString, KABC::Addressee>::ConstIterator it;
  it = mUsers.find( id );
  if ( it == mUsers.end() ) {
    requestAccounts();
    return KABC::Addressee();
  } else {
    return *it;
  }
}

QString SloxAccounts::lookupId( const QString &email )
{
  kdDebug() << "SloxAccounts::lookupId() " << email << endl;

  QMap<QString, KABC::Addressee>::ConstIterator it;
  for( it = mUsers.begin(); it != mUsers.end(); ++it ) {
    kdDebug() << "PREF: " << (*it).preferredEmail() << endl;
    kdDebug() << "KEY: " << it.key() << endl;
    if ( (*it).preferredEmail() == email ) return it.key();
  }
  requestAccounts();

  int pos = email.find( '@' );
  if ( pos < 0 ) return email;
  else return email.left( pos );
}

void SloxAccounts::requestAccounts()
{
  kdDebug() << "SloxAccounts::requestAccounts()" << endl;

  if ( mDownloadJob ) {
    kdDebug() << "SloxAccount::requestAccounts(): Download still in progress"
              << endl;
    return;
  }

  KURL url( "http://" + mServer + "/servlet/webdav.groupuser?" +
            "user=*&group=*&groupres=*&res=*&details=t" );

  kdDebug() << "SloxAccounts::requestAccounts() URL: " << url << endl;

  KConfig cfg( "sloxrc" );
  cfg.setGroup( "General" );
  QString user = cfg.readEntry( "User" );
  QString password = KStringHandler::obscure( cfg.readEntry( "Password" ) );
  
  url.setUser( user );
  url.setPass( password );

  mDownloadJob = KIO::file_copy( url, cacheFile(), -1, true );
  connect( mDownloadJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotResult( KIO::Job * ) ) );
}

void SloxAccounts::slotResult( KIO::Job *job )
{
  kdDebug() << "SloxAccounts::slotResult()" << endl;

  if ( job->error() ) {
    job->showErrorDialog( 0 );
  } else {
    kdDebug() << "SloxAccounts::slotResult() success" << endl;
  
    readAccounts();
  }

  mDownloadJob = 0;
}

QString SloxAccounts::cacheFile() const
{
  return locateLocal( "cache", "slox/accounts" );
}

void SloxAccounts::readAccounts()
{
  kdDebug() << "SloxAccounts::readAccounts()" << endl;

  QFile f( cacheFile() );
  if ( !f.open( IO_ReadOnly ) ) {
    kdDebug() << "Unable to open '" << cacheFile() << "'" << endl;
    requestAccounts();
    return;
  }

  QDomDocument doc;
  doc.setContent( &f );

//  kdDebug() << "SLOX ACCOUNTS: " << doc.toString( 2 ) << endl;

  QDomElement docElement = doc.documentElement();

  mUsers.clear();  

  QDomNode node;
  for( node = docElement.firstChild(); !node.isNull();
       node = node.nextSibling() ) {
    QDomElement element = node.toElement();
    if ( element.isNull() ) continue;
    if ( element.tagName() == "user" ) {
      QString id;
      KABC::Addressee a;
      QDomNode n;
      for( n = element.firstChild(); !n.isNull(); n = n.nextSibling() ) {
        QDomElement e = n.toElement();
        QString tag = e.tagName();
        QString value = e.text();
        if ( tag == "uid" ) id = value;
        else if ( tag == "mail" ) a.insertEmail( value );
        else if ( tag == "forename" ) a.setGivenName( value );
        else if ( tag == "surename" ) a.setFamilyName( value );
      }
//      kdDebug() << "MAIL: " << a.preferredEmail() << endl;
      insertUser( id, a );
    }
  }
}

#include "sloxaccounts.moc"
