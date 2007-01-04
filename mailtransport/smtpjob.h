/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef KPIM_SMTPJOB_H
#define KPIM_SMTPJOB_H

#include <mailtransport/transportjob.h>

namespace KIO {
class Job;
class Slave;
}

namespace KPIM {

/**
  Mail transport job for SMTP.
  Internally, all jobs for a specific transport are queued to use the same
  KIO::Slave. This avoids multiple simultaneous connections to the server,
  which is not always allowed. Also, re-using an already existing connection
  avoids the login overhead and can improve performance.
*/
class MAILTRANSPORT_EXPORT SmtpJob : public TransportJob
{
  Q_OBJECT
  public:
    /**
      Creates a SendmailJob.
      @param transport The transport settings.
      @param parent The parent object.
    */
    SmtpJob( Transport* transport, QObject* parent = 0 );

    /**
      Deletes this job.
    */
    virtual ~SmtpJob();

  protected:
    virtual void doStart();
    virtual bool doKill();

  protected slots:
    virtual void slotResult( KJob *job );
    void slaveError(KIO::Slave *slave, int errorCode, const QString &errorMsg);

  private slots:
    void dataRequest( KIO::Job* job, QByteArray &data );

  private:
    KIO::Slave* mSlave;
};

}

#endif
