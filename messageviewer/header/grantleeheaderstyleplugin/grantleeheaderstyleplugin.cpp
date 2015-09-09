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

#include "grantleeheaderstyleinterface.h"
#include "grantleeheaderstyleplugin.h"
#include "header/grantleeheaderstyle.h"
//Temporary
#include "header/headerstrategy_p.h"
#include <kpluginfactory.h>
using namespace MessageViewer;

K_PLUGIN_FACTORY_WITH_JSON(MessageViewerGrantleeHeaderStylePluginFactory, "messageviewer_grantleeheaderstyleplugin.json", registerPlugin<GrantleeHeaderStylePlugin>();)

GrantleeHeaderStylePlugin::GrantleeHeaderStylePlugin(QObject *parent, const QList<QVariant> &)
    : MessageViewer::HeaderStylePlugin(parent),
      mHeaderStyle(new GrantleeHeaderStyle),
      mHeaderStrategy(new GrantleeHeaderStrategy)
{
}

GrantleeHeaderStylePlugin::~GrantleeHeaderStylePlugin()
{

}

HeaderStyle *GrantleeHeaderStylePlugin::headerStyle() const
{
    return mHeaderStyle;
}

HeaderStrategy *GrantleeHeaderStylePlugin::headerStrategy() const
{
    return mHeaderStrategy;
}

HeaderStyleInterface *GrantleeHeaderStylePlugin::createView(KActionCollection *ac, QObject *parent)
{
    MessageViewer::HeaderStyleInterface *view = new MessageViewer::GrantleeHeaderStyleInterface(mHeaderStyle, mHeaderStrategy, parent);
    view->createAction(ac);
    return view;
}

QString GrantleeHeaderStylePlugin::name() const
{
    return QStringLiteral("grantlee");
}

#include "grantleeheaderstyleplugin.moc"
