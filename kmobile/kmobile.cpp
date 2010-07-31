/*  This file is part of the KDE KMobile library
    Copyright (C) 2003 Helge Deller <deller@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include <tqdragobject.h>
#include <kprinter.h>
#include <tqpainter.h>
#include <tqpaintdevicemetrics.h>

#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <kkeydialog.h>
#include <kaccel.h>
#include <kio/netaccess.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kconfig.h>

#include <kedittoolbar.h>

#include <kstdaccel.h>
#include <kaction.h>
#include <kstdaction.h>

#include <kpushbutton.h>
#include <kdebug.h>
#include <kstandarddirs.h>

#include "kmobile.h"
#include "pref.h"

#include "systemtray.h"

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
    connect(m_view, TQT_SIGNAL(signalChangeStatusbar(const TQString&)),
            this,   TQT_SLOT(changeStatusbar(const TQString&)));

    // restore all configured devices
    restoreAll();

    // setup the system tray
    m_systemTray = new SystemTray(this, "systemTray");
    m_systemTray->show();
    connect(m_systemTray, TQT_SIGNAL(quitSelected()), this, TQT_SLOT(slotQuit()));
}

KMobile::~KMobile()
{
   delete m_config;
}

void KMobile::setupActions()
{
    KStdAction::close(this, TQT_SLOT(dockApplication()), actionCollection());
    KStdAction::quit(kapp, TQT_SLOT(quit()), actionCollection());

    m_toolbarAction = KStdAction::showToolbar(this, TQT_SLOT(optionsShowToolbar()), actionCollection());
    optionsShowToolbar();
    m_statusbarAction = KStdAction::showStatusbar(this, TQT_SLOT(optionsShowStatusbar()), actionCollection());

    KStdAction::keyBindings(this, TQT_SLOT(optionsConfigureKeys()), actionCollection());
    KStdAction::configureToolbars(this, TQT_SLOT(optionsConfigureToolbars()), actionCollection());
    KStdAction::preferences(this, TQT_SLOT(optionsPreferences()), actionCollection());

    new KAction(i18n("&Add Device..."), "folder_new", 0,
		this, TQT_SLOT(addDevice()), actionCollection(), "device_add");
    new KAction( KGuiItem( i18n("&Remove Device"), "edittrash", i18n("Remove this device") ),
		"Delete", this,  TQT_SLOT(removeDevice()), actionCollection(), "device_remove");
    new KAction(i18n("Re&name Device..."), 0, Key_F2,
		this, TQT_SLOT(renameDevice()), actionCollection(), "device_rename");
    new KAction(i18n("&Configure Device..."), "configure", 0,
		this, TQT_SLOT(configDevice()), actionCollection(), "device_configure");

    createGUI();

    connect( kapp, TQT_SIGNAL(aboutToQuit()), this, TQT_SLOT(saveAll()) );
}


void KMobile::dockApplication()
{
    // dock to system tray
    hide();
}

bool KMobile::queryClose()
{
    dockApplication();
    return false;
}

bool KMobile::queryExit()
{
    dockApplication();
    return false;
}

void KMobile::slotQuit()
{
    kapp->quit();
}

void KMobile::showMinimized()
{
    dockApplication();
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

    TQString url = config->readPathEntry("lastURL");
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
    KKeyDialog::configure(actionCollection());
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
#if 0
    KMobilePreferences dlg;
    if (dlg.exec())
    {
        // redo your settings
    }
#endif
}


void KMobile::renameDevice()
{
    // rename the current selected device
    TQIconViewItem *item = m_view->currentItem();
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

  dialog->setCaption( i18n("Add New Mobile or Portable Device") );

  dialog->helpText->setText( i18n("Please select the category to which your new device belongs:") );
  dialog->addButton->setText( i18n("&Scan for New Devices...") );
  dialog->addButton->setDisabled(true);
  dialog->iconView->connect( dialog->iconView, TQT_SIGNAL(doubleClicked(TQIconViewItem*)),
			dialog, TQT_SLOT(accept()) );
  dialog->selectButton->setText( i18n("&Add") );
  dialog->selectButton->connect( dialog->selectButton, TQT_SIGNAL(clicked()), dialog, TQT_SLOT(accept()) );
  dialog->cancelButton->connect( dialog->cancelButton, TQT_SIGNAL(clicked()), dialog, TQT_SLOT(reject()) );

  KTrader::OfferList list = KMobileItem::getMobileDevicesList();
  KTrader::OfferListIterator it;
  KService::Ptr ptr;
  for ( it = list.begin(); it != list.end(); ++it ) {
    ptr = *it;
    kdDebug() << TQString("LIBRARY: '%1', NAME: '%2', ICON: '%3', COMMENT: '%4'\n")
		.arg(ptr->library()).arg(ptr->name()).arg(ptr->icon()).arg(ptr->comment());

    TQString iconName = ptr->icon();
    if (iconName.isEmpty())
	iconName = KMOBILE_ICON_UNKNOWN;
    TQPixmap pm = KGlobal::instance()->iconLoader()->loadIcon(iconName, KIcon::Desktop );
    
    TQIconViewItem *item;
    item = new TQIconViewItem( dialog->iconView, ptr->name(), pm );

    //if (!ptr->comment().isNull())
    //	TQToolTip::add(item->pixmap(), ptr->comment() );
  }

  int index = -1;
  if (dialog->exec() == TQDialog::Accepted)
     index = dialog->iconView->currentItem()->index(); // get index of selected item
  delete dialog;

  if (index<0 || index>=(int)list.count())
    return;

  ptr = list[index];

  // add the new device to the list
  if (!m_view->addNewDevice(m_config, ptr)) {
	KMessageBox::error(this, 
		TQString("<qt>KMobile could not load the <b>%1</b> Device Driver.<p>"
		     "Please use the Skeleton- or Gnokii Device Driver during development.<p>"
		     "This driver will still be visible, but you won't be able to access it "
		     "from Konqueror or any other application.</qt>").arg(ptr->name()),
		kapp->name());
  }

  saveAll();
}

#if 0
/*
 * show dialog to user, in which he may choose and select one of the already
 * configured mobile devices.
 */
