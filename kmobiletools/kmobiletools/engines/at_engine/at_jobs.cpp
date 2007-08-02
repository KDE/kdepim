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
#include "at_jobs.h"

#include <math.h>

#include <libkmobiletools/serialdevice.h>
#include <qregexp.h>
#include <unistd.h>
#include <qstringlist.h>
#include <qdatetime.h>
#include <threadweaver/DependencyPolicy.h>
#include "at_engine.h"
#include <libkmobiletools/encodingshelper.h>

kmobiletoolsATJob::kmobiletoolsATJob(KMobileTools::SerialManager *device, AT_Engine* parent)
  : KMobileTools::Job( parent->objectName(), (QObject*)parent)
{
  p_device=device;
  engine = parent;
}
kmobiletoolsATJob::kmobiletoolsATJob(KMobileTools::Job *pjob, KMobileTools::SerialManager *device, AT_Engine* parent)
  : KMobileTools::Job( parent->objectName(), (QObject*)parent)
{
  if(pjob) ThreadWeaver::DependencyPolicy::instance().addDependency(this, pjob);
  p_device=device;
  engine = parent;
}

initPhoneJob::initPhoneJob( KMobileTools::SerialManager *device, AT_Engine* parent)
    : kmobiletoolsATJob( device, parent )
{
}

void initPhoneJob::run ()
{
    p_device->setSpeed(engine->config()->at_baudrate() );
    p_device->open(this);
}

QStringList kmobiletoolsATJob::formatBuffer( QString buffer ) // krazy:exclude=passbyvalue
{
    // remove linefeeds
    buffer = buffer.replace('\r','\n');
    while ( buffer.contains("\n\n")>0  ) buffer = buffer.replace("\n\n","\n");

    // split buffer
    QStringList tempbuffer = buffer.split( '\n', QString::SkipEmptyParts );

    // remove empty lines
    QStringList::Iterator it = tempbuffer.begin();
    while ( it != tempbuffer.end() )
    {
        if ( (*it).isNull() || (*it)==QLatin1String("OK") )
            it = tempbuffer.erase( it );
        else
            ++it;
    }

    return tempbuffer;
}

QStringList kmobiletoolsATJob::parseList( QString list,char begins ) // krazy:exclude=passbyvalue
{
    QStringList alist,tlist;
    list=list.replace( '\r', '\n' ).replace("\n\n","\n"); // Converting Windows-Like CRLF to UNIX termination line
    list=list.replace( "\",\"", ","); // Removing stripping quotes
    list=list.replace( "),(", "," ); // remove middle list separators
    // remove preceding CXXXX
    QRegExp Cxxx( QString("[+]") + begins + "\\w{3}:" );
    list = list.remove( Cxxx ).trimmed();
    // remove brackets
    if ( list[0]=='(' && list[list.length()-1]==')' ) list = list.mid( 1, list.length()-2 );
    alist=list.split(',', QString::SkipEmptyParts);
    // split
/*    uint left = 0;
    while ( left<list.length() )
    {
        int leftPos = left, quotationMark, comma;
        do
        {
            quotationMark = list.find( '"', leftPos );
            comma = list.find( ',', leftPos );
            if ( quotationMark >=0 && quotationMark < comma ) leftPos = list.find( '"', quotationMark+1 );
            if ( comma<0 ) comma = list.length();
        } while ( comma < leftPos );
        alist.append( list.mid( left, comma-left ) );
        left = comma+1;
    }
*/
    // remove quotation marks
    alist=alist.replaceInStrings("\"","");
//     alist=alist.replaceInStrings("\"$", "");
    alist = alist.replaceInStrings( QRegExp( "^\\s*\"(.*)\"\\s*$" ), "\\1" );
//     alist.sort();
    QStringList::iterator oldString;
    for (QStringList::Iterator it=alist.begin(); it!=alist.end(); ++it)
        if(tlist.indexOf( *it) == -1 ) tlist.append( *it );
    return tlist;
}

