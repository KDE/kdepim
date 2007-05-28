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


#include "obexwrapper.h"
#include <kdebug.h>

#include <iostream>
#include <unistd.h>
//Added by qt3to4:
#include <Q3ValueList>

using namespace std;

OBEXWrapper *OBEXWrapper::p_instance=0;


OBEXWrapper::OBEXWrapper( QObject *parent, const char *name) : QObject(parent, name),
    client(0), n_files(0)
{
    p_instance=this;
}

void OBEXWrapper::setupParameters(	int transport,
                            int channel,
                            const QString &device,
                            const uint8_t *use_uuid,
                            int use_uuid_len,
                            int use_conn,
                            int use_path)
{
    i_transport = transport;
    i_channel = channel;
    c_device = device;
    c_use_uuid = use_uuid;
    i_use_conn = use_conn;
    i_use_path = use_path;
    i_use_uuid_len = use_uuid_len;
    client=0;
    kDebug() << "Initialized obex lib" << endl;
    n_files=0;
}

OBEXWrapper::~OBEXWrapper()
{
    kDebug() << "OBEXWrapper::~OBEXWrapper()\n";
    kDebug() << "OBEXWrapper::ClosingPhone\n";

    if (client != NULL) {
        /* Disconnect */
        (void) obexftp_disconnect (client);
        /* Close */
        obexftp_close (client);
    }
}


static void info_cb(int event, const char *msg, int len, void *data)
{
    static unsigned int i = 0;
    char progress[] = "\\|/-";
    kDebug() << "Info_cb: " << endl;

    switch (event) {

    case OBEXFTP_EV_ERRMSG:
        kDebug() << "Error: " << msg << endl;
        i=0;
        break;

    case OBEXFTP_EV_ERR:
        kDebug() << "failed: " << msg << endl;
        i=0;
        break;
    case OBEXFTP_EV_OK:
        kDebug() << "done" << endl;
        i=0;
        break;

    case OBEXFTP_EV_CONNECTING:
        kDebug() << "Connecting..." << endl;
//        fprintf(stderr, "");
        break;
    case OBEXFTP_EV_DISCONNECTING:
        kDebug() << "Disconnecting..." << endl;
        break;
    case OBEXFTP_EV_SENDING:
        kDebug() << "Sending \"" << msg << "\"... " << endl;
        break;
    case OBEXFTP_EV_RECEIVING:
        kDebug() << "Reciving \"" << msg << "\"... " << endl;
        break;

    case OBEXFTP_EV_LISTENING:
        kDebug() << "Waiting for incoming connection" << endl;
        break;

    case OBEXFTP_EV_CONNECTIND:
        kDebug() << "Incoming connection" << endl;
        break;
    case OBEXFTP_EV_DISCONNECTIND:
        kDebug() << "Disconnecting" << endl;
        break;

    case OBEXFTP_EV_INFO:
        kDebug() << "Got info " << msg << ": " << endl; // 64 bit problems ?
        break;

    case OBEXFTP_EV_BODY:
        //if (c == 'l' || c == 'X' || c == 'P') {
            if (msg == NULL)
                kDebug() << "No body." << endl;
            else if (len == 0)
                kDebug() << "Empty body." << endl;
            else
                kDebug() << msg << endl;
                write(STDOUT_FILENO, msg, len);
                ;
        //}  <- if
        break;

    case OBEXFTP_EV_PROGRESS:
        kDebug() <<  "Progress: " << i++ << "; " << progress[i % strlen(progress) ] << endl;
        kDebug() << "Current buffer size: " << OBEXWrapper::instance()->bufferSize() << endl;
        break;
    }
}

/*

extern "C" { QValueList<obex_fileInfo> kioobex_fileList; }

extern "C"
        void onFile( obex_fileInfo file)
{
    kioobex_fileList+=(file);
}
*/

bool OBEXWrapper::connectClient()
{
    kDebug() << " OBEXWrapper::connectClient(): checking client;" << endl;
    if (client)
    {
        kDebug() << "Already connected\n";
        return true;
    }
    kDebug() << "Opening client, transport: " << i_transport << endl ;
        /* Open */
    client = obexftp_open (i_transport, NULL, info_cb, NULL);
    if(!client) {
        kDebug() << "Error opening obexftp-client" << endl;
        return false;
    }
    if (!i_use_conn) {
        client->quirks &= ~OBEXFTP_CONN_HEADER;
    }
    if (!i_use_path) {
        client->quirks &= ~OBEXFTP_SPLIT_SETPATH;
    }
    for (int retry = 0; retry < 3; retry++) {
        kDebug() << "Connecting: " << c_device << endl;
        /* Connect*/
        if (obexftp_connect_uuid (client, c_device, i_channel , UUID_FBS, sizeof(UUID_FBS)) >= 0) {
            kDebug() << "Phone connected in obex mode" << endl;
            return true;
        }
        sleep (2);
        kDebug() << "Still trying to connect ( try " << retry << " )" << endl;
        perror("Connection error: ");
    } // <- for

    obexftp_close(client);
    client = 0L;
    return false;
}

void OBEXWrapper::disconnectClient()
{
    kDebug() << "OBEXWrapper::disconnectClient()" << endl;

    if (client != NULL) {
        /* Disconnect */
        (void) obexftp_disconnect (client);
        /* Close */
        obexftp_close (client);
        client = NULL;
    }

    kDebug() << "**************** Client disconnected" << endl;
}

bool OBEXWrapper::fetchFileList(const QString &path) 
{
    kDebug() << "OBEXWrapper::fetchFileList()" << endl;

    files.clear();
    n_files = 0;

    Q3ValueList<stat_entry_t>::Iterator it = files.begin();

    if( connectClient() ) {
        /* List folder */
        //stat_entry_t *ent;
        void *dir = obexftp_opendir(client, path.latin1() );
        while ( true ) {
            
            stat_entry_t* ent;
            if ( (ent = obexftp_readdir(dir)) == NULL ) break;
            files.append(*ent);
            n_files++;
        }
        obexftp_closedir(dir);

        kDebug() << "OBEXWrapper::fetchFileList() number of files:  " << n_files  << endl;
        
        return true;
    }
    return false;
}

/**
* @brief Get a file, the file is stored in uint8_t* client->buf_data, the size of file is stored in uint32_t client->buf_size
* @param path the path to the file.
* @return 1 if successful, 0 otherwise.
*/
int OBEXWrapper::getFile(const QString &path)
{
    kDebug() << "OBEXWrapper::getFile(" << path << ",...)\n";
    

    if( ! connectClient() ) return 0;

    if( obexftp_get(client, NULL, path.latin1() ) ) return 0;

    return 1;

}

int OBEXWrapper::putFile( const QString &path, char* buffer, int size)
{
    kDebug() << "OBEXWrapper::putFile(" << path << ",.......,......)\n";
    if( ! connectClient() ) return -1;
    kDebug() << "Writing " << path << " to the client, size: " << size << " bytes.\n";

    int ret = obexftp_put_data( client, buffer, size, path.latin1() );

    return ret;
}



bool OBEXWrapper::deleteFile( const QString &path )
{
    kDebug() << "OBEXWrapper::deleteFile(" << path << ")\n";
    if( ! connectClient() ) return false;

    bool ret = obexftp_del( client, path.latin1() );

    return ret;
}

bool OBEXWrapper::mkDir( const QString &path )
{
    kDebug() << "OBEXWrapper::makeDir(" << path << ")\n";
    if( ! connectClient() ) return false;

    bool ret = obexftp_mkpath( client, path.latin1() );

    return ret;
}

