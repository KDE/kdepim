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

#include "kngroupmanager.h"
#include "knserverinfo.h"

#include <kdebug.h>

KNode::GroupFetchJob::GroupFetchJob( KNJobConsumer * c, KNServerInfo * a, KNJobItem * i ) :
  KNJobData( KNJobData::JTFetchGroups, c, a, i )
{
}

void KNode::GroupFetchJob::execute()
{
  KNGroupListData *target = static_cast<KNGroupListData *>( data() );

  KURL destination;
  if ( account()->encryption() == KNServerInfo::SSL )
    destination.setProtocol( "nntps" );
  else
    destination.setProtocol( "nntp" );
  destination.setHost( account()->server() );
  destination.setPort( account()->port() );
  if ( account()->needsLogon() ) {
    destination.setUser( account()->user() );
    destination.setPass( account()->pass() );
  }
  QString query;
  if ( target->getDescriptions )
    query = "desc=true";
  destination.setQuery( query );
  KIO::Job* job = KIO::listDir( destination, false, true );
  connect( job, SIGNAL(entries(KIO::Job*, const KIO::UDSEntryList&)),
           SLOT(slotEntries(KIO::Job*, const KIO::UDSEntryList&)) );
  if ( account()->encryption() == KNServerInfo::TLS )
    job->addMetaData( "TLS", "on" );
  else
    job->addMetaData( "TLS", "off" );
  setJob( job );
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
    } else
      subscribed = false;
    target->groups->append( new KNGroupInfo( name, desc, false, subscribed, access ) );
  }
}

#include "nntpjobs.moc"
