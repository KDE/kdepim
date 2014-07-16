/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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
#include "extendattributesutils_p.h"

namespace ComposerEditorNG
{

QMap<QString, QStringList> ExtendAttributesUtils::listGlobalAttribute()
{
    QMap<QString, QStringList> globalAttr;
    globalAttr.insert(QLatin1String("id"),QStringList());
    globalAttr.insert(QLatin1String("class"),QStringList());
    globalAttr.insert(QLatin1String("title"),QStringList());
    globalAttr.insert(QLatin1String("lang"),QStringList());
    QStringList dir;
    dir<<QLatin1String("ltr");
    dir<<QLatin1String("rtl");
    dir<<QLatin1String("auto");
    globalAttr.insert(QLatin1String("dir"),dir);

    return globalAttr;
}

QMap<QString, QStringList> ExtendAttributesUtils::globalAttribute()
{
    QMap<QString, QStringList> globalAttr;
    globalAttr.insert(QLatin1String("accesskey"),QStringList());
    globalAttr.insert(QLatin1String("class"),QStringList());
    QStringList dir;
    dir<<QLatin1String("ltr");
    dir<<QLatin1String("rtl");
    dir<<QLatin1String("auto");
    globalAttr.insert(QLatin1String("dir"),dir);
    globalAttr.insert(QLatin1String("id"),QStringList());
    globalAttr.insert(QLatin1String("lang"),QStringList());
    globalAttr.insert(QLatin1String("style"),QStringList());
    globalAttr.insert(QLatin1String("tabindex"),QStringList());
    globalAttr.insert(QLatin1String("title"),QStringList());
    return globalAttr;
}

QMap<QString, QStringList> ExtendAttributesUtils::attributesMap(ExtendAttributesDialog::ExtendType type)
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
    case ExtendAttributesDialog::Body:
        return attributesMapBody();
    case ExtendAttributesDialog::ListUL:
        return attributesMapListUL();
    case ExtendAttributesDialog::ListOL:
        return attributesMapListOL();
    case ExtendAttributesDialog::ListDL:
        return attributesMapListDL();
    default:
        break;
    }
    return QMap<QString, QStringList>();
}

