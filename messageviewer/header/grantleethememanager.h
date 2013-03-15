/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#ifndef GRANTLEETHEMEMANAGER_H
#define GRANTLEETHEMEMANAGER_H

#include "header/grantleetheme.h"

#include <QObject>
#include <QMap>


namespace MessageViewer {
class GrantleeThemeManager : public QObject
{
    Q_OBJECT
public:
    explicit GrantleeThemeManager(const QString &path, QObject *parent = 0);
    ~GrantleeThemeManager();

    QMap<QString, GrantleeTheme> themes() const;
private:
    Q_PRIVATE_SLOT( d, void directoryChanged() )
    class Private;
    Private *const d;
};
}
#endif // GRANTLEETHEMEMANAGER_H