QStringList kmobiletoolsATJob::parseMultiList( QString list ) // krazy:exclude=passbyvalue
{
    // remove preceding CXXXX
    QRegExp Cxxx( "^[+]C\\w{3}:" );
    list = list.remove( Cxxx ).trimmed();

    QStringList alist;


    while ( list.contains( '(' )>0 )
    {
        int left = 0, right = 0;
        left = list.indexOf( '(' )+1;
        right = list.indexOf( ')', left );
        alist.append( list.mid( left, right-left ) );
        list = list.mid( right+1 );
    }

    return alist;
}

QString kmobiletoolsATJob::parseInfo( const QString &buffer )
{
    QString tmp = buffer.section("OK\r\n",0,0).remove( '\r' ).remove( '\n' );
    int i = tmp.indexOf( ':' );
    if ( i>0 && i<=6 && tmp.at(0)=='+' )
        tmp = tmp.section( ":",1 );
    tmp = tmp.trimmed();
    if ( tmp.at(0)=='"' && tmp.at(tmp.length()-1)=='"' )
        tmp = tmp.mid( 1, tmp.length()-2 );
    return tmp;
}

QString kmobiletoolsATJob::decodeString( const QString &text )
{
    QString encoding = engine->config()->at_encoding();
    if ( encoding.length()==3 && encoding.contains( "GSM", Qt::CaseInsensitive ) )
        return KMobileTools::EncodingsHelper::decodeGSM( text );
    else if ( encoding.contains( "UCS2", Qt::CaseInsensitive ) )
        return KMobileTools::EncodingsHelper::fromUCS2( text ) ;

    return text;
}

QString kmobiletoolsATJob::encodeString( const QString &text )
{
    QString encoding = engine->config()->at_encoding();
    if(encoding.indexOf( "UCS2", Qt::CaseInsensitive ) != -1)
        return KMobileTools::EncodingsHelper::toUCS2(text);
    return text;
}


/*
 * class SelectCharacterSet
 */

SelectCharacterSet::SelectCharacterSet( KMobileTools::Job *pjob, const QString &characterSet, KMobileTools::SerialManager *device, AT_Engine* parent  ) : kmobiletoolsATJob( pjob, device, parent )
{
    this->characterSet = characterSet;
}

void SelectCharacterSet::run ()
{
    QString buffer;
    buffer = p_device->sendATCommand(this,  "AT+CSCS=\""+characterSet+"\"\r" );
    if ( KMobileTools::SerialManager::ATError(buffer) ) return;
}

/*
 * class PollStatus
 */


PollStatus::PollStatus(KMobileTools::Job *pjob, KMobileTools::SerialManager *device, AT_Engine* parent) : kmobiletoolsATJob(pjob, device, parent)
        , i_charge(0), i_signal(0), m_chargeType(KMobileTools::EngineData::Battery), b_calling(false)
{}
void PollStatus::run ()
{
    // the job will take (... sleep) a random number of msec between 1 and
    // 1000:
    if(! p_device) return;
    QString str_status=p_device->sendATCommand(this, "AT+CSQ\r");
    slotPercentDone( 50 );
    str_status+=p_device->sendATCommand(this, "AT+CBC\r");
    slotPercentDone( 99 ); // Don't emit 100, since we still must do some math calculations, 100 will be automatically emitted when the job is destroyed
    if( str_status.contains("+CBC",Qt::CaseSensitive) )
    {
        QString s_charge=str_status.right( str_status.length() - str_status.indexOf("+CBC:") - 5 );
        s_charge=s_charge.left( s_charge.indexOf("\r") );
        s_charge=s_charge.trimmed();

        i_charge=s_charge.section(",",1,1).toInt();
        m_chargeType=(KMobileTools::EngineData::ChargeType) s_charge.section(",",0,0).toInt();
    }
    else { i_charge=-1; m_chargeType=KMobileTools::EngineData::Unknown; }

    if( str_status.contains("+CSQ",Qt::CaseSensitive) )
    {
        QString s_charge=str_status.right( str_status.length() - str_status.indexOf("+CSQ:") - 5 );
        s_charge=s_charge.left( s_charge.indexOf('\r') );
        s_charge=s_charge.trimmed();
        i_signal=s_charge.section(",",0,0).toInt();
        i_signal=i_signal*100/31;
    }
    else i_signal=-1;

    if( str_status.contains("RING", Qt::CaseSensitive) )
        b_calling=true; else b_calling=false;
//     sleep(10);
    if ( true /* ! Config->getMotoBattery() */ ) return;

    if(m_chargeType==KMobileTools::EngineData::Battery)
    {
        if( log(i_charge/1.4)/log(1.038) > 30 )
            i_charge=(int) (log((double) i_charge/1.4)/log(1.038));
    }
    if(m_chargeType==KMobileTools::EngineData::ACAdaptor)
    {
        i_charge= (int) ((int) pow( (double) (i_charge-6), (4.0/6.0) ) * 5.2) +2;
    }
}

