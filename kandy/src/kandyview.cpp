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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <unistd.h>

#include <tqpainter.h>
#include <tqlayout.h>
#include <tqhbox.h>
#include <tqvbox.h>
#include <tqtextedit.h>
#include <tqlistview.h>
#include <tqdom.h>
#include <tqtextstream.h>
#include <tqfile.h>
#include <tqlineedit.h>
#include <tqcheckbox.h>
#include <tqlabel.h>
#include <tqpushbutton.h>

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

KandyView::KandyView(CommandScheduler *scheduler,TQWidget *parent)
    : TQWidget(parent)
{
  mModified = false;
  mScheduler = scheduler;

  TQBoxLayout *topLayout = new TQVBoxLayout( this );

  TQSplitter *mainSplitter = new TQSplitter( Horizontal, this );
  topLayout->addWidget( mainSplitter );

  TQWidget *commandBox = new TQWidget( mainSplitter );

  TQBoxLayout *commandLayout = new TQVBoxLayout( commandBox );
  commandLayout->setMargin( KDialog::marginHint() );
  commandLayout->setSpacing( KDialog::spacingHint() );

  mCommandList = new TQListView( commandBox );
  mCommandList->addColumn( i18n( "Name" ) );
  mCommandList->addColumn( i18n( "Command" ) );
  mCommandList->addColumn( i18n( "Hex" ) );
  commandLayout->addWidget( mCommandList );

  connect( mCommandList, TQT_SIGNAL( doubleClicked(TQListViewItem*) ),
           TQT_SLOT( executeCommand() ) );

  TQPushButton *buttonAdd = new TQPushButton( i18n("Add..."), commandBox );
  commandLayout->addWidget( buttonAdd );
  connect( buttonAdd, TQT_SIGNAL( clicked() ), TQT_SLOT( addCommand() ) );

  TQPushButton *buttonEdit = new TQPushButton( i18n("Edit..."), commandBox );
  commandLayout->addWidget( buttonEdit );
  connect( buttonEdit, TQT_SIGNAL( clicked() ), TQT_SLOT( editCommand() ) );

  TQPushButton *buttonDelete = new TQPushButton( i18n("Delete"), commandBox );
  commandLayout->addWidget( buttonDelete );
  connect( buttonDelete, TQT_SIGNAL( clicked() ), TQT_SLOT( deleteCommand() ) );

  TQPushButton *buttonExecute = new TQPushButton( i18n("Execute"), commandBox );
  commandLayout->addWidget( buttonExecute );
  connect( buttonExecute, TQT_SIGNAL( clicked() ), TQT_SLOT( executeCommand() ) );

  TQSplitter *ioSplitter = new TQSplitter( Vertical, mainSplitter );

  TQWidget *inBox = new TQWidget( ioSplitter );
  
  TQBoxLayout *inLayout = new TQVBoxLayout( inBox );

  TQLabel *inLabel = new TQLabel( i18n("Input:"), inBox );
  inLabel->setMargin( 2 );
  inLayout->addWidget( inLabel );

  mInput = new TQTextEdit( inBox );
  inLayout->addWidget( mInput );
  
  TQWidget *outBox = new TQWidget( ioSplitter );

  TQBoxLayout *outLayout = new TQVBoxLayout( outBox );

  TQLabel *outLabel = new TQLabel( i18n( "Output:"), outBox );
  outLabel->setMargin( 2 );
  outLayout->addWidget( outLabel );

  mOutput = new TQTextEdit( outBox );
  mOutput->setReadOnly( true );
  outLayout->addWidget( mOutput );

  TQVBox *resultBox = new TQVBox( mainSplitter );

  TQLabel *resultLabel = new TQLabel( i18n("Result:"), resultBox );
  resultLabel->setMargin( 2 );
    
  mResultView = new TQTextEdit( resultBox );
  mResultView->setReadOnly( true );
  
  connect (mInput,TQT_SIGNAL(returnPressed()),TQT_SLOT(processLastLine()));

  connect(mScheduler->modem(),TQT_SIGNAL(gotLine(const char *)),
          TQT_SLOT(appendOutput(const char *)));

  connect(mScheduler,TQT_SIGNAL(result(const TQString &)),
          mResultView,TQT_SLOT(setText(const TQString &)));
  connect(mScheduler,TQT_SIGNAL(commandProcessed(ATCommand *)),
          TQT_SLOT(setResult(ATCommand *)));
}

KandyView::~KandyView()
{
}


void KandyView::print(TQPainter *, int, int)
{
    // do the actual printing, here
    // p->drawText(etc..)
}

void KandyView::importPhonebook()
{
#if 0
  createMobileGui();
  connect (mMobileGui,TQT_SIGNAL(phonebookRead()),mMobileGui,TQT_SLOT(writeKab()));
  mMobileGui->readPhonebook();
#endif
}

void KandyView::slotSetTitle(const TQString& title)
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

    kdDebug(5960) << "processLastLine(): " << mLastInput << endl;

    mScheduler->execute(mLastInput);
  }
}

void KandyView::appendOutput(const char *line)
{
//  kdDebug(5960) << "OUT: " << line << endl;
  mOutput->append(line);
  mOutput->setCursorPosition(mOutput->paragraphs()-1,0);
}

void KandyView::setResult(ATCommand *command)
{
  if (command == 0) {
    kdDebug(5960) << "KandyView::setResult(): Error! No command." << endl;
    mResultView->setText(i18n("Error"));
    return;
  }
  
//  kdDebug(5960) << "KandyView::setResult(): " << endl << mResult << endl
//            << mLastCommand->processOutput(mResult) << endl;
  
  mResultView->setText(command->cmdName() + ":\n" + command->processOutput());
}

void KandyView::addCommand()
{
  ATCommand *cmd = new ATCommand(mLastInput);

  CmdPropertiesDialog *dlg = new CmdPropertiesDialog(cmd,this,"cmdprop",true);

  int result = dlg->exec();

  if (result == TQDialog::Accepted) {
    new CommandItem(mCommandList,cmd);
    mScheduler->commandSet()->addCommand(cmd);
    setModified();
  } else {
    delete cmd;
  }
}

void KandyView::editCommand()
{
  TQListViewItem *item = mCommandList->currentItem();
  if (item) {
    CommandItem *cmdItem = (CommandItem *)item;
    ATCommand *cmd = cmdItem->command();

    CmdPropertiesDialog *dlg = new CmdPropertiesDialog(cmd,this,"cmdprop",true);

    int result = dlg->exec();

    if (result == TQDialog::Accepted) {
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
    TQPtrList<ATParameter> paraList = cmd->parameters();
    for(uint i=0;i<paraList.count();++i) {
      ATParameter *p = paraList.at(i);
      if (p->userInput()) {
        bool ok = false;
        TQString value = KInputDialog::getText(TQString::null,
            i18n("Enter value for %1:").arg(p->name()),TQString::null,&ok,this);
        if (!ok)
          return;
        p->setValue(value);
      }
    }
    kdDebug(5960) << "KandyView::executeCommand(): " << cmd->cmd() << endl;
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

bool KandyView::loadFile(const TQString& filename)
{
  mCommandList->clear();

  if (!mScheduler->loadProfile(filename)) return false;

  TQPtrList<ATCommand> *cmds = mScheduler->commandSet()->commandList();

  for(uint i=0;i<cmds->count();++i) {
    new CommandItem(mCommandList,cmds->at(i));
  }

  KConfig *config = KGlobal::config();
  config->setGroup("General");
  config->writeEntry("CurrentProfile",filename);

  setModified(false);

  return true;
}

bool KandyView::saveFile(const TQString& filename)
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
