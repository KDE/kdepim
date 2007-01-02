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

#ifndef KPIM_SENDMAILJOB_H
#define KPIM_SENDMAILJOB_H

#include <mailtransport/transportjob.h>

class KProcess;

namespace KPIM {

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
    SendmailJob( Transport* transport, QObject* parent = 0 );

    virtual void start();

  private slots:
    void sendmailExited();
    void wroteStdin();
    void receivedStdErr( KProcess *proc, char* data, int len );

  private:
    KProcess* mProcess;
    QString mLastError;
};

}

#endif
