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

#include "sms_jobs.h"
#include "at_engine.h"
#include <libkmobiletools/serialdevice.h>
#include <qregexp.h>
#include <Q3MemArray>
#include <math.h>
#include <qdatetime.h>
#include <libkmobiletools/encodingshelper.h>
#include <libkmobiletools/kmobiletoolshelper.h>
#include <libkmobiletools/smslist.h>
#include <klocale.h>


/*
 * class ATSMS
 * Description:
 * Some special attributes for the SMS class
 */

ATSMS::ATSMS(const QStringList & numbers, const QString & text ) : SMS ( numbers, text )
{
    concatenated=false;
    multiPart=NULL;
}

ATSMS::ATSMS(const QStringList & numbers, const QString & text, const QDateTime & datetime ) : SMS ( numbers, text, datetime ) {
    concatenated=false;
    multiPart=NULL;
}

ATSMS::~ATSMS()
{
    /* Clean up */
    if ( concatenated && multiPart )
    {
        for ( uint i=0; i<multiPart->count(); i++ )
            if ( multiPart->at(i) && multiPart->at(i)!=this )
                delete multiPart->at(i);
        delete multiPart;
    }
}

ATSMS *ATSMS::fromSMS( SMS *sms)
{
    ATSMS *atsms=(ATSMS*) sms;
    return atsms;
}

void ATSMS::setMultiPart( uint refNumber, uint seqNumber, uint mesCount)
{
    concatenated = true;
    referenceNumber = refNumber;
    sequenceNumber = seqNumber;
    maxMessages = mesCount;
    multiPart = new Q3PtrVector<ATSMS>( mesCount );
    multiPart->insert( seqNumber-1, this );
}

void ATSMS::merge( ATSMS *asms )
{
    if ( !concatenated ) return;
    for(QList<int>::Iterator it=asms->idList()->begin(); it!=asms->idList()->end(); ++it)
        idList()->append(*it);
    multiPart->insert( asms->sequenceNumber-1, asms );
}

QString ATSMS::getText() const
{
    if ( !concatenated ) return SMS::getText();
    QString text;
    for ( uint i=0; i<multiPart->count(); i++ )
    {
        if ( (*multiPart)[i] )
            text.append( multiPart->at(i)->getText() );
        else
            text.append( QString("[Part %1 of %2 missing]").arg(i+1).arg(multiPart->count()+1 ));
    }
    return text;
}

/*
 * class SMSDecoding
 * Description:
 * Helper class for decoding of the sms data
 */

ATSMS *SMSDecoder::decodeSMS( const QString &abuffer, uint index, uint stat )
{
    ATSMS *sms = decodeSMS( abuffer, (stat<2) );
    sms->idList()->append( index);
    switch( stat ){
        case 0:
            sms->setType(SMS::Unread); break;
        case 1:
            sms->setType(SMS::Read); break;
        case 2:
            sms->setType(SMS::Unsent); break;
        case 3:
            sms->setType(SMS::Sent); break;
        case 4:
            sms->setType(SMS::All); break;
    }
//     sms->setType( (SMS::SMSType)stat );
    return sms;
}

ATSMS *SMSDecoder::decodeSMS( const QString &abuffer, bool incoming )
{
    buffer = abuffer;
    // Reset variables
    concatenated = false;

    // SCA
    uint scaSize = getByte();
    QString scaNumber;
    if ( scaSize> 1) scaNumber = getNumber( scaSize*2-2 );

    // SM-TL header
    SMTL = getByte();

    // Message Reference
    uint MR = 0;
    if ( /*hasMR()*/ !incoming )
        MR = getByte();
    // Number
    uint numberLen = getByte();
    QString number = getNumber( numberLen );
    // Protocol Identifier
//    uint PID = getByte();
    skipBytes( 1 ); /* avoid warning */

    // Data Coding Scheme
    DCS = getByte();
    charset = SMSGSMCharset;/*
    if ( DCS & 0xC8 == 0x00 ) charset = SMSGSMCharset;
    if ( DCS & 0xC8 == DCSUCS2Encoding ) charset = SMSUCS2Charset;*/
    charset=(DCS >> 2) & 11;
    charset++;
    kDebug() << "SMS Charset: " << DCS << "; " << charset << endl;

    // Validity Period
    uint VP = 0;
    if ( (!incoming) && ( (bool) HAS_VP(SMTL) ) ) {
        VP = getByte();
    }
    // Timestamp
    timezone = 0;
    if ( incoming && hasTimeStamp() )
    {
        int y = getDecimal();
        int m = getDecimal();
        int d = getDecimal();
        datetime.setDate( QDate( 2000+y, m, d ) );
        int h = getDecimal();
        int mm = getDecimal();
        int s = getDecimal();
        datetime.setTime( QTime( h, mm, s ) );
        timezone = getByte();
    }
    // Message
//     message = "";
    userDataLen = getByte();
    bitsLeft = 0;
    bitBuffer = 0;
    // User data header
    if ( HAS_USERDATAHEADER(SMTL) )
        parseUserDataHeader();
    kDebug() << "Raw message:" << buffer << endl;
//     while( userDataLen>0 )
//         message.append( getUDChar() );
    message=getUserMessage();

    ATSMS *sms = new ATSMS( QStringList() << number, message, datetime );
    if ( concatenated )
    {
        sms->setMultiPart( referenceNumber, sequenceNumber, maxMessages );
    }

    return sms;
}

