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

#include "allheaderstyleplugin.h"
#include "header/plainheaderstyle.h"
#include "allheaderstyleinterface.h"
#include "allheaderstrategy.h"
#include <kpluginfactory.h>
using namespace MessageViewer;

K_PLUGIN_FACTORY_WITH_JSON(MessageViewerFancyHeaderStylePluginFactory, "messageviewer_allheaderstyleplugin.json", registerPlugin<AllHeaderStylePlugin>();)

AllHeaderStylePlugin::AllHeaderStylePlugin(QObject *parent, const QList<QVariant> &)
    : MessageViewer::HeaderStylePlugin(parent),
      mHeaderStyle(new PlainHeaderStyle),
      mHeaderStrategy(new AllHeaderStrategy)
{
}

AllHeaderStylePlugin::~AllHeaderStylePlugin()
{

}

HeaderStyle *AllHeaderStylePlugin::headerStyle() const
{
    return mHeaderStyle;
}

HeaderStrategy *AllHeaderStylePlugin::headerStrategy() const
{
    return mHeaderStrategy;
}

HeaderStyleInterface *AllHeaderStylePlugin::createView(KActionMenu *menu, QActionGroup *actionGroup, KActionCollection *ac, QObject *parent)
{
    MessageViewer::AllHeaderStyleInterface *view = new MessageViewer::AllHeaderStyleInterface(this, parent);
    view->createAction(menu, actionGroup, ac);
    return view;
}

QString MessageViewer::AllHeaderStylePlugin::name() const
{
    return QStringLiteral("all-headers");
}

#include "allheaderstyleplugin.moc"

