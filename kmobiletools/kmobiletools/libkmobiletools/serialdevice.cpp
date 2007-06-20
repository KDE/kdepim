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
#include "serialdevice.h"

#include <qmutex.h>
#include <kdebug.h>
#include <qtimer.h>
#include <qstring.h>
#include <qregexp.h>
#include <qdir.h>
#include <qfile.h>
#include <kuser.h>
#include "qserial.h"
#include "weaver.h"
#include <kglobal.h>
#include <kstandarddirs.h>
#include <QTextStream>
#include "kmobiletoolshelper.h"

#include <config-kmobiletools.h>
/*#define KBLUETOOTH 1*/
#define MAXBUFSIZE 32

#ifdef KBLUETOOTH
#include <kdebluetooth/rfcommsocketdevice.h>
#include <qsocketnotifier.h>
#endif

#include <iostream>

#include "engineslist.h"
#include "devicesconfig.h"


using namespace std;
using namespace KMobileTools;

//TODO: Remove this define
// Define from at_jobs.cpp

class KMobileTools::SerialManagerPrivate {
    public:
        SerialManagerPrivate() : /*lockfilecreated(-1), modem(-1), locked(false), connectEmitted(false), disconnectEmitted(false), timeouts(0), */
            b_connected(false), gotData(0), serial(0L), m_baudrate(QSerial::BAUD_57600), bluetooth(false), errnum(0)
        {
            mutex=new QMutex(QMutex::Recursive);
#ifdef KBLUETOOTH
            rfcomm=0L;
#endif
        }
        ~SerialManagerPrivate() { delete mutex; }

        bool b_connected;
//         QStringList commandQueueStack;
        QString buffer;
        QMutex *mutex;
        uint gotData;
        QSerial *serial;
#ifdef KBLUETOOTH
        KBluetooth::RfcommSocketDevice *rfcomm;
#endif
        QIODevice *device;
        QString s_devicePath;
        QStringList deviceInitStrings;
        QSerial::Baud m_baudrate;
        bool bluetooth;
        bool log;
        int errnum;
        QFile logfile;
        QTextStream logstream;
};

void SerialManager::lockMutex() {
    if(!d || ! d->mutex) return;
    d->mutex->lock();
}
void SerialManager::unlockMutex() {
    if(!d || ! d->mutex) return;
    d->mutex->unlock();
}


//TODO: Remove this function
// Function from at_jobs.cpp
QString parseInfo( const QString &buffer )
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

SerialManager::SerialManager(QObject * parent, const QString &objname, const QString &devicePath, const QStringList &initStrings)
    : QObject(parent), d(new SerialManagerPrivate)
{
    setObjectName(objname);
    if(objectName() !="nodevice")
        d->log=DEVCFG(objectName() )->verbose();
    else d->log=DEFAULT_VERBOSE;
    if(devicePath.length() && (QFile::exists( devicePath ) || devicePath.contains( "bluetooth://")) ) d->s_devicePath=devicePath;
    if(initStrings.count() ) d->deviceInitStrings=initStrings;
}

bool SerialManager::isConnected() { return d->b_connected; }
SerialManager::~SerialManager()
{
    close();
    delete d;
}

void SerialManager::setDevicePath(const QString &path)
{
    if(d->serial)
    {
        if(d->serial->isOpen())
        {
            close();
            setDevicePath(path);
            open(0);
        } else d->s_devicePath=path;
    }
}

