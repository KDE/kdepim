/***************************************************************************
   Copyright (c) 2004 Kevin Ottens <ervin ipsquad net>
   Rearranged by Marco Gulino <marco@kmobiletools.org> for the mobile protocol

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

#include "mobile.h"

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
#include "deviceIFace_stub.h"

#include <kapplication.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kinstance.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kurl.h>
#include <ksock.h>
#include <kmobiletools_devices.h>

using namespace KIO;


kio_mobileProtocol::kio_mobileProtocol(const Q3CString &pool_socket, const Q3CString &app_socket)
    : SlaveBase("kio_mobile", pool_socket, app_socket)
{
    kDebug() << "kio_mobileProtocol::kio_mobileProtocol()" << endl;
    m_dcop.attach();
}


kio_mobileProtocol::~kio_mobileProtocol()
{
    kDebug() << "kio_mobileProtocol::~kio_mobileProtocol()" << endl;
}

// From here, it's copied and adapted from kdebase/kioslave/system
void kio_mobileProtocol::listDir(const KUrl &url)
{
    kDebug() << "MobileProtocol::listDir: " << url << endl;

    if ( url.path().length() <= 1 )
    {
        listRoot();
        return;
    }
    QString host=QStringList::split("/",url.path() ).first();
    KMobileTools::DevicesConfig *hostCFG=KMobileTools::DevicesConfig::prefs(host);
    hostCFG->readConfig();
    kDebug() << "Reading config\n";
    kDebug() << "FSType:" << hostCFG->fstype() << endl;
    KUrl target;
    switch( hostCFG->fstype() ){
        case 1:
            kDebug() << "P2K Filesystem\n";
            target.setProtocol("p2k");
            target.setUser( host);
            target.setHost(QString("%1.%2")
                    .arg(hostCFG->p2k_vendor())
                    .arg(hostCFG->p2k_prodid()) );
            break;
        case 2:
            kDebug() << "ObexFTP Filesystem\n";
            target.setProtocol("obex2");
            switch( hostCFG->obex_transp() ){
                case 0:
                    target.setHost( "irda"); break;
                case 1:
                    target.setHost("inet"); break;
                case 2:
                    target.setHost("custom"); break;
                case 3:
                default:
                    target.setHost("bluetooth"); break;
                case 4:
                    target.setHost("usb"); break;
            }
            kDebug() << "Obex transport: " << hostCFG->obex_transp() << "; device: " << hostCFG->obex_device() << "; port: " << hostCFG->obex_port() << endl;
            target.setUser(hostCFG->obex_device().replace("/", "\\") );
            if(hostCFG->obex_port()) target.setPort(hostCFG->obex_port());
            break;
    }
    kDebug() << "Resulting URL: \"" << target << "\"; is valid: " << target.isValid() << endl;
//     target.setProtocol("p2k");
//     target.setHost(host);
//     kDebug() << "Path: " << url.path() << "; host: " << url.host() << endl;
//     kDebug() << "possible redirection target : " << target << endl;
    if( target.isValid() )
    {
        DeviceIFace_stub dcopDevice(&m_dcop, Q3CString("kmobiletools"), host.utf8() );
        kDebug() << "launching switch2filesystem() to `" << dcopDevice.app() << "` -- `" << dcopDevice.obj() << "`" << endl;
        dcopDevice.switch2filesystem();
        kDebug() << "Call failed: " << (dcopDevice.status() == DCOPStub::CallFailed) << endl;
        redirection(target);
        finished();
        return;
    }

    error(KIO::ERR_MALFORMED_URL, url.prettyUrl());
}

void kio_mobileProtocol::stat(const KUrl &url)
{
    kDebug() << "MobileProtocol::stat: " << url << endl;

    QString path = url.path();
    if ( path.isEmpty() || path == "/" )
    {
                // The root is "virtual" - it's not a single physical directory
        KIO::UDSEntry entry;
        m_impl.createTopLevelEntry( entry );
        statEntry( entry );
        finished();
        return;
    }


//     error(KIO::ERR_MALFORMED_URL, url.prettyUrl());
}


void kio_mobileProtocol::listRoot()
{
    KIO::UDSEntry entry;

    KIO::UDSEntryList mobile_entries;
    bool ok = m_impl.listRoot(mobile_entries);

    if (!ok)
    {
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

    finished();
}

// end of the system kioslave part

extern "C"
{
    int kdemain(int argc, char **argv)
    {
        KInstance instance( "kio_mobile" );
        
        kDebug(7101) << "*** Starting kio_mobile " << endl;
        
        if (argc != 4) {
            kDebug(7101) << "Usage: kio_mobile  protocol domain-socket1 domain-socket2" << endl;
            exit(-1);
        }
        
        kio_mobileProtocol slave(argv[2], argv[3]);
        slave.dispatchLoop();
        
        kDebug(7101) << "*** kio_mobile Done" << endl;
        return 0;
    }
} 
