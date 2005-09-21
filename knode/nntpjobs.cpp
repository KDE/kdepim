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

KNode::GroupFetchJob::GroupFetchJob( KNJobConsumer * c, KNServerInfo * a, KNJobItem * i ) :
  KNJobData( KNJobData::JTFetchGroups, c, a, i )
{
}

void KNode::GroupFetchJob::execute()
{
  KNGroupListData *target = static_cast<KNGroupListData *>( data() );

  KURL destination = baseUrl();
  QStringList query;
  if ( target->getDescriptions )
    query << "desc=true";
  if ( type() == JTCheckNewGroups )
    query << QString( "since=%1%2%3 000000" )
        .arg( target->fetchSince.year() % 100, 2, 10, QChar( '0' ) )
        .arg( target->fetchSince.month(), 2, 10, QChar( '0' ) )
        .arg( target->fetchSince.day(), 2, 10, QChar( '0' ) );
  destination.setQuery( query.join( "&" ) );
  KIO::Job* job = KIO::listDir( destination, false, true );
  connect( job, SIGNAL(entries(KIO::Job*, const KIO::UDSEntryList&)),
           SLOT(slotEntries(KIO::Job*, const KIO::UDSEntryList&)) );
  connect( job, SIGNAL( result(KIO::Job*) ),
           SLOT( slotResult(KIO::Job*) ) );
  if ( account()->encryption() == KNServerInfo::TLS )
    job->addMetaData( "TLS", "on" );
  else
    job->addMetaData( "TLS", "off" );
  setJob( job );
  setStatus( i18n("Downloading group list...") );
}

void KNode::GroupFetchJob::slotEntries( KIO::Job * job, const KIO::UDSEntryList & list )
{
   KNGroupListData *target = static_cast<KNGroupListData *>( data() );

  QString name, desc;
  bool subscribed;
  KNGroup::Status access;
  for( KIO::UDSEntryListConstIterator it = list.begin(); it != list.end(); ++it ) {
    name = QString::null;
    desc = QString::null;
    access = KNGroup::unknown;
    for ( KIO::UDSEntry::ConstIterator it2 = (*it).begin(); it2 != (*it).end(); ++it2 ) {
      if ( (*it2).m_uds == KIO::UDS_NAME )
        name = (*it2).m_str;
      else if ( (*it2).m_uds == KIO::UDS_ACCESS ) {
        if ( (*it2).m_long & S_IWOTH )
          access = KNGroup::postingAllowed;
        else if ( (*it2).m_long & S_IWGRP )
          access = KNGroup::moderated;
        else
          access = KNGroup::readOnly;
      } else if ( (*it2).m_uds == KIO::UDS_EXTRA )
        desc = (*it2).m_str;
    }
    if ( name.isEmpty() )
      continue;
    if ( target->subscribed.contains( name ) ) {
      target->subscribed.remove( name );    // group names are unique, we wont find it again anyway...
      subscribed = true;
    } else {
      subscribed = false;
    }
    if ( type() == JTCheckNewGroups )
      mGroupList.append( new KNGroupInfo( name, desc, true, subscribed, access ) );
    else
      target->groups->append( new KNGroupInfo( name, desc, false, subscribed, access ) );
  }
}

void KNode::GroupFetchJob::slotResult( KIO::Job * job )
{
  if ( job->error() )
    setErrorString( job->errorString() );
  else {
    KNGroupListData *target = static_cast<KNGroupListData *>( data() );

    // TODO: use thread weaver here?
    if ( type() == JTCheckNewGroups ) {
      setStatus( i18n("Loading group list from disk...") );
      if ( !target->readIn() ) {
        setErrorString( i18n("Unable to read the group list file") );
        emitFinished();
        return;
      }
      target->merge( &mGroupList );
    }
    setStatus( i18n("Writing group list to disk...") );

    if ( !target->writeOut() )
      setErrorString( i18n("Unable to write the group list file") );
  }

  emitFinished();
}



KNode::GroupUpdateJob::GroupUpdateJob( KNJobConsumer * c, KNServerInfo * a, KNJobItem * i ) :
  KNode::GroupFetchJob( c, a, i )
{
  t_ype = JTCheckNewGroups;
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
    setErrorString(i18n("Unable to read the group list file"));

  emitFinished();
}



KNode::ArticleListJob::ArticleListJob( KNJobConsumer * c, KNServerInfo * a, KNJobItem * i ) :
    KNJobData( JTfetchNewHeaders, c, a, i )
{
}

void KNode::ArticleListJob::execute()
{
  KNGroup* target = static_cast<KNGroup*>( data() );

  KURL destination = baseUrl();
  destination.setPath( target->groupname() );
  QStringList query;
  query << "first=" + QString::number( target->lastNr() + 1 );
  if ( target->lastNr() <= 0 ) // first fetch
    query << "max=" + QString::number( target->maxFetch() );
  destination.setQuery( query.join( "&" ) );
  KIO::Job* job = KIO::listDir( destination, false, true );
  connect( job, SIGNAL(entries(KIO::Job*, const KIO::UDSEntryList&)),
           SLOT(slotEntries(KIO::Job*, const KIO::UDSEntryList&)) );
  connect( job, SIGNAL( result(KIO::Job*) ),
           SLOT( slotResult(KIO::Job*) ) );
  if ( account()->encryption() == KNServerInfo::TLS )
    job->addMetaData( "TLS", "on" );
  else
    job->addMetaData( "TLS", "off" );
  setJob( job );
  setStatus( i18n("Downloading new headers...") );
}

void KNode::ArticleListJob::slotEntries( KIO::Job * job, const KIO::UDSEntryList & list )
{
  mArticleList += list;
}

void KNode::ArticleListJob::slotResult( KIO::Job * job )
{
  kdDebug(5003) << k_funcinfo << mArticleList.count() << endl;
  KNGroup* target = static_cast<KNGroup*>( data() );

  target->insortNewHeaders( mArticleList );

  int lastSerNum = 0;
  if ( job->metaData().contains( "LastSerialNumber" ) )
    lastSerNum = job->metaData()["LastSerialNumber"].toInt();
  target->setLastNr( lastSerNum );

  emitFinished();
}

#include "nntpjobs.moc"
