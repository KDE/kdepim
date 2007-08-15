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

#ifndef KMOBILETOOLS_IFACESACTIONPROVIDER_H
#define KMOBILETOOLS_IFACESACTIONPROVIDER_H

#include "kmobiletools_export.h"

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtGui/QAction>

namespace KMobileTools {

namespace Ifaces {

/**
 * This interface provides an action collection which can be used by a service
 * to provide custom actions for the toolbar
 *
 * @author Matthias Lechner <matthias@lmme.de>
 */
class KMOBILETOOLS_EXPORT ActionProvider {
public:
    /**
     * Returns a list of actions to add to the main part
     *
     * @return a list of actions
     */
    virtual QList<QAction*> actionList() const = 0;

    virtual ~ActionProvider();

};

}

}

Q_DECLARE_INTERFACE(KMobileTools::Ifaces::ActionProvider, "org.kde.KMobileTools.Ifaces.ActionProvider/0.1")

#endif