void SMSDecoder::parseUserDataHeader()
{
    uint udhlen = getByte();
    uint udhrem = udhlen;
    while ( udhrem>2 )
    {
        uint identifier = getByte();
        uint len = getByte();
        if ( udhrem<2+len )
            break;
        udhrem -= 2+len;
        switch ( identifier )
        {
            case UDHIConcatenated8BitReference:
                if ( len!= 3 ) break;
                concatenated = true;
                referenceNumber = getByte();
                maxMessages = getByte();
                sequenceNumber = getByte();
                break;
            default:
                skipBytes( len );
                break;
        }
    }
    if ( udhrem!=0 )
    {
        kDebug() << "Spurious SMS (trailing characters parsing user data header)" << endl;
    }

    // recalculate user data len
    switch ( charset )
    {
        case SMSGSMCharset:
            bitsLeft = ( (udhlen+1)*8 ) % 7;
            userDataLen -= ((udhlen+1)*8+6)/7;
            if ( bitsLeft!=0 )
            {
                bitBuffer = getByte() >> (7-bitsLeft);
                bitsLeft = bitsLeft + 1;
            }
            break;
        case SMSUCS2Charset:
            kDebug() << "UCS2 header\n";
            bitsLeft = ( (udhlen+1)*8 ) % 16;
            userDataLen -= ((udhlen+1)*8+6)/16;
            if ( bitsLeft!=0 )
            {
                bitBuffer = getByte() >> (16-bitsLeft);
                bitsLeft = bitsLeft + 1;
            }
            break;
        default:
            kDebug() << "Fixme: Unsupported character encoding (SMS: " << ( DCS & DCSAlphabetMask ) << ")" << endl;
    }
}

uint SMSDecoder::get7Bit()
{
    userDataLen--;
    while ( bitsLeft<7 )
    {
        bitBuffer = bitBuffer | ( getByte() << bitsLeft );
        bitsLeft += 8;
    }
    uint res = ( bitBuffer & 127 );
    bitsLeft -= 7;
    bitBuffer = bitBuffer >> 7;
    return res;
}

QString SMSDecoder::getUserMessage()
{
    Q3MemArray<QChar> parsedBuffer;
    QString out;
    int i=0;
    switch (charset)
    {
        case SMSGSMCharset:
            kDebug() << "Decoding from GSM Charset\n";
            while( userDataLen>0 )
            {
                i++;
                parsedBuffer.resize( i);
                parsedBuffer[i-1]=get7Bit();
            }
            return KMobileTools::EncodingsHelper::decodeGSM( parsedBuffer );
        case SMS8BitCharset:
            kDebug() << "Decoding from 8 BIT Charset\n";
            return KMobileTools::EncodingsHelper::from8bit( buffer );
        case SMSUCS2Charset:
            kDebug() << "Decoding from UCS2 16BIT Charset\n";
            return KMobileTools::EncodingsHelper::fromUCS2( buffer );
        default:
            kDebug() << "Fixme: Unsupported character encoding (SMS: " << ( DCS & DCSAlphabetMask ) << ")" << endl;
            return i18n("Unsupported character encoding");
    }
}

uint SMSDecoder::getByte()
{
    if ( buffer.length()<2 )
    {
        buffer.clear();
        return 0;
    }
    uint result = buffer.left( 2 ).toInt( 0, 16 );
    buffer.remove( 0, 2 );
    return result;
}

