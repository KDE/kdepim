/*
    This file is part of kdepim.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qapplication.h>
#include <qstringlist.h>

#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <kurl.h>

#include "libkcal/journal.h"

#include "knotes/resourcemanager.h"

#include "knotes_resourcexmlrpc.h"
#include "knotes_resourcexmlrpcconfig.h"

#include "xmlrpciface.h"

using namespace KNotes;

extern "C"
{
  void *init_knotes_xmlrpc()
  {
    return new KRES::PluginFactory<ResourceXMLRPC, ResourceXMLRPCConfig>();
  }
}

static const QString SearchNotesCommand = "infolog.boinfolog.search";
static const QString AddNoteCommand = "infolog.boinfolog.write";
static const QString DeleteNoteCommand = "infolog.boinfolog.delete";
static const QString LoadNoteCategoriesCommand = "infolog.boinfolog.categories";

ResourceXMLRPC::ResourceXMLRPC( const KConfig* config )
  : ResourceNotes( config ), mServer( 0 )
{
  if ( config )
    readConfig( config );
  else
    mDomain = "default";

  init();
}

ResourceXMLRPC::ResourceXMLRPC( )
  : ResourceNotes( 0 ), mServer( 0 )
{
  init();
}

ResourceXMLRPC::~ResourceXMLRPC()
{
  delete mServer;
}

void ResourceXMLRPC::init()
{
  setType( "xmlrpc" );

  mSyncComm = false;
}

void ResourceXMLRPC::readConfig( const KConfig* config )
{
  mURL = config->readEntry( "XmlRpcUrl" );
  mDomain = config->readEntry( "XmlRpcDomain", "default" );
  mUser = config->readEntry( "XmlRpcUser" );
  mPassword = KStringHandler::obscure( config->readEntry( "XmlRpcPassword" ) );
}

void ResourceXMLRPC::writeConfig( KConfig* config )
{
  ResourceNotes::writeConfig( config );

  config->writeEntry( "XmlRpcUrl", mURL.url() );
  config->writeEntry( "XmlRpcDomain", mDomain );
  config->writeEntry( "XmlRpcUser", mUser );
  config->writeEntry( "XmlRpcPassword", KStringHandler::obscure( mPassword ) );
}


void ResourceXMLRPC::setURL( const KURL& url )
{
  mURL = url;
}

KURL ResourceXMLRPC::url() const
{
  return mURL;
}

void ResourceXMLRPC::setDomain( const QString& domain )
{
  mDomain = domain;
}

QString ResourceXMLRPC::domain() const
{
  return mDomain;
}

void ResourceXMLRPC::setUser( const QString& user )
{
  mUser = user;
}

QString ResourceXMLRPC::user() const
{
  return mUser;
}

void ResourceXMLRPC::setPassword( const QString& password )
{
  mPassword = password;
}

QString ResourceXMLRPC::password() const
{
  return mPassword;
}

bool ResourceXMLRPC::load()
{
  mCalendar.close();

  if ( mServer )
    delete mServer;

  mServer = new KXMLRPC::Server( KURL(), this );
	mServer->setUrl( mURL );
  mServer->setUserAgent( "KDE-Notes" );

  QMap<QString, QVariant> args, columns;
  args.insert( "domain", mDomain );
  args.insert( "username", mUser );
  args.insert( "password", mPassword );

  mServer->call( "system.login", QVariant( args ),
                 this, SLOT( loginFinished( const QValueList<QVariant>&, const QVariant& ) ),
                 this, SLOT( fault( int, const QString&, const QVariant& ) ) );

  enter_loop();

  columns.insert( "type", "task" );
  args.clear();
  args.insert( "filter", "none" );
  args.insert( "col_filter", columns );
  args.insert( "order", "id_parent" );

  mServer->call( SearchNotesCommand, args,
                 this, SLOT( listNotesFinished( const QValueList<QVariant>&, const QVariant& ) ),
                 this, SLOT( fault( int, const QString&, const QVariant& ) ) );

  enter_loop();

  return true;
}

bool ResourceXMLRPC::save()
{
  mCalendar.close();

  return true;
}

bool ResourceXMLRPC::addNote( KCal::Journal *journal )
{
  QMap<QString, QVariant> args;
  writeNote( journal, args );

  KCal::Journal *oldJournal = mCalendar.journal( journal->uid() );

  bool added = false;
  if ( oldJournal ) {
    if ( !oldJournal->isReadOnly() ) {
      writeNote( journal, args );
      args.insert( "id", mUidMap[ journal->uid() ].toInt() );
      mServer->call( AddNoteCommand, QVariant( args ),
                     this, SLOT( updateNoteFinished( const QValueList<QVariant>&, const QVariant& ) ),
                     this, SLOT( fault( int, const QString&, const QVariant& ) ) );
      mCalendar.addJournal( journal );
      added = true;
    }
  } else {
    mServer->call( AddNoteCommand, QVariant( args ),
                   this, SLOT( addNoteFinished( const QValueList<QVariant>&, const QVariant& ) ),
                   this, SLOT( fault( int, const QString&, const QVariant& ) ),
                   QVariant( journal->uid() ) );

    mCalendar.addJournal( journal );
    manager()->registerNote( this, journal );
    added = true;
  }

  if ( added )
    enter_loop();

  return true;
}

bool ResourceXMLRPC::deleteNote( KCal::Journal *journal )
{
  int id = mUidMap[ journal->uid() ].toInt();

  mServer->call( DeleteNoteCommand, id,
                 this, SLOT( deleteNoteFinished( const QValueList<QVariant>&, const QVariant& ) ),
                 this, SLOT( fault( int, const QString&, const QVariant& ) ),
                 QVariant( journal->uid() ) );
  enter_loop();

  return true;
}

void ResourceXMLRPC::loginFinished( const QValueList<QVariant>& variant,
                                    const QVariant& )
{
  QMap<QString, QVariant> map = variant[ 0 ].toMap();

  KURL url = mURL;
  if ( map[ "GOAWAY" ].toString() == "XOXO" ) { // failed
    mSessionID = mKp3 = "";
  } else {
    mSessionID = map[ "sessionid" ].toString();
    mKp3 = map[ "kp3" ].toString();
  }

  url.setUser( mSessionID );
  url.setPass( mKp3 );
  mServer->setUrl( url );

  exit_loop();
}

void ResourceXMLRPC::logoutFinished( const QValueList<QVariant>& variant,
                                     const QVariant& )
{
  QMap<QString, QVariant> map = variant[ 0 ].toMap();

  if ( map[ "GOODBYE" ].toString() != "XOXO" )
    kdError() << "logout failed" << endl;

  KURL url = mURL;
  mSessionID = mKp3 = "";
  url.setUser( mSessionID );
  url.setPass( mKp3 );
  mServer->setUrl( url );

  exit_loop();
}

void ResourceXMLRPC::listNotesFinished( const QValueList<QVariant> &list, const QVariant& )
{
  QMap<QString, QString>::Iterator uidIt;
  for ( uidIt = mUidMap.begin(); uidIt != mUidMap.end(); ++uidIt ) {
    KCal::Journal *journal = mCalendar.journal( uidIt.key() );
    mCalendar.deleteJournal( journal );
  }

  mUidMap.clear();

  QValueList<QVariant> noteList = list[ 0 ].toList();
  QValueList<QVariant>::Iterator noteIt;

  for ( noteIt = noteList.begin(); noteIt != noteList.end(); ++noteIt ) {
    QMap<QString, QVariant> map = (*noteIt).toMap();

    KCal::Journal *journal = new KCal::Journal();

    QString uid;
    readNote( map, journal, uid );
    mUidMap.insert( journal->uid(), uid );

    mCalendar.addJournal( journal );
    manager()->registerNote( this, journal );
  }

  exit_loop();
}

void ResourceXMLRPC::addNoteFinished( const QValueList<QVariant> &list, const QVariant &id )
{
  int uid = list[ 0 ].toInt();
  mUidMap.insert( id.toString(), QString::number( uid ) );

  exit_loop();
}

void ResourceXMLRPC::updateNoteFinished( const QValueList<QVariant>&, const QVariant& )
{
  exit_loop();
}

void ResourceXMLRPC::deleteNoteFinished( const QValueList<QVariant>&, const QVariant &id )
{
  mUidMap.erase( id.toString() );

  KCal::Journal *journal = mCalendar.journal( id.toString() );
  mCalendar.deleteJournal( journal );

  exit_loop();
}

void ResourceXMLRPC::fault( int error, const QString& errorMsg, const QVariant& )
{
  kdError() << "Server send error " << error << ": " << errorMsg << endl;
  exit_loop();
}

void ResourceXMLRPC::writeNote( KCal::Journal* journal, QMap<QString, QVariant>& args )
{
  args.insert( "subject", journal->summary() );
  args.insert( "des", journal->description() );
  args.insert( "access",
               (journal->secrecy() == KCal::Journal::SecrecyPublic ? "public" : "private" ) );
}

void ResourceXMLRPC::readNote( const QMap<QString, QVariant>& args, KCal::Journal *journal, QString &uid )
{
  uid = args[ "id" ].toString();

  journal->setSummary( args[ "subject" ].toString() );
  journal->setDescription( args[ "des" ].toString() );
  journal->setSecrecy( args[ "access" ].toString() == "public" ?
                       KCal::Journal::SecrecyPublic : KCal::Journal::SecrecyPrivate );
}

void qt_enter_modal( QWidget* widget );
void qt_leave_modal( QWidget* widget );

void ResourceXMLRPC::enter_loop()
{
  QWidget dummy( 0, 0, WType_Dialog | WShowModal );
  dummy.setFocusPolicy( QWidget::NoFocus );
  qt_enter_modal( &dummy );
  mSyncComm = true;
  qApp->enter_loop();
  qt_leave_modal( &dummy );
}

void ResourceXMLRPC::exit_loop()
{
  if ( mSyncComm ) {
    mSyncComm = false;
    qApp->exit_loop();
  }
}

#include "knotes_resourcexmlrpc.moc"
