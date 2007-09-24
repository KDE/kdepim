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

#ifndef KMOBILETOOLS_IFACESJOBPROVIDER_H
#define KMOBILETOOLS_IFACESJOBPROVIDER_H

#include <libkmobiletools/kmobiletools_export.h>
#include <libkmobiletools/jobxp.h>

namespace KMobileTools {

namespace Ifaces {

/**
 * This interface is intended to be used by engines to allow
 * the gui to control the engine's jobs and display their status
 *
 * @author Matthias Lechner <matthias@lmme.de>
 */
class KMOBILETOOLS_EXPORT JobProvider{
public:
    virtual ~JobProvider();

protected:
// Q_SIGNALS:
    virtual void jobCreated( KMobileTools::JobXP* job ) = 0;
};

}

}

Q_DECLARE_INTERFACE(KMobileTools::Ifaces::JobProvider, "org.kde.KMobileTools.Ifaces.JobProvider/0.1")

#endif