bool SerialManager::open(KMobileTools::Job *job)
{
    bool isOpen;
#ifdef KBLUETOOTH
    QRegExp addr("bluetooth://\\[(([A-F\\d]{2,2}:*){6,6})\\]:([\\d]+)");
    if( addr.search( d->s_devicePath ) != -1 )
    {
        d->bluetooth=true;
        d->rfcomm=new KBluetooth::RfcommSocketDevice();
        uint i=0;
        do {
            if(job && ! d->rfcomm->isOpen())
                job->thread()->msleep(2);
            else usleep(200000);
            d->rfcomm->connect(KBluetooth::DeviceAddress(addr.cap(1)), addr.cap(3).toInt());
            i++;
        } while ( ! d->rfcomm->isOpen() && i<5 );
        d->serial=(d->rfcomm);
        d->rfcomm->setBlocking(false);
        isOpen=d->rfcomm->isOpen();
//         kDebug() << "RFComm socket now should be connected: isOpen is returning " << d->serial->isOpen() << endl;
    } else {
#endif
    d->bluetooth=false;
    d->serial=new QSerial(d->s_devicePath);
    d->serial=(d->serial);
    d->serial->setBaud( d->m_baudrate );
    d->serial->setStopBits( QSerial::STOP_BITS_1 );
    d->serial->setDatabits( QSerial::DATABITS_8 );
    d->serial->setFlowControl( QSerial::FLOW_CONTROL_HARD );
    d->serial->setParity( QSerial::PARITY_NONE );
    connect(d->serial, SIGNAL(gotData()), this, SLOT(gotData()));
    isOpen= d->serial->open(QIODevice::ReadWrite/* | O_NOCTTY | O_NONBLOCK */);
#ifdef KBLUETOOTH
}
#endif
    if(!isOpen) return false;

    if(d->log)
    {
        d->logfile.setFileName(KGlobal::dirs()->saveLocation("tmp", "kmobiletools", true) + objectName() + ".log" );
        kDebug() << "Starting log to " << d->logfile.fileName() << endl;
        d->logfile.open(QIODevice::WriteOnly);
        d->logstream.setDevice(&(d->logfile));
    }
//     d->serial->reset();
    d->buffer=sendATCommand(job, "ATZ\r", 300);
    if(ATError(d->buffer))
    {
        kDebug() << "Error while sending ATZ. Device closed.\n";
        close();
        return false;
    }
//     sendATCommand(job, "AT\r", 100);

    for ( QStringList::Iterator it=d->deviceInitStrings.begin(); it!=d->deviceInitStrings.end(); ++it)
        if((*it).trimmed().length()>1) {
            d->buffer=sendATCommand(job, *it + "\r");
            if(ATError(d->buffer))
            {
                kDebug() << "Error while sending " << *it << ". Device closed.\n";
                close();
                return false;
            }
        }
    emit connected();
    d->b_connected=true;
    return true;
}

#include "serialdevice.moc"
void SerialManager::close()
{
    if( !d->serial || !d->serial->isOpen() ) return;
    d->serial->close();
    delete d->serial;
    d->serial=0;
    d->serial=0;
    d->logfile.close();
#ifdef KBLUETOOTH
    d->rfcomm=0;
#endif
    d->b_connected=false;
    emit disconnected();
}

QString SerialManager::devicePath() const {
    return d->s_devicePath;
}

