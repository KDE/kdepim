/*
 * kandy.cpp
 *
 * Copyright (C) 2000 Cornelius Schumacher <schumacher@kde.org>
 */
#include "kandy.h"

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
#include <kstddirs.h>
#include <kedittoolbar.h>

#include <kstdaccel.h>
#include <kaction.h>
#include <kstdaction.h>

#include "kandyprefsdialog.h"

Kandy::Kandy()
    : KMainWindow( 0, "Kandy" ),
      m_view(new KandyView(this)),
      m_printer(0)
{
  mPreferencesDialog = 0;

    // accept dnd
    setAcceptDrops(true);

    // tell the KMainWindow that this is indeed the main widget
    setCentralWidget(m_view);

    // then, setup our actions
    setupActions();

    // and a status bar
    statusBar()->show();

    // allow the view to change the statusbar and caption
    connect(m_view, SIGNAL(signalChangeStatusbar(const QString&)),
            this,   SLOT(changeStatusbar(const QString&)));
    connect(m_view, SIGNAL(signalChangeCaption(const QString&)),
            this,   SLOT(changeCaption(const QString&)));

  connect(m_view,SIGNAL(modifiedChanged(bool)),SLOT(setTitle()));

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
  if (!m_view->loadFile(filename)) {
    KMessageBox::error(this,i18n("Could not load file %1").arg(filename));
  }

  mFilename = filename;
  setTitle();
}

void Kandy::save(const QString & filename)
{
  if (!filename.isEmpty()) {
    if (!m_view->saveFile(filename)) {
      KMessageBox::error(this,i18n("Couldn't save file %1.").arg(filename)); 
    } else {
      mFilename = filename;
      setTitle();
    }
  }
}

void Kandy::setupActions()
{
    KStdAction::openNew(this, SLOT(fileNew()), actionCollection());
    KStdAction::open(this, SLOT(fileOpen()), actionCollection());
    KStdAction::save(this, SLOT(fileSave()), actionCollection());
    KStdAction::saveAs(this, SLOT(fileSaveAs()), actionCollection());
    KStdAction::print(this, SLOT(filePrint()), actionCollection());
    KStdAction::quit(this, SLOT(close()), actionCollection());

    m_toolbarAction = KStdAction::showToolbar(this, SLOT(optionsShowToolbar()), actionCollection());
    m_statusbarAction = KStdAction::showStatusbar(this, SLOT(optionsShowStatusbar()), actionCollection());

    KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
    KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
    KStdAction::preferences(this, SLOT(optionsPreferences()), actionCollection());

    new KAction(i18n("Mobile GUI"),0,m_view,SLOT(showMobileGui()),
                actionCollection(),"show_mobilegui");

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

void Kandy::fileNew()
{
    // this slot is called whenever the File->New menu is selected,
    // the New shortcut is pressed (usually CTRL+N) or the New toolbar
    // button is clicked

    // create a new window
    (new Kandy)->show();
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
    if (!m_printer) m_printer = new QPrinter;
    if (QPrintDialog::getPrinterSetup(m_printer))
    {
        // setup the printer.  with Qt, you always "print" to a
        // QPainter.. whether the output medium is a pixmap, a screen,
        // or paper
        QPainter p;
        p.begin(m_printer);

        // we let our view do the actual printing
        QPaintDeviceMetrics metrics(m_printer);
        m_view->print(&p, metrics.height(), metrics.width());

        // and send the result to the printer
        p.end();
    }
}

void Kandy::optionsShowToolbar()
{
    // this is all very cut and paste code for showing/hiding the
    // toolbar
    if (m_toolbarAction->isChecked())
        toolBar()->show();
    else
        toolBar()->hide();
}

void Kandy::optionsShowStatusbar()
{
    // this is all very cut and paste code for showing/hiding the
    // statusbar
    if (m_statusbarAction->isChecked())
        statusBar()->show();
    else
        statusBar()->hide();
}

void Kandy::optionsConfigureKeys()
{
    KKeyDialog::configureKeys(actionCollection(), "kandyui.rc");
}

void Kandy::optionsConfigureToolbars()
{
    // use the standard toolbar editor
    KEditToolbar dlg(actionCollection());
    if (dlg.exec())
    {
        // recreate our GUI
        createGUI();
    }
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
    setCaption(i18n("New Profile"),m_view->isModified());
  } else {
    setCaption(mFilename,m_view->isModified());
  }
}

bool Kandy::queryClose()
{
  if (m_view->isModified()) {
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

#include "kandy.moc"