// Extracts number as described in GSM03.40, 9.1.2.4
QString SMSDecoder::getNumber( uint len )
{
    // Type of number
    enum TypeOfNumber { TONUnknown = 0, TONInternational = 0x10, TONNational = 0x20,
        TONNetworkSpecificNumber = 0x30, TONSubscriberNumber = 0x40, TONAlphaNumeric = 0x50,
        TONAbbreviatedNumber = 0x60, TONReserved = 0x70 };
        const uint TypeOfNumberMask = 0x70;

    // Numbering plan, not used yet
    /*
        enum NumberingPlan { NPUnknown = 0x00, NPISDNTelephon = 0x01, NPData = 0x03, NPTelex = 0x04,
        NPNational = 0x08, NPPrivate = 0x09, NPErmes = 0x0A, NPReserved = 0x0F };
        uint NumberingPlanMask = 0x0F;
    */
        QString number;
        uint typeOfAddress = getByte();
        if ( ( typeOfAddress & TypeOfNumberMask ) == TONInternational ) number = "+";
        if ( ( typeOfAddress & TypeOfNumberMask ) == TONAlphaNumeric )
        {
            if(len%2) len++;
            bitsLeft = 0;
            bitBuffer = 0;
            QString oldbuffer=buffer.mid(len);
            int olduserDataLen=userDataLen;
            buffer=buffer.left( len );
            userDataLen=0;
            Q3MemArray<QChar> parsedBuffer;
            int i=0;
            while( buffer.length() )
            {
                i++;
                parsedBuffer.resize( i);
                parsedBuffer[i-1]=get7Bit();
            }
            buffer=oldbuffer;
            userDataLen=olduserDataLen;
            number=KMobileTools::EncodingsHelper::decodeGSM( parsedBuffer );
            return number;
        }
        for ( uint i=0; i<len; i += 2 )
        {
            switch ( ( typeOfAddress & TypeOfNumberMask ) )
            {
                int dat;
                case TONAlphaNumeric:
                    dat = get7Bit();
                    number = number + QChar( dat );
                    break;
                default:
                    dat = getByte();
                    number = number + QString::number( dat & 15 );
                    if ( len-i>1 ) number = number + QString::number( dat / 16  );
                    break;
            };
        }
        return number;
}

uint SMSDecoder::getDecimal()
{
    uint i = getByte();
    return ( 10*(i & 15) + ( i / 16 ) );
}

SendStoredSMS::SendStoredSMS ( KMobileTools::Job *pjob, SMS *sms, KMobileTools::SerialManager *device, AT_Engine* parent )
    : kmobiletoolsATJob( pjob, device, parent )
{
    p_sms=sms;
    engine->suspendStatusJobs( true );
}

void SendStoredSMS::run()
{
    QString buffer=p_device->sendATCommand(this,  QString("AT+CPMS=\"%1\"\r").arg(p_sms->rawSlot() ) );
    if(KMobileTools::SerialManager::ATError(buffer))return;
    for (QList<int>::Iterator it=p_sms->idList()->begin(); it!=p_sms->idList()->end(); ++it)
    {
        buffer==p_device->sendATCommand(this,  QString("AT+CMSS=%1\r").arg( *it ), 10000 );
//         idlist+=QString("%1, ").arg(*it);
    }
}

/*
 * class FetchSMS
 * Description:
 * Fetches the sms
 */

FetchSMS::FetchSMS( KMobileTools::Job *pjob, SMS::SMSType type, KMobileTools::SerialManager *device, bool lastSlot, AT_Engine* parent ) : kmobiletoolsATJob( pjob, device, parent )
{
    fetchType = type;
    smsList=new SMSList();
    b_last=lastSlot;
}

