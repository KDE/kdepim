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

#include "customheaderstyleplugin.h"
#include "customheaderstyleinterface.h"
#include "customheaderstyle.h"
#include "customheaderstrategy.h"
#include <kpluginfactory.h>
using namespace MessageViewer;

K_PLUGIN_FACTORY_WITH_JSON(MessageViewerCustomHeaderStylePluginFactory, "messageviewer_customheaderstyleplugin.json", registerPlugin<CustomHeaderStylePlugin>();)

CustomHeaderStylePlugin::CustomHeaderStylePlugin(QObject *parent, const QList<QVariant> &)
    : MessageViewer::HeaderStylePlugin(parent),
      mHeaderStyle(new CustomHeaderStyle),
      mHeaderStrategy(new CustomHeaderStrategy)
{
}

CustomHeaderStylePlugin::~CustomHeaderStylePlugin()
{

}

HeaderStyle *CustomHeaderStylePlugin::headerStyle() const
{
    return mHeaderStyle;
}

HeaderStrategy *CustomHeaderStylePlugin::headerStrategy() const
{
    return mHeaderStrategy;
}

HeaderStyleInterface *CustomHeaderStylePlugin::createView(KActionMenu *menu, QActionGroup *actionGroup, KActionCollection *ac, QObject *parent)
{
    MessageViewer::HeaderStyleInterface *view = new MessageViewer::CustomHeaderStyleInterface(this, parent);
    if (ac) {
        view->createAction(menu, actionGroup, ac);
    }
    return view;
}

QString CustomHeaderStylePlugin::name() const
{
    return QStringLiteral("custom");
}

#include "customheaderstyleplugin.moc"
