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
#ifndef KMOBILETOOLSAT_ENGINE_H
#define KMOBILETOOLSAT_ENGINE_H

#include <klibloader.h>

#include <config-kmobiletools.h>

#ifdef HAVE_KCAL
#include "calendar_jobs.h"
#endif

#include "atabilities.h"
#include "atengineconfig.h"

#include <libkmobiletools/engine.h>
#include <libkmobiletools/serialdevice.h>
//Added by qt3to4:
#include <Q3ValueList>
/*class Device;*/
class QWizardPage;
class ATDevicesConfig;
class TestPhoneDeviceJob;
class FindDeviceDataJob;
/**
@author Marco Gulino
*/
using namespace ThreadWeaver;

typedef QHash<QString,KMobileTools::EngineData> ATDevices;

class AT_Engine : public KMobileTools::Engine
{
Q_OBJECT
public:
    explicit AT_Engine(QObject *parent = 0, const QString &name = QString() );

    ~AT_Engine();
    enum ATJobTypes { TestPhoneDevice=KMobileTools::Job::UserJob+1, FindDeviceData=KMobileTools::Job::UserJob+2 };
//     enum jobTypes
//     { PollStatus=1, PollSMS=2 };

        void setATAbilities( ATAbilities atAbilities );
        ATAbilities getATAbilities() const;
        int availPbSlots();
    static QString getPBMemSlotString(int memslot);
    void queryClose();
    bool pdu();
    QString currentDeviceName() const;
    QString engineLibName() const;
    void setDevice ( const QString &deviceName);
    QList<QWizardPage*> wizardPages(QWidget *parentWidget);
    QList<QWidget*> configWidgets(QWidget *parentWidget);
    /*!
        \fn AT_Engine::encodings()
     */
    QStringList encodings()
    {
        return atAbilities.getCharacterSets();
    }
    QString parseWizardSummary(const QString &strtemplate, const QString &deviceName) const;
    ATDevicesConfig *config(bool forceNew=false, const QString &groupName=QString() );
    private:
        /** A collection of all abilities supported by the phone
        */
        ATAbilities atAbilities;
        KMobileTools::SerialManager *device;
        bool queue_sms;
        /// @TODO remove this
        QStringList initStrings();
        KMobileTools::Job *p_lastJob;
        void searchPhones(ATDevicesConfig::Connection connections, const QStringList &bluetoothDevices, const QStringList &customDevices);
        QList<TestPhoneDeviceJob*> l_testphonejobs;
        void enqueueTPJob(TestPhoneDeviceJob*);

public slots:
    void slotPollStatus();
    void processSlot(KMobileTools::Job* );
    void slotFetchInfos();
    void slotInitPhone();
    void slotFetchPhonebook();
    void slotFetchSMS();
    void slotAddAddressee(const KABC::Addressee::List&);
    void slotDelAddressee(const KABC::Addressee::List&);

    void slotEditAddressee(const KABC::Addressee& p_oldAddressee, const KABC::Addressee& p_newAddressee);
    void slotWeaverSuspended();
    void slotResumeDevice();
    void slotStopDevice();
    static void closeDevice(KMobileTools::SerialManager *dev)
    {
        if(!dev) return;
        dev->close();
    }
    void closeDevice() { AT_Engine::closeDevice(device); }
    void slotDelSMS(SMS* sms);
    void slotSendStoredSMS(SMS*);
    void slotStoreSMS(const QString &number, const QString &text);
    void slotSendSMS(const QString &number, const QString &text);
    void slotSearchPhone();
    void slotSendSMS(SMS*);
    void slotStoreSMS(SMS*);
//     void invalidLockFile(const QString &);
    void slotDial(DialActions, const QString & =QString() );


protected slots:
    void slotFetchCalendar();
    void slotSwitchToFSMode();

private slots:
    void connectionStateChanged();

signals:
    void foundDeviceData(FindDeviceDataJob*);
};


class AT_EngineFactory : public KLibFactory
{
   Q_OBJECT
public:
    AT_EngineFactory();
    virtual ~AT_EngineFactory();
    virtual AT_Engine* createObject( QObject *parent, const char *classname, const QStringList &args );

private:
};


#endif