void FetchSMS::run()
{
    int sms_Slot;
    QString buffer;
    buffer=p_device->sendATCommand(this,  "AT+CPMS?\r" );
    buffer=kmobiletoolsATJob::parseInfo( buffer);
    QRegExp CMGL;
    CMGL.setPattern( ".*([A-Z][A-Z]).*([A-Z][A-Z]).*([A-Z][A-Z]).*" );
    QString in_memslot, memslot2, memslot3;
    if(CMGL.indexIn( buffer )>=0)
    {
        in_memslot=CMGL.cap( 1 );
        memslot2=CMGL.cap( 2);
        memslot3=CMGL.cap( 3 );
    }
    if (in_memslot=="ME" || in_memslot== "MT" || in_memslot=="OM" || in_memslot=="IM") sms_Slot= SMS::Phone;
    if (in_memslot=="SM" || in_memslot== "MT" || in_memslot=="OM" || in_memslot=="IM") sms_Slot= SMS::SIM;
//     kDebug() << "Current SMS Slot:" << in_memslot << ";" << memslot2 << ";" << memslot3 << ";\n";
    
    // Get the slot number
    int numSlot;
    switch (fetchType) {
        case SMS::Unread:
            numSlot = 0;
            break;
        case SMS::Read:
            numSlot = 1;
            break;
        case SMS::Unsent:
            numSlot = 2;
            break;
        case SMS::Sent:
            numSlot = 3;
            break;
        default:
            numSlot = 4;
    }
    
    // Fetch the sms
    if ( engine->getATAbilities().canSMGL() )
    {
        buffer = p_device->sendATCommand(this,  "AT^SMGL="+QString::number( numSlot )+"\r", 10000);
    }
    else
        if ( engine->getATAbilities().isPDU() )
            buffer = p_device->sendATCommand(this,  "AT+CMGL="+QString::number( numSlot )+"\r" , 10000);
        else
            buffer = p_device->sendATCommand(this,  "AT+CMGL=\""+SMS::SMSTypeString( fetchType  )+"\"\r", 10000 );

//     if ( KMobileTools::SerialManager::ATError(buffer) ) return;
    kDebug() << "Buffer:\n" << buffer << endl;
    // format
    QStringList tempbuffer = kmobiletoolsATJob::formatBuffer( buffer );
     if( engine->getATAbilities().isPDU() )
         CMGL.setPattern( "^(?:\\+CMGL|\\^SMGL):\\s*(\\d+),([0-4]),(.*),(\\d+)\\s*$" );
     else
//         CMGL.setPattern("^(\\+CMGL|\\^SMGL)[\\s]*:(.*),\"(.*)\",\"(.*)\",*(.*)$");
         CMGL.setPattern( "^(\\+CMGL|\\^SMGL)[\\s]*:[\\s]*([\\d]*)[\\s]*,[\\s]*\"*([A-Za-z\\s]*)\"*,[\\s]*\"*([+\\d]*)\"*,*\"*([\\d+:,/-]*)\"*$" );

    QStringList::Iterator it = tempbuffer.begin();
    kDebug() << "Decoding sms list: tempbuffer==" << endl;
    while ( it != tempbuffer.end() )
    {
        if (engine->getATAbilities().isPDU() )
        {
                if ( CMGL.indexIn( *it )==0 )
                {
                index = CMGL.cap(1).toInt();
                stat = CMGL.cap(2).toInt();
                ATSMS *sms = decoder.decodeSMS( *(++it), index, stat );
//                 sms->idList()->append( index );
                sms->setRawSlot(in_memslot);
                sms->setSlot( sms_Slot );
//                 sms->setUID(index + (engine->smsSlot() * 0x100) );
//                 sms->setText( sms->getText().prepend( QString("UID=").append(QString::number(sms->uid() )).append(";") ) );
//                 kDebug() << "New sms MD5SUM: " << sms->uid() << endl;
                addToList( sms );
                };
                ++it;
        } else
        {
//             kDebug() << "Regexp searching..." << CMGL.search( *it ) << endl;
                if (CMGL.indexIn(*it) == 0 )
                {
                    index = CMGL.cap(2).toInt();
                    QString perhapsdate=CMGL.cap( 5 );
                    stat = SMS::SMSIntType(CMGL.cap(3).trimmed().replace("\"","") );
                    kDebug() << CMGL.cap(1) << "<-->" << CMGL.cap(2) << "(" << index << ")<-->" << CMGL.cap(3) << "(" << stat << ")<-->" << CMGL.cap(4) << endl;
                    ATSMS *sms = new ATSMS( QStringList( decodeString( CMGL.cap(4)) ) , decodeString(*(++it) ) );
                    sms->setRawSlot(in_memslot);
                    sms->idList()->append( index );
                    sms->setSlot( sms_Slot );
                    sms->setType( (SMS::SMSType) stat);
                    QString header=*it;
                    if( !perhapsdate.contains( '/' ) )
                    {
                        QRegExp CMGR;
                        buffer=p_device->sendATCommand(this,  QString("AT+CMGR=%1\r").arg(index), 5000 );
                        QStringList tempSL=kmobiletoolsATJob::formatBuffer( buffer );
                        QStringList::Iterator j;
                        CMGR.setPattern( "^\\+CMGR:[\\s]*\"*[A-Z\\s]*\"*,[\\s]*\"*[\\+\\d]*\"*,*[\\s]*\"*([\\d/,+:]*)\"*$" );
//                         kDebug() << "CMGR search pattern:" << CMGR.pattern() << endl;
                        for(j=tempSL.begin(); j!=tempSL.end(); ++j)
                        {
                            if(CMGR.indexIn( *j )!=-1)
                            {
                                header=*j;/*
                                kDebug() << CMGR.capturedTexts().join("\n") << endl;
                                kDebug() << "Date/time detected:" << CMGR.cap(1) << endl;*/
                                perhapsdate=CMGR.cap(1);
                                break;
                            }
                        }
                    }
                    perhapsdate=perhapsdate.replace("\"", "");
                    QStringList parsedDate=perhapsdate.split( ',', QString::SkipEmptyParts );
                    QString tdate=parsedDate.filter( "/", Qt::CaseSensitive ).first();
                    QString ttime=parsedDate.filter( ":", Qt::CaseSensitive ).first();
//                     date=date.replace( '/', '-' );
                    QString date="%1-%2-%3";
                    date=date.arg( tdate.section( '/', 0, 0 ), 4)
                            .arg( tdate.section( '/', 1, 1 ), 2 )
                            .arg( tdate.section( '/', 2, 2 ), 2 ).replace( ' ', '0' );
                    if(date.at(0) == '0' ) date.replace(0,1,'2');
                    if(date.indexOf( '-') <3 ) date=date.prepend( "20" );
                    QString time="%1:%2:%3";
                    time=time.arg( ttime.section( ':', 0, 0 ) ,2 )
                            .arg( ttime.section( ':', 1, 1 ) ,2 )
                            .arg( ttime.section( ':', 2, 2  ),2 )
                            .replace(' ', '0');
                    sms->setDateTime( QDateTime(
                            QDate::fromString( date, Qt::ISODate ),
                    QTime::fromString( time )
                                               ) );
                    addToList(sms);
                };
                ++it;
        }
    }
}

