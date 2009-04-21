/*
    This file is part of KContactManager.

    Copyright (c) 2009 Laurent Montel <montel@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "kcontactmanager_part.h"
#include <kiconloader.h>
#include "aboutdata.h"

#include <kcomponentdata.h>
#include <klocale.h>
#include <kparts/genericfactory.h>
#include <kparts/statusbarextension.h>

K_PLUGIN_FACTORY( KContactManagerFactory, registerPlugin<KContactManagerPart>(); )
K_EXPORT_PLUGIN( KContactManagerFactory( AboutData() ) )

KContactManagerPart::KContactManagerPart( QWidget *parentWidget, QObject *parent,
                                    const QVariantList & )
  : KParts::ReadOnlyPart( parent )
{
  setComponentData( KContactManagerPart::componentData() );

  // create a canvas to insert our widget
  QWidget *canvas = new QWidget( parentWidget );
  canvas->setFocusPolicy( Qt::ClickFocus );
  setWidget( canvas );

  KIconLoader::global()->addAppDir( "kcontactmanager" );

  setXMLFile( "kcontactmanager_part.rc" );
}

KContactManagerPart::~KContactManagerPart()
{
}

bool KContactManagerPart::openFile()
{
  return false;
}
