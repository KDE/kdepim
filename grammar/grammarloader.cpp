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
#include "grammarsettings.h"

#include <kservicetypetrader.h>
#include <KDebug>
#include <KConfigGroup>

namespace Grammar {
class GrammarLoaderPrivate
{
public:
    GrammarLoaderPrivate(GrammarLoader *qq)
        : settings(0),
          q(qq)
    {
    }

    ~GrammarLoaderPrivate()
    {
        plugins.clear();
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

        Grammar::GrammarClient *client = service->createInstance<Grammar::GrammarClient>(q, QVariantList(), &error);
        if (client) {
            const QStringList languages = client->languages();
            clients.append(client->name());
            QStringList::const_iterator itrEnd(languages.end());
            for (QStringList::const_iterator itr = languages.begin(); itr != itrEnd; ++itr) {
                if (!languageClients[*itr].isEmpty() && (client->reliability() < languageClients[*itr].first()->reliability()))
                    languageClients[*itr].append(client);
                else
                    languageClients[*itr].prepend(client);
            }
            //kDebug() << "Successfully loaded plugin:" << service->entryPath();
        } else {
            kDebug() << error;
        }
    }


    KService::List plugins;
    QStringList clients;
    // <language, Clients with that language >
    QMap<QString, QList<Grammar::GrammarClient*> > languageClients;
    GrammarSettings *settings;
    GrammarLoader *q;
};

K_GLOBAL_STATIC(GrammarLoader, s_loader)

GrammarLoader *GrammarLoader::openGrammarLoader()
{
    if (s_loader.isDestroyed()) {
        return 0;
    }

    return s_loader;
}

GrammarLoader::GrammarLoader()
    : d(new GrammarLoaderPrivate(this))
{
    d->settings = new GrammarSettings;//
    d->loadPlugins();
    KConfig config(QString::fromLatin1("grammarrc"));
    KConfigGroup grp = config.group(QLatin1String("General"));
    d->settings->readSettings(grp);
}

GrammarLoader::~GrammarLoader()
{
    delete d;
}

QStringList GrammarLoader::clients() const
{
    return d->clients;
}

QStringList GrammarLoader::languages() const
{
    return d->languageClients.keys();
}

GrammarSettings *GrammarLoader::settings() const
{
    return d->settings;
}

}

