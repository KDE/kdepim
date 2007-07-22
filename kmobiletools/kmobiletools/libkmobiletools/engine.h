/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>
   by Matthias Lechner <matthias@lmme.de>

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
// #define HAVE_KCAL 1

class KPluginInfo;
class SMSList;
class QWizardPage;

namespace KMobileTools {
class SMS;
class Weaver;
class ContactsList;
class EnginePrivate;
class DevicesConfig;
class EngineData;

/**
 * @author Marco Gulino
 * @author Matthias Lechner
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
    enum PhoneBookMemorySlot { PB_SIM = 2, PB_Phone = 1, PB_DataCard = 4 }; /// @TODO remove

//      enum SMSSlots { SMS_SIM = 0x1, SMS_PHONE = 0x2, SMS_DATACARD = 0x4 }; // krazy:exclude=inline
/*
        enum ProbeType { PROBE_Manufacturer = 0x1, PROBE_Model = 0x2, PROBE_Revision = 0x4,
                         PROBE_IMEI = 0x8, PROBE_SMSSLOTS = 0x10, PROBE_PBSLOTS = 0x20,
                         PROBE_CHARSET = 0x40, PROBE_SMSCENTER=0x80 };*/
        enum DialActions { DIAL_DIAL = 0x1, DIAL_HANGUP = 0x2 }; /// @TODO remove

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
        explicit Engine( QObject *parent = 0, const QString &name = QString() );

        /**
         * Destroys a Engine object.
         */
        virtual ~Engine();

        /**
         * Returns a const pointer to the engine's data for read-only access
         *
         * @return a const pointer to EngineData
         */
        const KMobileTools::EngineData* constEngineData() const;

        /**
         * Returns the engine's ThreadWeaver instance
         *
         * @return the thread weaver instance
         */
        KMobileTools::Weaver *ThreadWeaver(); /// @TODO move to protected

        /**
         * Retrieves the phone SMS folders.
         *
         * @return the phone SMS folders
         */
        QStringList smsFolders(); /// @TODO remove

        /**
         * Retrieves the numbers of phonebook memory slots.
         *
         * @return the numbers of phonebook memory slots
         */ 
        virtual int availPbSlots() = 0;  /// @TODO remove

        /**
         * Set the SMS slot (sim, phone, datacard) to be used.
         *
         * @param slot the slot that be used
         */
        void setSMSSlot( int slot ); /// @TODO remove

        /**
         * Retrieves the SMS slot used.
         *
         * @return the sms slot used.
         */
        int smsSlot(); /// @TODO remove

        /**
         * Shows if mobile phone can encode text in PDU mode.
         *
         * @return true if mobile phone is PDU able.
         */
        virtual bool pdu()=0; /// @TODO remove

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

        virtual QStringList encodings() = 0; /** @TODO remove?! strings should be passed as utf8
                                               * and converted by the engine if needed
                                               */
        int currentPBMemSlot(); /// @TODO remove
        void setCurrentPBMemSlot(int type); /// @TODO remove

        /**
         * Adds the specified @p job to the queue
         *
         * @param job the job to be added to the queue
         */
        void enqueueJob( KMobileTools::Job* job ); /// @TODO move to protected (wizard class can be a friend)

        void suspendStatusJobs(bool suspend); /// @TODO move to protected
        int statusJobsSuspended() const; /// @TODO remove?!
        /**
         * Format a string to be displayed in the final page of the wizard, to show device parameters.
         *
         * @param strtemplate The string to be used as template
         * @param deviceName the internal name of the device, for grabbing settings
         * @return A string with the device information, or a null string if an error occurred
         */
        virtual QString parseWizardSummary(const QString &strtemplate, const QString &deviceName) const=0;
        /**
         * Returns a list of QWizardPage objects to be used in the New Phone Wizard.
         * Each engine should reimplement this to provide different wizard pages depending on the engine settings.
         *
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
         *
         * @param parentWidget the parent widget to be used when creating widgets.
         * @return a QList<QWidget*> object containing configuration pages.
         */
        virtual QList<QWidget*> configWidgets(QWidget *parentWidget)=0;
        /**
         * Convenience method returning a specific KMobileTools::DevicesConfig object with the engine specific configuration items.
         * Calling without parameters return settings for current engine.
         *
         * @param forceNew if true, force creating a new KConfigSkeleton::DevicesConfig object instead of searching for an existing one.
         * @param groupName specify the group of settings to be loaded. If null, engine->objectName() will be used.
         * @return A KConfigSkeleton::DevicesConfig with this engine settings, or NULL in case of errors.
         */
        virtual KMobileTools::DevicesConfig *config( bool forceNew=false, const QString &groupName=QString() )=0;
        /**
         * Convenience method to load an engine object through KLibFactory
         *
         * @return a KMobileTools::Engine object, or a NULL pointer if something went wrong.
         */
        static Engine *load(const QString &libname, QObject *parent=0);

