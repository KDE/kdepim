/*
 * Copyright (C) 2003 Helge Deller <deller@kde.org>
 */

#include <qdragobject.h>
#include <kprinter.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>

#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <kkeydialog.h>
#include <kaccel.h>
#include <kio/netaccess.h>
#include <kfiledialog.h>
#include <kconfig.h>

#include <kedittoolbar.h>

#include <kstdaccel.h>
#include <kaction.h>
#include <kstdaction.h>

#include <kpushbutton.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstddirs.h>

#include "kmobile.h"
#include "pref.h"

#include "kmobileitem.h"
#include "kmobile_selectiondialog.h"

KMobile::KMobile()
    : KMainWindow( 0, "kmobile" )
{
    m_config = new KConfig("kmobilerc");

    m_view = new KMobileView(this, m_config);

    // tell the KMainWindow that this is indeed the main widget
    setCentralWidget(m_view);

    // then, setup our actions
    setupActions();

    toolBar()->show();
    statusBar()->show();

    // apply the saved mainwindow settings, if any, and ask the mainwindow
    // to automatically save settings if changed: window size, toolbar
    // position, icon size, etc.
    setAutoSaveSettings();

    // allow the view to change the statusbar and caption
    connect(m_view, SIGNAL(signalChangeStatusbar(const QString&)),
            this,   SLOT(changeStatusbar(const QString&)));

    // restore all configured devices
    restoreAll();
}

KMobile::~KMobile()
{
   delete m_config;
}

void KMobile::setupActions()
{
    KStdAction::save(this, SLOT(fileSave()), actionCollection());
    KStdAction::quit(kapp, SLOT(quit()), actionCollection());

    m_toolbarAction = KStdAction::showToolbar(this, SLOT(optionsShowToolbar()), actionCollection());
    optionsShowToolbar();
    m_statusbarAction = KStdAction::showStatusbar(this, SLOT(optionsShowStatusbar()), actionCollection());

    KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
    KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
    KStdAction::preferences(this, SLOT(optionsPreferences()), actionCollection());

    new KAction(i18n("&Add Device..."), "folder_new", 0,
		this, SLOT(addDevice()), actionCollection(), "device_add");
    new KAction( KGuiItem( i18n("&Remove Device"), "edittrash", i18n("Remove this device") ),
		"Delete", this,  SLOT(removeDevice()), actionCollection(), "device_remove");
    new KAction(i18n("Re&name Device..."), 0, 0,
		this, SLOT(renameDevice()), actionCollection(), "device_rename");
    new KAction(i18n("&Configure Device..."), "configure", 0,
		this, SLOT(configDevice()), actionCollection(), "device_configure");

    createGUI();

    connect( kapp, SIGNAL(aboutToQuit()), this, SLOT(saveAll()) );
}


void KMobile::saveAll()
{
    m_view->saveAll(); 
}

void KMobile::restoreAll()
{
    m_view->restoreAll(); 
}

void KMobile::fileSave()
{
    // this slot is called whenever the File->Save menu is selected,
    // the Save shortcut is pressed (usually CTRL+S) or the Save toolbar
    // button is clicked

    // save the current file
}


void KMobile::saveProperties(KConfig *)
{
    // the 'config' object points to the session managed
    // config file.  anything you write here will be available
    // later when this app is restored
}

void KMobile::readProperties(KConfig *config)
{
    // the 'config' object points to the session managed
    // config file.  this function is automatically called whenever
    // the app is being restored.  read in here whatever you wrote
    // in 'saveProperties'

    QString url = config->readEntry("lastURL");
}

void KMobile::optionsShowToolbar()
{
    // this is all very cut and paste code for showing/hiding the
    // toolbar
    if (m_toolbarAction->isChecked())
        toolBar()->show();
    else
        toolBar()->hide();
}

void KMobile::optionsShowStatusbar()
{
    // this is all very cut and paste code for showing/hiding the
    // statusbar
    if (m_statusbarAction->isChecked())
        statusBar()->show();
    else
        statusBar()->hide();
}

void KMobile::optionsConfigureKeys()
{
    KKeyDialog::configureKeys(actionCollection(), "kmobileui.rc");
}

void KMobile::optionsConfigureToolbars()
{
    // use the standard toolbar editor
    saveMainWindowSettings(KGlobal::config(), autoSaveGroup());
}

void KMobile::newToolbarConfig()
{
    // this slot is called when user clicks "Ok" or "Apply" in the toolbar editor.
    // recreate our GUI, and re-apply the settings (e.g. "text under icons", etc.)
    createGUI();

    applyMainWindowSettings(KGlobal::config(), autoSaveGroup());
}

void KMobile::optionsPreferences()
{
    // popup some sort of preference dialog, here
    KMobilePreferences dlg;
    if (dlg.exec())
    {
        // redo your settings
    }
}


void KMobile::renameDevice()
{
    QIconViewItem *item = m_view->currentItem();
    if (item)
       item->rename();
}

/*
 * Add a new Device (Dialog)
 */

