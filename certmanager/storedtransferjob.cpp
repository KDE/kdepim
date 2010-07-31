/*
    Copyright (C) 2004 David Faure <faure@kde.org>

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

#include "storedtransferjob.h"

using namespace KIOext;

#define KIO_ARGS TQByteArray packedArgs; TQDataStream stream( packedArgs, IO_WriteOnly ); stream

StoredTransferJob::StoredTransferJob(const KURL& url, int command,
                                     const TQByteArray &packedArgs,
                                     const TQByteArray &_staticData,
                                     bool showProgressInfo)
    : KIO::TransferJob( url, command, packedArgs, _staticData, showProgressInfo ),
      m_uploadOffset( 0 )
{
    connect( this, TQT_SIGNAL( data( KIO::Job *, const TQByteArray & ) ),
             TQT_SLOT( slotData( KIO::Job *, const TQByteArray & ) ) );
    connect( this, TQT_SIGNAL( dataReq( KIO::Job *, TQByteArray & ) ),
             TQT_SLOT( slotDataReq( KIO::Job *, TQByteArray & ) ) );
}

void StoredTransferJob::setData( const TQByteArray& arr )
{
    Q_ASSERT( m_data.isNull() ); // check that we're only called once
    Q_ASSERT( m_uploadOffset == 0 ); // no upload started yet
    m_data = arr;
}

void StoredTransferJob::slotData( KIO::Job *, const TQByteArray &data )
{
  // check for end-of-data marker:
  if ( data.size() == 0 )
    return;
  unsigned int oldSize = m_data.size();
  m_data.resize( oldSize + data.size(), TQGArray::SpeedOptim );
  memcpy( m_data.data() + oldSize, data.data(), data.size() );
}

void StoredTransferJob::slotDataReq( KIO::Job *, TQByteArray &data )
{
  // Inspired from kmail's KMKernel::byteArrayToRemoteFile
  // send the data in 64 KB chunks
  const int MAX_CHUNK_SIZE = 64*1024;
  int remainingBytes = m_data.size() - m_uploadOffset;
  if( remainingBytes > MAX_CHUNK_SIZE ) {
    // send MAX_CHUNK_SIZE bytes to the receiver (deep copy)
    data.duplicate( m_data.data() + m_uploadOffset, MAX_CHUNK_SIZE );
    m_uploadOffset += MAX_CHUNK_SIZE;
    //kdDebug() << "Sending " << MAX_CHUNK_SIZE << " bytes ("
    //                << remainingBytes - MAX_CHUNK_SIZE << " bytes remain)\n";
  } else {
    // send the remaining bytes to the receiver (deep copy)
    data.duplicate( m_data.data() + m_uploadOffset, remainingBytes );
    m_data = TQByteArray();
    m_uploadOffset = 0;
    //kdDebug() << "Sending " << remainingBytes << " bytes\n";
  }
}

////

StoredTransferJob *KIOext::storedGet( const KURL& url, bool reload, bool showProgressInfo )
{
    // Send decoded path and encoded query
    KIO_ARGS << url;
    StoredTransferJob * job = new StoredTransferJob( url, KIO::CMD_GET, packedArgs, TQByteArray(), showProgressInfo );
    if (reload)
       job->addMetaData("cache", "reload");
    return job;
}

StoredTransferJob *KIOext::put( const TQByteArray& arr, const KURL& url, int permissions,
                                bool overwrite, bool resume, bool showProgressInfo )
{
    KIO_ARGS << url << Q_INT8( overwrite ? 1 : 0 ) << Q_INT8( resume ? 1 : 0 ) << permissions;
    StoredTransferJob * job = new StoredTransferJob( url, KIO::CMD_PUT, packedArgs, TQByteArray(), showProgressInfo );
    job->setData( arr );
    return job;
}

#include "storedtransferjob.moc"
