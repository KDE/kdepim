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

#include "kandyview.h"
#include "kandyview.moc"

#include "modem.h"
#include "cmdpropertiesdialog.h"
#include "commanditem.h"
#include "atcommand.h"
#include "mobilegui.h"
#include "commandscheduler.h"
#include "kandyprefs.h"

KandyView::KandyView(QWidget *parent)
    : KandyView_base(parent),
      DCOPObject("KandyIface"),
      mMobileGui(0)
{
  mModified = false;

  connect (mInput,SIGNAL(returnPressed()),SLOT(processLastLine()));

  initModem();

  mScheduler = new CommandScheduler(mModem,this);
  connect(mScheduler,SIGNAL(result(const QString &)),
          mResultView,SLOT(setText(const QString &)));
  connect(mScheduler,SIGNAL(commandProcessed(ATCommand *)),
          SLOT(setResult(ATCommand *)));
}

KandyView::~KandyView()
{
}

void KandyView::initModem()
{
  mModem = new Modem(this);

  kdDebug() << "Opening serial Device: "
            << KandyPrefs::instance()->mSerialDevice
            << endl;

  mModem->setDevice(KandyPrefs::instance()->mSerialDevice);
  mModem->setSpeed(19200);
  mModem->setData(8);
  mModem->setParity('N');
  mModem->setStop(1);

  if (!mModem->open()) {
    KMessageBox::sorry(this,
        i18n("Cannot open modem device %1.")
        .arg(KandyPrefs::instance()->mSerialDevice), i18n("Modem Error"));
    return;
  }

#if 0
  if (!mModem->dsrOn()) {
    KMessageBox::sorry(this, i18n("Modem is off."), i18n("Modem Error"));
    mModem->close();
    return;
  }
  if (!mModem->ctsOn()) {
    KMessageBox::sorry(this, i18n("Modem is busy."), i18n("Modem Error"));
    mModem->close();
    return;
  }
#endif

  connect(mModem,SIGNAL(gotLine(const char *)),
          SLOT(appendOutput(const char *)));

#if 0
  mModem->writeLine("");
  usleep(250000);
  mModem->flush();
  mModem->writeLine("ATZ");
#endif
}

void KandyView::print(QPainter *, int, int)
{
    // do the actual printing, here
    // p->drawText(etc..)
}

void KandyView::importPhonebook()
{
  createMobileGui();
  connect (mMobileGui,SIGNAL(phonebookRead()),mMobileGui,SLOT(writeKab()));
  mMobileGui->readPhonebook();
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
  QListViewItem *item = mCommandList->currentItem();
  if (item) {
    delete item;
    setModified();
  }
}

bool KandyView::loadFile(const QString& filename)
{
//  kdDebug() << "KandyView::loadFile(): " << filename << endl;

  QDomDocument doc("Kandy");
  QFile f(filename);
  if (!f.open(IO_ReadOnly))
    return false;
  if (!doc.setContent(&f)) {
    f.close();
    return false;
  }
  f.close();

  mCommandList->clear();

  QDomNodeList commands = doc.elementsByTagName("command");
  for(uint i=0;i<commands.count();++i) {
    QDomElement c = commands.item(i).toElement();
    if (!c.isNull()) {
      CommandItem *cmdItem = new CommandItem(mCommandList,new ATCommand);
      cmdItem->load(&c);
    }
  }

  KConfig *config = KGlobal::config();
  config->setGroup("General");
  config->writeEntry("CurrentProfile",filename);

  setModified(false);

  return true;
}

bool KandyView::saveFile(const QString& filename)
{
  QDomDocument doc("Kandy");
  QDomElement set = doc.createElement("commandset");
  doc.appendChild(set);

  CommandItem *item = (CommandItem *)(mCommandList->firstChild());
  while (item) {
    item->save(&doc,&set);
    item = (CommandItem *)(item->nextSibling());
  }
  
  QFile xmlfile(filename);
  if (!xmlfile.open(IO_WriteOnly)) return false;
  QTextStream ts(&xmlfile);
  doc.documentElement().save(ts,2);
  xmlfile.close();

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

void KandyView::createMobileGui()
{
  if (!mMobileGui) {
    mMobileGui = new MobileGui(mScheduler,this);
    connect(mMobileGui,SIGNAL(sendCommand(const QString &)),
            SLOT(executeCommandId(const QString &)));
    mMobileGui->readModelInformation();
    mMobileGui->refreshStatus();
  }
}

void KandyView::showMobileGui()
{
  createMobileGui();
  mMobileGui->show();
  mMobileGui->raise();
}

// This function really belongs to the CommandScheduler. But we need a way to
// handle the list of commands independently of the GUI before we can move it.
void KandyView::executeCommandId(const QString &id)
{
  CommandItem *item = (CommandItem *)(mCommandList->firstChild());
  while (item) {
    if (item->command()->id() == id) {
      mScheduler->execute(item->command());
      return;
    }
    item = (CommandItem *)(item->nextSibling());
  }
  kdDebug() << "executeCommandId: Id '" << id << "' not found" << endl;
}
