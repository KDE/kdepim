//Added by qt3to4:
//#include <QPixmap>
/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

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

#ifndef DEVICESCONFIG_H
#define DEVICESCONFIG_H

// mainly derived from kdemultimedia/kaudiocreator

#include <libkmobiletools/kmobiletools_export.h>
#include <libkmobiletools/kmobiletools_devices.h>

#define KMT_FILESYSTEM_NONE 0
#define KMT_FILESYSTEM_Pk2 1
#define DEVCFG(x) ( KMobileTools::DevicesConfig::prefs(x) )
#ifndef DEFAULT_VERBOSE
#ifndef NO_DEBUG
#define DEFAULT_VERBOSE true
#else
#define DEFAULT_VERBOSE false
#endif
#endif

#include <kicontheme.h>

class DevicesConfigPrivate;

namespace KMobileTools {
class KMOBILETOOLS_EXPORT DevicesConfig : public DevicesConfigBase
{
    public:
    DevicesConfig(const QString &);
    ~DevicesConfig();
    static KMobileTools::DevicesConfig *prefs(const QString &groupName);
    static bool hasPrefs(const QString &groupName);
    static void deletePrefs(const QString &groupName);
    static const QString deviceGroup( const QString &devicename);
    static const QString firstFreeGroup();
    static const QString engineTypeName(const QString &libName);
    static const QPixmap deviceTypeIcon(const QString &groupName, K3Icon::Group group, int size=0);
    static const QString deviceTypeIconPath(const QString &groupName, int groupOrSize);
    private:
        DevicesConfigPrivate *const d;
};

}
#endif
