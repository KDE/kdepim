/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>

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

#include "groupwaredataadaptor.h"
#include <kdebug.h>
#include <kio/job.h>

using namespace KPIM;
    
GroupwareUploadItem::GroupwareUploadItem( UploadType type ) : mType( type )
{
}

KIO::TransferJob *GroupwareUploadItem::createUploadJob( GroupwareDataAdaptor *adaptor, const KURL &url )
{
  Q_ASSERT( adaptor );
  if ( !adaptor ) return 0;
  const QString dta = data();
  //kdDebug(7000) << "Uploading: " << data << endl;
  //kdDebug(7000) << "Uploading to: " << url.prettyURL() << endl;
  KIO::TransferJob *job = KIO::storedPut( dta.utf8(), url, -1, true, false, false );
  job->addMetaData( "PropagateHttpHeader", "true" );
  if ( adaptor )
    job->addMetaData( "content-type", adaptor->mimeType() );
  return job;
}


GroupwareDataAdaptor::GroupwareDataAdaptor()
  : mFolderLister( 0 ), mIdMapper( 0 )
{
}

GroupwareDataAdaptor::~GroupwareDataAdaptor()
{
}

void GroupwareDataAdaptor::setUserPassword( KURL &url )
{
  kdDebug(5800) << "GroupwareDataAdaptor::setUserPassword, mUser=" << mUser << endl;
  url.setUser( mUser );
  url.setPass( mPassword );
}

KIO::TransferJob *GroupwareDataAdaptor::createUploadJob( const KURL &url, GroupwareUploadItem *item )
{
  if ( item )
    return item->createUploadJob( this, url );
  else return 0;
}
KIO::TransferJob *GroupwareDataAdaptor::createUploadNewJob( const KURL &url, GroupwareUploadItem *item )
{
  if ( item )
    return item->createUploadNewJob( this, url );
  else return 0;
}

