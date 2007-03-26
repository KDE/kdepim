/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

    Based on KMail code by:
    Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
    Copyright (c) 2000-2002 Michael Haeckel <haeckel@kde.org>

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

#ifndef MAILTRANSPORT_PRECOMMANDJOB_H
#define MAILTRANSPORT_PRECOMMANDJOB_H

#include <kjob.h>

class K3Process;

namespace MailTransport {

/**
  Job to execute commands before connecting to an account.
*/
class PrecommandJob : public KJob
{
  Q_OBJECT

  public:
    /**
      Creates a new precommand job.
      @param precommand The command to run.
      @param parent The parent object.
    */
    explicit PrecommandJob( const QString &precommand, QObject *parent = 0 );

    /**
      Destroys this job.
    */
    virtual ~PrecommandJob();

    virtual void start();

  protected:
    virtual bool doKill();

  private slots:
    void processExited(K3Process *process);

  private:
    K3Process *mProcess;
    QString mPrecommand;
};

}

#endif
