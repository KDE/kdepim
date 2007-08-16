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

#ifndef KMOBILETOOLSIFACESGUISERVICE_H
#define KMOBILETOOLSIFACESGUISERVICE_H

#include <libkmobiletools/kmobiletools_export.h>

#include <QtCore/QObject>
#include <QtGui/QWidget>
#include <KDE/KIcon>

namespace KMobileTools {

namespace Ifaces {
/**
 * This interface defines a KMobileTools GUI service
 *
 * @author Matthias Lechner <matthias@lmme.de>
 */
class KMOBILETOOLS_EXPORT GuiService {
public:
    /**
     * Returns the service's icon
     *
     * @return the service icon
     */
    virtual KIcon icon() const = 0;

    /**
     * Returns the widget associated with the service
     *
     * @return the associated widget
     */
    virtual QWidget* widget() const = 0;

    virtual ~GuiService();

};

}

}

Q_DECLARE_INTERFACE(KMobileTools::Ifaces::GuiService, "org.kde.KMobileTools.Ifaces.GuiService/0.1")

#endif
