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


#ifndef SYSTEMIMPL_H
#define SYSTEMIMPL_H

#include <kio/global.h>
#include <kio/job.h>
#include <kurl.h>

#include <qobject.h>
#include <qstring.h>
//Added by qt3to4:
#include <Q3ValueList>

#include <obexftp/client.h>


class OBEXWrapper;
class OBEXImpl : public QObject
{
    Q_OBJECT
public:
    OBEXImpl();
    enum transport { null=0,
        Bluetooth = OBEX_TRANS_BLUETOOTH,
        IrDA=OBEX_TRANS_IRDA,
        Inet=OBEX_TRANS_INET,
        Custom=OBEX_TRANS_CUSTOM,
        FD=OBEX_TRANS_FD,
        USB=OBEX_TRANS_USB };

    void createTopLevelEntry(KIO::UDSEntry& entry) const;
    bool listDirectory(const KUrl &url, Q3ValueList<KIO::UDSEntry> &list);
    bool statEntry(const KUrl &url, KIO::UDSEntry &entry);

    OBEXWrapper* wrapper() { return obexWrapper; }
    int lastErrorCode() const { return m_lastErrorCode; }
    QString lastErrorMessage() const { return m_lastErrorMessage; }

public slots:
    void fetchFilesList( const QString &path );
    int getFile(const KUrl &url);
    void setHostConfig(const QString &device, int port, transport i_transport );

private slots:
    void slotEntries(KIO::Job *job, const KIO::UDSEntryList &list);
    void slotResult(KIO::Job *job);

private:
    void createEntry(KIO::UDSEntry& entry, const KUrl &url, stat_entry_t &file);

    bool m_lastListingEmpty;

    /// Last error code stored in class to simplify API.
    /// Note that this means almost no method can be const.
    int m_lastErrorCode;
    QString m_lastErrorMessage;
    OBEXWrapper *obexWrapper;
};

#endif
