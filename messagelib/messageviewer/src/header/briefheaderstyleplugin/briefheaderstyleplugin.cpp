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

#include "briefheaderstyleplugin.h"
#include "briefheaderstyleinterface.h"
#include "briefheaderstyle.h"
#include "briefheaderstrategy.h"
#include <kpluginfactory.h>
using namespace MessageViewer;

K_PLUGIN_FACTORY_WITH_JSON(MessageViewerBriefHeaderStylePluginFactory, "messageviewer_briefheaderstyleplugin.json", registerPlugin<BriefHeaderStylePlugin>();)

BriefHeaderStylePlugin::BriefHeaderStylePlugin(QObject *parent, const QList<QVariant> &)
    : MessageViewer::HeaderStylePlugin(parent),
      mHeaderStyle(new BriefHeaderStyle),
      mHeaderStrategy(new BriefHeaderStrategy)
{
}

BriefHeaderStylePlugin::~BriefHeaderStylePlugin()
{

}

HeaderStyle *BriefHeaderStylePlugin::headerStyle() const
{
    return mHeaderStyle;
}

HeaderStrategy *BriefHeaderStylePlugin::headerStrategy() const
{
    return mHeaderStrategy;
}

MessageViewer::HeaderStyleInterface *BriefHeaderStylePlugin::createView(KActionMenu *menu, QActionGroup *actionGroup, KActionCollection *ac, QObject *parent)
{
    MessageViewer::BriefHeaderStyleInterface *view = new MessageViewer::BriefHeaderStyleInterface(this, parent);
    if (ac) {
        view->createAction(menu, actionGroup, ac);
    }
    return view;
}

QString BriefHeaderStylePlugin::name() const
{
    return QStringLiteral("brief");
}

#include "briefheaderstyleplugin.moc"
