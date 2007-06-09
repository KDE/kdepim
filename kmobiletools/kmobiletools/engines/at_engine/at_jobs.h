/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>,
   Alexander Rensmann <zerraxys@gmx.net>

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

#ifndef ATJOBSS_H
#define ATJOBSS_H

#include <libkmobiletools/enginedata.h>
#include <libkmobiletools/engine.h>
#include <libkmobiletools/sms.h>

#include "sms_defs.h"

#include <q3ptrlist.h>
#include <kabc/addressee.h>
#include <q3ptrvector.h>
#include "atabilities.h"

#define odd(a) ( a % 2 == 1 )
#define even(a) ( a % 2 == 0 )



namespace KMobileTools{
	class SerialManager;
}

class QStringList;
class AT_Engine;

/** \brief Wrapper providing some functions to parse the response to AT commands.
 */
class kmobiletoolsATJob : public KMobileTools::Job
{
    public:
      explicit kmobiletoolsATJob(KMobileTools::SerialManager *device, AT_Engine* parent = 0);
      kmobiletoolsATJob(KMobileTools::Job *pjob, KMobileTools::SerialManager *device, AT_Engine* parent = 0);
        /** Returns a QStringList containing each relevant line in the string buffer.
        * The buffer is split by newlines. Then empty lines and the line containing the
        * closing OK are removed.
        */
        static QStringList formatBuffer( QString buffer );
        /** Returns a QStringList containing each entry in the passed comma separated list.
        * If buffer begins with "+CXXX:" this is ignored. All clinched quotation marks are removed.
        */
        static QStringList parseList( QString list, char begins='C' );
        /** Returns a QStringList containing each entry/list in the nested list. The passed argument
        * is treated as a nested list so each entry in the return argument can be another comma separated
        * list.
        */
        static QStringList parseMultiList( QString list );
        /** Decoes a string using the character encoding given in the configuration.
        */
        QString decodeString( const QString &text );
        /** Decoes a string using the character encoding given in the configuration.
        */
        QString encodeString( const QString &text );

        static QString parseInfo( const QString &buffer );
    protected:
        KMobileTools::SerialManager *p_device;
        AT_Engine *engine;
        int i_retry;
};

class initPhoneJob : public kmobiletoolsATJob
{
    public:
      explicit initPhoneJob( KMobileTools::SerialManager *device, AT_Engine* parent = 0);
        JobType type()            { return KMobileTools::Job::initPhone; }
    protected:
        void run ();
};

class PollStatus : public kmobiletoolsATJob
{
    public:
      PollStatus (KMobileTools::Job *pjob, KMobileTools::SerialManager *device, AT_Engine* parent = 0 );
        int phoneCharge()     { return i_charge; }
        KMobileTools::EngineData::ChargeType phoneChargeType() {
            return m_chargeType;
        }
        int phoneSignal()     { return i_signal; }
        bool ringing()        { return b_calling; }
        JobType type()            { return KMobileTools::Job::pollStatus; }
    protected:
        void run ();
    private:
        int i_charge, i_signal;
        KMobileTools::EngineData::ChargeType m_chargeType;
        bool b_calling;
};

class FetchPhoneInfos : public kmobiletoolsATJob
{
    public:
      FetchPhoneInfos (KMobileTools::Job *pjob, KMobileTools::SerialManager *device, AT_Engine* parent = 0 );
      explicit FetchPhoneInfos (KMobileTools::SerialManager *device, AT_Engine* parent = 0 );
      QString rawManufacturer() const { return s_manufacturer; }
        QString model() const { return s_model; }
        QString imei() const { return s_imei; }
        QString revision() const { return s_revision; }
        QString smsCenter() const { return s_smscenter; }
        JobType type()            { return KMobileTools::Job::fetchPhoneInfos; }

    protected:
        void run () ;
    private:
        QString s_manufacturer, s_model, s_imei, s_revision, s_smscenter;
};

/** \brief Select TE character set
*
* This job uses the command AT+CSCS to select a character set used by the mobile phone to send
* messages.
*/

class SelectCharacterSet : public kmobiletoolsATJob
{
    public:
      SelectCharacterSet( KMobileTools::Job *pjob, const QString &characterSet, KMobileTools::SerialManager *device, AT_Engine* parent = 0);
        JobType type()            { return KMobileTools::Job::selectCharacterSet; }
    protected:
        void run ();
        QString characterSet;
};

/** \brief Select preferred short message storage.
*
* This job uses the command AT+CPMS to select the slot for reading short messages. Short messages
* are read with the job FetchSMS.
*/
class SelectSMSSlot : public kmobiletoolsATJob
{
    public:
      SelectSMSSlot( KMobileTools::Job *pjob, QString slot, KMobileTools::SerialManager *device, AT_Engine* parent = 0);
        JobType type()            { return KMobileTools::Job::selectSMSSlot; }
        const QString getReadSlot() { return readSlot;}
        bool done() { return b_done;}
    protected:
        void run ();
        QString readSlot;
        bool b_done;
};

/** This job tests the abilities of the phone. See ATAbilities for more details.
*/
class TestPhoneFeatures : public kmobiletoolsATJob
{
    public:
      TestPhoneFeatures (KMobileTools::Job *pjob, KMobileTools::SerialManager *device, AT_Engine* parent);
      TestPhoneFeatures (KMobileTools::SerialManager *device, AT_Engine* parent);
      JobType type()            { return KMobileTools::Job::testPhoneFeatures; }
        ATAbilities &getAbilities() { return abilities; }
    protected:
        void run ();
        ATAbilities abilities;
};

/**  \brief Syncs the computers clock to the phones clock.
*
* This job uses the AT+CCLK command to read the time from the phone. If this time
* differs more than two seconds from the system time, the phones time is synchronized
* to the systems time.
*/

class SyncDateTime : public kmobiletoolsATJob
{
    public:
      SyncDateTime( KMobileTools::Job *pjob, KMobileTools::SerialManager *device, AT_Engine* parent = 0 );
        JobType type()            { return KMobileTools::Job::syncDateTimeJob; }
    protected:
        void run ();
};

#endif
