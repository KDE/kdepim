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
#include "at_scanprogresspage.h"
#include "at_engine.h"
#include "testphonedevice.h"
#include <libkmobiletools/engineslist.h>
#include <libkmobiletools/enginedata.h>
#include "atengineconfig.h"
#include <klocalizedstring.h>

AT_ScanProgressPage::AT_ScanProgressPage(QWidget *parent)
 : ScanProgressPage(parent), totaljobs(0), donejobs(0)
{
}


AT_ScanProgressPage::~AT_ScanProgressPage()
{
}

#include "at_scanprogresspage.moc"



/*!
    \fn AT_ScanProgressPage::isComplete()
 */
bool AT_ScanProgressPage::isComplete() const {
    bool complete=(donejobs!=0 && totaljobs==donejobs);
    kDebug() << "isComplete==" << complete << " (progress=" << donejobs << "/" << totaljobs << ")\n";
    return complete;
}


/*!
    \fn AT_ScanProgressPage::cleanupPage()
 */
void AT_ScanProgressPage::cleanupPage()
{
    kDebug() << "AT_ScanProgressPage::cleanupPage()\n";
    ScanProgressPage::cleanupPage();
    disconnect(engine, SIGNAL(foundDeviceData(FindDeviceDataJob*)), this, SLOT(deviceProbed(FindDeviceDataJob*)) );
    /// @todo implement me
    totaljobs=0;
    l_devices.clear();
}


/*!
    \fn AT_ScanProgressPage::initializePage()
 */
void AT_ScanProgressPage::initializePage() {
    kDebug() << "AT_ScanProgressPage::initializePage()\n";
    wizard()->setProperty("scanprogress_id", wizard()->currentId());
    engine=(AT_Engine*) KMobileTools::EnginesList::instance()->wizardEngine();
    connect(engine, SIGNAL(foundDeviceData(FindDeviceDataJob*)), this, SLOT(deviceProbed(FindDeviceDataJob*)) );
    cfg=(ATDevicesConfig*) DEVCFG(wizard()->objectName() );
    startScan();
}


/*!
    \fn AT_ScanProgressPage::deviceProbed(FindDeviceDataJob*)
 */
void AT_ScanProgressPage::deviceProbed(FindDeviceDataJob* job)
{
    kDebug() << "Job done: " << job->path() << "; found: " << job->found() << endl;
    if(job->found()) l_devices+=job->data();
    donejobs++;
    setProgress( (donejobs*100)/totaljobs);
    emit completeChanged();
    if(donejobs && (donejobs==totaljobs) ) {
        setStatusString( i18nc("Wizard - probe finished", "All devices were analyzed.") );
        wizard()->next();
    }
        else setStatusString( i18nc("Wizard - probed device..", "%1 done.", job->path() ) );
}


/*!
    \fn AT_ScanProgressPage::startScan()
 */
void AT_ScanProgressPage::startScan()
{
    donejobs=0;
    if(!cfg->at_connections() ) return;
    QStringList devices;
    TestPhoneDeviceJob *curjob;
    if(cfg->at_connections() & AT_Engine::ConnectionUSB)
        for(uchar i=0; i<10; i++) {
            engine->enqueueJob( new FindDeviceDataJob(QString("/dev/ttyACM%1").arg(i), engine) );
            engine->enqueueJob(new FindDeviceDataJob(QString("/dev/ttyUSB%1").arg(i), engine) );
            totaljobs+=2;
        }
    if(cfg->at_connections() & AT_Engine::ConnectionIrDA)
        for(uchar i=0; i<10; i++) {
            engine->enqueueJob(new FindDeviceDataJob(QString("/dev/ircomm%1").arg(i), engine) );
            totaljobs++;
        }
    if(cfg->at_connections() & AT_Engine::ConnectionSerial)
        for(uchar i=0; i<10; i++) {
            engine->enqueueJob(new FindDeviceDataJob(QString("/dev/ttyS%1").arg(i), engine) );
            totaljobs++;
        }
/*    if(cfg->at_connections() & ConnectionBluetooth)
        for(QStringList::ConstIterator it=bluetoothDevices.begin(); it!=bluetoothDevices.end(); ++it)
            engine->enqueueJob(new FindDeviceDataJob(*it, engine) );*/
    if(cfg->at_connections() & AT_Engine::ConnectionUser)
        for(QStringList::ConstIterator it=cfg->at_userdevices().begin(); it!=cfg->at_userdevices().end(); ++it) {
            engine->enqueueJob(new FindDeviceDataJob(*it, engine) );
            totaljobs++;
        }
}
