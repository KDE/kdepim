/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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
    globalAttr.insert(QStringLiteral("id"), QStringList());
    globalAttr.insert(QStringLiteral("class"), QStringList());
    globalAttr.insert(QStringLiteral("title"), QStringList());
    globalAttr.insert(QStringLiteral("lang"), QStringList());
    QStringList dir;
    dir << QStringLiteral("ltr");
    dir << QStringLiteral("rtl");
    dir << QStringLiteral("auto");
    globalAttr.insert(QStringLiteral("dir"), dir);

    return globalAttr;
}

QMap<QString, QStringList> ExtendAttributesUtils::globalAttribute()
{
    QMap<QString, QStringList> globalAttr;
    globalAttr.insert(QStringLiteral("accesskey"), QStringList());
    globalAttr.insert(QStringLiteral("class"), QStringList());
    QStringList dir;
    dir << QStringLiteral("ltr");
    dir << QStringLiteral("rtl");
    dir << QStringLiteral("auto");
    globalAttr.insert(QStringLiteral("dir"), dir);
    globalAttr.insert(QStringLiteral("id"), QStringList());
    globalAttr.insert(QStringLiteral("lang"), QStringList());
    globalAttr.insert(QStringLiteral("style"), QStringList());
    globalAttr.insert(QStringLiteral("tabindex"), QStringList());
    globalAttr.insert(QStringLiteral("title"), QStringList());
    return globalAttr;
}

