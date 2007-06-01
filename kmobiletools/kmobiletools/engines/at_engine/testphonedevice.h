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
#ifndef TESTPHONEDEVICE_H
#define TESTPHONEDEVICE_H

#include "at_jobs.h"
#include "at_engine.h"

/**
    @author Marco Gulino
*/
class KMobileTools::EngineData;
class TestPhoneDeviceJob : public kmobiletoolsATJob
{
/// This job just check if a device exists, opens it, and try to get the IMEI code from it.
/// It is used to scan a devices range, to find the phone to be initialized in the AT Engine.
Q_OBJECT
public:
    TestPhoneDeviceJob(const QString &devicename, AT_Engine* parent);

    ~TestPhoneDeviceJob();
    virtual JobType type() { return static_cast<JobType>(AT_Engine::TestPhoneDevice); }
    KMobileTools::EngineData *data() { return enginedata; }
    bool found() { return b_found; }
    QString path() const { return deviceName; }
protected:
    virtual void run ();
    QString deviceName;
    KMobileTools::EngineData* enginedata;
    bool b_found;
    bool b_closeafterimei; /// if false, we could subclass run() to extend the job taking other parameters than the imei.
};

class FindDeviceDataJob : public TestPhoneDeviceJob {
Q_OBJECT
/// This one extends the previous class, being able to find more informations about the device.
/// It can be used in the wizard, so the user can also get more data about found phones.
public:
    FindDeviceDataJob(const QString &devicename, AT_Engine* parent);
    JobType type() { return static_cast<JobType>(AT_Engine::FindDeviceData); }
protected:
    void run();
};

#endif
