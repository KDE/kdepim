/*
    This file is part of libkmobile.
    Copyright (c) 2003 - 2003 Helge Deller <deller@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef LIBKMOBILE_GNOKII_H
#define LIBKMOBILE_GNOKII_H

#include <kmobiledevice.h>
#include <kabc/addressee.h>

class KMobileGnokii : public KMobileDevice
{
Q_OBJECT
public:
    KMobileGnokii( QObject *obj=0, const char *name=0, const QStringList &args=QStringList() );
    ~KMobileGnokii();

    // createObject needs to be reimplemented by every KMobileDevice driver
    QObject *createObject( QObject *parent=0, const char *name=0, 
	const char *classname="QObject", const QStringList &args=QStringList() );

    // connect, disconnect and current status
    bool connectDevice(QWidget *parent);
    bool disconnectDevice(QWidget *parent);

    // provide a device-specific configure dialog
    bool configDialog(QWidget *parent);

    // filename and path to gnokii-icon
    QString iconFileName() const;

    QString deviceUniqueID();

    /*
     * Addressbook / Phonebook support
     */
    int numAddresses();
    int readAddress( int index, KABC::Addressee &adr );
    int storeAddress( int index, const KABC::Addressee &adr, bool append = false );

    /*
     * Calendar support
     */
    int numCalendarEntries();
    int readCalendarEntry( int index, KCal::Event &entry );
    int storeCalendarEntry( int index, const KCal::Event &entry );

    /*
     * Notes support
     */
    int numNotes();
    int readNote( int index, QString &note );

signals:
    void connectionChanged( bool connected );
    void message( int msgLevel, const QString &msg );

protected:
    bool setGnokiiStateMachine();
    bool saveConfig( KConfig &conf, QString group );
    bool loadConfig( KConfig &conf, QString group );
    bool saveGnokiiConfiguration();
    bool loadGnokiiConfiguration();
    bool saveDeviceConfiguration();
    bool loadDeviceConfiguration();

private:
    QString m_modelnr, m_connection, m_port, m_baud;

    int m_numAddresses;
    KABC::Addressee::List m_addrList;
};

#endif