void FetchSMS::addToList( ATSMS *sms )
{
    if (smsList->find( sms->uid() ) >=0 ) return;
//     kDebug() << "FetchSMS::addToList(" << sms->uid() << ")\n";
    if ( sms->isMultiPart() )
    {
        ATSMS *smsit=0;
        for ( int i=0; i<smsList->size(); i++)
        {
            smsit = (ATSMS*) smsList->at(i);
            if ( smsit->isMultiPart() & smsit->getReferenceNumber()==sms->getReferenceNumber() )
            {
                smsit->merge( sms );
                break;
            }
        }
        if ( !smsit )
            smsList->append( sms );
    } else {
        smsList->append( sms );
    }
}

/*
 * class UpdateSMS
 */
UpdateSMS::UpdateSMS( KMobileTools::Job *pjob, SMSList *smsList, SMS::SMSType type, KMobileTools::SerialManager *device, AT_Engine* parent ) : FetchSMS( pjob, type, device, true, parent )
{
    this->smsList = smsList;
}

void UpdateSMS::addToList( ATSMS *sms )
{
    // check if this message is already in the list
    for ( int i=0; i<smsList->size(); i++)
        if ( smsList->at(i)->idList()==sms->idList() )
        {
            delete sms;
            return;
        }

    if ( sms->isMultiPart() )
    {
        ATSMS *smsit = 0;
        for ( int i=0; i<smsList->size(); i++ )
        {
            smsit=(ATSMS*) smsList->at(i);
            if ( smsit->isMultiPart() & smsit->getReferenceNumber()==sms->getReferenceNumber() )
            {
                if ( smsit->hasPart( sms->getSequenceNumber() ) )
                {
                    delete sms;
                    return;
                }
                smsit->merge( sms );
                break;
            }
        }
        if ( !smsit )
            smsList->append( sms );
    } else {
        smsList->append( sms );
    }
}

/*
 * class SendSMS
 */

