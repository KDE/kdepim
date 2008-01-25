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

#include <kio/job.h>

#ifndef JOBS_H
#define JOBS_H

namespace Scalix {

  enum DelegateTypes
  {
    SendOnBehalfOf = 1,
    SeePrivate = 2,
    GetMeetings = 4,
    InsteadOfMe = 8
  };

  class SetPasswordJob;
  class SetDelegateJob;
  class DeleteDelegateJob;
  class GetDelegatesJob;
  class AddOtherUserJob;
  class DeleteOtherUserJob;
  class GetOtherUsersJob;
  class SetOutOfOfficeJob;
  class GetOutOfOfficeJob;

  class Delegate
  {
    public:
      typedef QValueList<Delegate> List;

      Delegate();
      Delegate( const QString &email, int rights );

      bool isValid() const;

      QString email() const;
      int rights() const;

      static QString rightsAsString( int rights );

    private:
      QString mEmail;
      int mRights;
  };

  /**
   * Sets/Changes the password of the user encoded in @p url.
   */
  SetPasswordJob* setPassword( KIO::Slave* slave, const KURL& url, const QString& oldPassword, const QString& newPassword );

  /**
   * Adds a delegate represented by @p email with the given @p params for the user encoded in @p url.
   */
  SetDelegateJob* setDelegate( KIO::Slave* slave, const KURL& url, const QString& email, int params );

  /**
   * Deletes the delegate represented by @p email for the user encoded in @p url.
   */
  DeleteDelegateJob* deleteDelegate( KIO::Slave* slave, const KURL& url, const QString& email );

  /**
   * Retrieves the delegates for the user encoded in @p url.
   */
  GetDelegatesJob* getDelegates( KIO::Slave* slave, const KURL& url );

  /**
   * Adds the mailbox of another user represented by @p email to the users 'Other Users' namespace.
   */
  AddOtherUserJob* addOtherUser( KIO::Slave* slave, const KURL& url, const QString& email );

  /**
   * Deletes the mailbox of another user represented by @p email from the users 'Other Users' namespace.
   */
  DeleteOtherUserJob* deleteOtherUser( KIO::Slave* slave, const KURL& url, const QString& email );

  /**
   * Retrieves the list of all other users.
   */
  GetOtherUsersJob* getOtherUsers( KIO::Slave* slave, const KURL& url );

  /**
   * Sets the out-of-office data.
   *
   * @param enabled Whether the out-of-office functionality is enabled.
   * @param msg The out-of-office message.
   */
  SetOutOfOfficeJob* setOutOfOffice( KIO::Slave* slave, const KURL& url, bool enabled, const QString& msg );

  /**
   * Retrieves the out-of-office data.
   */
  GetOutOfOfficeJob* getOutOfOffice( KIO::Slave* slave, const KURL& url );


  class SetPasswordJob : public KIO::SimpleJob
  {
    public:
      SetPasswordJob( const KURL& url, const QByteArray &packedArgs, bool showProgressInfo );
  };

  class SetDelegateJob : public KIO::SimpleJob
  {
    public:
      SetDelegateJob( const KURL& url, const QByteArray &packedArgs, bool showProgressInfo );
  };

  class DeleteDelegateJob : public KIO::SimpleJob
  {
    public:
      DeleteDelegateJob( const KURL& url, const QByteArray &packedArgs, bool showProgressInfo );
  };

  class GetDelegatesJob : public KIO::SimpleJob
  {
    Q_OBJECT

    public:
      GetDelegatesJob( const KURL& url, const QByteArray &packedArgs, bool showProgressInfo );

      Delegate::List delegates() const;

    private slots:
      void slotInfoMessage( KIO::Job*, const QString& );

    private:
      Delegate::List mDelegates;
  };

  class AddOtherUserJob : public KIO::SimpleJob
  {
    public:
      AddOtherUserJob( const KURL& url, const QByteArray &packedArgs, bool showProgressInfo );
  };

  class DeleteOtherUserJob : public KIO::SimpleJob
  {
    public:
      DeleteOtherUserJob( const KURL& url, const QByteArray &packedArgs, bool showProgressInfo );
  };

  class GetOtherUsersJob : public KIO::SimpleJob
  {
    Q_OBJECT

    public:
      GetOtherUsersJob( const KURL& url, const QByteArray &packedArgs, bool showProgressInfo );

      QStringList otherUsers() const;

    private slots:
      void slotInfoMessage( KIO::Job*, const QString& );

    private:
      QStringList mOtherUsers;
  };

  class SetOutOfOfficeJob : public KIO::SimpleJob
  {
    public:
      SetOutOfOfficeJob( const KURL& url, const QByteArray &packedArgs, bool showProgressInfo );
  };

  class GetOutOfOfficeJob : public KIO::SimpleJob
  {
    Q_OBJECT

    public:
      GetOutOfOfficeJob( const KURL& url, const QByteArray &packedArgs, bool showProgressInfo );

      bool enabled() const;
      QString message() const;

    private slots:
      void slotInfoMessage( KIO::Job*, const QString& );

    private:
      bool mEnabled;
      QString mMessage;
  };
}

#endif
