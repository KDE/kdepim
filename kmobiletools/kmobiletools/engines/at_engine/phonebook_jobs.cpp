/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>,
   Alexander Rensmann <zerraxys@gmx.net>,
   S�rgio Gomes <sergiomdgomes@gmail.com>

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

#include "phonebook_jobs.h"
#include "at_engine.h"
#include <libkmobiletools/engine.h>
#include <libkmobiletools/serialdevice.h>

#include <qstringlist.h>
#include <qregexp.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <kabc/vcardconverter.h>


FetchAddressee::FetchAddressee( KMobileTools::Job *pjob, int memslots, KMobileTools::SerialManager *device, AT_Engine* parent) : kmobiletoolsATJob(pjob, device, parent)
{
  i_slots=memslots;
  b_partialUpdates=false;
  total=0;
  jdone=0;
  connect(this, SIGNAL(SPR()), this, SLOT(execSPR()));
}

void FetchAddressee::execSPR()
{
    // Triggering GUI update from main thread
    if(percentDone() ) return;
    emit gotAddresseeList(i_curslot, addresseeList);
}

void FetchAddressee::run()
{
    engine->suspendStatusJobs( true );
    p_fullAddresseeList.clear();
    if (i_slots & engine->PB_DataCard ) total++;
    if (i_slots & engine->PB_SIM ) total++;
    if (i_slots & engine->PB_Phone ) total++;
    if (i_slots & engine->PB_DataCard ) fetchMemSlot(engine->PB_DataCard);
    if (i_slots & engine->PB_SIM ) fetchMemSlot(engine->PB_SIM);
    if (i_slots & engine->PB_Phone ) fetchMemSlot(engine->PB_Phone);
}

