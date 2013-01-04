/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/
#include "extendattributesutil_p.h"

namespace ComposerEditorNG
{
QMap<QString, QStringList> ExtendAttributesUtil::attributesMap(ExtendAttributesDialog::ExtendType type)
{
    switch(type) {
    case ExtendAttributesDialog::Image:
        return attributesMapImage();
    case ExtendAttributesDialog::Table:
        return attributesMapTable();
    case ExtendAttributesDialog::Cell:
        return attributesMapCell();
    case ExtendAttributesDialog::Link:
        return attributesMapLink();
    }
    return QMap<QString, QStringList>();
}

QMap<QString, QStringList> ExtendAttributesUtil::attributesMapImage()
{
    QMap<QString, QStringList> map;
    return map;
}

QMap<QString, QStringList> ExtendAttributesUtil::attributesMapTable()
{
    QMap<QString, QStringList> map;
    return map;
}

QMap<QString, QStringList> ExtendAttributesUtil::attributesMapCell()
{
    QMap<QString, QStringList> map;
    return map;
}

QMap<QString, QStringList> ExtendAttributesUtil::attributesMapLink()
{
    QMap<QString, QStringList> map;
    return map;
}

}
