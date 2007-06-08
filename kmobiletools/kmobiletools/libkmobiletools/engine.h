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

#ifndef KMOBILETOOLSVOIDENGINE_H
#define KMOBILETOOLSVOIDENGINE_H

#include <libkmobiletools/kmobiletools_export.h>

#define PB_SIM_TEXT i18n("Sim PhoneBook")
#define PB_PHONE_TEXT i18n("Mobile PhoneBook")
#define PB_DATACARD_TEXT i18n("Datacard PhoneBook")

#include <QtCore/QByteArray>

#include <kabc/addressee.h>
#include <libkmobiletools/job.h>

/**
@author Marco Gulino
*/

class KPluginInfo;
class SMSList;
class SMS;
class QWizardPage;

namespace KMobileTools {
class Weaver;
class ContactsList;
class EnginePrivate;
class DevicesConfig;
class EngineData;

/**
 * @author Marco Gulino
 */
class KMOBILETOOLS_EXPORT Engine : public QObject
{
    Q_OBJECT

    public:
        /**
         * This enum type defines the type of phone book memory slot.
         *
         * - PB_SIM : Sim card
         * - PB_Phone : Phone
         * - PB_DataCard : Data card
         */
        enum PhoneBookMemorySlot { PB_SIM = 2, PB_Phone = 1, PB_DataCard = 4 };

//      enum SMSSlots { SMS_SIM = 0x1, SMS_PHONE = 0x2, SMS_DATACARD = 0x4 }; // krazy:exclude=inline
/*
        enum ProbeType { PROBE_Manufacturer = 0x1, PROBE_Model = 0x2, PROBE_Revision = 0x4,
                         PROBE_IMEI = 0x8, PROBE_SMSSLOTS = 0x10, PROBE_PBSLOTS = 0x20,
                         PROBE_CHARSET = 0x40, PROBE_SMSCENTER=0x80 };*/
        enum DialActions { DIAL_DIAL = 0x1, DIAL_HANGUP = 0x2 };

        /**
         * This enum type defines the phone manufacturers.
         *
         * - Unknown : Unknown manufacturer
         * - Motorola : Motorola
         * - Siemens : Siemens
         * - SonyEricsson : Sony Ericcson
         * - Nokia : Nokia
         */
        enum ManufacturerEnum { Unknown = 0, Motorola = 1, Siemens = 2,
                                SonyEricsson = 3, Nokia = 4 };

        /**
         * Creates a new Engine object.
         * @param parent the parent QObject
         * @param name the object name
         */
        explicit Engine(QObject *parent = 0, const QString &name = QString());

        /**
         * Destroys a Engine object.
         */
        virtual ~Engine();

        /**
         * Returns a const reference to the engine's data
         *
         * @return a const reference to EngineData
         */
        const KMobileTools::EngineData& constEngineData() const;

        KMobileTools::Weaver *ThreadWeaver();
        /**
         * Retrieves the new SMS number.
         * @return the new SMS number
         */
        int newSMSCount();

        /**
         * Retrieves the total SMS number.
         * @return the total SMS number
         */
        int totalSMSCount();

        SMSList* diffSMSList() const; // @TODO remove?

        /**
         * Retrieves the phone SMS folders.
         * @return the phone SMS folders
         */
        QStringList smsFolders();

        /**
         * Retrieves the numbers of phonebook memory slots.
         * @return the numbers of phonebook memory slots
         */ 
        virtual int availPbSlots() = 0;

        /**
         * Set the SMS slot (sim, phone, datacard) to be used.
         * @param slot the slot that be used
         */
        void setSMSSlot(int slot);

        /**
         * Retrieves the SMS slot used.
         * @return the sms slot used.
         */
        int smsSlot();

        /**
         * Retrieves if phone is connected.
         * @return true if phone is connected.
         */
        bool isConnected();
        /**
         * Shows if mobile phone can encode text in PDU mode.
         * @return true if mobile phone is PDU able.
         */
        virtual bool pdu()=0; /// @TODO check removal

