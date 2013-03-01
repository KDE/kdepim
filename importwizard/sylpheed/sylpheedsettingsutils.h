/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>
  
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

#ifndef SYLPHEEDSETTINGSUTILS_H
#define SYLPHEEDSETTINGSUTILS_H

#include <QString>
#include <KConfigGroup>

namespace SylpheedSettingsUtils {
bool readConfig( const QString& key, const KConfigGroup& accountConfig, int& value, bool remove_underscore );
bool readConfig( const QString& key, const KConfigGroup& accountConfig, QString& value, bool remove_underscore );
}

#endif // SYLPHEEDSETTINGSUTILS_H
