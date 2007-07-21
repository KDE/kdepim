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

#ifndef KMOBILETOOLSIFACESDIALACTION_H
#define KMOBILETOOLSIFACESDIALACTION_H

#include <QtCore/QObject>

#include <libkmobiletools/kmobiletools_export.h>


namespace KMobileTools {

namespace Ifaces {

/**
    This interface provides methods to trigger dialing on the phone

    @author Matthias Lechner <matthias@lmme.de>
*/
class KMOBILETOOLS_EXPORT DialAction {
public:
    /**
     * Triggers dialing of the given @p number
     *
     * @param number the number to dial
     */
    virtual void dial( const QString& number ) = 0;

    /**
     * Triggers hanging up of the current phone call
     */
    virtual void hangup() = 0;

    virtual ~DialAction();
};

}
}

Q_DECLARE_INTERFACE(KMobileTools::Ifaces::DialAction, "org.kde.KMobileTools.Ifaces.DialAction/0.1")


#endif
