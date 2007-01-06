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

#ifndef KPIM_PRECOMMANDJOB_H
#define KPIM_PRECOMMANDJOB_H

#include <kjob.h>

class KProcess;

namespace KPIM {

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
    PrecommandJob( const QString &precommand, QObject *parent = 0 );

    /**
      Destroys this job.
    */
    virtual ~PrecommandJob();

    virtual void start();

  protected:
    virtual bool doKill();

  private slots:
    void processExited(KProcess *process);

  private:
    KProcess *mProcess;
    QString mPrecommand;
};

}

#endif
