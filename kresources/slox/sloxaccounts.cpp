/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2005 Volker Krause <volker.krause@rwth-aachen.de>

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

#include "sloxaccounts.h"
#include "sloxbase.h"
#include "webdavhandler.h"

#include <kcal/freebusyurlstore.h>

#include <kstaticdeleter.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kio/job.h>
#include <kio/davjob.h>
#include <kstringhandler.h>
#include <kconfig.h>

#include <QFile>
#include <qdom.h>
#include <QString>
//Added by qt3to4:
#include <QTextStream>

SloxAccounts::SloxAccounts( SloxBase *res, const KUrl &baseUrl )
  : mBaseUrl( baseUrl ), mRes( res )
{
  kDebug() << "SloxAccounts(): " << baseUrl << endl;

  mDownloadJob = 0;

  QString server = mBaseUrl.host();

  QStringList l = server.split( '.' );

  if ( l.count() < 2 ) mDomain = server;
  else mDomain = l[ l.count() - 2 ] + "." + l[ l.count() - 1 ];

  readAccounts();
}

SloxAccounts::~SloxAccounts()
{
  kDebug() << "~SloxAccounts()" << endl;

  if ( mDownloadJob ) mDownloadJob->kill();
}

void SloxAccounts::insertUser( const QString &id, const KABC::Addressee &a )
{
  kDebug() << "SloxAccount::insertUser() " << id << endl;

  mUsers.insert( id, a );

  QString email = a.preferredEmail();

  QString url = "http://" + mBaseUrl.host() + "/servlet/webdav.freebusy?username=";
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
  kDebug() << "SloxAccounts::lookupId() " << email << endl;

  QMap<QString, KABC::Addressee>::ConstIterator it;
  for( it = mUsers.begin(); it != mUsers.end(); ++it ) {
    kDebug() << "PREF: " << (*it).preferredEmail() << endl;
    kDebug() << "KEY: " << it.key() << endl;
    if ( (*it).preferredEmail() == email ) return it.key();
  }
  requestAccounts();

  int pos = email.indexOf( '@' );
  if ( pos < 0 ) return email;
  else return email.left( pos );
}

void SloxAccounts::requestAccounts()
{
  kDebug() << "SloxAccounts::requestAccounts()" << endl;

  if ( mDownloadJob ) {
    kDebug() << "SloxAccount::requestAccounts(): Download still in progress"
              << endl;
    return;
  }

  if ( mRes->resType() == "slox" ) {
    KUrl url = mBaseUrl;
    url.addPath( "/servlet/webdav.groupuser" );
    url.setQuery( "?user=*&group=*&groupres=*&res=*&details=t" );

    kDebug() << "SloxAccounts::requestAccounts() URL: " << url << endl;

    mDownloadJob = KIO::file_copy( url, cacheFile(), -1, true, false, false );
  } else if ( mRes->resType() == "ox" ) {
    KUrl url = mBaseUrl;
    url.setPath( "/servlet/webdav.groupuser/" );

    QDomDocument doc;
    QDomElement root = WebdavHandler::addDavElement( doc, doc, "propfind" );
    QDomElement prop = WebdavHandler::addDavElement( doc, root, "prop" );
    WebdavHandler::addSloxElement( mRes, doc, prop, "user", "*" );
    WebdavHandler::addSloxElement( mRes, doc, prop, "group", "*" );
    WebdavHandler::addSloxElement( mRes, doc, prop, "resource", "*" );
    WebdavHandler::addSloxElement( mRes, doc, prop, "resourcegroup", "*" );

    kDebug() << k_funcinfo << doc.toString( 2 ) << endl;

    mDownloadJob = KIO::davPropFind( url, doc, "0", false );
  }

  connect( mDownloadJob, SIGNAL( result( KJob * ) ),
           SLOT( slotResult( KJob * ) ) );
}

void SloxAccounts::slotResult( KJob *job )
{
  kDebug() << "SloxAccounts::slotResult()" << endl;

  if ( job->error() ) {
    static_cast<KIO::Job*>(job)->showErrorDialog( 0 );
  } else {
    if ( mRes->resType() == "ox" ) {
      QFile f( cacheFile() );
      if ( !f.open( QIODevice::WriteOnly ) ) {
        kWarning() << "Unable to open '" << cacheFile() << "'" << endl;
        return;
      }
      QTextStream stream ( &f );
      stream << static_cast<KIO::DavJob*>( mDownloadJob )->response();
      f.close();
    }
    readAccounts();
  }

  mDownloadJob = 0;
}

QString SloxAccounts::cacheFile() const
{
  QString host = mBaseUrl.host();

  QString file = KStandardDirs::locateLocal( "cache", "slox/accounts_" + host );

  kDebug() << "SloxAccounts::cacheFile(): " << file << endl;

  return file;
}

void SloxAccounts::readAccounts()
{
  kDebug() << "SloxAccounts::readAccounts()" << endl;

  QFile f( cacheFile() );
  if ( !f.open( QIODevice::ReadOnly ) ) {
    kDebug() << "Unable to open '" << cacheFile() << "'" << endl;
    requestAccounts();
    return;
  }

  QDomDocument doc;
  doc.setContent( &f );

//  kDebug() << "SLOX ACCOUNTS: " << doc.toString( 2 ) << endl;

  QDomElement docElement = doc.documentElement();

  mUsers.clear();

  QDomNodeList nodes = doc.elementsByTagName( mRes->resType() == "ox" ? "ox:user" : "user" );
  for( int i = 0; i < nodes.count(); ++i ) {
    QDomElement element = nodes.item(i).toElement();
    QString id;
    KABC::Addressee a;
    QDomNode n;
    for( n = element.firstChild(); !n.isNull(); n = n.nextSibling() ) {
      QDomElement e = n.toElement();
      QString tag = e.tagName();
      // remove XML namespace
      tag = tag.right( tag.length() - ( tag.indexOf( ':' ) + 1 ) );
      QString value = e.text();
      if ( tag == "uid" ) id = value;
      else if ( tag == "mail" ) a.insertEmail( value, true );
      else if ( tag == "forename" ) a.setGivenName( value );
      else if ( tag == "surename" ) a.setFamilyName( value );
    }
//     kDebug() << "MAIL: " << a.preferredEmail() << endl;
    insertUser( id, a );
  }
}

#include "sloxaccounts.moc"