void KMobile::addDevice()
{
  KMobile_SelectionDialog *dialog = new KMobile_SelectionDialog(m_view);
  if (!dialog)
    return;

  dialog->setCaption( i18n("Add New Mobile Device") );

  dialog->helpText->setText( i18n("Please select the category to which your new device belongs:") );
  dialog->addButton->setText( i18n("&Scan for new devices...") );
  dialog->addButton->setDisabled(true);
  dialog->iconView->connect( dialog->iconView, SIGNAL(doubleClicked(QIconViewItem*)),
			dialog, SLOT(accept()) );
  dialog->selectButton->setText( i18n("&Add") );
  dialog->selectButton->connect( dialog->selectButton, SIGNAL(clicked()), dialog, SLOT(accept()) );
  dialog->cancelButton->connect( dialog->cancelButton, SIGNAL(clicked()), dialog, SLOT(reject()) );

  KTrader::OfferList list = KMobileItem::getMobileDevicesList();
  KTrader::OfferListIterator it;
  KService::Ptr ptr;
  for ( it = list.begin(); it != list.end(); ++it ) {
    ptr = *it;
    kdDebug() << QString("LIBRARY: '%1', NAME: '%2', ICON: '%3', COMMENT: '%4'\n")
		.arg(ptr->library()).arg(ptr->name()).arg(ptr->icon()).arg(ptr->comment());

    QString iconName = ptr->icon();
    if (iconName.isEmpty())
	iconName = "unknown.png";
    QPixmap pm( ::locate("data", "kmobile/pics/"+iconName) );
    
    QIconViewItem *item;
    item = new QIconViewItem( dialog->iconView, ptr->name(), pm );

    //if (!ptr->comment().isNull())
    //	QToolTip::add(item->pixmap(), ptr->comment() );
  }

  int index = -1;
  if (dialog->exec() == QDialog::Accepted)
     index = dialog->iconView->currentItem()->index(); // get index of selected item
  delete dialog;

  if (index<0 || index>=(int)list.count())
    return;

  ptr = list[index];
  kdDebug() << "Selected item:\n";
  kdDebug() << QString("LIBRARY: '%1', NAME: '%2', ICON: '%3', COMMENT: '%4'\n")
		.arg(ptr->library()).arg(ptr->name()).arg(ptr->icon()).arg(ptr->comment());

  new KMobileItem(m_view, m_config, ptr);

  saveAll();
}

#if 0
/*
 * show dialog to user, in which he may choose and select one of the already
 * configured mobile devices.
 */
KMobileDevice * KMobileFactory::chooseDeviceDialog( QWidget *parent, 
		enum KMobileDevice::ClassType /*type*/, enum KMobileDevice::Capabilities /*caps*/ )
{
  int num;

  m_parent = parent;

  // do we already have some devices configured ?
  num = readDevicesList();
  if (!num) {
     int answ;
     answ = KMessageBox::questionYesNo(parent,
		i18n( "<qt>You have no mobile devices configured yet.<p>"
			"Do you want to add a device now ?</qt>" ),
		i18n( "KDE Mobile Device Access" ) );
     if (answ != KMessageBox::Yes)
	return 0L;
     // add a new device
     addDeviceDialog(parent);
  }
  num = readDevicesList();
  if (!num) 
    return 0L;

  // let the user select one of the configured devices
  KMobile_selectiondialog *dialog = new KMobile_selectiondialog(parent);
  if (!dialog)
    return 0L;

  dialog->addButton->connect( dialog->addButton, SIGNAL(clicked()), this, SLOT(slotAddDevice()) );
  dialog->iconView->connect( dialog->iconView, SIGNAL(doubleClicked(QIconViewItem*)),
			dialog, SLOT(accept()) );
  dialog->selectButton->connect( dialog->selectButton, SIGNAL(clicked()), dialog, SLOT(accept()) );
  dialog->cancelButton->connect( dialog->cancelButton, SIGNAL(clicked()), dialog, SLOT(reject()) );

  for (int i=0; i<countDevices(); i++) {
    KService::Ptr ptr;
    ptr = ServiceForEntry(i);
    if (!ptr)
	continue;

    // kdDebug() << QString("LIBRARY: '%1', NAME: '%2', ICON: '%3', COMMENT: '%4'   #%5\n")
    //		.arg(ptr->library()).arg(ptr->name()).arg(ptr->icon()).arg(ptr->comment()).arg(i);

    QString iconName = ptr->icon();
    if (iconName.isEmpty())
	iconName = "unknown.png";
    QPixmap pm( ::locate("data", "kmobile/pics/"+iconName) );
    
    QIconViewItem *item;
    item = new QIconViewItem( dialog->iconView, ptr->name(), pm );

  }

  int index = -1;
  if (dialog->exec() == QDialog::Accepted)
     index = dialog->iconView->currentItem()->index(); // get index of selected item
  delete dialog;

  if (index<0 || index>=countDevices())
    return 0L;

  return getDevice(index);
}
#endif


void KMobile::removeDevice()
{
    QIconViewItem *item = m_view->currentItem();
    if (item)
       m_view->removeDevice( item->text() );
}

void KMobile::configDevice()
{
    QIconViewItem *item = m_view->currentItem();
    if (item)
       m_view->configDevice( item->text() );
}

void KMobile::changeStatusbar(const QString& text)
{
    // display the text on the statusbar
    statusBar()->message(text);
}

#include "kmobile.moc"
