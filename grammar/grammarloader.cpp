/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "grammarloader.h"
#include "grammarclient_p.h"

#include <kservicetypetrader.h>

namespace Grammar {
class GrammarLoaderPrivate
{
public:
    GrammarLoaderPrivate()
    {
        loadPlugins();
    }

    void loadPlugins()
    {
        plugins = KServiceTypeTrader::self()->query(QString::fromLatin1("Grammar/GrammarClient"));

        KService::List::const_iterator end(plugins.constEnd());
        for (KService::List::const_iterator itr = plugins.constBegin(); itr != end; ++itr ) {
            loadPlugin((*itr));
        }
    }

    void loadPlugin(const KSharedPtr<KService> &service)
    {
        QString error;

        Grammar::GrammarClient *client = service->createInstance<Grammar::GrammarClient>(this, QVariantList(), &error);
        if (client) {
            const QStringList languages = client->languages();
            clients.append(client->name());
#if 0
            for (QStringList::const_iterator itr = languages.begin();
                 itr != languages.end(); ++itr) {
                if (!d->languageClients[*itr].isEmpty() &&
                        client->reliability() <
                        d->languageClients[*itr].first()->reliability())
                    d->languageClients[*itr].append(client);
                else
                    d->languageClients[*itr].prepend(client);
            }
#endif
            //kDebug() << "Successfully loaded plugin:" << service->entryPath();
        } else {
            kDebug() << error;
        }
    }


    KService::List plugins;
};

GrammarLoader::GrammarLoader()
    : d(new GrammarLoaderPrivate)
{
}

GrammarLoader::~GrammarLoader()
{
    delete d;
}

QStringList GrammarLoader::clients() const
{
    return d->clients;
}


}

#include "grammarloader.moc"
