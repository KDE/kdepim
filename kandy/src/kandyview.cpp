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

#include <unistd.h>

#include <qpainter.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qtextedit.h>
#include <qlistview.h>
#include <qdom.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include <kurl.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kinputdialog.h>
#include <kdialog.h>

#include "modem.h"
#include "cmdpropertiesdialog.h"
#include "commanditem.h"
#include "atcommand.h"
#include "commandscheduler.h"
#include "kandyprefs.h"

#include "kandyview.h"
#include "kandyview.moc"

KandyView::KandyView(CommandScheduler *scheduler,QWidget *parent)
    : QWidget(parent)
{
  mModified = false;
  mScheduler = scheduler;

  QBoxLayout *topLayout = new QVBoxLayout( this );

  QSplitter *mainSplitter = new QSplitter( Horizontal, this );
  topLayout->addWidget( mainSplitter );

  QWidget *commandBox = new QWidget( mainSplitter );

  QBoxLayout *commandLayout = new QVBoxLayout( commandBox );
  commandLayout->setMargin( KDialog::marginHint() );
  commandLayout->setSpacing( KDialog::spacingHint() );

  mCommandList = new QListView( commandBox );
  mCommandList->addColumn( i18n( "Name" ) );
  mCommandList->addColumn( i18n( "Command" ) );
  mCommandList->addColumn( i18n( "Hex" ) );
  commandLayout->addWidget( mCommandList );

  connect( mCommandList, SIGNAL( doubleClicked(QListViewItem*) ),
           SLOT( executeCommand() ) );

  QPushButton *buttonAdd = new QPushButton( i18n("Add..."), commandBox );
  commandLayout->addWidget( buttonAdd );
  connect( buttonAdd, SIGNAL( clicked() ), SLOT( addCommand() ) );

  QPushButton *buttonEdit = new QPushButton( i18n("Edit..."), commandBox );
  commandLayout->addWidget( buttonEdit );
  connect( buttonEdit, SIGNAL( clicked() ), SLOT( editCommand() ) );

  QPushButton *buttonDelete = new QPushButton( i18n("Delete"), commandBox );
  commandLayout->addWidget( buttonDelete );
  connect( buttonDelete, SIGNAL( clicked() ), SLOT( deleteCommand() ) );

  QPushButton *buttonExecute = new QPushButton( i18n("Execute"), commandBox );
  commandLayout->addWidget( buttonExecute );
  connect( buttonExecute, SIGNAL( clicked() ), SLOT( executeCommand() ) );

  QSplitter *ioSplitter = new QSplitter( Vertical, mainSplitter );

  QWidget *inBox = new QWidget( ioSplitter );
  
  QBoxLayout *inLayout = new QVBoxLayout( inBox );

  QLabel *inLabel = new QLabel( i18n("Input:"), inBox );
  inLabel->setMargin( 2 );
  inLayout->addWidget( inLabel );

  mInput = new QTextEdit( inBox );
  inLayout->addWidget( mInput );
  
  QWidget *outBox = new QWidget( ioSplitter );

  QBoxLayout *outLayout = new QVBoxLayout( outBox );

  QLabel *outLabel = new QLabel( i18n( "Output:"), outBox );
  outLabel->setMargin( 2 );
  outLayout->addWidget( outLabel );

  mOutput = new QTextEdit( outBox );
  mOutput->setReadOnly( true );
  outLayout->addWidget( mOutput );

  QVBox *resultBox = new QVBox( mainSplitter );

  QLabel *resultLabel = new QLabel( i18n("Result:"), resultBox );
  resultLabel->setMargin( 2 );
    
  mResultView = new QTextEdit( resultBox );
  mResultView->setReadOnly( true );
  
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
  int para = 0;
  int row = 0;
  mInput->getCursorPosition( &para, &row );
  
  if ( para > 0 ) {
    mLastInput = mInput->text( para - 1 );

    kdDebug() << "processLastLine(): " << mLastInput << endl;

    mScheduler->execute(mLastInput);
  }
}

void KandyView::appendOutput(const char *line)
{
//  kdDebug() << "OUT: " << line << endl;
  mOutput->append(line);
  mOutput->setCursorPosition(mOutput->paragraphs()-1,0);
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
    QPtrList<ATParameter> paraList = cmd->parameters();
    for(uint i=0;i<paraList.count();++i) {
      ATParameter *p = paraList.at(i);
      if (p->userInput()) {
        bool ok = false;
        QString value = KInputDialog::getText(QString::null,
            i18n("Enter value for %1:").arg(p->name()),QString::null,&ok,this);
        if (!ok)
          return;
        p->setValue(value);
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

  QPtrList<ATCommand> *cmds = mScheduler->commandSet()->commandList();

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
