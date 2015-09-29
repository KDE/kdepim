/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "viewerplugincreatetodo.h"
#include "viewerplugincreatetodointerface.h"
#include <KActionCollection>
#include <KToggleAction>
#include <kpluginfactory.h>

using namespace MessageViewer;
K_PLUGIN_FACTORY_WITH_JSON(ViewerPluginCreatetodoFactory, "messageviewer_createtodoplugin.json", registerPlugin<ViewerPluginCreatetodo>();)

ViewerPluginCreatetodo::ViewerPluginCreatetodo(QObject *parent, const QList<QVariant> &)
    : MessageViewer::ViewerPlugin(parent)
{

}

ViewerPluginInterface *ViewerPluginCreatetodo::createView(QWidget *parent, KActionCollection *ac)
{
    MessageViewer::ViewerPluginInterface *view = new MessageViewer::ViewerPluginCreateTodoInterface(ac, parent);
    return view;
}

QString ViewerPluginCreatetodo::viewerPluginName() const
{
    return QStringLiteral("create-todo");
}

#include "viewerplugincreatetodo.moc"
