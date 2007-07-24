/*
    Copyright (c) 2005 by Volker Krause <volker.krause@rwth-aachen.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include "nntpjobs.h"

#include "kngroup.h"
#include "kngroupmanager.h"
#include "knserverinfo.h"
#include <kdebug.h>
#include <klocale.h>
#include <QDir>

KNode::GroupListJob::GroupListJob( KNJobConsumer * c, KNServerInfo * a, KNJobItem * i, bool incremental ) :
  KNJobData( KNJobData::JTFetchGroups, c, a, i ),
  mIncremental( incremental )
{
}

void KNode::GroupListJob::execute()
{
  mGroupList.clear();

  KNGroupListData *target = static_cast<KNGroupListData *>( data() );

  KUrl destination = baseUrl();
  QStringList query;
  if ( target->getDescriptions )
    query << "desc=true";
  if ( mIncremental )
    query << QString( "since=%1%2%3 000000" )
        .arg( target->fetchSince.year() % 100, 2, 10, QChar( '0' ) )
        .arg( target->fetchSince.month(), 2, 10, QChar( '0' ) )
        .arg( target->fetchSince.day(), 2, 10, QChar( '0' ) );
  destination.setQuery( query.join( "&" ) );
  KIO::Job* job = KIO::listDir( destination, false, true );
  connect( job, SIGNAL(entries(KIO::Job*, const KIO::UDSEntryList&)),
           SLOT(slotEntries(KIO::Job*, const KIO::UDSEntryList&)) );
  connect( job, SIGNAL( result(KJob*) ), SLOT( slotResult(KJob*) ) );
  setupKIOJob( job );
}

void KNode::GroupListJob::slotEntries( KIO::Job * job, const KIO::UDSEntryList & list )
{
  Q_UNUSED( job );
   KNGroupListData *target = static_cast<KNGroupListData *>( data() );

  QString name, desc;
  bool subscribed;
  KNGroup::Status access;
  for( KIO::UDSEntryList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
    access = KNGroup::unknown;
    name = (*it).stringValue( KIO::UDSEntry::UDS_NAME );
    desc = (*it).stringValue( KIO::UDSEntry::UDS_EXTRA );

    int value = (*it).numberValue( KIO::UDSEntry::UDS_ACCESS, -1 );
    if ( value != -1 ) {
      if( value & S_IWOTH )
        access = KNGroup::postingAllowed;
      else if ( value & S_IWGRP )
        access = KNGroup::moderated;
      else
        access = KNGroup::readOnly;
    }

    if ( name.isEmpty() )
      continue;
    if ( target->subscribed.contains( name ) ) {
      target->subscribed.removeAll( name );    // group names are unique, we wont find it again anyway...
      subscribed = true;
    } else {
      subscribed = false;
    }
    if ( mIncremental )
      mGroupList.append( new KNGroupInfo( name, desc, true, subscribed, access ) );
    else
      target->groups->append( new KNGroupInfo( name, desc, false, subscribed, access ) );
  }
}

void KNode::GroupListJob::slotResult( KJob * job )
{
  if ( job->error() )
    setError( job->error(), job->errorString() );
  else {
    KNGroupListData *target = static_cast<KNGroupListData *>( data() );

    // TODO: use thread weaver here?
    if ( mIncremental ) {
      setStatus( i18n("Loading group list from disk...") );
      if ( !target->readIn() ) {
        setError( KIO::ERR_COULD_NOT_READ, i18n("Unable to read the group list file") );
        emitFinished();
        return;
      }
      target->merge( &mGroupList );
    }
    setStatus( i18n("Writing group list to disk...") );

    if ( !target->writeOut() )
      setError( KIO::ERR_COULD_NOT_WRITE, i18n("Unable to write the group list file") );
  }

  emitFinished();
}



KNode::GroupLoadJob::GroupLoadJob( KNJobConsumer * c, KNServerInfo * a, KNJobItem * i ) :
  KNJobData( KNJobData::JTLoadGroups, c, a, i )
{
}

void KNode::GroupLoadJob::execute( )
{
  KNGroupListData *target = static_cast<KNGroupListData *>( data() );

  setStatus( i18n("Loading group list from disk...") );
  // TODO: use the thread weaver here
  if ( !target->readIn() )
    setError( KIO::ERR_COULD_NOT_READ, i18n("Unable to read the group list file") );

  emitFinished();
}



KNode::ArticleListJob::ArticleListJob( KNJobConsumer * c, KNServerInfo * a, KNJobItem * i, bool silent ) :
    KNJobData( JTfetchNewHeaders, c, a, i ),
    mSilent( silent )
{
}

void KNode::ArticleListJob::execute()
{
  mArticleList.clear();

  KNGroup* target = static_cast<KNGroup*>( data() );

  KUrl destination = baseUrl();
  destination.setPath( target->groupname() );
  QStringList query;
  query << "first=" + QString::number( target->lastNr() + 1 );
  if ( target->lastNr() <= 0 ) // first fetch
    query << "max=" + QString::number( target->maxFetch() );
  destination.setQuery( query.join( "&" ) );
  KIO::Job* job = KIO::listDir( destination, false, true );
  connect( job, SIGNAL(entries(KIO::Job*, const KIO::UDSEntryList&)),
           SLOT(slotEntries(KIO::Job*, const KIO::UDSEntryList&)) );
  connect( job, SIGNAL( result(KJob*) ), SLOT( slotResult(KJob*) ) );
  setupKIOJob( job );
}

void KNode::ArticleListJob::slotEntries( KIO::Job * job, const KIO::UDSEntryList & list )
{
  Q_UNUSED( job );
  mArticleList += list;
}

void KNode::ArticleListJob::slotResult( KJob * _job )
{
  Q_ASSERT( mJob == _job );
  KIO::Job *job = static_cast<KIO::Job*>( _job );
  if ( job->error() )
    setError( job->error(), job->errorString() );
  else {
    KNGroup* target = static_cast<KNGroup*>( data() );
    target->setLastFetchCount( 0 );

    setStatus( i18n("Sorting...") );

    if ( job->metaData().contains( "FirstSerialNumber" ) ) {
      int firstSerNum = job->metaData()["FirstSerialNumber"].toInt();
      target->setFirstNr( firstSerNum );
    }

    target->insortNewHeaders( mArticleList );

    if ( job->metaData().contains( "LastSerialNumber" ) ) {
      int lastSerNum = job->metaData()["LastSerialNumber"].toInt();
      target->setLastNr( lastSerNum );
    }
  }

  emitFinished();
}



KNode::ArticleFetchJob::ArticleFetchJob( KNJobConsumer * c, KNServerInfo * a, KNJobItem * i, bool parse ) :
    KNJobData( JTfetchArticle, c, a, i ),
    mParseArticle( parse )
{
}

void KNode::ArticleFetchJob::execute()
{
  KNRemoteArticle *target = static_cast<KNRemoteArticle*>( data() );
  QString path = static_cast<KNGroup*>( target->collection() )->groupname();

  KUrl url = baseUrl();
  path += QDir::separator();
  path += target->messageID()->as7BitString( false );
  url.setPath( path );

  KIO::Job* job = KIO::storedGet( url, false, false );
  connect( job, SIGNAL( result(KJob*) ), SLOT( slotResult(KJob*) ) );
  setupKIOJob( job );
}

void KNode::ArticleFetchJob::slotResult( KJob * job )
{
  if ( job->error() )
    setError( job->error(), job->errorString() );
  else {
    KNRemoteArticle *target = static_cast<KNRemoteArticle*>( data() );
    KIO::StoredTransferJob *j = static_cast<KIO::StoredTransferJob*>( job );
    QByteArray buffer = j->data();
    buffer.replace( "\r\n", "\n" ); // TODO: do this in the io-slave?
    target->setContent( buffer );
    if ( mParseArticle )
      target->parse();
  }

  emitFinished();
}



KNode::ArticlePostJob::ArticlePostJob( KNJobConsumer * c, KNServerInfo * a, KNJobItem * i ) :
    KNJobData( JTpostArticle, c, a, i )
{
}

void KNode::ArticlePostJob::execute( )
{
  KNLocalArticle *target = static_cast<KNLocalArticle*>( data() );

  KUrl url = baseUrl();

  KIO::Job* job = KIO::storedPut( target->encodedContent( true ), url, -1, true, false, false );
  connect( job, SIGNAL( result(KJob*) ), SLOT( slotResult(KJob*) ) );
  setupKIOJob( job );
}

void KNode::ArticlePostJob::slotResult( KJob * job )
{
  if ( job->error() )
    setError( job->error(), job->errorString() );

  emitFinished();
}

#include "nntpjobs.moc"
