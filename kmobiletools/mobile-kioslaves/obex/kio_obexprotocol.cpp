/***************************************************************************
*   Copyright (C) 2006 by                                                 *
*   Marcin Przylucki <marcin.przylucki@kdemail.net>                       *
*   Marco Gulino <marco@kmobiletools.org>                                 *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,       *
*  Boston, MA 02110-1301, USA.                                            *
***************************************************************************/


#include "kio_obexprotocol.h"
#include "obexwrapper.h"
#include "obeximpl.h"

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
#include <qfile.h>


using namespace KIO;


kio_obexProtocol::kio_obexProtocol(const Q3CString &pool_socket, const Q3CString &app_socket)
    : SlaveBase("kio_obex2", pool_socket, app_socket)
{
    kDebug() <<"kio_obexProtocol::kio_obexProtocol()";

}


kio_obexProtocol::~kio_obexProtocol()
{
    kDebug() <<"kio_obexProtocol::~kio_obexProtocol()";
    m_impl.wrapper()->disconnectClient();
}

void kio_obexProtocol::setHost( const QString & host, int port, const QString &user, const QString &pass )
{
    kDebug() <<"kio_obexProtocol::setHost(host=" << host <<", port=" << port <<", user=" << user <<", pass=" << pass <<")";
    QString bhost=user;
    bhost=bhost.replace('\\', "/");
    if(bhost.contains("nodevice")) bhost.clear() ;
    /**
    this is what i have in mind:
     * host = the "device" parameter of obexwrapper
     * port = the "port" parameter
     * user = the transport, i.e. bt or bluetooth, or irda, usb, and so on...
    */
    OBEXImpl::transport transport=OBEXImpl::null;
    if(host.contains( "bt") || host.contains( "bluetooth") ) transport=OBEXImpl::Bluetooth;
    if(host.contains( "irda") ) transport=OBEXImpl::IrDA;
    if(host.contains( "inet") ) transport=OBEXImpl::Inet;
    if(host.contains( "usb") ) transport=OBEXImpl::USB;
    if(host.contains( "fd") ) transport=OBEXImpl::FD;
    if(host.contains( "custom") ) transport=OBEXImpl::Custom;
    if(transport==OBEXImpl::null) transport=OBEXImpl::Bluetooth; // backward compatibility with the kdebluetooth obex kioslave
    m_impl.setHostConfig(bhost, port, transport);
}


void kio_obexProtocol::listDir(const KUrl &url)
{
    kDebug() <<"obexProtocol::listDir:" << url;
    //if (! setHostConfig( url.host() ) )return;

    KIO::UDSEntry entry;
    KIO::UDSEntryList mobile_entries;

    bool ok = m_impl.listDirectory(url,mobile_entries);
    if (!ok)
    {
        error( m_impl.lastErrorCode(), m_impl.lastErrorMessage() );
        return;
    }
    totalSize(mobile_entries.count()+1);

    m_impl.createTopLevelEntry(entry);
    
    kDebug() <<"obexProtocol::listDir: mobile_entries.count()+1 =" << mobile_entries.count()+1;

    listEntry(entry, false);

    KIO::UDSEntryListIterator it = mobile_entries.begin();
    KIO::UDSEntryListIterator end = mobile_entries.end();

    Q3ValueListIterator<UDSAtom> ait;
    Q3ValueListIterator<UDSAtom> aend;

    for(; it!=end; ++it)
    {
        listEntry(*it, false);
    }

    entry.clear();
    listEntry(entry, true);

    finished();
    disconnectTimeout();
}


void kio_obexProtocol::stat(const KUrl &url)
{
    kDebug() <<"**************** obexProtocol::stat:" << url;
    
    //if (! setHostConfig( url.host() ) )return;

    KIO::UDSEntry entry;
    if ( url.path().isEmpty() || url.path() == "/" )
    {
        m_impl.createTopLevelEntry( entry );
        statEntry( entry );
        finished();
        return;
    }
    bool ok = m_impl.statEntry( url, entry );
    if (!ok)
    {
        m_impl.createTopLevelEntry(entry);
        statEntry(entry);
        finished();
        return;
    }
    statEntry( entry );

    finished();
    disconnectTimeout();
}


void kio_obexProtocol::listRoot(const KUrl &url)
{
    kDebug() <<"obexProtocol::listRoot()";
    listDir(url);
}


/*!
    \fn kio_obexProtocol::get(const KUrl &url)
*/

void kio_obexProtocol::get(const KUrl &url)
{
    kDebug() <<"***************** kio_obex getFile:" <<  url;

    if ( m_impl.getFile(url) ) return;

    int size = m_impl.wrapper()->bufferSize();

    char* buffer = (char*)m_impl.wrapper()->getClient()->buf_data;

    kDebug() <<"**************************GetFile processed; size returned:" << size <</*"; raw data:" << buffer <<*/ endl;

    if(size<=0)
    {
        finished();
        return;
    }

    QByteArray m_data;
    m_data.setRawData( buffer, size);
    data( m_data );
    processedSize( m_data.size() );

    m_data.resetRawData( buffer, size );
    data(QByteArray());
    finished();
    disconnectTimeout();
}


/*!
    \fn kio_obexProtocol::put(const KUrl &url, int permissions, bool overwrite, bool resume)
*/
void kio_obexProtocol::put(const KUrl &url, int permissions, bool overwrite, bool resume)
{
    kDebug() <<"kio_obexProtocol::put()  URL:" << url <<"; permissions:" << permissions <<"; overwrite:" << overwrite <<"; resume:" << resume;

    QByteArray buffer;
    dataReq();
    int ret = 0;

    int result = readData( buffer );
    if ( result ) {
        ret = m_impl.wrapper()->putFile( url.path(), buffer.data(), buffer.size() );
    }

    kDebug() <<"SendData result code:" << ret;

    finished();
    disconnectTimeout();
}


/*!
    \fn kio_obexProtocol::del (const KUrl &url, bool isfile)
*/
void kio_obexProtocol::del (const KUrl &url, bool isfile)
{
    kDebug() <<"Deleting" << url <<"; is file?" << isfile <<"; result:" << m_impl.wrapper()->deleteFile( url.path() );
    finished();
    disconnectTimeout();
}


void kio_obexProtocol::disconnectTimeout()
{
    kDebug() <<"Finished, trying to disconnect.";
    QByteArray cmd( 1 );
    cmd[0] = 'd';
    setTimeoutSpecialCommand( 2, cmd );
}

void kio_obexProtocol::special(const QByteArray &data)
{
    if ( data.size() == 1 && data.at(0) == 'd' ) {
    // we got a disconnect command
        infoMessage( i18n( "Disconnecting" ) );
        m_impl.wrapper()->disconnectClient();
        infoMessage( i18n( "Disconnected" ) );
    }
}


void kio_obexProtocol::mkdir( const KUrl &url, int permissions )
{
    kDebug() <<"Makeing dir" << url <<"; permissions" << permissions <<"; result:" << m_impl.wrapper()->mkDir( url.path() );
    finished();
    disconnectTimeout();
}


extern "C"
{
int kdemain(int argc, char **argv)
{
    KInstance instance( "kio_obex2" );

    kDebug(7101) <<"*** Starting kio_obex";

    if (argc != 4)
    {
    kDebug(7101) <<"Usage: kio_obex  protocol domain-socket1 domain-socket2";
    exit(-1);
    }

    kio_obexProtocol slave(argv[2], argv[3]);
    slave.dispatchLoop();

    kDebug(7101) <<"*** kio_obex Done";
    return 0;
}
}
