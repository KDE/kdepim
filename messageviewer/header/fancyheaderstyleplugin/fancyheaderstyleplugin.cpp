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

#include "fancyheaderstyleplugin.h"
#include "header/fancyheaderstyle.h"
//Temporary
#include "header/headerstrategy_p.h"
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

HeaderStyleInterface *FancyHeaderStylePlugin::createView(KActionCollection *ac, QObject *parent)
{
    //TODO
    return Q_NULLPTR;
}

#include "fancyheaderstyleplugin.moc"