void FetchAddressee::fetchMemSlot(int i_slot, bool updatePercent)
{
    i_curslot=i_slot;
    QRegExp expr;
    QString command;
    addresseeList.clear();
    QString buffer, temp;
    int thisJobPercent;
    if(updatePercent) thisJobPercent=0;
    int busyMem=0;

    if(i_slot && i_slot != engine->currentPBMemSlot() )
    {
        QString buffer=p_device->sendATCommand(this,  QString("AT+CPBS=%1\r").arg( AT_Engine::getPBMemSlotString(i_slot) ) );
        if(!KMobileTools::SerialManager::ATError(buffer)) engine->setCurrentPBMemSlot(i_slot);
    }
    expr.setPattern( "\\+CPBS:[\\s]*\"?([A-Za-z]*)\"?[\\s]*,*([\\d]*)" );
    buffer=p_device->sendATCommand(this,  "AT+CPBS?\r" );
    if( expr.indexIn( buffer ) != -1)
        busyMem=expr.cap(2).toInt();
    kDebug() << "Busy memory positions: " << busyMem << endl;
    if(updatePercent) {thisJobPercent=10;
        slotPercentDone( ( (jdone*100) + thisJobPercent) / total );}

    // Enumerate entries
    buffer = p_device->sendATCommand(this,  "AT+CPBR=?\r" );
    if ( KMobileTools::SerialManager::ATError(buffer) ) return;
    if(updatePercent) {thisJobPercent=20;
        slotPercentDone( ( (jdone*100) + thisJobPercent) / total );}

    // Format buffer
    QStringList list = formatBuffer( buffer );
    if ( list.count()!=1 && !list[0].startsWith( "+CPBR:" ) ) return;

    // Get List
    temp = list[0].remove( '(' ).remove( ')' ).replace( '-', ',' );
    list = kmobiletoolsATJob::parseList( temp );

    int leftIndex = list[0].toInt();
    int rightIndex = list[1].toInt();
    // Request entries
    buffer.clear();
    if ( engine->getATAbilities().isMotorola() )
    {
        command="AT+MPBR=%1\r";
        expr.setPattern( "^[+]MPBR:\\s*(\\d+),\"?([^\",]*)\"?,\\d+,\"(.*)\",(\\d)" );
    }
    else
    {
        command="AT+CPBR=%1\r";
        expr.setPattern( "^[+]CPBR:\\s*(\\d+),\"?([^\",]*)\"?,\\d+,\"(.*)\"" );
    }
    expr.setMinimal( true);
    if(busyMem)
    {
        int founds=0;
        QString tbuffer;
        for(int i=leftIndex; i<=rightIndex; i++)
        {
            tbuffer=p_device->sendATCommand(this,  command.arg(i) );
            kDebug() << "Contact found:" << bool(tbuffer.contains( "+CPBR:") || tbuffer.contains( "+MPBR:")) << "; contacts found: " << founds << endl;
            if( tbuffer.contains( "+CPBR:") || tbuffer.contains( "+MPBR:") )
            {
                founds++;
                buffer+=tbuffer;
            }
            if(founds>=busyMem) break;
        }
    }
    else
    {
        buffer=p_device->sendATCommand(this,  command.arg( QString("%1,%2").arg(leftIndex).arg(rightIndex) ), 200 * (rightIndex-leftIndex ), false);
    }
    if ( KMobileTools::SerialManager::ATError(buffer) ) return;
    if(updatePercent) {thisJobPercent=33;
        slotPercentDone( ( (jdone*100) + thisJobPercent) / total );}
    // get a nice QStringList
    QStringList tempbuffer = kmobiletoolsATJob::formatBuffer( buffer );
    // Regular expression for response
    bool new_addr=true;
    KABC::PhoneNumber::TypeFlag PhoneType=KABC::PhoneNumber::Cell;
    KABC::Addressee addr;
    KABC::Addressee tempaddr;
    QString givenName;
    for ( int i=0; i<tempbuffer.count(); i++ )
    {
        if(updatePercent) {thisJobPercent+=( (i*66) / tempbuffer.count() );
            slotPercentDone( ( (jdone*100) + thisJobPercent) / total );}
        if ( expr.indexIn(tempbuffer[i])<0 ) continue;
        addr=KABC::Addressee();
        new_addr=true;
        PhoneType=KABC::PhoneNumber::Pref;
        addr=KABC::Addressee();
        tempaddr=KABC::Addressee();
        givenName=decodeString( expr.cap( 3 ) );
        if( engine->getATAbilities().isSonyEricsson() )
        {
            QString  type=givenName.mid(givenName.lastIndexOf('/') + 1);
            givenName=givenName.left(givenName.lastIndexOf('/'));
            if(type=="W") PhoneType=KABC::PhoneNumber::Work;
            if(type=="M") PhoneType=KABC::PhoneNumber::Cell;
            if(type=="H") PhoneType=KABC::PhoneNumber::Home;
            if(type=="F") PhoneType=KABC::PhoneNumber::Fax;
            if(type=="O") PhoneType=KABC::PhoneNumber::Pref;
        }
        if ( engine->getATAbilities().isMotorola() )
        {
            int type=decodeString( expr.cap( 4 ) ).toInt();
            if(type == 0 ) PhoneType=KABC::PhoneNumber::Work;
            if(type == 1 ) PhoneType=KABC::PhoneNumber::Home;
            if(type == 2 ) PhoneType=KABC::PhoneNumber::Pref;
            if(type == 3 ) PhoneType=KABC::PhoneNumber::Cell;
            if(type == 4 ) PhoneType=KABC::PhoneNumber::Fax;
            if(type == 5 ) PhoneType=KABC::PhoneNumber::Pager;
            if(type == 6 ) PhoneType=KABC::PhoneNumber::Msg;
            if(type == 7 ) PhoneType=KABC::PhoneNumber::Msg;
        }
        foreach ( KABC::Addressee tempaddr, addresseeList )
        {
            if(tempaddr.formattedName() == givenName && tempaddr.custom("KMobileTools","memslot").toInt() == i_slot)
                // Well, it's ok to have multinumber Phonebook, but it's better to not mess different memory slots ;-)
            {
                addr=tempaddr;
                new_addr=false;
            }
        }
        addr.insertPhoneNumber( KABC::PhoneNumber( decodeString(expr.cap( 2 )) , PhoneType ) );
        addr.setFormattedName( givenName );
        if(new_addr) addr.insertCustom("KMobileTools", "index", expr.cap(1)  );
        else addr.insertCustom("KMobileTools","index", addr.custom("KMobileTools","index").append(",").append(expr.cap(1) ) );
        addr.insertCustom("KMobileTools","memslot",QString::number(i_slot) );
        if(new_addr)
        {
            addresseeList.append( addr );
            p_fullAddresseeList.append(addr);
        }
        buffer = buffer.remove( expr.pos(), expr.matchedLength() );
    }
    b_partialUpdates=true;
    jdone++;
    if(updatePercent) { /** triggerSPR(); @TODO port SPR job handling */
        slotPercentDone( (jdone*100) / total );}
}