SendSMS::SendSMS( KMobileTools::Job *pjob, const QString &number, const QString &text, KMobileTools::SerialManager *device, AT_Engine* parent ) : kmobiletoolsATJob( pjob, device, parent )
{
    engine->suspendStatusJobs( true );
    pdu=engine->getATAbilities().isPDU();
    p_sms=new ATSMS( QStringList() << number, text);
    p_sms->setType( SMS::Unsent );
}

SendSMS::SendSMS( KMobileTools::Job *pjob, SMS *sms, KMobileTools::SerialManager *device, AT_Engine* parent ) : kmobiletoolsATJob( pjob, device, parent )
{
    engine->suspendStatusJobs( true );
    pdu=engine->getATAbilities().isPDU();
    p_sms=ATSMS::fromSMS( sms );
}

void SendSMS::run()
{
    p_device->lockMutex();
    QStringList::iterator num, txt;
    QStringList numbers=p_sms->getTo();
    QStringList texts=p_sms->getMultiText();
    for(num=numbers.begin(); num!=numbers.end(); ++num)
    {
        for(txt=texts.begin(); txt!=texts.end(); ++txt)
            sendSingleSMS( (*num), (*txt) );
    }

    p_device->unlockMutex();
}


bool SendSMS::sendSingleSMS(const QString &number, const QString &text)
{
    QString buffer;
    if(engine->config()->smscenter().length() )
    {
        /// @TODO move this to a more generic sms job..
        buffer=p_device->sendATCommand(this, "AT+CSCA?\r" );
        QString smscenter=kmobiletoolsATJob::parseInfo( buffer);
        QRegExp tempRegexp;
        tempRegexp.setPattern( ".*\"(.*)\".*");
        if(tempRegexp.indexIn(smscenter)>=0) smscenter=tempRegexp.cap( 1 ); else smscenter.clear();
        kDebug() << "*************** SMS Center found:" << smscenter << "\\n" << endl;
        if(! KMobileTools::KMobiletoolsHelper::compareNumbers(smscenter, engine->config()->smscenter()) )
        {
            kDebug() << "It seems that SMS center is NOT correct on your phone, setting it...\n";
            buffer=p_device->sendATCommand(this,  QString("AT+CSCA=\"%1\"\r")
                    .arg( encodeString( engine->config()->smscenter() ) ) );
            buffer=p_device->sendATCommand(this,  "AT+CSCA?\r" );
            if(! buffer.contains(  encodeString(engine->config()->smscenter() ) ) )
            kDebug() << "SendSMS::run() ********** WARNING ******* Could not set SMS Center\n";
        }
    }
    if(pdu)
    {
        QString pduSMS=SMSEncoder::encodeSMS( number, text );
        kDebug() << "SendSMS::run() pduLength=" << pduSMS.length() << endl;
        buffer=p_device->sendATCommand(this,  QString("AT+CMGS=%1\r").arg((pduSMS.length()/2 )-1),5  );
        buffer=p_device->sendATCommand(this,  pduSMS.append( "\x1A") );
        kDebug() << "SendSMS::run() buffer saved:" << buffer << ";\n";
    }
    else {
        buffer=p_device->sendATCommand(this,  QString("AT+CMGS=\"%1\"\r").arg(encodeString(number)), 5 );
        if( KMobileTools::SerialManager::ATError(buffer) && engine->config()->at_encoding().contains("UCS2", Qt::CaseSensitive ) )
            buffer=p_device->sendATCommand(this,  QString("AT+CMGS=\"%1\"\r").arg((number)) ); // resend command without encoding phonenumber for buggy phones (like mine.. :(

//         if ( KMobileTools::SerialManager::ATError(buffer) ) return;
        buffer=p_device->sendATCommand(this,  QString("%1\x1A").arg(encodeString(text)) , 10000  );
        kDebug() << "SendSMS::run() done; result=" << buffer << endl;
//         QRegExp CMGW;
//         CMGW.setPattern(".*\\+CMGW[\\s]*:[\\s*]([\\d]*).*");
//         if(CMGW.search(buffer) == -1 ) return;
//         i_savedIndex=CMGW.cap(1).toInt();
    }
    return (bool) KMobileTools::SerialManager::ATError(buffer);
}


/*
 * class StoreSMS
 */

StoreSMS::StoreSMS( KMobileTools::Job *pjob, const QString &number, const QString &text, KMobileTools::SerialManager *device, AT_Engine* parent ) : kmobiletoolsATJob( pjob, device, parent )
{
    engine->suspendStatusJobs( true );

    pdu=engine->getATAbilities().isPDU();
    p_sms=new ATSMS( QStringList() << number, text);
    p_sms->setType( SMS::Unsent );
}

