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

#include "isgdshorturlengineplugin.h"
#include "isgdshorturlengineinterface.h"
#include <kpluginfactory.h>

using namespace PimCommon;

K_PLUGIN_FACTORY_WITH_JSON(IsgdShortUrlEnginePluginFactory, "pimcommon_isgdshorturlengineplugin.json", registerPlugin<IsgdShortUrlEnginePlugin>();)

IsgdShortUrlEnginePlugin::IsgdShortUrlEnginePlugin(QObject *parent, const QList<QVariant> &)
    : PimCommon::ShortUrlEnginePlugin(parent)
{

}

IsgdShortUrlEnginePlugin::~IsgdShortUrlEnginePlugin()
{

}

PimCommon::ShortUrlEngineInterface *IsgdShortUrlEnginePlugin::createInterface(QObject *parent)
{
    return new PimCommon::IsgdShortUrlEngineInterface(this, parent);
}

QString IsgdShortUrlEnginePlugin::engineName() const
{
    return QStringLiteral("isdgshorturl");
}

#include "isgdshorturlengineplugin.moc"
