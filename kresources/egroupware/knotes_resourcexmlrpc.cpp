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

#include "knotes_egroupwareprefs.h"
#include "knotes_resourcexmlrpc.h"
#include "knotes_resourcexmlrpcconfig.h"

#include "synchronizer.h"
#include "xmlrpciface.h"

using namespace KNotes;

typedef KRES::PluginFactory< ResourceXMLRPC, ResourceXMLRPCConfig> XMLRPCFactory;
K_EXPORT_COMPONENT_FACTORY( knotes_xmlrpc, XMLRPCFactory )

static const QString SearchNotesCommand = "infolog.boinfolog.search";
static const QString AddNoteCommand = "infolog.boinfolog.write";
static const QString DeleteNoteCommand = "infolog.boinfolog.delete";
static const QString LoadNoteCategoriesCommand = "infolog.boinfolog.categories";

ResourceXMLRPC::ResourceXMLRPC( const KConfig* config )
  : ResourceNotes( config ), mServer( 0 )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  if ( config )
    readConfig( config );
}

ResourceXMLRPC::ResourceXMLRPC( )
  : ResourceNotes( 0 ), mServer( 0 )
{
  init();

  mPrefs->addGroupPrefix( identifier() );
}

ResourceXMLRPC::~ResourceXMLRPC()
{
  delete mServer;
  mServer = 0;

  delete mPrefs;
  mPrefs = 0;

  delete mSynchronizer;
  mSynchronizer = 0;
}

void ResourceXMLRPC::init()
{
  setType( "xmlrpc" );

  mPrefs = new EGroupwarePrefs;

  mSynchronizer = new Synchronizer;
}

void ResourceXMLRPC::readConfig( const KConfig* )
{
  mPrefs->readConfig();
}

void ResourceXMLRPC::writeConfig( KConfig* config )
{
  ResourceNotes::writeConfig( config );

  mPrefs->writeConfig();
}

bool ResourceXMLRPC::load()
{
  mCalendar.close();

  if ( mServer )
    delete mServer;

  mServer = new KXMLRPC::Server( KURL(), this );
  mServer->setUrl( KURL( mPrefs->url() ) );
  mServer->setUserAgent( "KDE-Notes" );

  QMap<QString, QVariant> args, columns;
  args.insert( "domain", mPrefs->domain() );
  args.insert( "username", mPrefs->user() );
  args.insert( "password", mPrefs->password() );

  mServer->call( "system.login", QVariant( args ),
                 this, SLOT( loginFinished( const QValueList<QVariant>&, const QVariant& ) ),
                 this, SLOT( fault( int, const QString&, const QVariant& ) ) );

  mSynchronizer->start();

  columns.insert( "type", "note" );
  args.clear();
  args.insert( "filter", "none" );
  args.insert( "col_filter", columns );
  args.insert( "order", "id_parent" );

  mServer->call( SearchNotesCommand, args,
                 this, SLOT( listNotesFinished( const QValueList<QVariant>&, const QVariant& ) ),
                 this, SLOT( fault( int, const QString&, const QVariant& ) ) );

  mSynchronizer->start();

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
    added = true;
  }

  if ( added )
    mSynchronizer->start();

  return true;
}

bool ResourceXMLRPC::deleteNote( KCal::Journal *journal )
{
  int id = mUidMap[ journal->uid() ].toInt();

  mServer->call( DeleteNoteCommand, id,
                 this, SLOT( deleteNoteFinished( const QValueList<QVariant>&, const QVariant& ) ),
                 this, SLOT( fault( int, const QString&, const QVariant& ) ),
                 QVariant( journal->uid() ) );
  mSynchronizer->start();

  return true;
}

KCal::Alarm::List ResourceXMLRPC::alarms( const QDateTime& from, const QDateTime& to )
{
    KCal::Alarm::List alarms;
    KCal::Journal::List notes = mCalendar.journals();
    KCal::Journal::List::ConstIterator note;
    for ( note = notes.begin(); note != notes.end(); ++note )
    {
        QDateTime preTime = from.addSecs( -1 );
        KCal::Alarm::List::ConstIterator it;
        for( it = (*note)->alarms().begin(); it != (*note)->alarms().end(); ++it )
        {
            if ( (*it)->enabled() )
            {
                QDateTime dt = (*it)->nextRepetition( preTime );
                if ( dt.isValid() && dt <= to )
                    alarms.append( *it );
            }
        }
    }

    return alarms;
}

void ResourceXMLRPC::loginFinished( const QValueList<QVariant>& variant,
                                    const QVariant& )
{
  QMap<QString, QVariant> map = variant[ 0 ].toMap();

  KURL url = KURL( mPrefs->url() );
  if ( map[ "GOAWAY" ].toString() == "XOXO" ) { // failed
    mSessionID = mKp3 = "";
  } else {
    mSessionID = map[ "sessionid" ].toString();
    mKp3 = map[ "kp3" ].toString();
  }

  url.setUser( mSessionID );
  url.setPass( mKp3 );
  mServer->setUrl( url );

  mSynchronizer->stop();
}

void ResourceXMLRPC::logoutFinished( const QValueList<QVariant>& variant,
                                     const QVariant& )
{
  QMap<QString, QVariant> map = variant[ 0 ].toMap();

  if ( map[ "GOODBYE" ].toString() != "XOXO" )
    kdError() << "logout failed" << endl;

  KURL url = KURL( mPrefs->url() );
  mSessionID = mKp3 = "";
  url.setUser( mSessionID );
  url.setPass( mKp3 );
  mServer->setUrl( url );

  mSynchronizer->stop();
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

  mSynchronizer->stop();
}

void ResourceXMLRPC::addNoteFinished( const QValueList<QVariant> &list, const QVariant &id )
{
  int uid = list[ 0 ].toInt();
  mUidMap.insert( id.toString(), QString::number( uid ) );

  mSynchronizer->stop();
}

void ResourceXMLRPC::updateNoteFinished( const QValueList<QVariant>&, const QVariant& )
{
  mSynchronizer->stop();
}

void ResourceXMLRPC::deleteNoteFinished( const QValueList<QVariant>&, const QVariant &id )
{
  mUidMap.erase( id.toString() );

  KCal::Journal *journal = mCalendar.journal( id.toString() );
  mCalendar.deleteJournal( journal );

  mSynchronizer->stop();
}

void ResourceXMLRPC::fault( int error, const QString& errorMsg, const QVariant& )
{
  kdError() << "Server send error " << error << ": " << errorMsg << endl;
  mSynchronizer->stop();
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

#include "knotes_resourcexmlrpc.moc"
