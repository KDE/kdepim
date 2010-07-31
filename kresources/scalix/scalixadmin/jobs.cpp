/*
 *   This file is part of ScalixAdmin.
 *
 *   Copyright (C) 2007 Trolltech ASA. All rights reserved.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <kio/scheduler.h>
#include <klocale.h>

#include "jobs.h"

using namespace Scalix;

Delegate::Delegate()
  : mRights( -1 )
{
}

Delegate::Delegate( const TQString &email, int rights )
  : mEmail( email ), mRights( rights )
{
}

bool Delegate::isValid() const
{
  return ( !mEmail.isEmpty() && mRights != -1 );
}

TQString Delegate::email() const
{
  return mEmail;
}

int Delegate::rights() const
{
  return mRights;
}

TQString Delegate::rightsAsString( int rights )
{
  TQStringList rightNames;

  if ( rights & SendOnBehalfOf )
    rightNames.append( i18n( "Send on behalf of" ) );
  if ( rights & SeePrivate )
    rightNames.append( i18n( "See private" ) );
  if ( rights & GetMeetings )
    rightNames.append( i18n( "Get meetings" ) );
  if ( rights & InsteadOfMe )
    rightNames.append( i18n( "Instead of me" ) );

  return rightNames.join( ", " );
}


SetPasswordJob* Scalix::setPassword( KIO::Slave* slave, const KURL& url,
                                     const TQString &oldPassword, const TQString& newPassword )
{
  TQByteArray packedArgs;
  TQDataStream stream( packedArgs, IO_WriteOnly );
  stream << (int)'X' << (int)'N'
         << TQString( "X-SCALIX-PASSWORD" ) << TQString( "%1 %2" ).arg( oldPassword ).arg( newPassword );

  SetPasswordJob* job = new SetPasswordJob( url, packedArgs, false );
  KIO::Scheduler::assignJobToSlave( slave, job );
  return job;
}

SetDelegateJob* Scalix::setDelegate( KIO::Slave* slave, const KURL& url, const TQString& email, int params )
{
  TQStringList types;
  if ( params & SendOnBehalfOf )
    types.append( "SOBO" );
  if ( params & SeePrivate )
    types.append( "SEEPRIVATE" );
  if ( params & GetMeetings )
    types.append( "GETMEETINGS" );
  if ( params & InsteadOfMe )
    types.append( "INSTEADOFME" );

  TQByteArray packedArgs;
  TQDataStream stream( packedArgs, IO_WriteOnly );
  stream << (int)'X' << (int)'N'
         << TQString( "X-SET-DELEGATE" ) << TQString( "%1 %2" ).arg( email ).arg( types.join( " " ) );

  SetDelegateJob* job = new SetDelegateJob( url, packedArgs, false );
  KIO::Scheduler::assignJobToSlave( slave, job );
  return job;
}

DeleteDelegateJob* Scalix::deleteDelegate( KIO::Slave* slave, const KURL& url, const TQString& email )
{
  TQByteArray packedArgs;
  TQDataStream stream( packedArgs, IO_WriteOnly );
  stream << (int)'X' << (int)'N'
         << TQString( "X-DELETE-DELEGATE" ) << email;

  DeleteDelegateJob* job = new DeleteDelegateJob( url, packedArgs, false );
  KIO::Scheduler::assignJobToSlave( slave, job );
  return job;
}

GetDelegatesJob* Scalix::getDelegates( KIO::Slave* slave, const KURL& url )
{
  TQByteArray packedArgs;
  TQDataStream stream( packedArgs, IO_WriteOnly );
  stream << (int)'X' << (int)'N' << TQString( "X-GET-DELEGATES" ) << TQString();

  GetDelegatesJob* job = new GetDelegatesJob( url, packedArgs, false );
  KIO::Scheduler::assignJobToSlave( slave, job );
  return job;
}

AddOtherUserJob* Scalix::addOtherUser( KIO::Slave* slave, const KURL& url, const TQString& email )
{
  TQByteArray packedArgs;
  TQDataStream stream( packedArgs, IO_WriteOnly );
  stream << (int)'X' << (int)'N'
         << TQString( "X-ADD-OTHER-USER" ) << email;

  AddOtherUserJob* job = new AddOtherUserJob( url, packedArgs, false );
  KIO::Scheduler::assignJobToSlave( slave, job );
  return job;
}

DeleteOtherUserJob* Scalix::deleteOtherUser( KIO::Slave* slave, const KURL& url, const TQString& email )
{
  TQByteArray packedArgs;
  TQDataStream stream( packedArgs, IO_WriteOnly );
  stream << (int)'X' << (int)'N'
         << TQString( "X-DELETE-OTHER-USER" ) << email;

  DeleteOtherUserJob* job = new DeleteOtherUserJob( url, packedArgs, false );
  KIO::Scheduler::assignJobToSlave( slave, job );
  return job;
}

GetOtherUsersJob* Scalix::getOtherUsers( KIO::Slave* slave, const KURL& url )
{
  TQByteArray packedArgs;
  TQDataStream stream( packedArgs, IO_WriteOnly );
  stream << (int)'X' << (int)'N'
         << TQString( "X-GET-OTHER-USERS" ) << TQString();

  GetOtherUsersJob* job = new GetOtherUsersJob( url, packedArgs, false );
  KIO::Scheduler::assignJobToSlave( slave, job );
  return job;
}

SetOutOfOfficeJob* Scalix::setOutOfOffice( KIO::Slave* slave, const KURL& url, bool enabled, const TQString& msg )
{
  const TQString argument = msg;
  const TQString command = TQString( "X-SET-OUT-OF-OFFICE %1 %2 {%3}" ).arg( enabled ? "ENABLED" : "DISABLED" )
                                                .arg( "UTF-8" )
                                                .arg( msg.utf8().length() );

  TQByteArray packedArgs;
  TQDataStream stream( packedArgs, IO_WriteOnly );
  stream << (int) 'X' << (int)'E' << command << argument;

  SetOutOfOfficeJob* job = new SetOutOfOfficeJob( url, packedArgs, false );
  KIO::Scheduler::assignJobToSlave( slave, job );
  return job;
}

GetOutOfOfficeJob* Scalix::getOutOfOffice( KIO::Slave* slave, const KURL& url )
{
  TQByteArray packedArgs;
  TQDataStream stream( packedArgs, IO_WriteOnly );
  stream << (int)'X' << (int)'N'
         << TQString( "X-GET-OUT-OF-OFFICE" ) << TQString();

  GetOutOfOfficeJob* job = new GetOutOfOfficeJob( url, packedArgs, false );
  KIO::Scheduler::assignJobToSlave( slave, job );
  return job;
}

SetPasswordJob::SetPasswordJob( const KURL& url, const TQByteArray &packedArgs, bool showProgressInfo )
  : KIO::SimpleJob( url, KIO::CMD_SPECIAL, packedArgs, showProgressInfo )
{
}

SetDelegateJob::SetDelegateJob( const KURL& url, const TQByteArray &packedArgs, bool showProgressInfo )
  : KIO::SimpleJob( url, KIO::CMD_SPECIAL, packedArgs, showProgressInfo )
{
}

DeleteDelegateJob::DeleteDelegateJob( const KURL& url, const TQByteArray &packedArgs, bool showProgressInfo )
  : KIO::SimpleJob( url, KIO::CMD_SPECIAL, packedArgs, showProgressInfo )
{
}

GetDelegatesJob::GetDelegatesJob( const KURL& url, const TQByteArray &packedArgs, bool showProgressInfo )
  : KIO::SimpleJob( url, KIO::CMD_SPECIAL, packedArgs, showProgressInfo )
{
  connect( this, TQT_SIGNAL( infoMessage( KIO::Job*, const TQString& ) ),
           this, TQT_SLOT( slotInfoMessage( KIO::Job*, const TQString& ) ) );
}

Delegate::List GetDelegatesJob::delegates() const
{
  return mDelegates;
}

void GetDelegatesJob::slotInfoMessage( KIO::Job*, const TQString &data )
{
  /**
   * The passed data have the following form:
   *
   * "user1@host.com:right1,right2,right4 user2@host.com:right3,right5"
   */
  TQStringList delegates = TQStringList::split( ' ', data );
  for ( uint i = 0; i < delegates.count(); ++i ) {
    TQStringList delegate = TQStringList::split( ':', delegates[ i ] );

    const TQString email = delegate[ 0 ];
    int rights = 0;

    TQStringList rightsList = TQStringList::split( ',', delegate[ 1 ] );
    for ( uint j = 0; j < rightsList.count(); ++j ) {
      if ( rightsList[ j ] == "SOBO" )
        rights |= SendOnBehalfOf;
      else if ( rightsList[ j ] == "SEEPRIVATE" )
        rights |= SeePrivate;
      else if ( rightsList[ j ] == "GETMEETINGS" )
        rights |= GetMeetings;
      else if ( rightsList[ j ] == "INSTEADOFME" )
        rights |= InsteadOfMe;
    }

    mDelegates.append( Delegate( email, rights ) );
  }
}

