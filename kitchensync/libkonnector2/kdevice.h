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
#include <qvaluelist.h>

/**
 *  Device stores informations about any konnector
 *
 */
namespace KSync {

class Device
{
  public:
    /**
     * Convenience typedef
     */
    typedef QValueList<Device> ValueList;

    /**
     *  C'TOR
     */
    Device();

    /**
     * C'tor
     * @param ident The identity of the Konnector/Device
     * @param group The Group/Category of the Konnector/Device
     * @param vendor The Vendor of the konnector/Device
     * @param library The libray where the Konnector/Device is in
     */
    Device( const QString &name, const QString &group,
            const QString &vendor, const QString &library,
            const QString &ident);
    Device( const Device & );
    ~Device();

    bool operator==(const Device &);
    /**
     * @return returns the translated name of the Device
     */
    QString name() const;

    /**
     * @return returns the group/category of the Device
     */
    QString group() const;

    /**
     * @return returns the vendor of the Device
     */
    QString vendor() const;

    /**
     * @return returns the library of the Device
     */
    // untranslated id
    QString identify()const;
    QString library() const;
    Device &operator=(const Device & );

  private:
    friend class Konnector;

    class DevicePrivate;
    DevicePrivate *d;
};

}

#endif
