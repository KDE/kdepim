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

#ifndef LIBKMOBILE_SKELETON_H
#define LIBKMOBILE_SKELETON_H

#include <kmobiledevice.h>

class KMobileSkeleton : public KMobileDevice
{
Q_OBJECT
public:
    KMobileSkeleton( QObject *obj=0, const char *name=0, const QStringList &args=QStringList() );
    ~KMobileSkeleton();

    // createObject needs to be reimplemented by every KMobileDevice driver
    QObject *createObject( QObject *parent=0, const char *name=0,
	const char *classname="QObject", const QStringList &args=QStringList() );

    // connect, disconnect and current status
    bool connectDevice(QWidget *parent);
    bool disconnectDevice(QWidget *parent);

    // returns true, if this device is read-only (default: false)
    bool isReadOnly() const;

    // you may provide your own icon() implementation to display
    // an appropriate Pixmap (e.g. a Palm Pilot or a Zaurus image).
    QString iconFileName() const;

    QString deviceUniqueID();

    /*
     * Addressbook / Phonebook support
     */
    int numAddresses();
    int readAddress( int index, KABC::Addressee &adr );
    int storeAddress( int index, const KABC::Addressee &adr, bool append = false );

    /*
     * Notes support
     */
    int numNotes();
    int readNote( int index, QString &note );

signals:
    void connectionChanged( bool connected );
    void message( int msgLevel, const QString &msg );
};

#endif
