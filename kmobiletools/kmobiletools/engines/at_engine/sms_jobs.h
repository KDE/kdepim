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

#ifndef SMSJOB
#define SMSJOB

#include <libkmobiletools/engine.h>
#include "at_jobs.h"
#include "atabilities.h"


/** \brief Some special attributes for the SMS class.
 */
class ATSMS : public KMobileTools::SMS
{
    public:
        ATSMS(const QStringList & numbers, const QString & text );
        ATSMS(const QStringList & numbers, const QString & text, const KDateTime & datetime );
        virtual ~ATSMS();
        //! Return true if the this object is part of a multipart message.
        bool isMultiPart() { return concatenated; };
        //! Returns the reference number of this message.
        /** The reference number is extracted from the pdu parsed from the mobile phone
         * and should be the same for all parts of the message.
         */
        uint getReferenceNumber() { return referenceNumber; };
        /** Merges one part of a multipart message into this object. Once the part
         * has been merged into this object, getText() will return the concatenated message.
         * After merging no reference to the passed argument should be kept, since this object will
         * be used later and freed in the destructor.
         */
        void merge( ATSMS *asms );
        //! Return the message part of this sms.
        /** If this message is a multipart message all available parts are concatenated and the
         * full message is returned. Missing parts are marked with the comment [Part x of y missing].
         */
        virtual QString getText() const;
        /** Only works for multipart messages. This method returns true if the part num is present in the
         * list. Warning: This is zero-indiced, i.e. the first part of message is part number 0.
         */
        bool hasPart( uint num ) { return multiPart && multiPart->at(num); };
        /** Only works for multipart messages.This method returns the position of this part in the multipart
         * message. Warning: This is zero-indiced, i.e. the first part of message is part number 0.
        */
        uint getSequenceNumber() { return sequenceNumber-1; };
        static ATSMS *fromSMS(KMobileTools::SMS *);
    protected:
        void setMultiPart( uint refNumber, uint seqNumber, uint mesCount);
        bool concatenated;
        uint referenceNumber, sequenceNumber, maxMessages;
        Q3PtrVector<ATSMS> *multiPart;
    friend class SMSDecoder;
};


/** \brief Helper class for decoding of the sms data.
 *
 * This class implements methods to parse the sms in pdu data.
 * The important method in ths class is decodeSMS( QString ).
 * This method returns an ATSMS object resulting from the given buffer.
 * The buffer is supposed to be a pdu encoded short message according to
 * GSM03.40.
 */
class SMSDecoder
{
    public:
        //! Default constructor.
        SMSDecoder() {};
        ATSMS *decodeSMS( const QString &abuffer, bool incoming );
        ATSMS *decodeSMS( const QString &abuffer, uint index, uint stat );
    protected:
        inline bool hasMR() { return HAS_MR(SMTL); };
        inline bool hasTimeStamp() { return HAS_TIMESTAMP(SMTL); };
        KDateTime datetime;
        QString message;
        int timezone;
        uint referenceNumber, sequenceNumber, maxMessages;
        bool concatenated;
        uint SMTL, DCS;
        uint bitsLeft, userDataLen;
        uint bitBuffer;
        uint getByte();
        inline void skipBytes( uint cnt ) { buffer.remove( 0, 2*cnt ); };
        QString buffer;
        void parseUserDataHeader();
        uint getDecimal();
        QString getNumber( uint len );
        uint get7Bit();
        QString getUserMessage();
        int charset;
};

/** \brief Helper class for encoding of the sms data.
 *
 * This method builds a pdu encoded short message to be stored on the phone
 * or sent via the phone.
 * Until now only 160 character messages are supported.
 */
class SMSEncoder
{
    public:
        SMSEncoder( const QStringList &, const QString & ) {};
        SMSEncoder( const QString &, const QString & );
        int getMessageCount() { return 0; };
        enum encodingType
        { SevenBit=0x1, EightBit=0x2, UCS2=0x4 };
        static QString encodeText(const QString &o_text, int encodingType);
        static QString encodeNumber( const QString &_number );
        static QString encodeSMS( const QString &number, const QString &message );
    protected:

};