FetchPhoneInfos::FetchPhoneInfos(KMobileTools::Job *pjob, KMobileTools::SerialManager *device, AT_Engine* parent) : kmobiletoolsATJob(pjob, device, parent){}

FetchPhoneInfos::FetchPhoneInfos(KMobileTools::SerialManager *device, AT_Engine* parent) : kmobiletoolsATJob(device, parent){}

void FetchPhoneInfos::run()
{
    if(! p_device) return;
    QString buffer=p_device->sendATCommand(this, "AT+CGMI\r");
    if( !KMobileTools::SerialManager::ATError(buffer) ) // Phone manufacturer
        s_manufacturer = parseInfo( buffer );
    else s_manufacturer.clear();
    slotPercentDone( 20 );

    buffer=p_device->sendATCommand(this, "AT+CGMR\r");
    if(!KMobileTools::SerialManager::ATError(buffer)) // Phone revision
        s_revision = parseInfo( buffer );
    else s_revision.clear();
    slotPercentDone( 60 );

    //if(engine->getATAbilities().isSonyEricsson()) returns false...
    if(s_manufacturer == "Sony Ericsson")
        buffer=p_device->sendATCommand(this, "ATI\r");
    else
        buffer=p_device->sendATCommand(this, "AT+CGMM\r");
    if(!KMobileTools::SerialManager::ATError(buffer)) // Phone model
    {
        s_model = parseInfo( buffer );
        if(s_manufacturer == "Sony Ericsson")
        {
            QStringList model = s_model.split(s_manufacturer, QString::KeepEmptyParts);
            s_model = model[1].trimmed();
        }
    } else s_model.clear();
    slotPercentDone( 40 );

    buffer=p_device->sendATCommand(this, "AT+CGSN\r");
    if(!KMobileTools::SerialManager::ATError(buffer)) // Phone imei
        s_imei = parseInfo( buffer );
    else s_imei.clear();
    slotPercentDone( 80 );

    buffer=p_device->sendATCommand(this, "AT+CSCA?\r");
    if(!KMobileTools::SerialManager::ATError(buffer)) // SMS Center
    {
        s_smscenter = parseInfo( buffer );
        s_smscenter=s_smscenter.split( ",", QString::SkipEmptyParts ).first();
        s_smscenter=s_smscenter.remove( '\"');
        s_smscenter=decodeString( s_smscenter );
    }
    else s_smscenter.clear();
    slotPercentDone( 99 );
}

/*
 * class SyncDateTime
 * Description:
 * Syncs the systems time to the mobile phone. The time on the phone is set
 * to the systems time if it differs more than two seconds.
 */

