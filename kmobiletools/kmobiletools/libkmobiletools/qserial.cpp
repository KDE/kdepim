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
#include "qserial.h"

// STD C includes
#include <fcntl.h>
#include <cerrno>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/select.h>

// QT/KDE Includes
#include <kdebug.h>
#include <qfile.h>
#include <qapplication.h>
#include <kuser.h>
#include <qdir.h>
#include <qsocketnotifier.h>
#include "kmobiletoolshelper.h"


namespace KMobileTools {

class QSerialPrivate {
    public:
        QSerialPrivate() : m_baudrate(QSerial::BAUD_DEFAULT), m_parity(QSerial::PARITY_DEFAULT),
        m_stopbits(QSerial::STOP_BITS_DEFAULT), m_flowcontrol(QSerial::FLOW_CONTROL_DEFAULT), m_databits(QSerial::DATABITS_DEFAULT),
        i_modem(-1), b_lock(false), notifier(0), b_opened(false)
        {};
        QString m_device;
        QSerial::Baud m_baudrate;
        QSerial::Parity m_parity;
        QSerial::StopBits m_stopbits;
        QSerial::FlowControl m_flowcontrol;
        QSerial::DataBits m_databits;
        int i_modem;
        bool b_lock;
        QString s_lockFile;
        QSocketNotifier *notifier;
        bool b_opened;
};

QSerial::QSerial()
    : QIODevice()
{
    createObject();
}

QSerial::QSerial( const QString &deviceName )
{
    kDebug() << "QSerial(" << deviceName << ")" << endl;
    createObject();
    setName(deviceName);
}

void QSerial::createObject()
{
    d=new QSerialPrivate;
    d->b_opened=false;
}


QSerial::~QSerial()
{
    close();
    delete d;
}

bool QSerial::open(OpenMode mode, bool createLockFile)
{
    kDebug() << "QSerial::open: device " << d->m_device << ", mode: " << mode << ", createlock=" << createLockFile << endl;
    int iomode=0, retry=3;
    // Fixing weird behaviour of QIODevice open flags
    if (mode & QIODevice::ReadWrite) iomode = iomode | O_RDWR;
    else
    {
        if ( mode & QIODevice::ReadOnly ) iomode = iomode | O_RDONLY;
        if ( mode & QIODevice::WriteOnly ) iomode = iomode | O_WRONLY;
    }
    iomode = iomode | ( mode & ~QIODevice::ReadWrite);
    if(d->m_device.isNull() || isOpen() ) return false;
    d->b_lock=false;
    if(createLockFile)
    {
        d->b_lock=lockFile( true );
        if(! d->b_lock)
        {
            kDebug() << "ERROR! Couldn't create lockfile for " << d->m_device << endl;
            return false;
        }
    }
    // Retry a few times.
    while( retry > 0 ) {
        d->i_modem = ::open( d->m_device.toLatin1() , iomode | O_NONBLOCK | O_NOCTTY /*| O_NOCTTY | O_NONBLOCK*/ );
//     kDebug() << "Trying to open " << d->m_device << " in mode " << iomode << ".." << d->i_modem << endl;
        if( d->i_modem != -1 ) break;

            KMobileTools::Thread::sleep(1);
        retry--;
    }
    if(d->i_modem==-1)
    {
        if(createLockFile) lockFile( false ); // Removing created lockfile
        perror( QString("Error while opening %1: ").arg(d->m_device).toLatin1() );
        return false;
    }
    setOpenMode(mode);
    // Patch from cutecom, provided by Bernhard Schiffner <bernhard AT schiffner-limbach DOT de>
    // flushing device right after opening to 'prevent first read and write to be spam'ish.'
    tcflush( d->i_modem, TCIOFLUSH);
    setupParameters();
    d->b_opened=true;
//     d->notifier=new QSocketNotifier(d->i_modem, QSocketNotifier::Read, this);
//     d->notifier->setEnabled(true);
//     connect(d->notifier, SIGNAL(activated(int)), this, SLOT(slotNotifierData( int ) ));
    return true;
}

void QSerial::slotNotifierData(int fd)
{
    kDebug() << "Serial::slotNotifierData(" << fd << ")" << endl;
    if(fd!=d->i_modem) return;
    emit gotData();
}

void QSerial::setupParameters()
{
    if( d->i_modem == -1 ) return;
    struct termios newtio;
    // Some methods are derived from cutecom
    // Copyright (C) 2007-2005 Alexander Neundorf <neundorf@kde.org>

    int n = fcntl(d->i_modem, F_GETFL, 0);
    fcntl(d->i_modem, F_SETFL, n & ~O_NDELAY);
//     memset(&newtio, 0, sizeof(newtio));
    if (tcgetattr(d->i_modem, &newtio)!=0)
    {
        kDebug() <<"tcgetattr() 3 failed"<<endl;
        perror("Error on setup: ");
    }
    /* We generate mark and space parity ourself. */
    if (d->m_databits == DATABITS_7 && ( d->m_parity==PARITY_MARK || d->m_parity==PARITY_SPACE ))
        d->m_databits = DATABITS_8;
    switch (d->m_databits)
    {
        case DATABITS_5:
            newtio.c_cflag = (newtio.c_cflag & ~CSIZE) | CS5;
            break;
        case DATABITS_6:
            newtio.c_cflag = (newtio.c_cflag & ~CSIZE) | CS6;
            break;
        case DATABITS_7:
            newtio.c_cflag = (newtio.c_cflag & ~CSIZE) | CS7;
            break;
        case DATABITS_8:
        default:
            newtio.c_cflag = (newtio.c_cflag & ~CSIZE) | CS8;
            break;
    }
    newtio.c_cflag |= CLOCAL | CREAD;

    newtio.c_cflag &= ~(PARENB | PARODD);
    if (d->m_parity == PARITY_EVEN)
        newtio.c_cflag |= PARENB;
    else if (d->m_parity== PARITY_ODD)
        newtio.c_cflag |= (PARENB | PARODD);
    if ( d->m_stopbits == STOP_BITS_2 )
        newtio.c_cflag |= CSTOPB;
    else
        newtio.c_cflag &= ~CSTOPB;
    /// @TODO verify this
    newtio.c_iflag=IGNBRK;

    if ( d->m_flowcontrol == FLOW_CONTROL_XONXOFF )
        newtio.c_iflag |= IXON | IXOFF;
    else
        newtio.c_iflag &= ~(IXON|IXOFF|IXANY);

    newtio.c_lflag=0;

    newtio.c_oflag=0;

    newtio.c_cc[VTIME]=17;
    newtio.c_cc[VMIN]=19;
    cfsetispeed(&newtio, (speed_t)d->m_baudrate );
    cfsetospeed(&newtio, (speed_t)d->m_baudrate );

    kDebug() << "BaudRate:" << d->m_baudrate<< endl;
    if (tcsetattr(d->i_modem, TCSANOW, &newtio)!=0)
    {
        kDebug() << "Unable to setup serial parameters (tcsetattr() 1)\n";
        perror("Error on setup: ");
    }
    int mcs=0;
//   ioctl(m_fd, TIOCMODG, &mcs);
    ioctl(d->i_modem, TIOCMGET, &mcs);
    mcs |= TIOCM_RTS;
    ioctl(d->i_modem, TIOCMSET, &mcs);

    if (tcgetattr(d->i_modem, &newtio)!=0)
    {
        kDebug() << "Unable to setup serial parameters (tcsetattr() 4)\n";
        perror("Error on setup: ");
    }
//hardware handshake
    if (d->m_flowcontrol==FLOW_CONTROL_HARD)
        newtio.c_cflag |= CRTSCTS;
    else
        newtio.c_cflag &= ~CRTSCTS;
/*  if (on)
    newtio.c_cflag |= CRTSCTS;
    else
    newtio.c_cflag &= ~CRTSCTS;*/
    if (tcsetattr(d->i_modem, TCSANOW, &newtio)!=0)
    {
        kDebug() << "Unable to setup serial parameters (tcsetattr() 2)\n";
        perror("Error on setup: ");
    }
    n = fcntl(d->i_modem, F_GETFL, 0);
    fcntl(d->i_modem, F_SETFL, n | O_NONBLOCK);
}




/*!
    \fn KMobileTools::QSerial::close()
 */
void QSerial::close()
{
    if(! isOpen() ) return;
//     delete d->notifier;
    usleep(2000);
    flush();
    if(d->i_modem != -1  && ::close(d->i_modem)==0 )
        d->i_modem=-1;
    if(d->i_modem==-1) d->b_opened=false;
    usleep(200000);
    if(d->b_lock) lockFile( false);
    setOpenMode(NotOpen);
}


/*!
    \fn KMobileTools::QSerial::lockFile(bool lock)
 */
bool QSerial::lockFile(bool lock)
{
    if(!lock)
    {
        kDebug() << "removing lockfile " << d->m_device << endl;
        // Here we close lock file.
        if(!d->b_lock) return false;
        if(unlink( d->s_lockFile.toLatin1() )==-1)
        {
            perror( QString("Error while removing lockfile %1").append(d->s_lockFile).toLatin1() );
            return false;
        } else
        {
            d->b_lock=false;
            return true;
        }
    }

    // Now for creating it.
    if(d->b_lock || d->m_device.isNull() ) return false;
    d->s_lockFile=lockFileName();
    kDebug() << "LockFileName==" << d->s_lockFile << endl;

    // First check if it already exists.
    QFile lck(d->s_lockFile);
    if(lck.exists())
    {
        QByteArray stream;
        if ( lck.open( QIODevice::ReadOnly ) ) {
            stream=lck.readAll();
            lck.close();
        }
        QString data(stream);
        int pid=data.section( ' ', 0, 0, QString::SectionSkipEmpty ).toInt();
        if( kill(pid,0)==-1 && errno==ESRCH )
        {
            if(lck.remove())
                kDebug() << "Correctly removed stale lock: " << lck.fileName() << endl;
            else
            {
                kDebug() << "!!! WARNING !!! couldn't remove stale lock file " << lck.fileName() << endl;
                return false;
            }
        } else
        {
            kDebug() << "QSerial::lockFile(): is already locked by another application\n";
            return false;
        }
    }
    int i_lockfile= ::open( d->s_lockFile.toLatin1(), O_WRONLY | O_CREAT | O_EXCL,
                            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH );
    if(i_lockfile!=-1)
    {
        QString towrite="     %1 %2 %3\x0A";
        towrite=towrite.arg(getpid() ).arg( QApplication::arguments().at(0) ).arg( KUser(getuid() ).loginName ());
        if( ::write(i_lockfile, towrite.toLatin1(), towrite.length() ) ==-1) perror("Error on write: ");
    }
    ::close (i_lockfile);
    kDebug() << "LockFile created: " << (i_lockfile!=-1) << endl;
    return (i_lockfile!=-1);
}

const QString QSerial::lockFileName()
{
    if(d->m_device.isNull() ) return QString() ;
    return QDir::cleanPath(d->m_device).section( QDir::separator() , -1 ).prepend( "/var/lock/LCK.." ); /// @TODO find if this could be a standard solution..
}

void QSerial::flush()
{
    if (!isOpen()) return;
    tcdrain(d->i_modem);
}

qint64 QSerial::writeData( const char *data, qint64 len )
{
    kDebug() << "QSerial::writeData()\n";
    if (!isOpen())
    {
        kDebug() << "Can't write to serial port: device \"" << d->m_device << "\" is still closed\n";
        return -1;
    }
//     kDebug() << "Serial port: sending " << data << " with length " << len << endl;
#define TEMPBUFFERSIZE 30
    char temp[TEMPBUFFERSIZE];
    long retval=0;
    int towrite=0;
    struct timeval tv;
    fd_set rfds;
    int c_retval;
    for(unsigned int i=0;i<len;i+=TEMPBUFFERSIZE)
    // it seems that sending more than 32 bytes without a tcdrain can hung the connection, so we're dividing the command string.
    {
        if(i>=len) break;
        memset(temp,0,sizeof(temp));
        towrite=(len-i>=TEMPBUFFERSIZE) ? TEMPBUFFERSIZE : len-i;
        memcpy(temp, &data[i], towrite);

        tv.tv_sec = 3;
        tv.tv_usec = 0;
        FD_ZERO(&rfds);
        FD_SET(d->i_modem, &rfds);
        select(d->i_modem+1, NULL, &rfds, NULL, &tv);
        char c_retry=0;

        do {
            if(d->i_modem!=-1) c_retval=::write(d->i_modem,temp, towrite); else return -1;
            flush();
            if (c_retval==-1)
            {
                usleep(TEMPBUFFERSIZE*5000); // since we already had a -1 (error), use a LARGE timeout for next retry
                if( c_retry>2 )
                {
                    perror( QString("Write error for %1: ").arg(d->m_device).toLatin1() );
                    return -1;
                }
            }
            c_retry++;
        } while (c_retval==-1);
        retval+=c_retval;
    }
    return retval;
}

qint64 QSerial::size() const
{
    qint64 u_toread=0;
    if(! isOpen() )
    {
        kDebug() << "Trying to get size() without having the serial port open, returning 0\n";
        return 0;
    }
    if( ioctl(d->i_modem,FIONREAD,&u_toread)== -1)
    {
        perror("Error reading avail size: ");
        return 0;
    }
//     kDebug() << "Reading size ok: avail size=" << u_toread << endl;
    return u_toread;
}

qint64 QSerial::readData( char *data, qint64 maxlen )
{
    if (!isOpen()) return -1;
//     memset(data, 0, maxlen);
    long retval=::read(d->i_modem, data, maxlen);
//     if(retval<0) perror("Read error: " );
    return retval;
}

int QSerial::getch()
{
    if (!isOpen() || !size() ) return -1;
    int getchar=0;
    if(::read(d->i_modem, &getchar, 1) == -1 )
    {
        return -1;
    }
    else return getchar;
}

int QSerial::putch(int ch)
{
    if (!isOpen() ) return -1;
    if( ::write(d->i_modem, &ch, 1)==-1)
    {
        return -1;
    }
    else
    {
        flush();
        return ch;
    }
}

bool QSerial::reset()
{
    if( ::write(d->i_modem, "\x1A\r", 2) == -1) return false;
    flush();
    return true;
}

bool QSerial::isOpen() const
{
    return d->b_opened;
}


QSerial::Baud QSerial::baud() { return d->m_baudrate; }

void QSerial::setBaud( QSerial::Baud baudrate)
{
    d->m_baudrate=baudrate;
    setupParameters();
}
QSerial::Parity QSerial::parity() { return d->m_parity; }
void QSerial::setParity( Parity parity)
{
    d->m_parity=parity;
    setupParameters();
}

QSerial::StopBits QSerial::stopBits() { return d->m_stopbits; }
void QSerial::setStopBits( QSerial::StopBits stopbits)
{
    d->m_stopbits=stopbits;
    setupParameters();
}

QSerial::FlowControl QSerial::flowControl() { return d->m_flowcontrol; }

void QSerial::setFlowControl( QSerial::FlowControl flowcontrol)
{
    d->m_flowcontrol=flowcontrol;
    setupParameters();
}
QSerial::DataBits QSerial::databits() { return d->m_databits; }
void QSerial::setDatabits( DataBits databits)
{
    d->m_databits=databits;
    setupParameters();
}


QString QSerial::name() const { return d->m_device; }
void QSerial::setName(const QString &name) { if (! isOpen() ) d->m_device=name; }
bool QSerial::open( OpenMode mode ) { return open(mode, true); }
int  QSerial::ungetch( int ch ) { /** @TODO unimplemented */ return  (ch * 0 ) -1; }
bool QSerial::isSequential() const { return true; }

}



#include "qserial.moc"

