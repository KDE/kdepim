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
#include "testphonedevice.h"
#include <libkmobiletools/serialdevice.h>
#include <libkmobiletools/enginedata.h>
#include <QFile>

TestPhoneDeviceJob::TestPhoneDeviceJob(const QString &devicename, AT_Engine* parent): kmobiletoolsATJob(NULL, parent),
    b_found(false), enginedata(0), b_closeafterimei(true)
{
    this->deviceName=devicename;
    p_device=new KMobileTools::SerialManager(parent, parent->objectName(), devicename);
}


TestPhoneDeviceJob::~TestPhoneDeviceJob()
{
    delete p_device;
    delete enginedata;
}

void TestPhoneDeviceJob::run() {
    if(! QFile::exists(deviceName)) { return; }
    if(!p_device->open(this)) return;
    enginedata=new KMobileTools::EngineData(0);
    enginedata->setProperty("devicePath", path() );
    QString buffer;
    const int probeTimeout=600;
    buffer=p_device->sendATCommand(this, "AT+CGSN\r", probeTimeout);
    kDebug() << "Sent CGSN probe, got " << buffer << endl;
    if( !KMobileTools::SerialManager::ATError(buffer) ) // Phone imei
        enginedata->setIMEI( kmobiletoolsATJob::parseInfo( buffer ) );
        else {
            p_device->close();
            return;
        }
    if(b_closeafterimei) p_device->close();
    b_found=true;
}

FindDeviceDataJob::FindDeviceDataJob(const QString &devicename, AT_Engine* parent): TestPhoneDeviceJob(devicename, parent)
{
    b_closeafterimei=false;
}

void FindDeviceDataJob::run() {
    TestPhoneDeviceJob::run();
    if(!found()) {
        p_device->close();
        return;
    }
    QString buffer, temp;
    const int probeTimeout=600;

    buffer=p_device->sendATCommand(this, "AT+CGMR\r", probeTimeout);
    if(!KMobileTools::SerialManager::ATError(buffer)) // Phone revision
        enginedata->setRevision( kmobiletoolsATJob::parseInfo( buffer ) );

    buffer=p_device->sendATCommand(this, "AT+CGMI\r", probeTimeout);
    if( !KMobileTools::SerialManager::ATError(buffer) ) // Phone manufacturer
        enginedata->setManufacturer( kmobiletoolsATJob::parseInfo( buffer ) );

    buffer=p_device->sendATCommand(this, "AT+CPMS=?\r", probeTimeout);
    if( !KMobileTools::SerialManager::ATError(buffer) ) // SMS Slots
        temp = kmobiletoolsATJob::parseInfo( buffer );
    else temp.clear();
//     enginedata->setSMSSlots( kmobiletoolsATJob::parseList( temp.replace("AT+CPMS=?", "") ) ); @TODO port

    buffer=p_device->sendATCommand(this, "AT+CPBS=?\r", probeTimeout);
    if( !KMobileTools::SerialManager::ATError(buffer) ) // PhoneBook Slots
        temp = kmobiletoolsATJob::parseInfo( buffer );
    else temp.clear();
//     enginedata->setPbSlots( kmobiletoolsATJob::parseList( temp.replace("AT+CPBS=?", "") ) ); @TODO port

    buffer=p_device->sendATCommand(this, "AT+CGMM\r", probeTimeout);
    if(!KMobileTools::SerialManager::ATError(buffer)) // Phone model
        enginedata->setModel( kmobiletoolsATJob::parseInfo( buffer ) );

    buffer=p_device->sendATCommand(this, "AT+CSCS=?\r", probeTimeout);
    if( !KMobileTools::SerialManager::ATError(buffer) ) // Charset
        temp = kmobiletoolsATJob::parseInfo( buffer );
    else temp.clear();
//     enginedata->setCharsets( kmobiletoolsATJob::parseList( temp ) ); @TODO port

    buffer=p_device->sendATCommand(this, "AT+CSCA?\r", probeTimeout); // SMS Center
    if(!KMobileTools::SerialManager::ATError(buffer))
    {
        temp=kmobiletoolsATJob::parseInfo( buffer);
        QRegExp tempRegexp;
        tempRegexp.setPattern( ".*\"(.*)\".*");
        if(tempRegexp.indexIn(temp)>=0) temp=tempRegexp.cap( 1 ); else temp.clear();
        enginedata->setSMSCenter( temp );
    }
    p_device->close();
}

#include "testphonedevice.moc"
