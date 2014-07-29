/*
    This file is part of Akonadi.

    Copyright (c) 2011 Stephen Kelly <steveire@gmail.com>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "coisceimpart.h"

#include <kaction.h>
#include <kactioncollection.h>
#include <kcomponentdata.h>
#include <kfiledialog.h>
#include <kstandardaction.h>

#include <kpluginfactory.h>
#include <klocale.h>

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QTimer>
#include <QTextEdit>
#include <QLabel>

K_PLUGIN_FACTORY(CoisceimPartFactory, registerPlugin<CoisceimPart>();)

CoisceimPart::CoisceimPart( QWidget *parentWidget, QObject *parent, const QVariantList & /*args*/ )
    : KParts::ReadOnlyPart(parent)
{
    // we need an instance
    //QT5 setComponentData( CoisceimPartFactory::componentData() );

    component = new CoisceimWidget(parentWidget);

    setWidget(component);
}

CoisceimPart::~CoisceimPart()
{
}
#include "coisceimpart.moc"