StoreSMS::StoreSMS( KMobileTools::Job *pjob, SMS *sms, KMobileTools::SerialManager *device, AT_Engine* parent ) : kmobiletoolsATJob( pjob, device, parent )
{
    engine->suspendStatusJobs( true );

    pdu=engine->getATAbilities().isPDU();
    p_sms=ATSMS::fromSMS( sms );
}

void StoreSMS::run()
{
//     kDebug() << "StoreSMS::run()\n";
//     kDebug() << "Numbers: " << p_sms->getTo() << endl;
    p_device->lockMutex();
    QStringList::iterator num, txt;
    QStringList numbers=p_sms->getTo();
    QStringList texts=p_sms->getMultiText();
    for(num=numbers.begin(); num!=numbers.end(); ++num)
    {
        for(txt=texts.begin(); txt!=texts.end(); ++txt)
            storeSingleSMS( (*num), (*txt) );
    }

    p_device->unlockMutex();
}

int StoreSMS::storeSingleSMS(const QString &number, const QString &text)
{
    QString buffer;
    if(engine->config()->smscenter().length() )
    {
        buffer=p_device->sendATCommand(this, "AT+CSCA?\r" );
        QString smscenter=kmobiletoolsATJob::parseInfo( buffer);
        QRegExp tempRegexp;
        tempRegexp.setPattern( ".*\"(.*)\".*");
        if(tempRegexp.indexIn(smscenter)>=0) smscenter=tempRegexp.cap( 1 ); else smscenter.clear();
        kDebug() << "*************** SMS Center found:" << smscenter << "\\n" << endl;
        if(! KMobileTools::KMobiletoolsHelper::compareNumbers(smscenter, engine->config()->smscenter()) )
        {
            kDebug() << "It seems that SMS center is NOT correct on your phone, setting it...\n";
            buffer=p_device->sendATCommand(this,  QString("AT+CSCA=\"%1\"\r")
                    .arg( encodeString( engine->config()->smscenter() ) ) );
            buffer=p_device->sendATCommand(this,  "AT+CSCA?\r" );
            if(! buffer.contains(  encodeString(engine->config()->smscenter() ) ) )
                kDebug() << "StoreSMS::run() ********** WARNING ******* Could not set SMS Center\n";
        }
    }
    if ( pdu )
    {
        QString pduSMS=SMSEncoder::encodeSMS( number, text );
        buffer=p_device->sendATCommand(this,  QString("AT+CMGW=%1\r").arg((pduSMS.length()/2) -1), 5 );
        buffer=p_device->sendATCommand(this,  pduSMS.append( "\x1A") );
    } else {
        buffer=p_device->sendATCommand(this,  QString("AT+CMGW=\"%1\"\r").arg(encodeString(number) ), 5 );
        if( KMobileTools::SerialManager::ATError(buffer) && engine->config()->at_encoding().contains("UCS2", Qt::CaseSensitive ) )
            buffer=p_device->sendATCommand(this,  QString("AT+CMGW=\"%1\"\r").arg((number)) ); // resend command without encoding phonenumber for buggy phones (like mine.. :(
//         if ( KMobileTools::SerialManager::ATError(buffer) ) return;
        buffer=p_device->sendATCommand(this,  QString("%1\x1A").arg(encodeString(text)), 10000 );
        QRegExp CMGW;
        CMGW.setPattern(".*\\+CMGW[\\s]*:[\\s*]([\\d]*).*");
        if(CMGW.indexIn(buffer) == -1 ) return -1;
        i_savedIndex=CMGW.cap(1).toInt();
    }
    return -1;
}


/*
 * class SMSEncoder
 */

SMSEncoder::SMSEncoder( const QString &number, const QString &message)
{
    encodeSMS( number, message );
}

QString SMSEncoder::encodeSMS( const QString &number, const QString &message )
{
    int encoding=KMobileTools::EncodingsHelper::hasEncoding( message, true);
    int msglen;
    QString pdu = "001100";  // SMS Message Sender number + SMS-Submit + TP-Message Reference
    pdu += encodeNumber( number );
    pdu += "00"; /* Protocol identifier */
//    pdu += "00"; /* Data coding scheme (default 7bit) */ /// @TODO Add other encoding schemes
    switch( encoding ){
        case KMobileTools::EncodingsHelper::GSM:
            pdu+="00";
            msglen=message.length();
            break;
        case KMobileTools::EncodingsHelper::Local8Bit:
            pdu+="04";
            msglen=message.length();
            break;
        default:
            pdu+="08";
            msglen=message.length()*2;
            break;
    }
    pdu += "AA"; /* Validity period */
    pdu += QString("%1").arg(msglen, 2, 16);
    pdu += encodeText( message, encoding );
    return pdu.toUpper().replace(" ", "0");
}

