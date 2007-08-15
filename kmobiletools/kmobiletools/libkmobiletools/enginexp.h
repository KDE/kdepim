/***************************************************************************
   Copyright (C) 2007 by Matthias Lechner <matthias@lmme.de>

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

#ifndef KMOBILETOOLSENGINEXP_H
#define KMOBILETOOLSENGINEXP_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtGui/QIcon>

#include <libkmobiletools/kmobiletools_export.h>

namespace KMobileTools {

/**
    This is KMobileTools' engine object. You can use it to create a
    backend for KMobileTools. To do so, create a class that derives
    from this object and derive from one or many interfaces from
    KMobileTools::Ifaces.

    @TODO change the object name back to Engine as soon as the transition
    to this object is made ;-) Sorry for the name, it was the first idea.

    @author Matthias Lechner <matthias@lmme.de>
*/
class KMOBILETOOLS_EXPORT EngineXP : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructs a new Engine object with the given @p parent
     *
     * @param p the object's parent
     */
    EngineXP( QObject *parent );

    /**
     * Destructs the Engine object
     */
    virtual ~EngineXP();

    /**
     * Initializes the engine and the communication with the
     * device with given @p deviceName
     *
     * @param deviceName the name of the device to initialize
     */
    virtual void initialize( const QString& deviceName ) = 0;

    /**
     * Checks if the engine implements a given interface
     *
     * @param interfaceName the plain name of the interface, e.g. "Status"
     *
     * @return true, if the engine implements the given interface
     */
    bool implements( const QString& interfaceName );

Q_SIGNALS:
    /**
     * This signal is emitted when the engine was initialized.
     *
     * @param successful true if the engine was successfully initialized and
     *                   the provided device was found
     */
    void initialized( bool successful );
};

}

#endif
