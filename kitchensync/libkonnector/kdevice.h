/* This file is part of the KDE libraries
    Copyright (C) 2002 Holger Freyther <freyher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/
#ifndef kdevice_h
#define kdevice_h

#include <qstring.h>

/**
 *  KDevice stores informations about any konnector
 *
 */

class KDevice {
public:
   /**
    *  C'TOR
    */
    KDevice();
    /**
     * C'tor
     * @param ident The identity of the Konnector/Device
     * @param group The Group/Category of the Konnector/Device
     * @param vendor The Vendor of the konnector/Device
     * @param library The libray where the Konnector/Device is in
     */
    KDevice(const QString &ident, const QString &group,
            const QString &vendor, const QString &library,
            const QString &id);
    KDevice( const KDevice & );
    ~KDevice();
    /**
     * @return returns the identity of the KDevice
     */
    QString identify() const;
    /**
     * @return returns the group/category of the KDevice
     */
    QString group() const;
    /**
     * @return returns the vendor of the KDevice
     */
    QString vendor() const;
    /**
     * @return returns the library of the KDevice
     */
    // untranslated id
    QString id()const;
    QString library() const;
    KDevice &operator=(const KDevice & );
private:
    friend bool operator==(const KDevice &, const KDevice );
    friend class Konnector;
    class KDevicePrivate;
    KDevicePrivate *d;
};

#endif