/** \brief Fetches the short messages from the phone.
 *
 * This job uses the AT+CGML command to read the messages in pdu form. The messages are
 * appended to the smsList.
 */
class FetchSMS : public kmobiletoolsATJob
{
    public:
        FetchSMS( KMobileTools::Job *pjob, KMobileTools::SMS::SMSType type, KMobileTools::SerialManager *device, bool lastSlot, AT_Engine* parent = 0  );
        SMSList *smsList;
        JobType type()            { return KMobileTools::Job::fetchSMS; }
        bool last() { return b_last;}
    protected:
        SMSDecoder decoder;
        KMobileTools::SMS::SMSType fetchType;
        int index, stat;
        void run();
        bool b_last;
        virtual void addToList( ATSMS *sms );
};

/** \brief Fetches the short messages from the phone.
*
* This job uses the AT+CGML command to read the messages in pdu form. This class
* takes an existing list as argument and updates this list. I.e. it checks the phone for
* unread messages and appends them to the list.
 */
class UpdateSMS : public FetchSMS
{
    public:
      UpdateSMS( KMobileTools::Job *pjob, SMSList *smsList, KMobileTools::SMS::SMSType type, KMobileTools::SerialManager *device, AT_Engine* parent = 0  );
        JobType type() { return KMobileTools::Job::fetchSMS; }
    protected:
        virtual void addToList( ATSMS *sms );
};


class SendStoredSMS : public kmobiletoolsATJob
{
    public:
      SendStoredSMS( KMobileTools::Job *pjob, KMobileTools::SMS *sms, KMobileTools::SerialManager *device, AT_Engine* parent = 0  );
        JobType type() { return KMobileTools::Job::sendStoredSMS; }
    protected:
        void run();
        KMobileTools::SMS *p_sms;
};


/** \brief Sends a short message via the mobile phone.
 */

class SendSMS : public kmobiletoolsATJob
{
    public:
      SendSMS( KMobileTools::Job *pjob, KMobileTools::SMS *sms, KMobileTools::SerialManager *device, AT_Engine* parent = 0  );
      SendSMS( KMobileTools::Job *pjob, const QString& number, const QString& text, KMobileTools::SerialManager *device, AT_Engine* parent = 0  );
        JobType type()            { return KMobileTools::Job::sendSMS; }
        bool succeeded() { return true; };
    protected:
        void run();
        bool pdu;
        bool sendSingleSMS(const QString &number, const QString &text);
        ATSMS *p_sms;
};

/** \brief Stores a short message in the mobile phone's memory.
 */

class StoreSMS : public kmobiletoolsATJob
{
    public:
      StoreSMS( KMobileTools::Job *pjob, KMobileTools::SMS *sms, KMobileTools::SerialManager *device, AT_Engine* parent = 0  );
      StoreSMS( KMobileTools::Job *pjob, const QString &number, const QString &text, KMobileTools::SerialManager *device, AT_Engine* parent = 0  );
        JobType type()            { return KMobileTools::Job::storeSMS; }
        bool succeeded() { return true; };
        int savedIndex() { return i_savedIndex;}
    protected:
        void run();
        int i_savedIndex;
        int storeSingleSMS(const QString &number, const QString &text);
        bool pdu;
        ATSMS *p_sms;
};


/** \brief Delete SMS from memory.
 */

class DeleteSMS : public kmobiletoolsATJob
{
    public:
        DeleteSMS( KMobileTools::Job *pjob, KMobileTools::SMS *sms, KMobileTools::SerialManager *device, AT_Engine* parent = 0  );
        JobType type()            { return KMobileTools::Job::delSMS; }
        bool succeeded() { return true; };
        KMobileTools::SMS *sms() { return p_sms;}
    protected:
        KMobileTools::SMS *p_sms;
        void run();
        bool b_succeeded;
};



#endif
