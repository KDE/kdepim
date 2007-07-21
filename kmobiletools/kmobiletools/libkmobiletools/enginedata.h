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
#ifndef ENGINEDATA_H
#define ENGINEDATA_H

#include <libkmobiletools/kmobiletools_export.h>
#include <QtCore/QObject>
#include <kcal/event.h>

class EngineDataPrivate;
class SMSList;

namespace KMobileTools {
    class ContactsList;
    class Engine;

    /**
    * @author Matthias Lechner
    * @author Marco Gulino
    */
    class KMOBILETOOLS_EXPORT EngineData : public QObject
    {
        Q_OBJECT
        public:
            enum ChargeType { Unknown = -1, Battery = 0, ACAdaptor = 1 };

            /**
             * Creates a new EngineData object.
             * This class can store data from engines, emit signals when it changes,
             * and act as an interface for the GUI.
             *
             * @param parentEngine the engine providing data to store.
             */
            EngineData( Engine *parentEngine );
            ~EngineData();

            /**
            * Retrieves if phone is connected.
            *
            * @return true if phone is connected.
            */
            bool phoneConnected() const;

            /**
            * Sets if phone is connected.
            *
            * @return true if phone is connected.
            */
            void setPhoneConnected(bool);

            /**
             * Returns the phone's signal strength
             * 
             * @return the signal strength in percent
             */
            int signalStrength() const;

            /**
             * Sets the phone's signal strength
             * 
             * @p signalStrength the signal strength in percent
             */
            void setSignalStrength( int signalStrength );

            /**
             * Returns the phone's charge in percent
             * 
             * @return the phone's charge in percent
             */
            int charge() const;

            /**
             * Sets the phone's charge in percent
             * 
             * @p charge the phone's charge in percent
             */
            void setCharge( int charge );

            /**
             * Returns the phone's charge type
             * 
             * @return the phone's charge type
             */
            int chargeType() const;

            /**
             * Sets the phone's charge type
             * 
             * @p chargeType the phone's charge type
             */
            void setChargeType( ChargeType chargeType );

            /**
             * Returns whether the phone is ringing
             * 
             * @return true if the phone is ringing
             */
            bool phoneRinging() const;

            /**
             * Sets whether the phone is ringing
             * 
             * @p chargeType true if the phone is ringing
             */
            void setPhoneRinging( bool ringing );

            /**
             * Returns the network the phone is currently logged in
             * 
             * @return the network name
             */
            QString networkName() const;

            /**
             * Sets the network name the phone is currently logged in
             * 
             * @param networkName the network name
             */
            void setNetworkName( const QString& networkName );

            /**
             * Retrieves the phone manufacturer as returned by the mobile phone.
             *
             * @return a QString containing the phone manufacturer.
             */
            QString manufacturer() const;

            /**
             * Sets phone manufacturer as returned by the mobile phone.
             *
             * @param manufacturer the manufacturer string.
             */
            void setManufacturer( const QString &manufacturer );

            /**
             * Retrieves the manufacturer ID.
             *
             * @return the manufacturer ID
             */
            int manufacturerID() const;

            /**
             * Sets the manufacturer ID.
             *
             * @param manufacturerID the manufacturer id
             */
            void setManufacturerID( int manufacturerID );

            /**
             * Retrieves the phone model.
             *
             * @return the phone model
             */
            QString model() const;

            /**
             * Sets the phone model
             *
             * @param model the phone model
             */
            void setModel( const QString& model );

            /**
             * Retrieves the phone raw IMEI.
             * The IMEI is a number unique to every GSM and UMTS mobile phone.
             *
             * @return the phone raw IMEI
             */
            QString imei() const;

            /**
             * Sets the phone raw IMEI.
             * The IMEI is a number unique to every GSM and UMTS mobile phone.
             *
             * @param imei the phone's imei
             */
            void setIMEI( const QString& imei );

            /**
             * Retrieves the SMS center number.
             *
             * @return the SMS center number
             */
            QString smsCenter() const;

            /**
             * Sets the SMS center number.
             *
             * @param smsc the SMS center number
             */
            void setSMSCenter( const QString& smsc );

            /**
             * Retrieves the phone firmware revision.
             *
             * @return the phone firmware revision
             */
            QString revision() const;

            /**
             * Sets the phone firmware revision.
             *
             * @param revision the phone's firmware revision
             */
            void setRevision( const QString& revision );

            /**
             * Retrieves the phone contact list.
             *
             * @return phone contact list
             */
            ContactsList* contactsList() const;  /// @TODO remove

            /**
             * Sets the phone contact list.
             *
             * @return phone contact list
             */
            void setContactsList( ContactsList* contactsList );  /// @TODO remove

            /**
             * Sets the current list of sms
             *
             * @param smsList the current list of sms
             */
            void setSMSList( SMSList* smsList );

            /**
             * Returns the fetched sms list for reading
             *
             * @return a SMSList object containing all fetched SMS.
             */
            const SMSList* smsList() const;

            /**
             * Sets the phone calendar
             *
             * @param calendar the phone calendar
             */
            void setCalendar( KCal::Event::List *calendar );

            /**
             * Returns the phone's calendar
             *
             * @return the phone calendar
             */
            const KCal::Event::List *calendar();

        Q_SIGNALS:
            /**
            * This signal is emitted when the phone is disconnected.
            */
            void disconnected();

            /**
            * This signal is emitted when the phone is connected.
            */
            void connected();

            /**
            * This signal is emitted whenever the signal strength has changed.
            *
            * @param signalStrength the signal level in percent
            */
            void signalStrengthChanged( int signalStrength );

            /**
            * This signal is whenever the phone charge changes
            *
            * @param charge the charge level in percent
            */
            void chargeChanged( int charge );

            /**
            * This signal is emitted whenever the phone's charge type changed
            *
            * @param chargeType the charge type
            */
            void chargeTypeChanged( ChargeType chargeType );

            /**
            * This signal whenever the phone is ringing
            *
            * @param ringing true if phone is ringing
            */
            void ringing( bool ringing );

            /**
            * This signal is emitted whenever the current network changes
            *
            * @param name the name of the network.
            */
            void networkNameChanged( const QString& name );

            /**
            * This signal is emitted when a SMS is added.
            *
            * @param sms the sms that is added
            */
            void smsAdded( const QString & sms );

            /**
            * This signal is emitted when a SMS is deleted.
            *
            * @param sms the sms that is deleted
            */
            void smsDeleted( const QString & sms );

            /**
            * This signal is emitted when a SMS is modified.
            *
            * @param sms the sms that is modified
            */
            void smsModified( const QString & sms );

            /**
            * This signal is emitted when the mobile's phone book has been changed.
            */
            void phoneBookChanged(); /// @TODO remove

            /**
            * This signal is emitted when calendar is modified.
            *
            */
            void calendarChanged();

        private:
            EngineDataPrivate *const d;
    };
}


#endif


