/*
    This file is part of Kandy.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kdebug.h>
#include <klocale.h>

#include "modem.h"

#include "commandscheduler.h"
#include "commandscheduler.moc"

CommandScheduler::CommandScheduler(Modem *modem,QObject *parent,
                                   const char *name) :
  QObject(parent,name),
  mModem(modem)
{
  connect(mModem,SIGNAL(gotLine(const char *)),
          SLOT(processOutput(const char *)));
}

void CommandScheduler::execute(ATCommand *command)
{
  if (!mModem->isOpen()) {
    kdDebug() << "Warning! Modem not open." << endl;
    return;
  }

  mCommandQueue.append(command);

//  if (mCommandQueue.count() == 1) sendCommand(command->cmdString());
  if (mCommandQueue.count() == 1) sendCommand(command->cmd());
}

void CommandScheduler::execute(const QString &command)
{
  ATCommand *cmd = new ATCommand("",command);
  cmd->setAutoDelete(true);

  execute(cmd);
}

void CommandScheduler::executeId(const QString &id)
{
  QPtrList<ATCommand> *cmds = mCommandSet.commandList();

  for(uint i=0;i<cmds->count();++i) {
    if (cmds->at(i)->id() == id) {
      execute(cmds->at(i));
      return;
    }
  }
  kdDebug() << "CommandScheduler::executeId(): Id '" << id << "' not found" << endl;
}

void CommandScheduler::sendCommand(const QString &command)
{
  if (command.isEmpty()) {
    kdDebug() << "CommandScheduler::sendCommand(): Warning! Empty command."
              << endl;
    return;
  }

  kdDebug() << "CommandScheduler:sendCommand(): " << command << endl;

  mModem->writeLine(command.latin1());
}


void CommandScheduler::processOutput(const char *line)
{
  QString l = line;
  ATCommand *cmd = mCommandQueue.first();
  if (l == "OK") {
    mState = WAITING;
    emit result(mResult);
    cmd->setResultString(mResult);
    emit commandProcessed(cmd);
    nextCommand();
  } else if (l == "ERROR") {
    mState = WAITING;
    emit result(i18n("Error"));
    nextCommand();
  } else {
    if (mState == WAITING) {
      mState = PROCESSING;
      mResult = "";
    } else if (mState == PROCESSING) {
      if (!l.isEmpty()) {
        mResult += l;
        mResult += "\n";
      }
    }
  }
}

void CommandScheduler::nextCommand()
{
  if (mCommandQueue.first()->autoDelete()) delete mCommandQueue.first();
  mCommandQueue.removeFirst();
  if (mCommandQueue.count() > 0) {
    sendCommand(mCommandQueue.first()->cmd());
  }
}

bool CommandScheduler::loadProfile(const QString& filename)
{
  mCommandSet.clear();

  if (!mCommandSet.loadFile(filename)) return false;

  return true;
}

bool CommandScheduler::saveProfile(const QString& filename)
{
  if (!mCommandSet.saveFile(filename)) return false;

  return true;
}
