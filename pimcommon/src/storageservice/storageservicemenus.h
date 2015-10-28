/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#ifndef STORAGESERVICEMENUS_H
#define STORAGESERVICEMENUS_H

#include <QObject>
#include "pimcommon_export.h"
class KActionMenu;
namespace PimCommon
{
class StorageServiceMenusPrivate;
class PIMCOMMON_EXPORT StorageServiceMenus : public QObject
{
    Q_OBJECT
public:
    explicit StorageServiceMenus(QObject *parent = Q_NULLPTR);
    ~StorageServiceMenus();

    KActionMenu *shareLinkServices(QWidget *parent) const;
    KActionMenu *downloadServices(QWidget *parent) const;
    KActionMenu *uploadServices(QWidget *parent) const;

private:
    StorageServiceMenusPrivate *const d;
};
}
#endif // STORAGESERVICEMENUS_H