SyncDateTime::SyncDateTime(KMobileTools::Job *pjob, KMobileTools::SerialManager *device, AT_Engine* parent) : kmobiletoolsATJob(pjob, device, parent){}
void SyncDateTime::run ()
{
    QString buffer;

    // Test service
    buffer = p_device->sendATCommand(this,  "AT+CCLK=?\r" );
    if ( KMobileTools::SerialManager::ATError(buffer) ) return;
    slotPercentDone( 33 );
    // Request date
    buffer = p_device->sendATCommand(this,  "AT+CCLK?\r" );
    if ( KMobileTools::SerialManager::ATError(buffer) ) return;
    slotPercentDone( 66 );

    // Parse the response
    QStringList tempbuffer = kmobiletoolsATJob::formatBuffer( buffer );
    if ( tempbuffer.count()!=1 && !tempbuffer[0].startsWith( "+CCLK:" ) ) return;

    // RegExp for the CCLK
    QRegExp CCLK( "^[+]CCLK:\\s*\"?(\\d{2,4})/(\\d{2})/(\\d{2}),(\\d{2}):(\\d{2}):(\\d{2})([+]\\d{2})?\"?$" );
    if ( CCLK.indexIn( tempbuffer[0])!=0 ) return;

    int year = CCLK.cap(1).toInt();
    if ( year<100 ) year = year + 2000;
    KDateTime datetime( QDate( year, CCLK.cap(2).toInt(), CCLK.cap(3).toInt() ), QTime( CCLK.cap(4).toInt(), CCLK.cap(5).toInt(), CCLK.cap(6).toInt() ) );

//     int timediff = abs( datetime.secsTo( KDateTime::currentDateTime() ) );
    if ( true /** @TODO Since we already have a config entry for syncing time, it's not a bad idea to sync ANYWAY if this config entry is enabled.
               * However i'll need to have a better look on it.
               * timediff>2 */ )
    {
        p_device->lockMutex();
        KDateTime now = KDateTime::currentLocalDateTime();
        p_device->sendATCommand(this,  "AT+CCLK=\""+now.toString("yy/MM/dd,hh:mm:ss")+CCLK.cap(7)+"\"\r" );
        slotPercentDone( 99 );
        p_device->unlockMutex();
    }
}

/*
 * class UpdateSMS
 */
SelectSMSSlot::SelectSMSSlot( KMobileTools::Job *pjob, QString slot, KMobileTools::SerialManager *device, AT_Engine* parent ) : kmobiletoolsATJob( pjob, device, parent )
{
    readSlot = slot;
    b_done=false;
}

void SelectSMSSlot::run()
{
    QString buffer;
    p_device->lockMutex();
    buffer = p_device->sendATCommand(this,  "AT+CPMS=\""+readSlot+"\"\r" );
    p_device->unlockMutex();
    if ( KMobileTools::SerialManager::ATError(buffer) ) return;
    b_done=true;
}

TestPhoneFeatures::TestPhoneFeatures( KMobileTools::Job *pjob, KMobileTools::SerialManager *p_device, AT_Engine* parent )
  : kmobiletoolsATJob( pjob, p_device, parent )
{
}

TestPhoneFeatures::TestPhoneFeatures( KMobileTools::SerialManager *p_device, AT_Engine* parent )
  : kmobiletoolsATJob( p_device, parent )
{
}

/**
 * Total steps: 12
 * remember to change the percentDone emit when adding new tests
 */
