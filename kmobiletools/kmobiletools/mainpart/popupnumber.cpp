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
#include "popupnumber.h"

#include <klocale.h>
#include <kglobal.h>
#include <kicon.h>
#include <kdebug.h>

#include "engineslist.h"
#if 0 // port to D-Bus!
#include "deviceIFace_stub.h"
#endif
#include "devicesconfig.h"


#define CALL_ID 1
#define CALL_O_ID 2
#define SMS_ID 3
#define SMS_O_ID 4

popupNumber::popupNumber(const QString &deviceName, const QString &number, QWidget *parent)
 : KMenu(parent)
{
    setObjectName(deviceName);
    kDebug() <<"popupNumber" << this;
    addTitle(number);
    this->number=number;
    addAction(KIcon("kaddressbook"), i18n("Call with this mobile phone"), this, SLOT(call() ) );
    addAction(KIcon("mail_generic"), i18n("Send a SMS with this mobile phone"), this, SLOT(newSMS() ) );
    if(KMobileTools::EnginesList::instance()->count() >1 )
    {
        devicesPopup *callsPopup=new devicesPopup(0);
        devicesPopup *smsPopup=new devicesPopup(0);
        callsPopup->setTitle(i18n("Call with...") );
        callsPopup->setIcon(KIcon("kaddressbook") );
        addMenu(callsPopup);
        smsPopup->setTitle(i18n("Send a SMS with...") );
        smsPopup->setIcon(KIcon("mail_generic") );
        addMenu(smsPopup);
        connect(callsPopup, SIGNAL(deviceActivated( const QString& ) ), this, SLOT(call( const QString& ) ) );
        connect(smsPopup, SIGNAL(deviceActivated( const QString& ) ), this, SLOT(newSMS( const QString& ) ) );
    }
}


popupNumber::~popupNumber()
{
}


void popupNumber::newSMS() { newSMS(objectName() ); }

void popupNumber::call() { call(objectName()); }


void popupNumber::call(const QString &deviceName)
{
    kDebug() <<"call:" << deviceName;
#if 0
    DeviceIFace_stub *stub=new DeviceIFace_stub("kmobiletools", deviceName.latin1() );
    stub->raiseDevice();
    stub->raisePage(2);
    stub->slotDialNumber(number);
#endif
}

void popupNumber::newSMS(const QString &deviceName)
{
    kDebug() <<"sms:" << deviceName;
#if 0
    (new DeviceIFace_stub("kmobiletools", deviceName.latin1() ))->slotNewSMS(number);
#endif
}



devicesPopup::devicesPopup( QWidget *parent)
    : KMenu( parent )
{
    sl_devices=KMobileTools::EnginesList::instance()->namesList( false );
    QStringList::Iterator it;
    QAction *curAction;
    int i=0;
    for(it=sl_devices.begin(); it!=sl_devices.end(); ++it)
    {
        if(!DEVCFG(*it)->devicename().length()) continue;
        //if( ! (*it).contains(name) )
        curAction=addAction(KIcon("kmobiletools"), DEVCFG(*it)->devicename() );
        curAction->setObjectName(*it);
        i++;
    }
}

devicesPopup::~devicesPopup()
{
}


void devicesPopup::activated( QAction * action )
{
    kDebug() <<"devicesPopup::activated(" << action->objectName() <<")";
    emit deviceActivated( action->objectName() );
}

#include "popupnumber.moc"

