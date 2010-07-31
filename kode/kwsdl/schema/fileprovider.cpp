/* 
    This file is part of KDE Schema Parser

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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

#include <unistd.h>

#include <tqapplication.h>
#include <tqeventloop.h>
#include <tqfile.h>

#include <kio/job.h>
#include <ktempfile.h>

#include "fileprovider.h"

using namespace Schema;

FileProvider::FileProvider()
  : TQObject( 0 ), mBlocked( false )
{
}

bool FileProvider::get( const TQString &url, TQString &target )
{
  if ( !mFileName.isEmpty() )
    cleanUp();

  if ( target.isEmpty() ) {
    KTempFile tmpFile;
    target = tmpFile.name();
    mFileName = target;
  }

  mData.truncate( 0 );

  qDebug( "Downloading external schema '%s'", url.latin1() );

  KIO::TransferJob* job = KIO::get( KURL( url ), false, false );
  connect( job, TQT_SIGNAL( data( KIO::Job*, const TQByteArray& ) ),
           this, TQT_SLOT( slotData( KIO::Job*, const TQByteArray& ) ) );
  connect( job, TQT_SIGNAL( result( KIO::Job* ) ),
           this, TQT_SLOT( slotResult( KIO::Job* ) ) );

  mBlocked = true;
  while ( mBlocked ) {
    qApp->eventLoop()->processEvents( TQEventLoop::ExcludeUserInput );
    usleep( 500 );
  }

  return true;
}

void FileProvider::cleanUp()
{
  ::unlink( TQFile::encodeName( mFileName ) );
  mFileName = TQString();
}

void FileProvider::slotData( KIO::Job*, const TQByteArray &data )
{
  unsigned int oldSize = mData.size();
  mData.resize( oldSize + data.size() );
  memcpy( mData.data() + oldSize, data.data(), data.size() );
}

void FileProvider::slotResult( KIO::Job *job )
{
  if ( job->error() ) {
    qDebug( "%s", job->errorText().latin1() );
    return;
  }

  TQFile file( mFileName );
  if ( !file.open( IO_WriteOnly ) ) {
    qDebug( "Unable to create temporary file" );
    return;
  }

  qDebug( "Download successful" );
  file.writeBlock( mData );
  file.close();

  mData.truncate( 0 );

  mBlocked = false;
}

#include "fileprovider.moc"
