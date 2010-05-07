/*
    Copyright (c) 2010 <vkrause@kde.org>

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

#include "declarativeeditor.h"

#include <messagecomposer/kmeditor.h>
#include <QtGui/QGraphicsProxyWidget>

DeclarativeEditor::DeclarativeEditor ( QDeclarativeItem* parent ) :
  QDeclarativeItem ( parent ),
  m_proxy( new QGraphicsProxyWidget( this ) )
{
  Message::KMeditor *editor = new Message::KMeditor;
  editor->setCheckSpellingEnabled( true );
  m_proxy->setWidget( editor );
}

void DeclarativeEditor::geometryChanged ( const QRectF& newGeometry, const QRectF& oldGeometry )
{
  QDeclarativeItem::geometryChanged ( newGeometry, oldGeometry );
  m_proxy->resize( newGeometry.size() );
}

#include "declarativeeditor.moc"
