/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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
#include "declarativeidentitycombobox.h"

#include <kpimidentities/identitycombo.h>
#include <QtGui/QGraphicsProxyWidget>
#include <kpimidentities/identitymanager.h>

DeclarativeIdentityComboBox::DeclarativeIdentityComboBox ( QDeclarativeItem* parent ) :
  QDeclarativeItem ( parent ),
  m_proxy( new QGraphicsProxyWidget( this ) ),
  m_manager( new KPIMIdentities::IdentityManager )
{
  KPIMIdentities::IdentityCombo* combo = new KPIMIdentities::IdentityCombo( m_manager.data(), 0 );
  m_proxy->setWidget( combo );
}

void DeclarativeIdentityComboBox::geometryChanged ( const QRectF& newGeometry, const QRectF& oldGeometry )
{
  QDeclarativeItem::geometryChanged ( newGeometry, oldGeometry );
  m_proxy->resize( newGeometry.size() );
}

#include "declarativeidentitycombobox.moc"