    public Q_SLOTS:
        /** @TODO implement this
            virtual bool changePhoneBookSlot (PhoneBookMemorySlot slot) = 0;
            virtual bool changeSMSMemSlot (SMS::MemorySlot slot ) = 0;
            virtual int parseSMSList() = 0;
            virtual int parseAddressBook() = 0;
        */

        virtual void slotFetchSMS() = 0;  /// @TODO remove
        virtual void slotFetchPhonebook() = 0;  /// @TODO remove
        virtual void slotPollStatus() = 0;
        virtual void slotFetchInfos() = 0;
        virtual void slotDial(DialActions, const QString & =QString() ) = 0; /// @TODO remove
        /**
         * This slot is called when slotSearchPhone has finished probing devices.
         * It will establish a link to the found device, and properly initialize engine communication.
         * Reimplement to correctly initialize engines.
         */
        virtual void slotInitPhone() = 0;

        /**
         * Start searching for our mobile phone.
         * It's the first method to be called when initializing the engine.
         * The slot will look the phone configuraton, searching for the correct device to be initialized.
         * This has to be reimplemented in each engine.
         */
        virtual void slotSearchPhone() = 0;

        virtual void slotAddAddressee(const KABC::Addressee::List&) = 0; /// @TODO remove
        virtual void slotDelAddressee(const KABC::Addressee::List&) = 0; /// @TODO remove
        virtual void slotDelSMS(SMS*) = 0; /// @TODO remove
        virtual void slotStoreSMS(const QString &number, const QString &text) = 0; /// @TODO remove
        virtual void slotSendSMS(const QString &number, const QString &text) = 0; /// @TODO remove
        virtual void slotStoreSMS(SMS*) = 0; /// @TODO remove
        virtual void slotSendSMS(SMS*) = 0; /// @TODO remove
        virtual void slotSendStoredSMS(SMS*) = 0; /// @TODO remove
        virtual void slotEditAddressee(const KABC::Addressee&, const KABC::Addressee&) = 0; /// @TODO remove
        virtual void slotStopDevice(); /// @TODO remove?! method is not actively used anyway
        virtual void slotResumeDevice(); /// @TODO remove?! method is not actively used anyway
        virtual void slotWeaverSuspended(); /// @TODO move to protected

        virtual void slotFetchCalendar() = 0;
        virtual void slotSwitchToFSMode();

    private:
        EnginePrivate *const d;

    protected:
        /**
         * Returns a pointer to the engine's data for read/write access
         *
         * @return the engine's data
         */
        KMobileTools::EngineData* engineData();

    protected Q_SLOTS:
        /**
         * This slot is called whenever an enqueued job has been finished.
         * Reimplement this method to further process the data that the job
         * has dealt with.
         *
         * @param job the job that has finished
         */
        virtual void processSlot( KMobileTools::Job* job );

    Q_SIGNALS:
        /**
         * This signal is emitted when the engine is suspended.
         */
        void suspended(); /// @TODO this signal is needed for dialing only, so probably remove

        /**
         * This signal is emitted when the engine is resumed.
         */
        void resumed(); /// @TODO this signal is needed for dialing only, so probably remove

        /**
         * This signal is emitted when a job is enqueued.
         *
         * @param job the job that has been enqueued
         */
        void jobEnqueued( KMobileTools::Job * job );

        /**
         * This signal is emitted when KMobileTools finishs a task.
         */
        void jobFinished( KMobileTools::Job::JobType );

        /**
         * This signal is emitted when a SMS folder is added
         * 
         * @TODO move this signal to engineData (and add a method to explicitly
         * add and remove sms folders so the signal gets emitted automatically)
         */
        void smsFoldersAdded(); /// @TODO remove
};
}
#endif

