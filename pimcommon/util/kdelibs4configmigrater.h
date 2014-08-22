/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef KDELIBS4CONFIGMIGRATER_H
#define KDELIBS4CONFIGMIGRATER_H

#include "pimcommon_export.h"
#include <QStringList>

namespace PimCommon
{
/**
 * \file kdelibs4configmigrater.h
 */

/**
  * Kdelibs4ConfigMigrater migrates specific config file and ui file
  * from KDE SC 4.0 to new QStandardPath.
  *
  * @short Class for migration of config files and ui file from KDE SC4
  * @since 5.2
  */

class PIMCOMMON_EXPORT Kdelibs4ConfigMigrater
{
public:
    /**
     * Constructs a Kdelibs4ConfigMigrater
     *
     * @param appName The application name of KDE SC 4.0
     */
    Kdelibs4ConfigMigrater(const QString &appName);

    /**
     * Destructor
     */
    ~Kdelibs4ConfigMigrater();

    /**
     * Return true if migrate was done. If we found kdehome directory
     */
    bool migrate();

    /**
     * Set list of config files we need to migrate for application
     * @param configFileNameList list of config file
     */
    void setConfigFiles(const QStringList &configFileNameList);

    /**
     * Set list of ui files to migrate
     * @param uiFileNameList list of ui file.
     */
    void setUiFiles(const QStringList &uiFileNameList);

private:
    class Private;
    friend class Private;
    Private *const d;
};
}

#endif // KDELIBS4CONFIGMIGRATER_H
