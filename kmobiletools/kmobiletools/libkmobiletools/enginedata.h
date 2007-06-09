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
    class KMOBILETOOLS_EXPORT EngineData : public QObject
    {
        Q_OBJECT
        public:
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
             * Sets phone manufacturer as returned by the mobile phone.
             *
             * @param manufacturer the manufacturer string.
             */
            void setManufacturer( const QString &manufacturer );

            /**
             * Retrieves the phone manufacturer as returned by the mobile phone.
             *
             * @return a QString containing the phone manufacturer.
             */
            QString manufacturer() const;

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
            ContactsList* contactsList() const;

            /**
             * Sets the phone contact list.
             *
             * @return phone contact list
             *
             * @TODO better usage
             */
            void setContactsList( ContactsList* contactsList );

            /**
             * The engine internal list of retrieved SMS
             *
             * @return a SMSList object containing all fetched SMS.
             *
             * @TODO make this method return a const reference
             */
            SMSList* smsList() const;

            /**
             * Retrieves the phone calendar.
             *
             * @return the phone calendar
             *
             * @TODO make this method return a const reference
             */
            KCal::Event::List *calendar();


        private:
            EngineDataPrivate *const d;
    };
}


#endif


