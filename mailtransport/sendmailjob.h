/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

    Based on KMail code by:
    Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef MailTransport_SENDMAILJOB_H
#define MailTransport_SENDMAILJOB_H

#include <mailtransport/transportjob.h>

class K3Process;

namespace MailTransport {

/**
  Mail transport job for sendmail.
*/
class MAILTRANSPORT_EXPORT SendmailJob : public TransportJob
{
  Q_OBJECT
  public:
    /**
      Creates a SendmailJob.
      @param transport The transport settings.
      @param parent The parent object.
    */
    explicit SendmailJob( Transport* transport, QObject* parent = 0 );

    /**
      Destroys this job.
    */
    virtual ~SendmailJob();

  protected:
    virtual void doStart();
    virtual bool doKill();

  private slots:
    void sendmailExited();
    void wroteStdin();
    void receivedStdErr( K3Process *proc, char* data, int len );

  private:
    K3Process* mProcess;
    QString mLastError;
};

}

#endif
