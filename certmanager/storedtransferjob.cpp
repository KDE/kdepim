#include "storedtransferjob.h"

using namespace KIOext;
using namespace KIO;

#define KIO_ARGS QByteArray packedArgs; QDataStream stream( packedArgs, IO_WriteOnly ); stream

StoredTransferJob::StoredTransferJob(const KURL& url, int command,
                                     const QByteArray &packedArgs,
                                     const QByteArray &_staticData,
                                     bool showProgressInfo)
    : TransferJob( url, command, packedArgs, _staticData, showProgressInfo ),
      m_uploadOffset( 0 )
{
    connect( this, SIGNAL( data( KIO::Job *job, const QByteArray &data ) ),
             SLOT( slotData( KIO::Job *job, const QByteArray &data ) ) );
    connect( this, SIGNAL( dataReq( KIO::Job *job, QByteArray &data ) ),
             SLOT( slotDataReq( KIO::Job *job, QByteArray &data ) ) );
}

void StoredTransferJob::setData( const QByteArray& arr )
{
    Q_ASSERT( m_data.isNull() ); // check that we're only called once
    Q_ASSERT( m_uploadOffset == 0 ); // no upload started yet
    m_data = arr;
}

void StoredTransferJob::slotData( KIO::Job *, const QByteArray &data )
{
  // check for end-of-data marker:
  if ( data.size() == 0 )
    return;
  unsigned int oldSize = m_data.size();
  m_data.resize( oldSize + data.size(), QGArray::SpeedOptim );
  memcpy( m_data.data() + oldSize, data.data(), data.size() );
}

void StoredTransferJob::slotDataReq( KIO::Job *, QByteArray &data )
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
    m_data = QByteArray();
    m_uploadOffset = 0;
    //kdDebug() << "Sending " << remainingBytes << " bytes\n";
  }
}

////

StoredTransferJob *KIOext::storedGet( const KURL& url, bool reload, bool showProgressInfo )
{
    // Send decoded path and encoded query
    KIO_ARGS << url;
    StoredTransferJob * job = new StoredTransferJob( url, CMD_GET, packedArgs, QByteArray(), showProgressInfo );
    if (reload)
       job->addMetaData("cache", "reload");
    return job;
}

StoredTransferJob *KIOext::put( const QByteArray& arr, const KURL& url, int permissions,
                                bool overwrite, bool resume, bool showProgressInfo )
{
    KIO_ARGS << url << Q_INT8( overwrite ? 1 : 0 ) << Q_INT8( resume ? 1 : 0 ) << permissions;
    StoredTransferJob * job = new StoredTransferJob( url, CMD_PUT, packedArgs, QByteArray(), showProgressInfo );
    job->setData( arr );
    return job;
}

#include "storedtransferjob.moc"
