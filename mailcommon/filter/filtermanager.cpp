/*
    Copyright (C) 2011 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "filtermanager.h"

#include "filteraction.h"
#include "filteractiondict.h"
#include "filterimporterexporter.h"
#include "mailfilteragentinterface.h"
#include <kconfiggroup.h>
#include <Monitor>
#include <Tag>
#include <TagFetchJob>
#include <TagFetchScope>
#include <TagAttribute>

#include <QDebug>

#include <QTimer>

namespace MailCommon
{

class FilterManager::Private
{
public:
    Private(FilterManager *qq)
        : q(qq), mMailFilterAgentInterface(0), mMonitor(new Akonadi::Monitor), mInitialized(false)
    {
        mMailFilterAgentInterface = new org::freedesktop::Akonadi::MailFilterAgent(QLatin1String("org.freedesktop.Akonadi.MailFilterAgent"),
                QLatin1String("/MailFilterAgent"),
                QDBusConnection::sessionBus(), q);
    }

    void readConfig();
    void writeConfig(bool withSync = true) const;
    void clear();

    QMap<QUrl, QString> mTagList;
    static FilterManager *mInstance;
    static FilterActionDict *mFilterActionDict;

    FilterManager *q;
    OrgFreedesktopAkonadiMailFilterAgentInterface *mMailFilterAgentInterface;
    QList<MailCommon::MailFilter *> mFilters;
    Akonadi::Monitor *mMonitor;
    bool mInitialized;
};

void FilterManager::Private::readConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig(QLatin1String("akonadi_mailfilter_agentrc"));
    clear();
    QStringList emptyFilters;
    mFilters = FilterImporterExporter::readFiltersFromConfig(config, emptyFilters);
    emit q->filtersChanged();
}

void FilterManager::Private::writeConfig(bool withSync) const
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig(QLatin1String("akonadi_mailfilter_agentrc"));

    // Now, write out the new stuff:
    FilterImporterExporter::writeFiltersToConfig(mFilters, config);
    KConfigGroup group = config->group("General");

    if (withSync) {
        group.sync();
    }
}

void FilterManager::Private::clear()
{
    qDeleteAll(mFilters);
    mFilters.clear();
}

}

using namespace MailCommon;

FilterManager *FilterManager::Private::mInstance = 0;
FilterActionDict *FilterManager::Private::mFilterActionDict = 0;

FilterManager *FilterManager::instance()
{
    if (!FilterManager::Private::mInstance) {
        FilterManager::Private::mInstance = new FilterManager;
    }

    return FilterManager::Private::mInstance;
}

FilterActionDict *FilterManager::filterActionDict()
{
    if (!FilterManager::Private::mFilterActionDict) {
        FilterManager::Private::mFilterActionDict = new FilterActionDict;
    }

    return FilterManager::Private::mFilterActionDict;
}

FilterManager::FilterManager()
    : d(new Private(this))
{
    updateTagList();

    d->mMonitor->setTypeMonitored(Akonadi::Monitor::Tags);
    d->mMonitor->tagFetchScope().fetchAttribute<Akonadi::TagAttribute>();
    connect(d->mMonitor, &Akonadi::Monitor::tagAdded, this, &FilterManager::slotTagAdded);
    connect(d->mMonitor, &Akonadi::Monitor::tagRemoved, this, &FilterManager::slotTagRemoved);
    connect(d->mMonitor, &Akonadi::Monitor::tagChanged, this, &FilterManager::slotTagChanged);

    qDBusRegisterMetaType<QList<qint64> >();
    Akonadi::ServerManager::State state = Akonadi::ServerManager::self()->state();
    if (state == Akonadi::ServerManager::Running) {
        QTimer::singleShot(0, this, SLOT(slotReadConfig()));
    } else {
        connect(Akonadi::ServerManager::self(), &Akonadi::ServerManager::stateChanged, this, &FilterManager::slotServerStateChanged);
    }
}

void FilterManager::slotServerStateChanged(Akonadi::ServerManager::State state)
{
    if (state == Akonadi::ServerManager::Running) {
        d->readConfig();
        disconnect(Akonadi::ServerManager::self(), SIGNAL(stateChanged(Akonadi::ServerManager::State)));
    }
}

void FilterManager::updateTagList()
{
    Akonadi::TagFetchJob *fetchJob = new Akonadi::TagFetchJob(this);
    fetchJob->fetchScope().fetchAttribute<Akonadi::TagAttribute>();
    connect(fetchJob, &Akonadi::TagFetchJob::result, this, &FilterManager::slotFinishedTagListing);
}

bool FilterManager::initialized() const
{
    return d->mInitialized;
}

void FilterManager::slotReadConfig()
{
    d->readConfig();
    d->mInitialized = true;
    Q_EMIT loadingFiltersDone();
}

void FilterManager::slotFinishedTagListing(KJob *job)
{
    if (job->error()) {
        qWarning() << "failed to retrieve tags " << job->errorString();
    }
    Akonadi::TagFetchJob *fetchJob = static_cast<Akonadi::TagFetchJob *>(job);
    Q_FOREACH (const Akonadi::Tag &tag, fetchJob->tags()) {
        d->mTagList.insert(tag.url(), tag.name());
    }

    Q_EMIT tagListingFinished();
}

void FilterManager::slotTagAdded(const Akonadi::Tag &tag)
{
    d->mTagList.insert(tag.url(), tag.name());
    Q_EMIT tagListingFinished();
}

void FilterManager::slotTagChanged(const Akonadi::Tag &tag)
{
    if (d->mTagList.contains(tag.url())) {
        d->mTagList.insert(tag.url(), tag.name());
    }
    Q_EMIT tagListingFinished();
}

void FilterManager::slotTagRemoved(const Akonadi::Tag &tag)
{
    if (d->mTagList.contains(tag.url())) {
        d->mTagList.remove(tag.url());
    }
    Q_EMIT tagListingFinished();
}

QMap<QUrl, QString> FilterManager::tagList() const
{
    return d->mTagList;
}

bool FilterManager::isValid() const
{
    return d->mMailFilterAgentInterface->isValid();
}

QString FilterManager::createUniqueFilterName(const QString &name) const
{
    return d->mMailFilterAgentInterface->createUniqueName(name);
}

void FilterManager::showFilterLogDialog(qlonglong windowId)
{
    d->mMailFilterAgentInterface->showFilterLogDialog(windowId);
}

void FilterManager::filter(const Akonadi::Item &item, const QString &identifier, const QString &resourceId) const
{
    d->mMailFilterAgentInterface->filter(item.id(), identifier, resourceId);
}

void FilterManager::filter(const Akonadi::Item &item, FilterSet set, bool account, const QString &resourceId) const
{
    d->mMailFilterAgentInterface->filterItem(item.id(), static_cast<int>(set), account ? resourceId : QString());
}

void FilterManager::filter(const Akonadi::Item::List &messages, FilterManager::FilterSet set) const
{
    QList<qint64> itemIds;

    foreach (const Akonadi::Item &item, messages) {
        itemIds << item.id();
    }

    d->mMailFilterAgentInterface->filterItems(itemIds, static_cast<int>(set));
}

void FilterManager::filter(const Akonadi::Item::List &messages, SearchRule::RequiredPart requiredPart, const QStringList &listFilters) const
{
    QList<qint64> itemIds;

    foreach (const Akonadi::Item &item, messages) {
        itemIds << item.id();
    }
    d->mMailFilterAgentInterface->applySpecificFilters(itemIds, static_cast<int>(requiredPart), listFilters);
}

void FilterManager::setFilters(const QList<MailCommon::MailFilter *> &filters)
{
    beginUpdate();
    d->clear();
    d->mFilters = filters;
    endUpdate();
}

QList<MailCommon::MailFilter *> FilterManager::filters() const
{
    return d->mFilters;
}

void FilterManager::appendFilters(const QList<MailCommon::MailFilter *> &filters, bool replaceIfNameExists)
{
    beginUpdate();
    if (replaceIfNameExists) {
        foreach (const MailCommon::MailFilter *newFilter, filters) {
            int numberOfFilters = d->mFilters.count();
            for (int i = 0; i < numberOfFilters; ++i) {
                MailCommon::MailFilter *filter = d->mFilters.at(i);
                if (newFilter->name() == filter->name()) {
                    d->mFilters.removeAll(filter);
                    i = 0;
                    numberOfFilters = d->mFilters.count();
                }
            }
        }
    }

    d->mFilters += filters;
    endUpdate();
}

void FilterManager::removeFilter(MailCommon::MailFilter *filter)
{
    beginUpdate();
    d->mFilters.removeAll(filter);
    endUpdate();
}

void FilterManager::beginUpdate()
{
}

void FilterManager::endUpdate()
{
    d->writeConfig(true);
    d->mMailFilterAgentInterface->reload();
    emit filtersChanged();
}

