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

#include "fancyheaderstyleinterface.h"
#include "fancyheaderstyleplugin.h"
#include "fancyheaderstyle.h"
#include "messageviewer/richheaderstrategy.h"
#include <kpluginfactory.h>
using namespace MessageViewer;

K_PLUGIN_FACTORY_WITH_JSON(MessageViewerFancyHeaderStylePluginFactory, "messageviewer_fancyheaderstyleplugin.json", registerPlugin<FancyHeaderStylePlugin>();)

FancyHeaderStylePlugin::FancyHeaderStylePlugin(QObject *parent, const QList<QVariant> &)
    : MessageViewer::HeaderStylePlugin(parent),
      mHeaderStyle(new FancyHeaderStyle),
      mHeaderStrategy(new RichHeaderStrategy)
{
}

FancyHeaderStylePlugin::~FancyHeaderStylePlugin()
{

}

HeaderStyle *FancyHeaderStylePlugin::headerStyle() const
{
    return mHeaderStyle;
}

HeaderStrategy *FancyHeaderStylePlugin::headerStrategy() const
{
    return mHeaderStrategy;
}

HeaderStyleInterface *FancyHeaderStylePlugin::createView(KActionMenu *menu, QActionGroup *actionGroup, KActionCollection *ac, QObject *parent)
{
    MessageViewer::HeaderStyleInterface *view = new MessageViewer::FancyHeaderStyleInterface(this, parent);
    if (ac) {
        view->createAction(menu, actionGroup, ac);
    }
    return view;
}

QString FancyHeaderStylePlugin::name() const
{
    return QStringLiteral("fancy");
}

int FancyHeaderStylePlugin::elidedTextSize() const
{
    return 1000;
}

#include "fancyheaderstyleplugin.moc"
