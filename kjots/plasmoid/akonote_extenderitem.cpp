/*
    Copyright (c) 2009 Stephen Kelly <steveire@gmail.com>

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

#include "akonote_extenderitem.h"

#include <QPainter>

#include <Plasma/FrameSvg>

AkonoteExtenderItem::AkonoteExtenderItem(Plasma::Extender* hostExtender, uint extenderItemId)
    : ExtenderItem(hostExtender, extenderItemId)
{
  m_theme = new Plasma::FrameSvg(this);
  m_theme->setImagePath("widgets/stickynote");
  m_theme->setEnabledBorders(Plasma::FrameSvg::AllBorders);

}

void AkonoteExtenderItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{

  Q_UNUSED(option);
  Q_UNUSED(widget);

  painter->setRenderHint(QPainter::SmoothPixmapTransform);
  painter->setRenderHint(QPainter::Antialiasing);

  m_theme->resize(90,90);

  painter->save();
  m_theme->paintFrame(painter);
  painter->restore();
//     Plasma::ExtenderItem::paint(painter, option, widget);
}