/*
 * class FetchAddresseeSiemens
 * Description:
 * Fetches the Siemens phone book.
 */


FetchAddresseeSiemens::FetchAddresseeSiemens(KMobileTools::Job *pjob, KMobileTools::SerialManager *device, AT_Engine* parent) : FetchAddressee( pjob, 0, device, parent )
{
  engine = parent;
}

void FetchAddresseeSiemens::run()
{
    engine->suspendStatusJobs( true );
    if ( engine->getATAbilities().canSiemensVCF() )
        fetchVCF();
    else
        if ( engine->getATAbilities().canSDBR() )
            fetchSDBR();
    p_fullAddresseeList=addresseeList;
}

void FetchAddresseeSiemens::fetchVCF()
{
    QString buffer;
    KABC::VCardConverter converter;

    uint index = 0;
    while ( 1 )
    {
        buffer = p_device->sendATCommand(this,  "AT^SBNR=vcf,"+QString::number(index)+"\r" );
        if ( KMobileTools::SerialManager::ATError(buffer) ) break;
        QStringList tempbuffer = kmobiletoolsATJob::formatBuffer( buffer );

        buffer.truncate( 0 );
        for ( int i=1; i<tempbuffer.count(); i += 2 )
            buffer.append( tempbuffer[i] );
        buffer = KMobileTools::SerialManager::decodePDU( buffer );

        addresseeList.append( KABC::Addressee( converter.parseVCard( buffer.toLocal8Bit() /** @TODO look if this conversion is the best one fitting.. */ ) ) );
        kDebug() << buffer << endl;

        index++;
    }
}