QMap<QString, QStringList> ExtendAttributesUtils::attributesMap(ExtendAttributesDialog::ExtendType type)
{
    switch (type) {
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
    align << QStringLiteral("top");
    align << QStringLiteral("bottom");
    align << QStringLiteral("middle");
    align << QStringLiteral("left");
    align << QStringLiteral("right");
    map.insert(QStringLiteral("align"), align);
    map.insert(QStringLiteral("alt"), QStringList());
    map.insert(QStringLiteral("border"), QStringList());
    map.insert(QStringLiteral("height"), QStringList());
    map.insert(QStringLiteral("hspace"), QStringList());
    map.insert(QStringLiteral("ismap"), QStringList());
    map.insert(QStringLiteral("longdesc"), QStringList());
    map.insert(QStringLiteral("src"), QStringList());
    map.insert(QStringLiteral("usemap"), QStringList());
    map.insert(QStringLiteral("vspace"), QStringList());
    map.insert(QStringLiteral("width"), QStringList());
    return map;
}

QMap<QString, QStringList> ExtendAttributesUtils::attributesMapTable()
{
    QMap<QString, QStringList> map;
    map = globalAttribute();
    QStringList align;
    align << QStringLiteral("left");
    align << QStringLiteral("center");
    align << QStringLiteral("right");
    map.insert(QStringLiteral("align"), align);
    map.insert(QStringLiteral("bgcolor"), QStringList());
    map.insert(QStringLiteral("border"), QStringList());
    map.insert(QStringLiteral("cellpadding"), QStringList());
    map.insert(QStringLiteral("cellspacing"), QStringList());
    QStringList frame;
    frame << QStringLiteral("void");
    frame << QStringLiteral("above");
    frame << QStringLiteral("below");
    frame << QStringLiteral("hsides");
    frame << QStringLiteral("lhs");
    frame << QStringLiteral("rhs");
    frame << QStringLiteral("vsides");
    frame << QStringLiteral("box");
    frame << QStringLiteral("border");
    map.insert(QStringLiteral("frame"), frame);
    QStringList rules;
    rules << QStringLiteral("none");
    rules << QStringLiteral("groups");
    rules << QStringLiteral("rows");
    rules << QStringLiteral("cols");
    rules << QStringLiteral("all");
    map.insert(QStringLiteral("rules"), rules);
    map.insert(QStringLiteral("summary"), QStringList());
    map.insert(QStringLiteral("width"), QStringList());
    return map;
}

QMap<QString, QStringList> ExtendAttributesUtils::attributesMapCell()
{
    QMap<QString, QStringList> map;
    map = globalAttribute();
    map.insert(QStringLiteral("abbr"), QStringList());
    QStringList align;
    align << QStringLiteral("top");
    align << QStringLiteral("bottom");
    align << QStringLiteral("middle");
    align << QStringLiteral("left");
    align << QStringLiteral("right");
    align << QStringLiteral("char");
    map.insert(QStringLiteral("align"), align);
    map.insert(QStringLiteral("axis"), QStringList());
    map.insert(QStringLiteral("bgcolor"), QStringList());
    map.insert(QStringLiteral("char"), QStringList());
    map.insert(QStringLiteral("charoff"), QStringList());
    map.insert(QStringLiteral("colspan"), QStringList());
    map.insert(QStringLiteral("headers"), QStringList());
    map.insert(QStringLiteral("height"), QStringList());
    map.insert(QStringLiteral("nowrap"), QStringList());
    map.insert(QStringLiteral("rowspan"), QStringList());
    QStringList scope;
    scope << QStringLiteral("col");
    scope << QStringLiteral("colgroup");
    scope << QStringLiteral("row");
    scope << QStringLiteral("rowgroup");
    map.insert(QStringLiteral("scope"), scope);
    QStringList valign;
    valign << QStringLiteral("top");
    valign << QStringLiteral("middle");
    valign << QStringLiteral("bottom");
    valign << QStringLiteral("baseline");
    map.insert(QStringLiteral("valign"), valign);
    map.insert(QStringLiteral("width"), QStringList());
    return map;
}

QMap<QString, QStringList> ExtendAttributesUtils::attributesMapLink()
{
    QMap<QString, QStringList> map;
    map = globalAttribute();
    map.insert(QStringLiteral("charset"), QStringList());
    map.insert(QStringLiteral("href"), QStringList());
    map.insert(QStringLiteral("hreflang"), QStringList());
    map.insert(QStringLiteral("media"), QStringList());
    QStringList rel;
    rel << QStringLiteral("alternate");
    rel << QStringLiteral("archives");
    rel << QStringLiteral("author");
    rel << QStringLiteral("bookmark");
    rel << QStringLiteral("external");
    rel << QStringLiteral("first");
    rel << QStringLiteral("help");
    rel << QStringLiteral("icon");
    rel << QStringLiteral("last");
    rel << QStringLiteral("license");
    rel << QStringLiteral("next");
    rel << QStringLiteral("nofollow");
    rel << QStringLiteral("noreferrer");
    rel << QStringLiteral("pingback");
    rel << QStringLiteral("prefetch");
    rel << QStringLiteral("prev");
    rel << QStringLiteral("search");
    rel << QStringLiteral("sidebar");
    rel << QStringLiteral("stylesheet");
    rel << QStringLiteral("tag");
    rel << QStringLiteral("up");
    map.insert(QStringLiteral("rel"), rel);
    map.insert(QStringLiteral("rev"), QStringList());
    QStringList target;
    target << QStringLiteral("_blank");
    target << QStringLiteral("_self");
    target << QStringLiteral("_top");
    target << QStringLiteral("_parent");
    map.insert(QStringLiteral("target"), target);
    map.insert(QStringLiteral("type"), QStringList());
    return map;
}

QMap<QString, QStringList> ExtendAttributesUtils::attributesMapListUL()
{
    QMap<QString, QStringList> map;
    map = listGlobalAttribute();
    QStringList rel;
    rel << QStringLiteral("compact");
    map.insert(QStringLiteral("compact"), rel);
    QStringList type;
    type << QStringLiteral("disc");
    type << QStringLiteral("square");
    type << QStringLiteral("circle");
    map.insert(QStringLiteral("type"), type);
    return map;
}

QMap<QString, QStringList> ExtendAttributesUtils::attributesMapListOL()
{
    QMap<QString, QStringList> map;
    map = listGlobalAttribute();
    QStringList rel;
    rel << QStringLiteral("compact");
    map.insert(QStringLiteral("compact"), rel);
    map.insert(QStringLiteral("start"), QStringList());
    QStringList type;
    type << QStringLiteral("1");
    type << QStringLiteral("A");
    type << QStringLiteral("a");
    type << QStringLiteral("I");
    type << QStringLiteral("i");
    map.insert(QStringLiteral("type"), type);

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
    map.insert(QStringLiteral("alink"), QStringList());
    map.insert(QStringLiteral("background"), QStringList());
    map.insert(QStringLiteral("bgcolor"), QStringList());
    map.insert(QStringLiteral("link"), QStringList());
    map.insert(QStringLiteral("text"), QStringList());
    map.insert(QStringLiteral("vlink"), QStringList());
    return map;
}

QMap<QString, QStringList> ExtendAttributesUtils::attributesJavascriptWindowAndBase()
{
    QMap<QString, QStringList> map;
    map = attributesJavascript();
    //WindowEvent
    map.insert(QStringLiteral("onload"), QStringList());
    map.insert(QStringLiteral("onunload"), QStringList());
    return map;
}

QMap<QString, QStringList> ExtendAttributesUtils::attributesJavascript()
{
    QMap<QString, QStringList> map;
    //Form event
    map.insert(QStringLiteral("onblur"), QStringList());
    map.insert(QStringLiteral("onchange"), QStringList());
    map.insert(QStringLiteral("onfocus"), QStringList());
    map.insert(QStringLiteral("onreset"), QStringList());
    map.insert(QStringLiteral("onselect"), QStringList());
    map.insert(QStringLiteral("onsubmit"), QStringList());
    //Keyboard event.
    map.insert(QStringLiteral("onkeydown"), QStringList());
    map.insert(QStringLiteral("onkeypress"), QStringList());
    map.insert(QStringLiteral("onkeyup"), QStringList());

    //Mouse event.
    map.insert(QStringLiteral("onclick"), QStringList());
    map.insert(QStringLiteral("ondblclick"), QStringList());
    map.insert(QStringLiteral("onmousedown"), QStringList());
    map.insert(QStringLiteral("onmousemove"), QStringList());
    map.insert(QStringLiteral("onmouseout"), QStringList());
    map.insert(QStringLiteral("onmouseover"), QStringList());
    map.insert(QStringLiteral("onmouseup"), QStringList());
    return map;
}

}
