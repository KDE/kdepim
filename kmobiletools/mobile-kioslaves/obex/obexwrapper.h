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

#ifndef OBEXWRAPPER_H
#define OBEXWRAPPER_H

#include <qobject.h>
#include <qstringlist.h>
#include <q3memarray.h>
//Added by qt3to4:
#include <Q3ValueList>

#include <obexftp/obexftp.h>
#include <obexftp/client.h>
#include <obexftp/uuid.h>

/**
@author Marcin Przylucki
*/
class OBEXWrapper : public QObject
{
Q_OBJECT
public:
    explicit OBEXWrapper( QObject *parent=0, const char *name=NULL);
    void setupParameters(int transport = OBEX_TRANS_IRDA,
                int channel = -1,
                const QString &device = NULL,
                const uint8_t *use_uuid = UUID_FBS,
                int use_uuid_len = sizeof(UUID_FBS),
                int use_conn = 1,
                int use_path = 1);

//	OBEXWrapper(QObject *parent = 0, const char *name = 0); // krazy:exclude=explicit
    ~OBEXWrapper();

    bool connected() { return ( client != NULL ); }
    Q3ValueList<stat_entry_t> getFiles() { return files; }
    obexftp_client_t* getClient() { return client;}
    uint bufferSize() { return (client->buf_size * sizeof(uint8_t) ) / sizeof(char) ; }
    static OBEXWrapper *instance() { return p_instance; }

private:
    Q3ValueList<stat_entry_t> files;
    obexftp_client_t *client;
    int n_files;
    int i_transport;
    int i_channel;
    int i_use_uuid_len;
    int i_use_conn;
    int i_use_path;
    QString c_device;
    const uint8_t *c_use_uuid;
    static OBEXWrapper *p_instance;
public slots:
    bool connectClient();
    void disconnectClient();

//    int countFiles() { return n_files(); }
    bool fetchFileList(const QString &path);
    int getFile( const QString &path);
    int putFile( const QString &path, char *buffer, int size);
    bool deleteFile( const QString &path );
    bool mkDir( const QString &path );
};

#endif