QString SerialManager::sendATCommand(KMobileTools::Job *job, const QString &cmd, uint timeout, bool tryBreakingTimeout)
{
//     timeout=timeout*100;
    if(!d || ! d->mutex) return QString("\rERROR\r");
    kDebug() << "sendATCommand: " << cmd << endl;
    QTime timer;
//     kDebug() << "Mutex is locked::" << d->mutex->locked() << endl;
    QMutexLocker mlocker(d->mutex);
    kDebug() << "Mutex locked\n";
    if(!cmd.length() || ! d->serial || !d->serial->isOpen()) return QString();
    kDebug() << "Serial port is open, continuing\n";
//     QString classcmd=cmd.section( QRegExp("^AT"), 1,1,QString::SectionCaseInsensitiveSeps);
//     classcmd=classcmd.left(classcmd.find(QRegExp("[^\\w]"), 1) );
//     d->commandQueueStack+=classcmd;
//     kDebug() << "QueueStack: " << d->commandQueueStack << endl;
//     timeout=(timeout * 1000)+1;

    d->buffer.clear();
    long err;
#ifdef KBLUETOOTH
    if(d->bluetooth)
        err=d->rfcomm->write(cmd.toLatin1(), cmd.length()); else
#endif
    err=d->serial->write(cmd.toLatin1(), cmd.length());
    if(err<0)
    {
        kDebug() << "Write error: closing device link: error=" << err << "\n";
        close();
        return QString();
    }
    if(!d->serial) return QString();
    else {
#ifdef KBLUETOOTH
        if(!d->bluetooth)
#endif
        d->serial->flush(); /// @TODO look if this can be valid for bluetooth socket too
        }
//     kDebug() << "Sent cmd: " << cmd.latin1() << endl;
//     std::cout << ">>>" << QString(cmd).replace("\r","\n").replace("\n\n", "\n") << endl;
//     std::cout << "<<<";
    log(false, cmd);
    QRegExp exitExp("(OK|ERROR)(\\n|\\r)");
    timer.start();
//     uint i_try=0;
    while ( d->serial && !KMobileTools::EnginesList::instance()->closing() )
    {
        if( d->serial->size() ) gotData(); /// @TODO remove the size() check, if possible, since it slows down serial access.
        if( d->gotData )
        {
            timer.restart();
//             kDebug() << "Got Chars, resetting timer: " << timer.elapsed() << endl;
            d->gotData=0;
        }
//         if(cmd.contains( "CPBR" ) || cmd.contains( "MPBR" ) ) kDebug() << "Time elapsed: " << timer.elapsed() << "; buflen:" << buflen << "; buflen==buffer:" << (buflen==d->buffer.length() ) << endl;
        if( timer.elapsed() >=3000 &&
            (timer.elapsed()%3000 == 0) && tryBreakingTimeout)
        {
#ifdef KBLUETOOTH
            if(d->bluetooth)
                err=d->rfcomm->write("AT\r", 3); else
#endif
            err=d->serial->write("AT\r", 3);
#ifdef KBLUETOOTH
            if(!d->bluetooth)
#endif
            d->serial->flush(); /// @TODO look if this can be valid for bluetooth socket too
            if(err==-1)
            {
                kDebug() << "Write error: closing device link\n";
                close();
                return QString();
            }
            kDebug() << "****************** WARNING!!!! Sending AT\\r to unblock the phone.\n";
            kDebug() << "****************** this can be a bug of the phone, or of kmobiletools.\n";
            kDebug() << "****************** please report to marco AT kmobiletools.org: AT command=" << cmd << endl;
        }
/*        if(job
#ifdef KBLUETOOTH
           && ! d->bluetooth
#endif
          )
            job->thread()->msleep( 1 );
        else if(!job)
        {*/
            KMobileTools::Thread::msleep(2);
//        }
#ifdef KBLUETOOTH
        if(d && d->bluetooth)
        {
//             kDebug() << "Bluetooth reading: try " << i_try << endl;
//             i_try++;
            char *buf=new char[MAXBUFSIZE+1];
            memset(buf, 0, MAXBUFSIZE+1);
            err=d->rfcomm->readBlock(buf, MAXBUFSIZE);
//             if(err==-1)
//             {
//                 delete [] buf;
//                 continue;
//                 kDebug() << "Read error: closing device link\n";
//                 close();
//                 return QString();
//             }
//             kDebug() << "Reading " << err << " characters.\n";
            if(err>0) d->buffer+=buf;
            d->gotData=strlen(buf);
            delete [] buf;
            if(job)job->thread()->msleep( 1 ); else usleep(1000);
        }
#endif

//         if(job)
//             job->thread()->msleep( 1 );
//         else usleep(1000);
        if(d && timeout)
        {
            if( (uint) timer.elapsed() >timeout)
            {
                kDebug() << "Timeout exit: max timeout was " << timeout << ", timer: " << timer.elapsed() << endl;
                break;
            }
            if(d->buffer.contains(exitExp) )
            {
//                 kDebug() << "Regexp exit\n";
                break;
            }
        }
    }
//     std::cout << endl;
    if( timeout<10 // Don't generate warnings for _wanted_ timeout-exit commands
        && (uint) timer.elapsed() >=timeout )
    {
        kDebug() << "****************** WARNING!!!! Phone seems to be locked.\n";
        kDebug() << "****************** this can be a bug of the phone, or of kmobiletools.\n";
        kDebug() << "****************** please report to marco AT kmobiletools.org: AT command=" << cmd << endl;
    }
    int found=d->buffer.indexOf(cmd);
//     kDebug() << "Got buffer: " << d->buffer << endl;
    if(found!=-1 && found < 2)
        d->buffer=d->buffer.remove( found, cmd.length() );
    /// @TODO handle also partial errors
    exitExp.setPattern( "ERROR(\\n|\\r)");
//     if( d->buffer.contains( exitExp ) ) d->buffer="ERROR";
//     d->commandQueueStack.remove(classcmd);
    log(true, d->buffer);
    return d->buffer;
}

