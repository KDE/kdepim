/*

 kandy.cpp

 Copyright (C) 2000,2001,2002 Cornelius Schumacher <schumacher@kde.org>

*/

#include <qdragobject.h>
#include <qlineedit.h>
#include <qprinter.h>
#include <qprintdialog.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>

#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmenubar.h>
#include <kkeydialog.h>
#include <kaccel.h>
#include <kio/netaccess.h>
#include <kfiledialog.h>
#include <kconfig.h>
#include <kurl.h>
#include <kurlrequesterdlg.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kedittoolbar.h>
#include <kstdaccel.h>
#include <kaction.h>
#include <kstdaction.h>

#include "kandyprefsdialog.h"
#include "commandscheduler.h"
#include "kandyprefs.h"
#include "modem.h"

#include "kandy.h"
#include <kstatusbar.h>
#include "kandy.moc"

Kandy::Kandy(CommandScheduler *scheduler)
    : KMainWindow( 0, "Kandy" ),
      mPrinter(0)
{
  mScheduler = scheduler;

  mPreferencesDialog = 0;

  mView = new KandyView(mScheduler,this);

  // accept dnd
  setAcceptDrops(true);

  // tell the KMainWindow that this is indeed the main widget
  setCentralWidget(mView);

  // then, setup our actions
  setupActions();

  statusBar()->insertItem(i18n(" Disconnected "),0,0,true);

  setAutoSaveSettings();

  // allow the view to change the statusbar and caption
  connect(mView, SIGNAL(signalChangeStatusbar(const QString&)),
          this,   SLOT(changeStatusbar(const QString&)));
  connect(mView, SIGNAL(signalChangeCaption(const QString&)),
          this,   SLOT(changeCaption(const QString&)));

  connect(mView,SIGNAL(modifiedChanged(bool)),SLOT(setTitle()));

  KConfig *config = KGlobal::config();
  config->setGroup("General");
  QString currentProfile = config->readEntry("CurrentProfile",
                                             locate("appdata","default.kandy"));
  if (!currentProfile.isEmpty()) load(currentProfile);
}

Kandy::~Kandy()
{
}

void Kandy::load(const QString& filename)
{
  if (!mView->loadFile(filename)) {
    KMessageBox::error(this,i18n("Could not load file %1").arg(filename));
  }

  mFilename = filename;
  setTitle();
}

void Kandy::save(const QString & filename)
{
  if (!filename.isEmpty()) {
    if (!mView->saveFile(filename)) {
      KMessageBox::error(this,i18n("Couldn't save file %1.").arg(filename));
    } else {
      mFilename = filename;
      setTitle();
    }
  }
}

void Kandy::setupActions()
{
  KStdAction::open(this, SLOT(fileOpen()), actionCollection());
  KStdAction::save(this, SLOT(fileSave()), actionCollection());
  KStdAction::saveAs(this, SLOT(fileSaveAs()), actionCollection());
//  KStdAction::print(this, SLOT(filePrint()), actionCollection());
  KStdAction::quit(this, SLOT(close()), actionCollection());

  createStandardStatusBarAction();
  setStandardToolBarMenuEnabled(true);
    
  KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
  KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
  KStdAction::preferences(this, SLOT(optionsPreferences()), actionCollection());

  new KAction(i18n("Mobile GUI"),0,this,SLOT(showMobileGui()),
              actionCollection(),"show_mobilegui");

  mConnectAction = new KAction(i18n("Connect"),0,this,SLOT(modemConnect()),
                               actionCollection(),"modem_connect");
  mDisconnectAction = new KAction(i18n("Disconnect"),0,this,
                                  SLOT(modemDisconnect()),actionCollection(),
                                  "modem_disconnect");

  createGUI();
}

void Kandy::saveProperties(KConfig */*config*/)
{
    // the 'config' object points to the session managed
    // config file.  anything you write here will be available
    // later when this app is restored
}

void Kandy::readProperties(KConfig */*config*/)
{
    // the 'config' object points to the session managed
    // config file.  this function is automatically called whenever
    // the app is being restored.  read in here whatever you wrote
    // in 'saveProperties'
}

void Kandy::dragEnterEvent(QDragEnterEvent *event)
{
    // do nothing
    KMainWindow::dragEnterEvent(event);

    // accept uri drops only
//    event->accept(QUriDrag::canDecode(event));
}

