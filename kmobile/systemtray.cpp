/*  This file is part of the KDE KMobile library.
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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kiconloader.h>
#include <kpopupmenu.h>
#include <kpassivepopup.h>
#include <kaction.h>
#include <kmainwindow.h>
#include <kiconeffect.h>
#include <kdebug.h>

#include <qhbox.h>
#include <qpushbutton.h>
#include <qtooltip.h>
#include <qpainter.h>

#include "systemtray.h"

#include "kmobile.h"
#include "kmobileview.h"


SystemTray::SystemTray(KMainWindow *parent, const char *name) : KSystemTray(parent, name)

{
    m_appPix = KGlobal::instance()->iconLoader()->loadIcon("kmobile", KIcon::Small);
    setPixmap(m_appPix);

    setToolTip();

    m_actionCollection = parent->actionCollection();
    KAction *addAction = m_actionCollection->action("device_add");
    
    KPopupMenu* menu = contextMenu();
    addAction->plug(menu);
    menu->insertSeparator();
}

SystemTray::~SystemTray()
{

}

#define SYSTEMTRAY_STARTID 1000

void SystemTray::contextMenuAboutToShow(KPopupMenu *menu)
{
    KMobile *main = static_cast<KMobile *>(parent());

    const int pos = 3;
    while (menu->idAt(pos)>=SYSTEMTRAY_STARTID &&
	   menu->idAt(pos)<=(SYSTEMTRAY_STARTID+1000))
	menu->removeItemAt(pos);

    // create menu entries for each mobile device and add it's icon
    QStringList list = main->mainView()->deviceNames();
    for (unsigned int no=0; no<list.count(); no++) {
	QString devName = list[no];
        QString iconName = main->mainView()->iconFileName(devName);
        QPixmap pm = KGlobal::instance()->iconLoader()->loadIcon(iconName, KIcon::Small);
	menu->insertItem(pm, devName, SYSTEMTRAY_STARTID+no, 3+no);
	menu->connectItem(SYSTEMTRAY_STARTID+no, this, SLOT(menuItemSelected()));
    }
    connect(menu, SIGNAL(activated(int)), this, SLOT(menuItemActivated(int)));
}

void SystemTray::menuItemSelected()
{
    if (m_menuID<SYSTEMTRAY_STARTID || m_menuID>SYSTEMTRAY_STARTID+1000)
	return;
    QString devName = contextMenu()->text(m_menuID);
    KMobile *main = static_cast<KMobile *>(parent());
    main->mainView()->startKonqueror(devName);
}

void SystemTray::menuItemActivated(int id)
{
    m_menuID = id;
}

void SystemTray::setToolTip(const QString &tip)
{
    if (tip.isEmpty())
        QToolTip::add(this, "KMobile");
    else
        QToolTip::add(this, tip);
}

#include "systemtray.moc"

// vim: ts=8