        /**
         * Ask engine to close before calling the destructor.
         */
        virtual void queryClose();
        virtual QString shortDesc();
        virtual QString longDesc();
        /**
         * The engine name provided in the .desktop file
         */


        virtual QString currentDeviceName() const;
        virtual QString engineLibName() const=0;
        KPluginInfo *pluginInfo();
        void enqueueJob(KMobileTools::Job*);

        void setConnected(bool);

        virtual QStringList encodings() = 0;
        int currentPBMemSlot();
        void setCurrentPBMemSlot(int type);
        void suspendStatusJobs(bool suspend);
        int statusJobsSuspended() const;
        /**
         * Format a string to be displayed in the final page of the wizard, to show device parameters.
         * @param strtemplate The string to be used as template
         * @param deviceName the internal name of the device, for grabbing settings
         * @return A string with the device information, or a null string if an error occurred
         */
        virtual QString parseWizardSummary(const QString &strtemplate, const QString &deviceName) const=0;
        /**
         * Returns a list of QWizardPage objects to be used in the New Phone Wizard.
         * Each engine should reimplement this to provide different wizard pages depending on the engine settings.
         * @return a QList<QWizardPage*> with the engine wizard pages, or NULL if the engine does not provide wizard pages.
         */
        virtual QList<QWizardPage*> wizardPages(QWidget *parentWidget)=0;
        /**
         * Setup list of QWidget to be added in a KConfigDialog, handling engine specific configuration options.
         * Be careful to reimplement this setting these properties:
         * - QString itemName
         * - QString pixmapName
         * - QString header
         * as they will be used as parameters for KConfigDialog::addPage().
         * @param parentWidget the parent widget to be used when creating widgets.
         * @return a QList<QWidget*> object containing configuration pages.
         */
        virtual QList<QWidget*> configWidgets(QWidget *parentWidget)=0;
        /**
         * Convenience method returning a specific KMobileTools::DevicesConfig object with the engine specific configuration items.
         * Calling without parameters return settings for current engine.
         * @param forceNew if true, force creating a new KConfigSkeleton::DevicesConfig object instead of searching for an existing one.
         * @param groupName specify the group of settings to be loaded. If null, engine->objectName() will be used.
         * @return A KConfigSkeleton::DevicesConfig with this engine settings, or NULL in case of errors.
         */
        virtual KMobileTools::DevicesConfig *config( bool forceNew=false, const QString &groupName=QString() )=0;
        /**
         * Convenience method to load an engine object through KLibFactory
         * @return a KMobileTools::Engine object, or a NULL pointer if something went wrong.
         */
        static Engine *load(const QString &libname, QObject *parent=0);

    public Q_SLOTS:
        /** TODO implement this
            virtual bool changePhoneBookSlot (PhoneBookMemorySlot slot) = 0;
            virtual bool changeSMSMemSlot (SMS::MemorySlot slot ) = 0;
            virtual int parseSMSList() = 0;
            virtual int parseAddressBook() = 0;
        */