void FetchAddresseeSiemens::fetchSDBR()
{
    QString buffer, temp;
    QRegExp expr;
    KABC::VCardConverter converter;
    int thisJobPercent = 0;
    addresseeList.clear();

    buffer = p_device->sendATCommand(this,  "AT^SDBR=?\r" );
    if ( KMobileTools::SerialManager::ATError(buffer) ) return;

    // Format buffer
    QStringList list = formatBuffer( buffer );
    if ( list.count()!=1 && !list[0].startsWith( "^SDBR:" ) ) return;
    
    expr.setPattern( "^\\^SDBR:\\s*(\\d+),\"([^\"]*)\",(\\d+),\"([^\"]*)\".*$" );

    // Get List
    temp = list[0].mid( 6 ).remove( '(' ).remove( ')' ).replace( '-', ',' );
    list = kmobiletoolsATJob::parseList( temp );

    int leftIndex = list[0].toInt();
    int rightIndex = list[1].toInt();

    kDebug() << "SDBR" << leftIndex << "-" << rightIndex << endl;

    for( int index=leftIndex; index <= rightIndex; index++ )
    {
        buffer = p_device->sendATCommand(this,  "AT^SDBR="+QString::number(index) + "\r" );
        if ( KMobileTools::SerialManager::ATError(buffer) ) return;

        // Format buffer
        list = formatBuffer( buffer );
        KABC::Addressee addr;
        for( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
        {
            // Parse this entry into a list of strings
            expr.indexIn( *it );
            QStringList parsedExp = expr.capturedTexts();

            addr.setFormattedName( kmobiletoolsATJob::decodeString( parsedExp[4] ) );

            KABC::PhoneNumber::TypeFlag PhoneType=KABC::PhoneNumber::Cell;
            switch( parsedExp[1].toInt() )
            {
                case 0: PhoneType = KABC::PhoneNumber::Cell; break;
                case 1: PhoneType = KABC::PhoneNumber::Work; break;
                case 2: PhoneType = KABC::PhoneNumber::Home; break;
                case 3: PhoneType = KABC::PhoneNumber::Fax; break;
            }

            addr.insertPhoneNumber( KABC::PhoneNumber( parsedExp[2] , PhoneType ) );

            addr.insertCustom("KMobileTools", "memslot", QString::number(engine->PB_Phone) );
            addr.insertCustom("KMobileTools", "index", QString::number(index)  );

            thisJobPercent = 100 * index / (rightIndex - leftIndex);
            slotPercentDone( thisJobPercent );

        }
        addresseeList.append(  addr );
    }

    if ( addresseeList.count()>0 )
    {
        emit gotAddresseeList(engine->PB_Phone, addresseeList);
    }


}


EditAddressees::EditAddressees(KMobileTools::Job *pjob, const QList<KABC::Addressee>& abclist, KMobileTools::SerialManager *device, bool b_todelete, AT_Engine* parent) : kmobiletoolsATJob(pjob, device, parent)
{
  engine->suspendStatusJobs( true );
    // this constructor add (or delete) a QValueList of addressee, depending on b_todelete.
  p_abclist=abclist;
  todelete=b_todelete;
  totaljobs=0;
  totalprogress=0;
  connect(this, SIGNAL(partialProgress(int) ), this, SLOT(updateProgress(int ) ) );
}


EditAddressees::EditAddressees( KMobileTools::Job *pjob, const KABC::Addressee& oldAddressee, const KABC::Addressee& newAddressee, KMobileTools::SerialManager *device, AT_Engine *parent)
  : kmobiletoolsATJob(pjob, device, parent)
{
    engine->suspendStatusJobs( true );
    // This constructor edit one addressee.
    p_oldAddressee=oldAddressee;
    p_newAddressee=newAddressee;
    totaljobs=0;
    totalprogress=0;
    connect(this, SIGNAL(partialProgress(int) ), this, SLOT(updateProgress(int ) ) );
}


int EditAddressees::addAddressee( const KABC::Addressee& addressee, int start)
{
    if(pb_full) return 0;
    QString buffer;
    int i_pbslot=addressee.custom("KMobileTools","memslot").toInt();
    if(i_pbslot && i_pbslot != engine->currentPBMemSlot() )
    {
        QString buffer=p_device->sendATCommand(this,  QString("AT+CPBS=%1\r").arg( AT_Engine::getPBMemSlotString(i_pbslot) ) );
        if(!KMobileTools::SerialManager::ATError(buffer)) engine->setCurrentPBMemSlot(i_pbslot);
    }
    int i_index=start;
    KABC::PhoneNumber::List phonenumbers=addressee.phoneNumbers();
    KABC::PhoneNumber::List::ConstIterator it;
    int curProgress=0;
    for ( it = phonenumbers.begin(); it != phonenumbers.end(); ++it )
    {
        i_retry=0;
        while( i_retry<3 ){
            i_index= findFreeIndex(i_index);
            if(!i_index)
            {
                pb_full=true;
                return 0;
            }
            QString command="AT+CPBW=%1,\"%2\",%4,\"%3\"\r";
            if ( engine->getATAbilities().isSonyEricsson() )
            {
                command="AT+CPBW=%2,\"%3\",%5,\"%4/%1\"\r";
                switch( (*it).type() ){
                    case KABC::PhoneNumber::Work:
                        command=command.arg("W");
                        break;
                    case KABC::PhoneNumber::Home:
                        command=command.arg("H");
                        break;
                    case KABC::PhoneNumber::Pref:
                        command=command.arg("O");
                        break;
                    case KABC::PhoneNumber::Cell:
                        command=command.arg("M");
                        break;
                    case KABC::PhoneNumber::Fax:
                        command=command.arg("F");
                        break;
                }
            }
            if ( engine->getATAbilities().isMotorola() )
            {
                command="AT+MPBW=%2,\"%3\",%5,\"%4\",%1\r";
                switch( (*it).type() ){
                    case KABC::PhoneNumber::Work:
                        command=command.arg("0");
                        break;
                    case KABC::PhoneNumber::Home:
                        command=command.arg("1");
                        break;
                    case KABC::PhoneNumber::Pref:
                        command=command.arg("2");
                        break;
                    case KABC::PhoneNumber::Cell:
                        command=command.arg("3");
                        break;
                    case KABC::PhoneNumber::Fax:
                        command=command.arg("4");
                        break;
                    case KABC::PhoneNumber::Pager:
                        command=command.arg("5");
                        break;
                    case KABC::PhoneNumber::Msg:
                        command=command.arg("6");
                        break;
                }
            }
            QString num_type;
            if((*it).number().contains("+") ) num_type="145"; else num_type="129";
            buffer=p_device->sendATCommand(this, command.arg(i_index).arg( encodeString((*it).number())).arg(encodeString(addressee.formattedName())).arg(num_type));
            if( KMobileTools::SerialManager::ATError(buffer) && engine->config()->at_encoding().contains("UCS2", Qt::CaseSensitive ) )
                buffer=p_device->sendATCommand(this, command.arg(i_index).arg( (*it).number()).arg(encodeString(addressee.formattedName())).arg(num_type));
            if(KMobileTools::SerialManager::ATError(buffer)) i_retry++; else i_retry=4;
        }
        curProgress++;
        emit partialProgress( ((curProgress*100) / phonenumbers.count() ) -1);
    }
    emit partialProgress(100);
    return i_index;
}

void EditAddressees::run()
{
    pb_full=false;
    if( !p_oldAddressee.isEmpty() && !p_newAddressee.isEmpty() )
    {
        // By now this edits only ONE addressee, removing the old values and readding the new ones.
        totaljobs=2;
        delAddressee( p_oldAddressee );
        addAddressee(p_newAddressee );
        return;
    }
    if(p_abclist.isEmpty())
        return;
    KABC::Addressee::List::ConstIterator it;
    int maxPBSlots=engine->availPbSlots()+1;
    int *startpoints=new int[ maxPBSlots ];
    for ( int i=0; i<maxPBSlots; i++) startpoints[i]=0;
    int curmemslot=0;
    totaljobs=p_abclist.count();
    for ( it = p_abclist.begin(); it != p_abclist.end(); ++it )
    {
        curmemslot=(*it).custom("KMobileTools","memslot").toInt();
        if(curmemslot==-1) continue;
        KABC::Addressee addressee(*it);
//         kDebug() << addressee->formattedName() << endl;
        if(todelete) delAddressee(addressee);
        else
        {
            startpoints[curmemslot]=addAddressee(addressee, startpoints[curmemslot] )+1;
        }
    }
    delete [] startpoints;
}

void EditAddressees::updateProgress(int progress)
{
    if(progress==100)
    {
        totalprogress++;
        slotPercentDone( (totalprogress*100) / totaljobs);
        return;
    }
    slotPercentDone( ( (totalprogress*100) + progress) / totaljobs );
}

int EditAddressees::findFreeIndex(int startpoint)
{
    QString buffer=p_device->sendATCommand(this, "AT+CPBR=?\r");
    if ( KMobileTools::SerialManager::ATError(buffer) ) return 0;
    QRegExp expr("^[+]CPBR:\\s?\\(?(\\d*)-(\\d*)\\)?.*$");
    QStringList tempbuffer = kmobiletoolsATJob::formatBuffer( buffer );
    for( int i=0; i<tempbuffer.count(); i++ ){
        if(expr.indexIn(tempbuffer[i])<0 ) break;
        int start=expr.cap(1).toInt();
        int end=expr.cap(2).toInt();
        if( ! (start * end) ) break;
        if (startpoint > start && startpoint < end) start=startpoint;
        for(int j=start; j<=end; j++ ){
            buffer=p_device->sendATCommand(this, QString("AT+CPBR=%1\r").arg(j) );
            if ( ! KMobileTools::SerialManager::ATError(buffer) ) if( !  kmobiletoolsATJob::formatBuffer(buffer).count() ) return j;
        }
    }
    return 0;
}

void EditAddressees::delAddressee(const KABC::Addressee& addressee)
{
    QString buffer;
    int i_pbslot=addressee.custom("KMobileTools","memslot").toInt();
    if(i_pbslot && i_pbslot != engine->currentPBMemSlot())
    {
        QString buffer=p_device->sendATCommand(this,  QString("AT+CPBS=%1\r").arg( AT_Engine::getPBMemSlotString(i_pbslot) ) );
        if(!KMobileTools::SerialManager::ATError(buffer)) engine->setCurrentPBMemSlot(i_pbslot);
    }
    QStringList indexes = addressee.custom("KMobileTools","index").split(",", QString::SkipEmptyParts);
    int currentItem=0;
    for ( QStringList::Iterator it = indexes.begin(); it != indexes.end(); ++it )
    {
        currentItem++;
        i_retry=0;
        while( i_retry<3 ){
            buffer=p_device->sendATCommand(this,  QString("AT+CPBW=%1\r").arg(*it) );
            if(KMobileTools::SerialManager::ATError(buffer)) i_retry++; else i_retry=4;
        }
        emit partialProgress( ( (currentItem*100) / indexes.count() ) -1 );
    }
    emit partialProgress(100);
}

#include "phonebook_jobs.moc"
