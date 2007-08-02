/* This file is part of the KDE project
   Copyright (c) 2004 Kevin Ottens <ervin ipsquad net>
   Rearranged by Marco Gulino <marco@kmobiletools.org> for the mobile protocol

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "kio_p2kprotocol.h"

#include <q3cstring.h>
#include <q3socket.h>
#include <qdatetime.h>
#include <qbitarray.h>

#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kinstance.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kurl.h>
#include <ksock.h>
#include "devicesconfig.h"

#include <qfile.h>
#include "p2kwrapper.h"
#include <dcopclient.h>


using namespace KIO;

kio_p2kProtocol::kio_p2kProtocol(const Q3CString &pool_socket, const Q3CString &app_socket)
    : SlaveBase("kio_p2k", pool_socket, app_socket)
{
    kDebug() <<"kio_p2kProtocol::kio_p2kProtocol";
//     kmt_stub=0;
//     p_dcop=new DCOPClient();
//     p_dcop->attach();
}


kio_p2kProtocol::~kio_p2kProtocol()
{
//     if ( kmt_stub ) kmt_stub->resumeDevice();

    kDebug() <<"kio_p2kProtocol::~kio_p2kProtocol";
}


void kio_p2kProtocol::disconnectTimeout()
{
    kDebug() <<"Finished, trying to disconnect.";
    QByteArray cmd( 1 );
    cmd[0] = 'd';
    setTimeoutSpecialCommand( 2, cmd );
}

void kio_p2kProtocol::special(const QByteArray &data)
{
    if ( data.size() == 1 && data.at(0) == 'd' ) {
    // we got a disconnect command
        infoMessage( i18n( "Disconnecting" ) );
        m_impl.wrapper()->closePhone();
        infoMessage( i18n( "Disconnected" ) );
    }
}


void kio_p2kProtocol::listDir(const KUrl &url)
{
    kDebug() <<"P2KProtocol::listDir:" << url;
    if ( url.path().length() <= 1 )
    {
        listRoot(url);
        disconnectTimeout();
        return;
    }
    kDebug() <<"not root, listing current directory";


    KIO::UDSEntry entry;

    KIO::UDSEntryList mobile_entries;
    bool ok = m_impl.listDirectory(url,mobile_entries);

    if (!ok)
    {
        error( m_impl.lastErrorCode(), m_impl.lastErrorMessage() );
        disconnectTimeout();
        return;
    }

    totalSize(mobile_entries.count()+1);

    m_impl.createTopLevelEntry(entry);
    listEntry(entry, false);

    KIO::UDSEntryListIterator it = mobile_entries.begin();
    KIO::UDSEntryListIterator end = mobile_entries.end();

    for(; it!=end; ++it)
    {
        listEntry(*it, false);
    }

    entry.clear();
    listEntry(entry, true);
    disconnectTimeout();
    finished();
}

void kio_p2kProtocol::setHost(const QString &host, int port, const QString &user, const QString &pass)
{
    kDebug() <<"kio_p2kProtocol::setHost(" << host <<"," << port <<"," << user <<"," << pass <<")";
    int v, p;
    v=host.section('.', 0, 0).toInt(0, 16);
    p=host.section('.', 1, 1).toInt(0, 16);
    m_impl.wrapper()->setP2KConfig(v, p);
}


void kio_p2kProtocol::stat(const KUrl &url)
{
    kDebug() <<"**************** P2KProtocol::stat:" << url;

//     listDir(url);
//     finished();
//     return;
    KIO::UDSEntry entry;
    if ( url.path().isEmpty() || url.path() == "/" )
    {
        kDebug() <<"<<<<<<<--------Stat for root directory //////////";
        m_impl.createTopLevelEntry( entry );
        statEntry( entry );
        finished();
        disconnectTimeout();

        return;
    }
    bool ok=m_impl.statEntry( url, entry );
    if (!ok)
    {
        kDebug() <<">>>>><<<<<m stat standard entry";
        m_impl.createTopLevelEntry(entry);
        statEntry(entry);
        finished();
        disconnectTimeout();

        return;
    } else {
        kDebug() <<"<<<<<<<<<<<<---------- stat void entry";
    statEntry( entry );
    disconnectTimeout();

    finished();
    return;
    }
/*    QString path = url.path();
    if ( true || path.isEmpty() || path == "/" )
    {
                // The root is "virtual" - it's not a single physical directory
        KIO::UDSEntry entry;
        m_impl.createTopLevelEntry( entry );
        statEntry( entry );
        finished();
        return;
    }
*/
}

