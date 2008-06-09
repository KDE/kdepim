/*
    This file is part of KJots.

    Copyright (c) 2008 Stephen Kelly <steveire@gmail.com>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "kjotspart.h"

#include "kjotspart.moc"
#include "kjotscomponent.h"
#include "aboutdata.h"

#include <kaction.h>
#include <kactioncollection.h>
#include <kcomponentdata.h>
#include <kfiledialog.h>
#include <kstandardaction.h>
#include <kstandarddirs.h>
#include <kpluginfactory.h>
#include <klocale.h>

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QTextEdit>

const KAboutData &createAboutData()
{
    static AboutData aboutData;
    return aboutData;
}

K_PLUGIN_FACTORY(KJotsPartFactory, registerPlugin<KJotsPart>();)
K_EXPORT_PLUGIN(KJotsPartFactory(createAboutData()))

KJotsPart::KJotsPart( QWidget *parentWidget, QObject *parent, const QVariantList & /*args*/ )
    : KParts::ReadOnlyPart(parent)
{
    // we need an instance
    setComponentData( KJotsPartFactory::componentData() );

    // this should be your custom internal widget
    component = new KJotsComponent(parentWidget, actionCollection());

    // notify the part that this is our internal widget
    setWidget(component);

    // set our XML-UI resource file
    setXMLFile(KStandardDirs::locate("data", "kjots/kjotspartui.rc"));
}

KJotsPart::~KJotsPart()
{
	component->queryClose();
}

bool KJotsPart::openFile()
{
    return false;
}
//
// bool KJotsPart::saveFile()
// {
//     return false;
// }
