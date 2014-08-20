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

namespace PimCommon {
class PIMCOMMON_EXPORT Kdelibs4ConfigMigrater
{
public:
    Kdelibs4ConfigMigrater(const QString &appName);
    ~Kdelibs4ConfigMigrater();

    virtual bool migrate();

    void setConfigFiles(const QStringList &configFileNameList);
    void setUiFiles(const QStringList &uiFileNameList);

private:
    class Private;
    friend class Private;
    Private *const d;
};
}

#endif // KDELIBS4CONFIGMIGRATER_H
