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

#include "translatorplugin.h"
#include "translatorview.h"
#include <KLocalizedString>
#include <kpluginfactory.h>
#include <customtools/customtoolswidgetng.h>

using namespace PimCommon;
K_PLUGIN_FACTORY_WITH_JSON(PimCommonTranslatorPluginFactory, "pimcommon_translatorplugin.json", registerPlugin<TranslatorPlugin>();)

TranslatorPlugin::TranslatorPlugin(QObject *parent, const QList<QVariant> &)
    : PimCommon::CustomToolsPlugin(parent)
{

}

TranslatorPlugin::~TranslatorPlugin()
{

}

CustomToolsViewInterface *TranslatorPlugin::createView(KActionCollection *ac, CustomToolsWidgetNg *parent)
{
    PimCommon::TranslatorView *view = new PimCommon::TranslatorView(ac, parent);

    connect(view, &PimCommon::TranslatorView::toolsWasClosed, parent, &CustomToolsWidgetNg::slotToolsWasClosed);
    connect(view, &PimCommon::TranslatorView::insertText, parent, &CustomToolsWidgetNg::insertText);
    connect(view, &PimCommon::TranslatorView::activateView, parent, &CustomToolsWidgetNg::slotActivateView);
    return view;

}

QString TranslatorPlugin::customToolName() const
{
    return i18n("Translator");
}

#include "translatorplugin.moc"
