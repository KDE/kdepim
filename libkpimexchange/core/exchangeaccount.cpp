/*
    This file is part of libkpimexchange

    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <tqstring.h>
#include <tqtextstream.h>
#include <tqapplication.h>
#include <tqdom.h>
#include <tqwidgetlist.h>
#include <tqwidget.h>
#include <tqfile.h>

#include <kurl.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kconfig.h>
#include <dcopclient.h>
#include <kcursor.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <kio/authinfo.h>
#include <kio/davjob.h>
#include <kio/job.h>
#include <kio/netaccess.h>

#include "exchangeaccount.h"
#include "utils.h"

using namespace KPIM;

ExchangeAccount::ExchangeAccount( const TQString &host, const TQString &port,
                                  const TQString &account,
                                  const TQString &password,
                                  const TQString &mailbox )
  : mError( false )
{
  KURL url( "webdav://" + host + "/exchange/" + account );

  if ( !port.isEmpty() )
  {
    url.setPort( port.toInt() );
  }

  mHost = host;
  mPort = port;
  mAccount = account;
  mPassword = password;

  if ( mailbox.isEmpty() ) {
    mMailbox = url.url();
    kdDebug() << "#!#!#!#!#!#!# mailbox url: " << mMailbox << endl;
  } else
    mMailbox = mailbox;

  kdDebug() << "ExchangeAccount: mMailbox: " << mMailbox << endl;

  mCalendarURL = 0;
}

ExchangeAccount::ExchangeAccount( const TQString& group )
{
  load( group );
}

ExchangeAccount::~ExchangeAccount()
{
}

TQString endecryptStr( const TQString &aStr ) 
{
  TQString result;
  for (uint i = 0; i < aStr.length(); i++)
    result += (aStr[i].unicode() < 0x20) ? aStr[i] :
              TQChar(0x1001F - aStr[i].unicode());
  return result;
}

void ExchangeAccount::save( TQString const &group )
{
  kapp->config()->setGroup( group );
  kapp->config()->writeEntry( "host", mHost );
  kapp->config()->writeEntry( "user", mAccount );
  kapp->config()->writeEntry( "mailbox", mMailbox );
  kapp->config()->writeEntry( "MS-ID", endecryptStr( mPassword ) );
  kapp->config()->sync();
}

void ExchangeAccount::load( TQString const &group )
{
  kapp->config()->setGroup( group );

  TQString host = kapp->config()->readEntry( "host" );
  if ( ! host.isNull() ) {
    mHost = host;
  } else {
    mHost = "mail.company.com";
  }

  TQString user = kapp->config()->readEntry( "user" );
  if ( ! user.isNull() ) {
    mAccount = user;
  } else {
    mAccount = "username";
  }

  TQString mailbox = kapp->config()->readEntry( "mailbox" );
  if ( ! mailbox.isNull() ) {
    mMailbox = mailbox;
  } else {
    mMailbox = "webdav://" + host + "/exchange/" + mAccount;
  }

  TQString password = endecryptStr( kapp->config()->readEntry( "MS-ID" ) );
  if ( ! password.isNull() ) {
    mPassword = password;
  }
}

KURL ExchangeAccount::baseURL()
{
  KURL url = KURL( mMailbox );
  return url;
}

KURL ExchangeAccount::calendarURL()
{
  if ( mCalendarURL ) {
    return *mCalendarURL;
  } else {
    KURL url = baseURL();
    url.addPath( "Calendar" );
    return url;
  }
}

bool ExchangeAccount::authenticate( TQWidget *window )
{
  if ( window )
    return authenticate( window->winId() );
  else
    return authenticate();
}

bool ExchangeAccount::authenticate()
{
  long windowId;
  TQWidgetList *widgets = TQApplication::topLevelWidgets();
  if ( widgets->isEmpty() )
    windowId = 0;
  else
    windowId = widgets->first()->winId();
  delete widgets;

  return authenticate( windowId );
}

bool ExchangeAccount::authenticate( int windowId )
{
  kdDebug() << "Entering ExchangeAccount::authenticate( windowId=" << windowId << " )" << endl;

  kdDebug() << "Authenticating to base URL: " << baseURL().prettyURL() << endl;

  KIO::AuthInfo info;
  info.url = baseURL();
  info.username = mAccount;
  info.password = mPassword;
  info.realmValue = mHost;
  info.digestInfo = "Basic";

  DCOPClient *dcopClient = new DCOPClient();
  dcopClient->attach();

  TQByteArray params;
  TQDataStream stream(params, IO_WriteOnly);
  stream << info << windowId;

  dcopClient->send( "kded", "kpasswdserver",
                    "addAuthInfo(KIO::AuthInfo, long int)", params );

  dcopClient->detach();
  delete dcopClient;

  mCalendarURL = 0;

  calcFolderURLs();

  // TODO: Remove this busy loop
  TQApplication::setOverrideCursor( KCursor::waitCursor() );
  do {
    qApp->processEvents();
  } while ( !mCalendarURL && !mError );
  TQApplication::restoreOverrideCursor();

  return !mError;
}

void ExchangeAccount::calcFolderURLs()
{
  kdDebug() << "ExchangeAccount::calcFolderURLs" << endl;
  TQDomDocument doc;
  TQDomElement root = addElement( doc, doc, "DAV:", "propfind" );
  TQDomElement prop = addElement( doc, root, "DAV:", "prop" );
  addElement( doc, prop, "urn:schemas:httpmail:", "calendar" );
// For later use:
// urn:schemas:httpmail:contacts Contacts 
// urn:schemas:httpmail:deleteditems Deleted Items 
// urn:schemas:httpmail:drafts Drafts 
// urn:schemas:httpmail:inbox Inbox 
// urn:schemas:httpmail:journal Journal 
// urn:schemas:httpmail:notes Notes 
// urn:schemas:httpmail:outbox Outbox 
// urn:schemas:httpmail:sentitems Sent Items 
// urn:schemas:httpmail:tasks Tasks 
// urn:schemas:httpmail:sendmsg Exchange Mail Submission URI 
// urn:schemas:httpmail:msgfolderroot Mailbox folder (root) 

  kdDebug() << "calcFolderUrls(): " << baseURL() << endl;

  mError = false;

  KIO::DavJob* job = KIO::davPropFind( baseURL(), doc, "1", false );
  job->addMetaData( "errorPage", "false" );
  connect( job, TQT_SIGNAL( result( KIO::Job * ) ),
           TQT_SLOT( slotFolderResult( KIO::Job * ) ) );
}

void ExchangeAccount::slotFolderResult( KIO::Job *job )
{
  kdDebug() << "ExchangeAccount::slotFolderResult()" << endl;
  if ( job->error() ) {
    kdError() << "Error: Cannot get well-know folder names; " << job->error() << endl;
    TQString text = i18n("ExchangeAccount\nError accessing '%1': %2")
                   .arg( baseURL().prettyURL() ).arg( job->errorString() );
    KMessageBox::error( 0, text );
    mError = true;
    return;
  }
  TQDomDocument &response = static_cast<KIO::DavJob *>( job )->response();

  TQDomElement prop = response.documentElement().namedItem( "response" )
                     .namedItem( "propstat" ).namedItem( "prop" ).toElement();
 
  TQDomElement calElement = prop.namedItem( "calendar" ).toElement();
  if ( calElement.isNull() ) {
    kdError() << "Error: no calendar URL in Exchange server reply" << endl;
    mError = true;
    return;
  }
  TQString calendar = calElement.text();

  kdDebug() << "ExchangeAccount: response calendarURL: " << calendar << endl;

  mCalendarURL = toDAV( new KURL( calendar ) );
  kdDebug() << "Calendar URL: " << mCalendarURL->url() << endl;
}

TQString ExchangeAccount::tryFindMailbox( const TQString& host, const TQString& port, const TQString& user, const TQString& password )
{
  kdDebug() << "Entering ExchangeAccount::tryFindMailbox()" << endl;

  KURL url("http://" + host + "/exchange");
  if (!port.isEmpty()) url.setPort(port.toInt());
  
  TQString result = tryMailbox( url.url(), user, password );
  if ( result.isNull() )
  {
    url.setProtocol("https");
    result = tryMailbox( url.url(), user, password );
  }
  return result;
}
  
TQString ExchangeAccount::tryMailbox( const TQString &_url, const TQString &user,
                                     const TQString &password )
{
  KURL url = KURL( _url );
  url.setUser( user );
  url.setPass( password );

  TQString tmpFile;
  if ( !KIO::NetAccess::download( url, tmpFile, 0 ) ) {
    kdWarning() << "Trying to find mailbox failed: not able to download " << url.prettyURL() << endl;
    return TQString::null;
  }
  TQFile file( tmpFile );
  if ( !file.open( IO_ReadOnly ) ) {
    kdWarning() << "Trying to find mailbox failed: not able to open temp file " << tmpFile << endl;
    KIO::NetAccess::removeTempFile( tmpFile );
    return TQString::null;
  }

  TQTextStream stream( &file );
  TQString line;
  TQString result;
  while ( !stream.eof() ) {
      line = stream.readLine(); // line of text excluding '\n'
      int pos = line.find( "<BASE href=\"", 0, FALSE );
      if ( pos < 0 )
        continue;
      int end = line.find( "\"", pos+12, FALSE );
      if ( pos < 0 ) {
        kdWarning() << "Strange, found no closing quote in " << line << endl;
        continue;
      } 
      TQString mailboxString = line.mid( pos+12, end-pos-12 );
      KURL mailbox( mailboxString );
      if ( mailbox.isEmpty() ) {
        kdWarning() << "Strange, could not get URL from " << mailboxString << " in line " << line << endl;
        continue;
      }
      result = toDAV( mailbox ).prettyURL( -1 ); // Strip ending slash from URL, if present
      kdDebug() << "Found mailbox: " << result << endl;
    }
    file.close();

    KIO::NetAccess::removeTempFile( tmpFile );
    return result;
}

#include "exchangeaccount.moc"
