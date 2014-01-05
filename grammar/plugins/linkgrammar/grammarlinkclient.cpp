/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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


#include "grammarlinkclient.h"
#include "grammarlinkplugin.h"

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kdebug.h>

K_PLUGIN_FACTORY( GrammarLinkClientFactory, registerPlugin<GrammarLinkClient>(); )
K_EXPORT_PLUGIN( GrammarLinkClientFactory( "grammar_link" ) )

GrammarLinkClient::GrammarLinkClient(QObject *parent, const QVariantList& /* args */)
    : Grammar::GrammarClient(parent)
{
}

GrammarLinkClient::~GrammarLinkClient()
{

}

int GrammarLinkClient::reliability() const
{
    return 20;
}

Grammar::GrammarPlugin *GrammarLinkClient::createGrammarChecker(const QString &language)
{
    GrammarLinkPlugin *plugin = new GrammarLinkPlugin(language);
    return plugin;
}

QStringList GrammarLinkClient::searchLanguages() const
{
    //TODO
    return QStringList();
}

QStringList GrammarLinkClient::languages() const
{
    //return searchLanguages();
    //TODO improve search
    return QStringList() <<QLatin1String("de")<<QLatin1String("en")<<QLatin1String("lt");
}

QString GrammarLinkClient::name() const
{
    return QLatin1String("grammarlink");
}


