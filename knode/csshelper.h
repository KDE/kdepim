/*
    KNode, the KDE newsreader
    Copyright (c) 2005 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNODE_CSSHELPER_H
#define KNODE_CSSHELPER_H

#include <messageviewer/viewer/csshelperbase.h>

namespace KNode {

/** Helper class to generate a CSS style sheet for article widget.
 *  @see KNode::ArticleWidget
 */
class CSSHelper : public MessageViewer::CSSHelperBase
{
  public:
    explicit CSSHelper( const QPaintDevice *pd );

};

}

#endif