QString SMSEncoder::encodeText(const QString &o_text, int encoding)
{
//     for(int i=0; i<text.length(); i++)
//         kDebug() << QString("SMSEncoder; original string; at %1:%2(%3)\n")
//                 .arg(i,2).arg(text.at(i)).arg( (uint) ( (uchar) text.at(i).latin1()), 8,2 );
    QString out;
    QString debugOut;
    Q3MemArray<QChar> text;
//     int encoding=KMobileTools::EncodingsHelper::hasEncoding( o_text, true);
    kDebug() << "Using encoding " << encoding << endl;
    if(encoding==KMobileTools::EncodingsHelper::GSM )
            text=KMobileTools::EncodingsHelper::encodeGSM(o_text);
//     kDebug() << "SMSEncoder; Sizeof char=" << sizeof(char) << endl;
    uchar curChar, nextChar, stolenBits, stolenMask;
    uint strAt=0, septetsAt=0;
    switch( encoding ){
        case KMobileTools::EncodingsHelper::GSM:
            curChar=text.at(0).toLatin1();
            do {
                if(strAt+1<text.size()) nextChar=text.at( strAt+1).toLatin1(); else nextChar=0;
                stolenMask= (1 << ((septetsAt%7) +1) )-1;
                stolenBits= (nextChar & stolenMask) << (7-(septetsAt%7));
                out += QString("%1").arg( (stolenBits | curChar ), 2, 16 );
                curChar=nextChar >> ( (septetsAt+1) % 7 );
                septetsAt++; strAt++;
                if( (!(septetsAt%7)) && septetsAt)
                {
                    strAt++;
                    curChar=text.at(strAt).toLatin1();
                }
            } while (strAt<text.size());
            out=out.replace( ' ', '0');
            break;
        case KMobileTools::EncodingsHelper::Local8Bit:
            out=KMobileTools::EncodingsHelper::getHexString( o_text, 2 );
            break;
        case KMobileTools::EncodingsHelper::UCS2:
            out=KMobileTools::EncodingsHelper::toUCS2( o_text );
    }
    return out;
}

QString SMSEncoder::encodeNumber( const QString &_number )
{
    QString res;
    QString number = _number;
    if ( number.startsWith("+") ) number = number.mid( 1 );
    int len = number.length();
    res += QString("%1").arg( len, 2, 16 ).replace(" ", "0");
    res += number.startsWith("+") ? "91" : "81"; // is the number type octet here?

    if ( odd(number.length()) ) number+='F';
    for(int i=0; i<number.length(); i+=2)
        res += number.mid(i+1,1) + number.mid(i,1);

    return res;
}

DeleteSMS::DeleteSMS( KMobileTools::Job *pjob,SMS *sms, KMobileTools::SerialManager *device, AT_Engine* parent ) : kmobiletoolsATJob( pjob, device, parent )
{
    p_sms=sms;
}

void DeleteSMS::run()
{
    QString buffer;
    buffer=p_device->sendATCommand(this,  "AT+CPMS?\r" );
    buffer=kmobiletoolsATJob::parseInfo( buffer);
    QRegExp CMGL;
    CMGL.setPattern( ".*([A-Z][A-Z]).*([A-Z][A-Z]).*([A-Z][A-Z]).*" );
    QString in_memslot, memslot2, memslot3;
    if(CMGL.indexIn( buffer )>=0)
    {
        in_memslot=CMGL.cap( 1 );
        memslot2=CMGL.cap( 2);
        memslot3=CMGL.cap( 3 );
    }
    if ( in_memslot != p_sms->rawSlot() ) return;
    for(QList<int>::Iterator it=p_sms->idList()->begin(); it!=p_sms->idList()->end(); ++it)
        buffer=p_device->sendATCommand(this,  QString("AT+CMGD=%1\r").arg(*it) );
    b_succeeded=KMobileTools::SerialManager::ATError(buffer);
}