        virtual void slotFetchSMS() = 0;
        virtual void slotFetchPhonebook() = 0;
        virtual void slotPollStatus() = 0;
        virtual void processSlot(KMobileTools::Job* job);
        virtual void slotFetchInfos() = 0;
        virtual void slotDial(DialActions, const QString & =QString() ) = 0;
        /**
         * This slot is called when slotSearchPhone has finished probing devices.
         * It will establish a link to the found device, and properly initialize engine communication.
         * Reimplement to correctly initialize engines.
         */
        virtual void slotInitPhone() = 0;
        /**
         * Start searching for our mobile phone.
         * It's the first method to be called when initializing the engine. The slot will look the phone configuration, searching for the correct device to be initialized.
         * This has to be reimplemented in each engine.
         */
        virtual void slotSearchPhone() = 0;
        virtual void slotAddAddressee(const KABC::Addressee::List&) = 0;
        virtual void slotDelAddressee(const KABC::Addressee::List&) = 0;
        virtual void slotDelSMS(SMS*) = 0;
        virtual void slotStoreSMS(const QString &number, const QString &text) = 0;
        virtual void slotSendSMS(const QString &number, const QString &text) = 0;
        virtual void slotStoreSMS(SMS*) = 0;
        virtual void slotSendSMS(SMS*) = 0;
        virtual void slotSendStoredSMS(SMS*) = 0;
        virtual void slotEditAddressee(const KABC::Addressee&, const KABC::Addressee&) = 0;
        virtual void slotStopDevice();
        virtual void slotResumeDevice();
        virtual void slotWeaverSuspended();
        virtual void slotFetchCalendar() = 0;
        virtual void slotSwitchToFSMode();

    private:
        EnginePrivate *const d;

    protected:
        KMobileTools::EngineData& engineData();

    protected Q_SLOTS:
        virtual void slotDevConnected();
        virtual void slotDevDisconnected();

    Q_SIGNALS:
        /**
         * This signal is emitted when KMobileTools finish a task.
         */
        void jobFinished(KMobileTools::Job::JobType);
        /**
         * This signal is emitted when the phone is disconnected.
         */
        void disconnected();

        /**
         * This signal is emitted when the phone is connected.
         */
        void connected();

        /**
         * This signal is emitted when an error is occurred.
         */
        void error();

        /**
         * This signal is emitted when a new SMS is received.
         */
        void newSMS(SMS*);

        /**
         * This signal is emitted when something is changed in the engine and
         * ask gui to update the page.
         */
        void updateInfoPage(int);

        /**
         * This signal is emitted when a SMS folder is added
         */
        void addSMSFolders();

        /**
         * This signal is emitted every phone poll.
         * @param signal the signal level in percentual.
         */
        void signal(int);

        /**
         * This signal is emitted every phone poll.
         * @param charge the charge level in percentual.
         */
        void charge(int);

        /**
         * This signal is emitted every phone poll.
         * Charge type is 1 when phone is connected to the adapter.
         * @param chargetype the type of charge
         */
        void chargeType(int);

        /**
         * This signal is emitted every phone poll.
         * @param ringing true if phone is ringing
         * @todo emit only if phone is ringing
         */
        void isRinging(bool);

        /**
         * This signal is emitted when the phone book is updating.
         * @todo to be deleted
         */
        void phoneBookUpdated(int, const ContactsList&);

        /**
         * This signal is emitted when the phone book is updated.
         */
        void phoneBookUpdated();

        /**
         * This signal is emitted when SMS list is updated.
         */
        void smsListUpdated();

        /**
         * This signal is emitted every phone infos fetch.
         * @param name the name of the network.
         */
        void networkName( const QString &);

        /**
         * This signal is emitted when is needed a new fetch.
         */
        void addressBookToUpdate();

        /**
         * This signal is emitted when the engine is suspended.
         */
        void suspended();

        /**
         * This signal is emitted when the engine is resumed.
         */
        void resumed();

        /**
         * This signal is emitted when a job is enqueued.
         */
        void jobEnqueued(KMobileTools::Job *);

        /**
         * This signal is emitted when a SMS is added.
         */
        void smsAdded(const QByteArray&);

        /**
         * This signal is emitted when a SMS is deleted.
         */
        void smsDeleted(const QByteArray&);

        /**
         * This signal is emitted when a SMS is modified.
         */
        void smsModified(const QByteArray&);

        /**
         * This signal is emitted when lock file is invalid.
         */
        void invalidLockFile(const QString &);

        /**
         * This signal is emitted when phonebook is full.
         */
        void fullPhonebook();

        /**
         * This signal is emitted when calendar is parsed.
         */
        void calendarParsed();
};
}
#endif

