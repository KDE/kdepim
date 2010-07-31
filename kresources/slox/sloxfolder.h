/*
    Copyright (c) 2005 by Volker Krause <volker.krause@rwth-aachen.de>
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

#include <tqstring.h>
#include <kdepimmacros.h>

class KListViewItem;

enum FolderType {
  Unbound,
  Calendar,
  Tasks,
  Contacts
};

class KDE_EXPORT SloxFolder
{
  public:
    SloxFolder( const TQString &id, const TQString &parentId, const TQString &type, const TQString &name, bool def = false );

    TQString id() const { return mId; }
    TQString parentId() const { return mParentId; }
    FolderType type() const { return mType; }
    TQString name() const;
    bool isDefault() const { return mDefault; }

    KListViewItem *item;

  private:
    TQString mId, mParentId;
    FolderType mType;
    TQString mName;
    bool mDefault;
};

#endif
