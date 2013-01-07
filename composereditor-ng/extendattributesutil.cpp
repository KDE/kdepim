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
    QStringList align;
    align<<QLatin1String("top");
    align<<QLatin1String("bottom");
    align<<QLatin1String("middle");
    align<<QLatin1String("left");
    align<<QLatin1String("right");
    map.insert(QLatin1String("align"),align);
    map.insert(QLatin1String("alt"),QStringList());
    map.insert(QLatin1String("border"),QStringList());
    map.insert(QLatin1String("height"),QStringList());
    map.insert(QLatin1String("hspace"),QStringList());
    map.insert(QLatin1String("ismap"),QStringList());
    map.insert(QLatin1String("longdesc"),QStringList());
    map.insert(QLatin1String("src"),QStringList());
    map.insert(QLatin1String("usemap"),QStringList());
    map.insert(QLatin1String("vspace"),QStringList());
    map.insert(QLatin1String("width"),QStringList());
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
    map.insert(QLatin1String("charset"),QStringList());
    map.insert(QLatin1String("href"),QStringList());
    map.insert(QLatin1String("hreflang"),QStringList());
    map.insert(QLatin1String("media"),QStringList());
    QStringList rel;
    rel<<QLatin1String("alternate");
    rel<<QLatin1String("archives");
    rel<<QLatin1String("author");
    rel<<QLatin1String("bookmark");
    rel<<QLatin1String("external");
    rel<<QLatin1String("first");
    rel<<QLatin1String("help");
    rel<<QLatin1String("icon");
    rel<<QLatin1String("last");
    rel<<QLatin1String("license");
    rel<<QLatin1String("next");
    rel<<QLatin1String("nofollow");
    rel<<QLatin1String("noreferrer");
    rel<<QLatin1String("pingback");
    rel<<QLatin1String("prefetch");
    rel<<QLatin1String("prev");
    rel<<QLatin1String("search");
    rel<<QLatin1String("sidebar");
    rel<<QLatin1String("stylesheet");
    rel<<QLatin1String("tag");
    rel<<QLatin1String("up");
    map.insert(QLatin1String("rel"),rel);
    map.insert(QLatin1String("rev"),QStringList());
    QStringList target;
    target<<QLatin1String("_blank");
    target<<QLatin1String("_self");
    target<<QLatin1String("_top");
    target<<QLatin1String("_parent");
    map.insert(QLatin1String("target"),target);
    map.insert(QLatin1String("type"),QStringList());

    //TODO add global attribute.
    return map;
}

}