KMobileDevice * KMobileFactory::chooseDeviceDialog( TQWidget *parent, 
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
		i18n( "KDE Mobile Device Access" ), KStdGuiItem::add(), i18n("Do Not Add") );
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

  dialog->addButton->connect( dialog->addButton, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotAddDevice()) );
  dialog->iconView->connect( dialog->iconView, TQT_SIGNAL(doubleClicked(TQIconViewItem*)),
			dialog, TQT_SLOT(accept()) );
  dialog->selectButton->connect( dialog->selectButton, TQT_SIGNAL(clicked()), dialog, TQT_SLOT(accept()) );
  dialog->cancelButton->connect( dialog->cancelButton, TQT_SIGNAL(clicked()), dialog, TQT_SLOT(reject()) );

  for (int i=0; i<countDevices(); i++) {
    KService::Ptr ptr;
    ptr = ServiceForEntry(i);
    if (!ptr)
	continue;

    // kdDebug() << TQString("LIBRARY: '%1', NAME: '%2', ICON: '%3', COMMENT: '%4'   #%5\n")
    //		.arg(ptr->library()).arg(ptr->name()).arg(ptr->icon()).arg(ptr->comment()).arg(i);

    TQString iconName = ptr->icon();
    if (iconName.isEmpty())
	iconName = KMOBILE_ICON_UNKNOWN;
    TQPixmap pm( ::locate("icon", iconName+".png") );
    
    TQIconViewItem *item;
    item = new TQIconViewItem( dialog->iconView, ptr->name(), pm );

  }

  int index = -1;
  if (dialog->exec() == TQDialog::Accepted)
     index = dialog->iconView->currentItem()->index(); // get index of selected item
  delete dialog;

  if (index<0 || index>=countDevices())
    return 0L;

  return getDevice(index);
}
#endif


void KMobile::removeDevice()
{
    // remove the current selected device
    TQIconViewItem *item = m_view->currentItem();
    if (item)
       m_view->removeDevice( item->text() );
}

void KMobile::configDevice()
{
    // configure the current selected device
    TQIconViewItem *item = m_view->currentItem();
    if (item)
       m_view->configDevice( item->text() );
}

void KMobile::changeStatusbar(const TQString& text)
{
    // display the text on the statusbar
    statusBar()->message(text);
}

#include "kmobile.moc"
