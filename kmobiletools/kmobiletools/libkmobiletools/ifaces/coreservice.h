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

#ifndef KMOBILETOOLSIFACESSERVICE_H
#define KMOBILETOOLSIFACESSERVICE_H

#include "kmobiletools_export.h"

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtGui/QWidget>
#include <KIcon>

namespace KMobileTools {

namespace Ifaces {
/**
 * This interface defines a KMobileTools core service
 *
 * @author Matthias Lechner <matthias@lmme.de>
 */
class KMOBILETOOLS_EXPORT CoreService {
public:
    /**
     * Returns the service's name
     *
     * @return the service name
     */
    virtual QString name() const = 0;

    /**
     * Returns a list of interfaces that an engine must have implemented
     * to use this service
     *
     * @return a list of required interfaces
     */
    virtual QStringList requires() const = 0;

    virtual ~CoreService();

};

}

}

Q_DECLARE_INTERFACE(KMobileTools::Ifaces::CoreService, "org.kde.KMobileTools.Ifaces.CoreService/0.1")

#endif
