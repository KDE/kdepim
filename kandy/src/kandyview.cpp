// $Id$

#include <unistd.h>

#include <qpainter.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qmultilineedit.h>
#include <qlistview.h>
#include <qdom.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qlineedit.h>
#include <qcheckbox.h>

#include <kurl.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <klineeditdlg.h>

#include "modem.h"
#include "cmdpropertiesdialog.h"
#include "commanditem.h"
#include "atcommand.h"
#include "commandscheduler.h"
#include "kandyprefs.h"

#include "kandyview.h"
#include "kandyview.moc"

KandyView::KandyView(CommandScheduler *scheduler,QWidget *parent)
    : KandyView_base(parent)
{
  mModified = false;
  mScheduler = scheduler;

  connect (mInput,SIGNAL(returnPressed()),SLOT(processLastLine()));

  connect(mScheduler->modem(),SIGNAL(gotLine(const char *)),
          SLOT(appendOutput(const char *)));

  connect(mScheduler,SIGNAL(result(const QString &)),
          mResultView,SLOT(setText(const QString &)));
  connect(mScheduler,SIGNAL(commandProcessed(ATCommand *)),
          SLOT(setResult(ATCommand *)));
}

KandyView::~KandyView()
{
}


void KandyView::print(QPainter *, int, int)
{
    // do the actual printing, here
    // p->drawText(etc..)
}

void KandyView::importPhonebook()
{
#if 0
  createMobileGui();
  connect (mMobileGui,SIGNAL(phonebookRead()),mMobileGui,SLOT(writeKab()));
  mMobileGui->readPhonebook();
#endif
}

void KandyView::slotSetTitle(const QString& title)
{
    emit signalChangeCaption(title);
}

void KandyView::processLastLine()
{
  int line = 0;
  int row = 0;
  mInput->getCursorPosition(&line,&row);
  
  if (line > 0) {
    mLastInput = mInput->textLine(line-1);
    mScheduler->execute(mLastInput);
  }
}

void KandyView::appendOutput(const char *line)
{
//  kdDebug() << "OUT: " << line << endl;
  mOutput->append(line);
  mOutput->setCursorPosition(mOutput->numLines()-1,0);
}

void KandyView::setResult(ATCommand *command)
{
  if (command == 0) {
    kdDebug() << "KandyView::setResult(): Error! No command." << endl;
    mResultView->setText(i18n("Error"));
    return;
  }
  
//  kdDebug() << "KandyView::setResult(): " << endl << mResult << endl
//            << mLastCommand->processOutput(mResult) << endl;
  
  mResultView->setText(command->cmdName() + ":\n" + command->processOutput());
}

void KandyView::addCommand()
{
  ATCommand *cmd = new ATCommand(mLastInput);

  CmdPropertiesDialog *dlg = new CmdPropertiesDialog(cmd,this,"cmdprop",true);

  int result = dlg->exec();

  if (result == QDialog::Accepted) {
    new CommandItem(mCommandList,cmd);
    mScheduler->commandSet()->addCommand(cmd);
    setModified();
  } else {
    delete cmd;
  }
}

void KandyView::editCommand()
{
  QListViewItem *item = mCommandList->currentItem();
  if (item) {
    CommandItem *cmdItem = (CommandItem *)item;
    ATCommand *cmd = cmdItem->command();

    CmdPropertiesDialog *dlg = new CmdPropertiesDialog(cmd,this,"cmdprop",true);

    int result = dlg->exec();

    if (result == QDialog::Accepted) {
      cmdItem->setItemText();
      setModified();
    }
  }
}

void KandyView::executeCommand()
{
  CommandItem *item = (CommandItem *)(mCommandList->currentItem());
  if (item) {
    ATCommand *cmd = item->command();
    QList<ATParameter> paraList = cmd->parameters();
    for(uint i=0;i<paraList.count();++i) {
      ATParameter *p = paraList.at(i);
      if (p->userInput()) {
        bool ok = false;
        QString value = KLineEditDlg::getText(i18n("Enter Value for %1").arg(p->name()),
                                              "",&ok,this);
        if (ok) {
          p->setValue(value);
        } else {
          return;
        }
      }
    }
    kdDebug() << "KandyView::executeCommand(): " << cmd->cmd() << endl;
    mScheduler->execute(cmd);
  }
}

void KandyView::deleteCommand()
{
  CommandItem *item = dynamic_cast<CommandItem *>(mCommandList->currentItem());
  if (item) {
    mScheduler->commandSet()->deleteCommand(item->command());
    delete item;
    setModified();
  }
}

bool KandyView::loadFile(const QString& filename)
{
  mCommandList->clear();

  if (!mScheduler->loadProfile(filename)) return false;

  QList<ATCommand> *cmds = mScheduler->commandSet()->commandList();

  for(uint i=0;i<cmds->count();++i) {
    new CommandItem(mCommandList,cmds->at(i));
  }

  KConfig *config = KGlobal::config();
  config->setGroup("General");
  config->writeEntry("CurrentProfile",filename);

  setModified(false);

  return true;
}

bool KandyView::saveFile(const QString& filename)
{
  if (!mScheduler->saveProfile(filename)) return false;

  KConfig *config = KGlobal::config();
  config->setGroup("General");
  config->writeEntry("CurrentProfile",filename);

  setModified(false);

  return true;
}

void KandyView::setModified(bool modified)
{
  if (modified != mModified) {
    mModified = modified;
    emit modifiedChanged(mModified);
  }
}

bool KandyView::isModified()
{
  return mModified;
}
