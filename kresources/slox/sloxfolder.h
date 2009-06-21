/*
    Copyright (c) 2005 by Volker Krause <vkrause@kde.org>
    Copyright (c) 2005 by Florian Schröder <florian@deltatauchi.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef SLOXFOLDER_H
#define SLOXFOLDER_H

#include <QString>
#include "slox_export.h"

class K3ListViewItem;

enum FolderType {
  Unbound,
  Calendar,
  Tasks,
  Contacts
};

class KSLOX_EXPORT SloxFolder
{
  public:
    SloxFolder( const QString &id, const QString &parentId, const QString &type, const QString &name, bool def = false );

    QString id() const { return mId; }
    QString parentId() const { return mParentId; }
    FolderType type() const { return mType; }
    QString name() const;
    bool isDefault() const { return mDefault; }

    K3ListViewItem *item;

  private:
    QString mId, mParentId;
    FolderType mType;
    QString mName;
    bool mDefault;
};

#endif
