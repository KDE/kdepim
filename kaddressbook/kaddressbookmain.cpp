/*
 * kaddressbookmain.cpp
 *
 * Copyright (C) 1999 Don Sanders <dsanders@kde.org>
 */

#include <qclipboard.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kglobal.h>
#include <kmenubar.h>
#include <kconfig.h>
#include <kaccel.h>
#include <kdebug.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kstatusbar.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kprotocolinfo.h>
#include <kedittoolbar.h>
#include <kkeydialog.h>

#include "kaddressbook.h"
#include "actionmanager.h"
#include "incsearchwidget.h"
#include "kaddressbookmain.h"
#include "kaddressbookmain.moc"

KAddressBookMain::KAddressBookMain()
  : KMainWindow(0), DCOPObject("KAddressBookIface")
{
    setCaption( i18n( "Address Book Browser" ));

    mWidget = new KAddressBook( this, "KAddressBook" );

    mActionManager = new ActionManager(this, mWidget, true, this);

    initActions();

    // tell the KMainWindow that this is indeed the main widget
    setCentralWidget(mWidget);

    // we do want a status bar
    statusBar()->show();

    // Tell the central widget to read its config
    mWidget->readConfig();

    // Finally create the GUI
    createGUI( "kaddressbookui.rc", false );
    // <HACK reason="there is no line edit action">
    // create the incremental search line edit manually:
    const int IncSearch=1; //the ID of the widget - just to be clear :-)
    KToolBar *isToolBar=toolBar("incSearchToolBar");
    IncSearchWidget *incSearchWidget=new IncSearchWidget(isToolBar);
    isToolBar->insertWidget(IncSearch, 0,  incSearchWidget, 0);
    isToolBar->setItemAutoSized(IncSearch);
    mWidget->setIncSearchWidget(incSearchWidget);
    // </HACK>
    mActionManager->initActionViewList();

    setAutoSaveSettings();
}

KAddressBookMain::~KAddressBookMain()
{
    mWidget->writeConfig();
}

void KAddressBookMain::saveProperties(KConfig *)
{
  // the 'config' object points to the session managed
  // config file.  anything you write here will be available
  // later when this app is restored

  //what I want to save
  //windowsize
  //background image/underlining color/alternating color1,2
  //chosen fields
  //chosen fieldsWidths

  // e.g., config->writeEntry("key", var);
}

void KAddressBookMain::readProperties(KConfig *)
{
  // the 'config' object points to the session managed
  // config file.  this function is automatically called whenever
  // the app is being restored.  read in here whatever you wrote
  // in 'saveProperties'

  // e.g., var = config->readEntry("key");
}

void KAddressBookMain::initActions()
{
  KStdAction::quit(this, SLOT(closeWithSave()), actionCollection());

  KStdAction::preferences( mWidget, SLOT( configure() ), actionCollection());
  KStdAction::configureToolbars( this, SLOT( configureToolbars() ),
                                 actionCollection() );
  KStdAction::keyBindings(this, SLOT( configureKeys()), actionCollection() );
}

void KAddressBookMain::configureToolbars()
{
  saveMainWindowSettings( KGlobal::config(), "MainWindow" );

  KEditToolbar dlg(factory());
  connect(&dlg,SIGNAL(newToolbarConfig()),this,SLOT(slotNewToolbarConfig()));

  dlg.exec();
}

void KAddressBookMain::slotNewToolbarConfig()
{
  mActionManager->initActionViewList();

  applyMainWindowSettings( KGlobal::config(), "MainWindow" );
}

void KAddressBookMain::configureKeys()
{
  KKeyDialog::configureKeys(actionCollection(),xmlFile(),true,this);
}

void KAddressBookMain::closeWithSave()
{
  if ( mActionManager->isModified() ) {
    QString text = i18n( "The address book was modified. Do you want to save your changes?" );
    int ret = KMessageBox::warningYesNoCancel( this, text, "",
                                              KStdGuiItem::yes(),
                                              KStdGuiItem::no(), "AskForSave" );
    if ( ret == KMessageBox::Yes )
      save();
    else if ( ret == KMessageBox::Cancel )
      return;
  }

  close();
}