QMap<QString, QStringList> ExtendAttributesUtils::attributesMapImage()
{
    QMap<QString, QStringList> map;
    map = globalAttribute();
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

QMap<QString, QStringList> ExtendAttributesUtils::attributesMapTable()
{
    QMap<QString, QStringList> map;
    map = globalAttribute();
    QStringList align;
    align<<QLatin1String("left");
    align<<QLatin1String("center");
    align<<QLatin1String("right");
    map.insert(QLatin1String("align"),align);
    map.insert(QLatin1String("bgcolor"),QStringList());
    map.insert(QLatin1String("border"),QStringList());
    map.insert(QLatin1String("cellpadding"),QStringList());
    map.insert(QLatin1String("cellspacing"),QStringList());
    QStringList frame;
    frame<<QLatin1String("void");
    frame<<QLatin1String("above");
    frame<<QLatin1String("below");
    frame<<QLatin1String("hsides");
    frame<<QLatin1String("lhs");
    frame<<QLatin1String("rhs");
    frame<<QLatin1String("vsides");
    frame<<QLatin1String("box");
    frame<<QLatin1String("border");
    map.insert(QLatin1String("frame"),frame);
    QStringList rules;
    rules<<QLatin1String("none");
    rules<<QLatin1String("groups");
    rules<<QLatin1String("rows");
    rules<<QLatin1String("cols");
    rules<<QLatin1String("all");
    map.insert(QLatin1String("rules"),rules);
    map.insert(QLatin1String("summary"),QStringList());
    map.insert(QLatin1String("width"),QStringList());
    return map;
}

QMap<QString, QStringList> ExtendAttributesUtils::attributesMapCell()
{
    QMap<QString, QStringList> map;
    map = globalAttribute();
    map.insert(QLatin1String("abbr"),QStringList());
    QStringList align;
    align<<QLatin1String("top");
    align<<QLatin1String("bottom");
    align<<QLatin1String("middle");
    align<<QLatin1String("left");
    align<<QLatin1String("right");
    align<<QLatin1String("char");
    map.insert(QLatin1String("align"),align);
    map.insert(QLatin1String("axis"),QStringList());
    map.insert(QLatin1String("bgcolor"),QStringList());
    map.insert(QLatin1String("char"),QStringList());
    map.insert(QLatin1String("charoff"),QStringList());
    map.insert(QLatin1String("colspan"),QStringList());
    map.insert(QLatin1String("headers"),QStringList());
    map.insert(QLatin1String("height"),QStringList());
    map.insert(QLatin1String("nowrap"),QStringList());
    map.insert(QLatin1String("rowspan"),QStringList());
    QStringList scope;
    scope<<QLatin1String("col");
    scope<<QLatin1String("colgroup");
    scope<<QLatin1String("row");
    scope<<QLatin1String("rowgroup");
    map.insert(QLatin1String("scope"),scope);
    QStringList valign;
    valign<<QLatin1String("top");
    valign<<QLatin1String("middle");
    valign<<QLatin1String("bottom");
    valign<<QLatin1String("baseline");
    map.insert(QLatin1String("valign"),valign);
    map.insert(QLatin1String("width"),QStringList());
    return map;
}

QMap<QString, QStringList> ExtendAttributesUtils::attributesMapLink()
{
    QMap<QString, QStringList> map;
    map = globalAttribute();
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
    return map;
}

QMap<QString, QStringList> ExtendAttributesUtils::attributesMapListUL()
{
    QMap<QString, QStringList> map;
    map = listGlobalAttribute();
    QStringList rel;
    rel<<QLatin1String("compact");
    map.insert(QLatin1String("compact"),rel);
    QStringList type;
    type<<QLatin1String("disc");
    type<<QLatin1String("square");
    type<<QLatin1String("circle");
    map.insert(QLatin1String("type"),type);
    return map;
}

QMap<QString, QStringList> ExtendAttributesUtils::attributesMapListOL()
{
    QMap<QString, QStringList> map;
    map = listGlobalAttribute();
    QStringList rel;
    rel<<QLatin1String("compact");
    map.insert(QLatin1String("compact"),rel);
    map.insert(QLatin1String("start"),QStringList());
    QStringList type;
    type<<QLatin1String("1");
    type<<QLatin1String("A");
    type<<QLatin1String("a");
    type<<QLatin1String("I");
    type<<QLatin1String("i");
    map.insert(QLatin1String("type"),type);

    return map;
}

QMap<QString, QStringList> ExtendAttributesUtils::attributesMapListDL()
{
    QMap<QString, QStringList> map;
    map = listGlobalAttribute();
    return map;
}



QMap<QString, QStringList> ExtendAttributesUtils::attributesMapBody()
{
    QMap<QString, QStringList> map;
    map = globalAttribute();
    map.insert(QLatin1String("alink"),QStringList());
    map.insert(QLatin1String("background"),QStringList());
    map.insert(QLatin1String("bgcolor"),QStringList());
    map.insert(QLatin1String("link"),QStringList());
    map.insert(QLatin1String("text"),QStringList());
    map.insert(QLatin1String("vlink"),QStringList());
    return map;
}

QMap<QString, QStringList> ExtendAttributesUtils::attributesJavascriptWindowAndBase()
{
    QMap<QString, QStringList> map;
    map = attributesJavascript();
    //WindowEvent
    map.insert(QLatin1String("onload"),QStringList());
    map.insert(QLatin1String("onunload"),QStringList());
    return map;
}

QMap<QString, QStringList> ExtendAttributesUtils::attributesJavascript()
{
    QMap<QString, QStringList> map;
    //Form event
    map.insert(QLatin1String("onblur"),QStringList());
    map.insert(QLatin1String("onchange"),QStringList());
    map.insert(QLatin1String("onfocus"),QStringList());
    map.insert(QLatin1String("onreset"),QStringList());
    map.insert(QLatin1String("onselect"),QStringList());
    map.insert(QLatin1String("onsubmit"),QStringList());
    //Keyboard event.
    map.insert(QLatin1String("onkeydown"),QStringList());
    map.insert(QLatin1String("onkeypress"),QStringList());
    map.insert(QLatin1String("onkeyup"),QStringList());

    //Mouse event.
    map.insert(QLatin1String("onclick"),QStringList());
    map.insert(QLatin1String("ondblclick"),QStringList());
    map.insert(QLatin1String("onmousedown"),QStringList());
    map.insert(QLatin1String("onmousemove"),QStringList());
    map.insert(QLatin1String("onmouseout"),QStringList());
    map.insert(QLatin1String("onmouseover"),QStringList());
    map.insert(QLatin1String("onmouseup"),QStringList());
    return map;
}

}