void Kandy::dropEvent(QDropEvent *event)
{
    // this is a very simplistic implementation of a drop event.  we
    // will only accept a dropped URL.  the Qt dnd code can do *much*
    // much more, so please read the docs there

    // do nothing
    KMainWindow::dropEvent(event);
/*
    QStrList uri;

    // see if we can decode a URI.. if not, just ignore it
    if (QUriDrag::decode(event, uri))
    {
        // okay, we have a URI.. process it
        QString url, target;
        url = uri.first();

        // load in the file
        load(url);
    }
*/
}

void Kandy::fileOpen()
{
    // this slot is called whenever the File->Open menu is selected,
    // the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
    // button is clicked
    QString filename = KFileDialog::getOpenFileName();
    if (!filename.isEmpty()) load(filename);
}

void Kandy::fileSave()
{
  if (mFilename.isEmpty()) fileSaveAs();
  else save(mFilename);
}

void Kandy::fileSaveAs()
{
  QString filename = KFileDialog::getSaveFileName();
  save(filename);
}

void Kandy::filePrint()
{
    // this slot is called whenever the File->Print menu is selected,
    // the Print shortcut is pressed (usually CTRL+P) or the Print toolbar
    // button is clicked
    if (!mPrinter) mPrinter = new QPrinter;
    if (QPrintDialog::getPrinterSetup(mPrinter))
    {
        // setup the printer.  with Qt, you always "print" to a
        // QPainter.. whether the output medium is a pixmap, a screen,
        // or paper
        QPainter p;
        p.begin(mPrinter);

        // we let our view do the actual printing
        QPaintDeviceMetrics metrics(mPrinter);
        mView->print(&p, metrics.height(), metrics.width());

        // and send the result to the printer
        p.end();
    }
}

void Kandy::optionsConfigureKeys()
{
    KKeyDialog::configureKeys(actionCollection(), "kandyui.rc");
}

void Kandy::optionsConfigureToolbars()
{
    // use the standard toolbar editor
    saveMainWindowSettings( KGlobal::config(), autoSaveGroup() );
    KEditToolbar dlg(actionCollection());
    connect(&dlg, SIGNAL(newToolbarConfig()), this, SLOT(newToolbarConfig()));
    dlg.exec();
}

void Kandy::newToolbarConfig()
{
    // this slot is called when user clicks "Ok" or "Apply" in the toolbar editor.
    // recreate our GUI, and re-apply the settings (e.g. "text under icons", etc.)
    createGUI();
    applyMainWindowSettings( KGlobal::config(),  autoSaveGroup() );
}

void Kandy::optionsPreferences()
{
  if (!mPreferencesDialog) {
    mPreferencesDialog = new KandyPrefsDialog(this);
    mPreferencesDialog->readConfig();
  }

  mPreferencesDialog->show();
  mPreferencesDialog->raise();
}

void Kandy::changeStatusbar(const QString& text)
{
    // display the text on the statusbar
    statusBar()->message(text);
}

void Kandy::changeCaption(const QString& text)
{
    // display the text on the caption
    setCaption(text);
}

void Kandy::setTitle()
{
  if (mFilename.isEmpty()) {
    setCaption(i18n("New Profile"),mView->isModified());
  } else {
    setCaption(mFilename,mView->isModified());
  }
}

bool Kandy::queryClose()
{
  if (mView->isModified()) {
    switch (KMessageBox::warningYesNoCancel(this,
        i18n("Save changes to profile %1?").arg(mFilename))) {
      case KMessageBox::Yes :
        fileSave();
        return true;
      case KMessageBox::No :
        return true;
      default: // cancel
        return false;
    }
  } else {
    return true;
  }
}

void Kandy::modemConnect()
{
  if (!mScheduler->modem()->open()) {
    KMessageBox::sorry(this,
        i18n("Cannot open modem device %1.")
        .arg(KandyPrefs::instance()->mSerialDevice), i18n("Modem Error"));
    return;
  }

  statusBar()->changeItem(i18n(" Connected "),0);

  emit connectStateChanged(true);
}

void Kandy::modemDisconnect()
{
  mScheduler->modem()->close();

  statusBar()->changeItem(i18n(" Disconnected "),0);

  emit connectStateChanged(false);
}

void Kandy::showMobileGui()
{
  emit showMobileWin();
}

void Kandy::showErrorMessage( const QString &text )
{
  KMessageBox::error( 0, text );
}
