// $Id$

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
  QList<ATCommand> *cmds = mCommandSet.commandList();

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

//  kdDebug() << "CommandScheduler:sendCommand(): " << command.latin1() << endl;

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
