/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/
#include "devicemenu.h"

#include <klocale.h>
#include <kglobal.h>
#include <kicon.h>
#include "devicesconfig.h"
#include "weaver.h"
#include "engine.h"

#define ID_LOAD 1
#define ID_UNLOAD 2
#define ID_SWITCHTO 4
#define ID_CONFIGURE 8
#define ID_PBOOK 0x10
#define ID_SMS 0x20

using namespace KMobileTools;

deviceMenu::deviceMenu(bool loaded, KMobileTools::Engine *engine, QWidget *parent, const QString &name)
 : KMenu(parent), p_engine(engine)
{
    setObjectName(name);
    addTitle( DEVCFG(name)->devicename() );
    if(loaded)
        addAction( KIcon( "connect_no" ), i18n("Unload"), this, SLOT(slotUnloadDevice()) );
    else addAction( KIcon( "connect_creating" ), i18n("Load"), this, SLOT(slotLoadDevice()) );
    if (!engine)
        addAction( KIcon( "goto" ), i18n("Switch to.."), this, SLOT(slotSwitchDevice()) );
    else
    {
        addSeparator();
        addAction( KIcon( "personal" ),  i18n("Phonebook"), this, SLOT(slotGoPhonebook()) );
        addAction( KIcon( "mail_get" ),  i18n("SMS"), this, SLOT(slotGoSMS()) );
    }
    addAction( KIcon( "configure" ), i18n("Configure"), this, SLOT(slotConfigureDevice()) );
}


deviceMenu::~deviceMenu()
{
}

void deviceMenu::slotSwitchDevice() { emit switchDevice(objectName() ); }

void deviceMenu::slotLoadDevice() { emit loadDevice(objectName() ); }
void deviceMenu::slotUnloadDevice() { emit unloadDevice(objectName() ); }

void deviceMenu::slotConfigureDevice() {
    if(p_engine) emit sendURL( KUrl(QString("%1:configure").arg(p_engine->objectName() ) ) );
    else emit configure( QString("configure:%1").arg(objectName() ) );
}

void deviceMenu::slotGoSMS() { emit sendURL( KUrl(QString("%1:phonebook").arg(p_engine->objectName() ) ) ); }

void deviceMenu::slotGoPhonebook() { emit sendURL( KUrl(QString("%1:sms").arg(p_engine->objectName() ) ) ); }


#include "devicemenu.moc"