void kio_p2kProtocol::listRoot(const KUrl &url)
{
    kDebug() <<"P2KProtocol::listRoot()";

    KIO::UDSEntry entry;

    KIO::UDSEntryList mobile_entries;
    bool ok = m_impl.listRoot(url,mobile_entries);

    if (!ok)
    {
        disconnectTimeout();
        error( m_impl.lastErrorCode(), m_impl.lastErrorMessage() );
        return;
    }

    totalSize(mobile_entries.count()+1);

    m_impl.createTopLevelEntry(entry);
    listEntry(entry, false);

    KIO::UDSEntryListIterator it = mobile_entries.begin();
    KIO::UDSEntryListIterator end = mobile_entries.end();

    for(; it!=end; ++it)
    {
        listEntry(*it, false);
    }

    entry.clear();
    listEntry(entry, true);
    disconnectTimeout();

    finished();
}

extern "C"
{
    int kdemain(int argc, char **argv)
    {
        KInstance instance( "kio_p2k" );
        
        kDebug(7101) <<"*** Starting kio_p2k";
        
        if (argc != 4) {
            kDebug(7101) <<"Usage: kio_p2k  protocol domain-socket1 domain-socket2";
            exit(-1);
        }
        
        kio_p2kProtocol slave(argv[2], argv[3]);
        slave.dispatchLoop();
        
        kDebug(7101) <<"*** kio_p2k Done";
        return 0;
    }
}


/*!
    \fn kio_p2kProtocol::get(const KUrl &url)
 */
void kio_p2kProtocol::get(const KUrl &url)
{
    kDebug() <<"***************** kio_p2k getFile:" <<  url;
    int size=m_impl.getFile(url, 0);
    char* buffer=new char[size];
    m_impl.getFile(url,buffer);
    kDebug() <<"**************************GetFile processed; size returned:" << size <<"; raw data:" << buffer;
    if(size<=0)
    {
        delete [] buffer;
        finished();
        disconnectTimeout();
        return;
    }
    QByteArray m_data;
    m_data.setRawData( buffer,size);
    data( m_data );
    processedSize( m_data.size() );

    m_data.resetRawData( buffer, size );
    data(QByteArray());
    delete [] buffer;
    disconnectTimeout();
    finished();
}


/*!
    \fn kio_p2kProtocol::put(const KUrl &url, int permissions, bool overwrite, bool resume)
 */
void kio_p2kProtocol::put(const KUrl &url, int permissions, bool overwrite, bool resume)
{
    kDebug() <<"URL:" << url <<"; permissions:" << permissions <<"; overwrite:" << overwrite <<"; resume:" << resume;
    QByteArray buffer;
    dataReq();
    int result= readData( buffer );
    kDebug() <<"SendData result code:" << m_impl.wrapper()->putFile( url.path(), buffer.data(), buffer.size() );
    disconnectTimeout();
    finished();
}


/*!
    \fn kio_p2kProtocol::del (const KUrl &url, bool isfile)
 */
void kio_p2kProtocol::del (const KUrl &url, bool isfile)
{
    kDebug() <<"Deleting" << url <<"; is file?" << isfile <<"; result:" << m_impl.wrapper()->deleteFile( url.path() );
    disconnectTimeout();
    finished();
}
