// $Id$

#include "commandscheduler.h"

#include "modem.h"

#include <kdebug.h>
#include <klocale.h>

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

void CommandScheduler::sendCommand(const QString &command)
{
  if (command.isEmpty()) {
    kdDebug() << "CommandScheduler::sendCommand(): Warning! Empty command."
              << endl;
    return;
  }

  mModem->writeLine(command.latin1());
}


void CommandScheduler::processOutput(const char *line)
{
  QString l = line;
  ATCommand *cmd = mCommandQueue.first();
  if (l == "OK") {
    mState = WAITING;
    if (!cmd->autoDelete()) {
      cmd->setResultString(mResult);
      emit commandProcessed(cmd);
      nextCommand();
    } else {
      emit result(mResult);
      nextCommand();
    }
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
    sendCommand(mCommandQueue.first()->cmdString());
  }
}
#include "commandscheduler.moc"