void TestPhoneFeatures::run ()
{
    QString buffer;

    // Test phonebook slots
    buffer = p_device->sendATCommand(this,  "AT+CPBS=?\r" );
    if ( !KMobileTools::SerialManager::ATError(buffer) )
    {
        QStringList tempbuffer = kmobiletoolsATJob::formatBuffer( buffer );
        if ( tempbuffer.count()==1 && tempbuffer[0].startsWith( "+CPBS:" ) )
            abilities.setPBSlots( kmobiletoolsATJob::parseList( tempbuffer[0] ) );
    }
    slotPercentDone( 1 * 100 /12);
    // Test FileSystem support
    buffer= p_device->sendATCommand(this, "ATI5\r");
    if ( !KMobileTools::SerialManager::ATError(buffer) )
    {
        QStringList tempbuffer = kmobiletoolsATJob::formatBuffer( buffer );
        if(tempbuffer.first() == "P2K" ) abilities.setFileSystem( KMT_FILESYSTEM_Pk2 );
    }
    slotPercentDone( 2 * 100 /12);

    // Test TE character sets
    buffer = p_device->sendATCommand(this,  "AT+CSCS=?\r" );
    if ( !KMobileTools::SerialManager::ATError(buffer) )
    {
        QStringList tempbuffer = kmobiletoolsATJob::formatBuffer( buffer );
        if ( tempbuffer.count()==1 && tempbuffer[0].startsWith( "+CSCS:" ) )
            abilities.setCharacterSets( kmobiletoolsATJob::parseList( tempbuffer[0] ) );
    }
    slotPercentDone( 3 * 100 /12);

    // Test PDU Mode
    buffer = p_device->sendATCommand(this,  "AT+CMGF=0\r" );
    kDebug() <<"KMobileTools::SerialManager::ATError(buffer):" << buffer.length() - buffer.lastIndexOf("ERROR");
    if ( !KMobileTools::SerialManager::ATError(buffer) )
        abilities.smsPDUMode = true;
    slotPercentDone( 4 * 100 /12);

    // Test sims slots
    buffer = p_device->sendATCommand(this,  "AT+CPMS=?\r" );
    if ( !KMobileTools::SerialManager::ATError(buffer) )
    {
        QStringList tempbuffer = kmobiletoolsATJob::formatBuffer( buffer );
        if ( tempbuffer.count()==1 && tempbuffer[0].startsWith( "+CPMS:" ) )
            abilities.setSMSSlots( kmobiletoolsATJob::parseList( kmobiletoolsATJob::parseMultiList( tempbuffer[0] )[0] ) );
    }
    slotPercentDone( 5 * 100 /12);


    // Get Manufacturer
    buffer = p_device->sendATCommand(this,  "AT+CGMI\r" );
    abilities.setManufacturer( parseInfo( buffer ) );
    slotPercentDone( 6 * 100 /12);

    // Siemens specific tests
    if ( abilities.isSiemens() )
    {
        buffer = p_device->sendATCommand(this,  "AT^SMGL=?\r" );
        if ( !KMobileTools::SerialManager::ATError(buffer) ) abilities.b_canSMGL = true;
        slotPercentDone( 7 * 100 /12);

        buffer = p_device->sendATCommand(this,  "AT^SBNR=?\r" );
        if ( !KMobileTools::SerialManager::ATError(buffer) ) abilities.b_canSiemensVCF = buffer.contains( "vcf", Qt::CaseInsensitive );
        slotPercentDone( 8 * 100 /12);

        buffer = p_device->sendATCommand(this,  "AT^SDBR=?\r" );
        if ( !KMobileTools::SerialManager::ATError(buffer) ) abilities.b_canSDBR = true;
        slotPercentDone( 9 * 100 /12);

    }

    /* Test if mobile phone can store messages
    */
    buffer = p_device->sendATCommand(this,  "AT+CMGW=?\r" );
    abilities.b_canStoreSMS = !KMobileTools::SerialManager::ATError(buffer);
    slotPercentDone( 10 * 100 /12);

        /* Test if mobile phone can send messages
        */
    buffer = p_device->sendATCommand(this,  "AT+CMGS=?\r" );
    abilities.b_canSendSMS = !KMobileTools::SerialManager::ATError(buffer);
    slotPercentDone( 11 * 100 /12);

        /* Test if mobile phone can remopve messages
        */
    buffer = p_device->sendATCommand(this,  "AT+CMGD=?\r" );
    abilities.b_canDeleteSMS = !KMobileTools::SerialManager::ATError(buffer);
    slotPercentDone( 99 );

    engine->setATAbilities( abilities );
}