AddOtherUserJob::AddOtherUserJob( const KURL& url, const TQByteArray &packedArgs, bool showProgressInfo )
  : KIO::SimpleJob( url, KIO::CMD_SPECIAL, packedArgs, showProgressInfo )
{
}

DeleteOtherUserJob::DeleteOtherUserJob( const KURL& url, const TQByteArray &packedArgs, bool showProgressInfo )
  : KIO::SimpleJob( url, KIO::CMD_SPECIAL, packedArgs, showProgressInfo )
{
}

GetOtherUsersJob::GetOtherUsersJob( const KURL& url, const TQByteArray &packedArgs, bool showProgressInfo )
  : KIO::SimpleJob( url, KIO::CMD_SPECIAL, packedArgs, showProgressInfo )
{
  connect( this, TQT_SIGNAL( infoMessage( KIO::Job*, const TQString& ) ),
           this, TQT_SLOT( slotInfoMessage( KIO::Job*, const TQString& ) ) );
}

TQStringList GetOtherUsersJob::otherUsers() const
{
  return mOtherUsers;
}

void GetOtherUsersJob::slotInfoMessage( KIO::Job*, const TQString &data )
{
  mOtherUsers = TQStringList::split( ' ', data );
}

SetOutOfOfficeJob::SetOutOfOfficeJob( const KURL& url, const TQByteArray &packedArgs, bool showProgressInfo )
  : KIO::SimpleJob( url, KIO::CMD_SPECIAL, packedArgs, showProgressInfo )
{
}

GetOutOfOfficeJob::GetOutOfOfficeJob( const KURL& url, const TQByteArray &packedArgs, bool showProgressInfo )
  : KIO::SimpleJob( url, KIO::CMD_SPECIAL, packedArgs, showProgressInfo )
{
  connect( this, TQT_SIGNAL( infoMessage( KIO::Job*, const TQString& ) ),
           this, TQT_SLOT( slotInfoMessage( KIO::Job*, const TQString& ) ) );
}

bool GetOutOfOfficeJob::enabled() const
{
  return mEnabled;
}

TQString GetOutOfOfficeJob::message() const
{
  return mMessage;
}

void GetOutOfOfficeJob::slotInfoMessage( KIO::Job*, const TQString &data )
{
  const TQStringList fields = TQStringList::split( '^', data );

  mEnabled = ( fields[ 0 ] == "ENABLED" );
  mMessage = fields[ 1 ];
}

#include "jobs.moc"