KMobileTools::QSerial *SerialManager::qserial()
{
    return d->serial;
}

void SerialManager::gotData()
{
    kDebug() << "gotData()" << endl;
    uint availData;
#ifdef KBLUETOOTH
    if(d->bluetooth)
        availData=d->rfcomm->size(); else
#endif
        availData=d->serial->size();
    kDebug() << "gotData() : got "<< availData << "bytes to read" << endl;
if(!availData) availData=MAXBUFSIZE; // fix for rfcomm wrong size
//     kDebug() << "GotData: Size=" << availData << endl;
    QByteArray buffer( availData+1, 0 );
    int readdata;
#ifdef KBLUETOOTH
    if(d->bluetooth)
    readdata=d->rfcomm->read(buffer.data(), availData);
#endif
    readdata=d->serial->read(buffer.data(), availData);
    if(readdata==-1)
    {
        kDebug() << "Read error: closing device link\n";
        close();
        return;
    }
    if(readdata>0)
    {
        kDebug() << "got data: " << buffer << endl;
        d->buffer+=buffer;
        d->gotData=availData;
//         std::cout << QString(buffer).replace("\r", "\n").replace("\n\n", "\n");
    }
}




/*!
    \fn SerialManager::speed(int value)
 */
 /// @TODO have to move this somewhere...
 /// @TODO better baudrate management

void SerialManager::setSpeed(int value)
{
    switch( value ){
        case 0:
            d->m_baudrate=QSerial::BAUD_9600;
            break;
        case 1:
            d->m_baudrate= QSerial::BAUD_19200;
            break;
        case 2:
            d->m_baudrate= QSerial::BAUD_38400;
            break;
        case 3:
            d->m_baudrate= QSerial::BAUD_57600;
            break;
        case 4:
            d->m_baudrate= QSerial::BAUD_115200;
            break;
        case 5:
            d->m_baudrate= QSerial::BAUD_230400;
            break;
        default:
            d->m_baudrate= QSerial::BAUD_57600;
            break;
    }
}

QString SerialManager::decodePDU( const QString &text )
{
    QString decoded;
    for ( int i=0; i<text.length(); i += 2 )
    {
        decoded.append( QChar( text.mid( i, 2 ).toInt( 0,16 ) ) );
    }
    return decoded;
}


/*!
    \fn KMobileTools::SerialManager::atError(const QString &buffer)
 */
bool KMobileTools::SerialManager::ATError(const QString &buffer)
{
    if(!buffer.length() ) return true;
    int i=buffer.lastIndexOf( "ERROR" );
    if(i==-1) return false;
    if((buffer.length()-i)==5) return true; // "ERROR" is the last part of the string
    if(buffer.mid(i+5).contains( "[^\\n\\r]" )) return false;
    return true;
}

void KMobileTools::SerialManager::gotError(int err)
{
    d->errnum=err;
}



/*!
    \fn KMobileTools::SerialManager::log(bool incoming, const QString &data)
 */
void KMobileTools::SerialManager::log(bool incoming, const QString &data)
{
    if(!d->log) return;
    if(incoming)
    {
        d->logstream << "<<<" << QString(data).replace("\r", "\n").replace("\n\n", "\n") << endl;
        return;
    }
    d->logstream << ">>>" << QString(data).replace("\r","\n").replace("\n\n", "\n") << endl;

}
