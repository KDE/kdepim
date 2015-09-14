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

#include "shorturlplugin.h"
#include "shorturlview.h"
#include <KLocalizedString>
#include <KToggleAction>
#include <kpluginfactory.h>
#include <customtools/customtoolswidgetng.h>

using namespace PimCommon;
K_PLUGIN_FACTORY_WITH_JSON(PimCommonShorturlPluginFactory, "pimcommon_shorturlplugin.json", registerPlugin<ShorturlPlugin>();)

ShorturlPlugin::ShorturlPlugin(QObject *parent, const QList<QVariant> &)
    : PimCommon::CustomToolsPlugin(parent)
{

}

ShorturlPlugin::~ShorturlPlugin()
{

}

CustomToolsViewInterface *ShorturlPlugin::createView(KActionCollection *ac, CustomToolsWidgetNg *parent)
{
    PimCommon::ShorturlView *view = new PimCommon::ShorturlView(ac, parent);

    connect(view, &PimCommon::ShorturlView::toolsWasClosed, parent, &CustomToolsWidgetNg::slotToolsWasClosed);
    connect(view, &PimCommon::ShorturlView::insertText, parent, &CustomToolsWidgetNg::insertText);
    connect(view, &PimCommon::ShorturlView::activateView, parent, &CustomToolsWidgetNg::slotActivateView);
    return view;
}

QString ShorturlPlugin::customToolName() const
{
    return i18n("Translator");
}

#include "shorturlplugin.moc"
