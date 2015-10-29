/*
  Copyright (C) 2014 Christian Mollekopf <mollekopf@kolabsys.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#include "personsearchjob.h"
#include "libkdepim_debug.h"

#include <AkonadiCore/EntityDisplayAttribute>
#include <AkonadiCore/CollectionModifyJob>
#include <AkonadiCore/CollectionFetchJob>
#include <AkonadiCore/CollectionFetchScope>
#include <AkonadiSearch/PIM/collectionquery.h>
#include <AkonadiCore/collectionidentificationattribute.h>

using namespace KPIM;

class KPIM::PersonSearchJobPrivate
{
public:
    PersonSearchJobPrivate(const QString &searchString)
        : mSearchString(searchString),
          mCollectionSearchDone(false),
          mLdapSearchDone(false)

    {

    }

    ~PersonSearchJobPrivate()
    {
        mLdapSearch.cancelSearch();
    }

    QString mSearchString;
    QHash<QString, Person> mMatches;
    KLDAP::LdapClientSearch mLdapSearch;
    bool mCollectionSearchDone;
    bool mLdapSearchDone;
};

PersonSearchJob::PersonSearchJob(const QString &searchString, QObject *parent)
    : KJob(parent),
      d(new KPIM::PersonSearchJobPrivate(searchString))
{
    connect(&d->mLdapSearch, static_cast<void (KLDAP::LdapClientSearch::*)(const KLDAP::LdapResultObject::List &)>(&KLDAP::LdapClientSearch::searchData),
            this, &PersonSearchJob::onLDAPSearchData);

    connect(&d->mLdapSearch, &KLDAP::LdapClientSearch::searchDone, this, &PersonSearchJob::onLDAPSearchDone);
}

PersonSearchJob::~PersonSearchJob()
{
    delete d;
}

bool PersonSearchJob::kill(KJob::KillVerbosity verbosity)
{
    d->mLdapSearch.cancelSearch();
    return KJob::kill(verbosity);
}

void PersonSearchJob::start()
{
    Akonadi::Search::PIM::CollectionQuery query;
    query.setNamespace(QStringList() << QStringLiteral("usertoplevel"));
    query.nameMatches(d->mSearchString);
    query.setLimit(200);
    Akonadi::Search::PIM::ResultIterator it = query.exec();
    Akonadi::Collection::List collections;
    while (it.next()) {
        collections << Akonadi::Collection(it.id());
    }
    qCDebug(LIBKDEPIM_LOG) << "Found persons " << collections.size();

    d->mCollectionSearchDone = false;
    d->mLdapSearchDone = false;
    if (collections.isEmpty()) {
        //We didn't find anything
        d->mCollectionSearchDone = true;
    }

    d->mLdapSearch.startSearch(QStringLiteral("*") + d->mSearchString);

    if (!collections.isEmpty()) {
        Akonadi::CollectionFetchJob *fetchJob = new Akonadi::CollectionFetchJob(collections, Akonadi::CollectionFetchJob::Base, this);
        fetchJob->fetchScope().setAncestorRetrieval(Akonadi::CollectionFetchScope::All);
        fetchJob->fetchScope().setListFilter(Akonadi::CollectionFetchScope::NoFilter);
        connect(fetchJob, &Akonadi::CollectionFetchJob::collectionsReceived, this, &PersonSearchJob::onCollectionsReceived);
        connect(fetchJob, &Akonadi::CollectionFetchJob::result, this, &PersonSearchJob::onCollectionsFetched);
    }

    //The IMAP resource should add a "Person" attribute to the collections in the person namespace,
    //the ldap query can then be used to update the name (entitydisplayattribute) for the person.
}

void PersonSearchJob::onLDAPSearchData(const KLDAP::LdapResultObject::List &list)
{
    QVector<Person> persons;
    Q_FOREACH (const KLDAP::LdapResultObject &item, list) {
        Person person;
        person.name = QString::fromUtf8(item.object.value(QStringLiteral("cn")));
        person.mail = QString::fromUtf8(item.object.value(QStringLiteral("mail")));

        const int depth = item.object.dn().depth();
        for (int i = 0; i < depth; ++i) {
            const QString rdnStr = item.object.dn().rdnString(i);
            if (rdnStr.startsWith(QStringLiteral("ou="), Qt::CaseInsensitive)) {
                person.ou = rdnStr.mid(3);
                break;
            }
        }
        const QStringList &parts = person.mail.split(QLatin1Char('@'));
        if (parts.count() == 2) {
            const QString &uid = parts.at(0);
            person.uid = uid;
            if (d->mMatches.contains(uid)) {
                const Person &p = d->mMatches.value(uid);
                if (p.mail != person.mail) {
                    if (p.rootCollection > -1) {
                        person.rootCollection = p.rootCollection;
                        person.updateDisplayName = p.updateDisplayName;
                        updatePersonCollection(person);
                        d->mMatches.insert(uid, person);
                    } else {
                        qCWarning(LIBKDEPIM_LOG) << "That should not happen: we found two times persons with the same uid (" << uid << "), but differnet name:" << p.name << "vs" << person.name;
                    }
                }
            } else {            //New person found
                d->mMatches.insert(uid, person);
                persons << person;
            }
        } else {
            qCWarning(LIBKDEPIM_LOG) << item.object.dn().toString() << ": invalid email address" << person.mail;
        }
    }
    if (!persons.isEmpty()) {
        Q_EMIT personsFound(persons);
    }
}

void PersonSearchJob::onLDAPSearchDone()
{
    d->mLdapSearchDone = true;
    if (d->mCollectionSearchDone) {
        emitResult();
    }
}

void PersonSearchJob::onCollectionsReceived(const Akonadi::Collection::List &list)
{
    QVector<Person> persons;
    Q_FOREACH (const Akonadi::Collection &col, list) {
        Person person;
        const QString &uid = col.name();
        const Akonadi::CollectionIdentificationAttribute *const attr = col.attribute<Akonadi::CollectionIdentificationAttribute>();
        const Akonadi::EntityDisplayAttribute *const displayname = col.attribute<Akonadi::EntityDisplayAttribute>();
        person.rootCollection = col.id();
        person.uid = uid;
        if (attr) {
            person.ou = QString::fromUtf8(attr->ou());
            person.mail = QString::fromUtf8(attr->mail());
            person.name = QString::fromUtf8(attr->identifier());
            if (!displayname || displayname->displayName().isEmpty() || displayname->displayName() == person.name) {
                person.updateDisplayName = true;
            }
        } else {
            person.name = col.displayName();
            if (!displayname || displayname->displayName().isEmpty()) {
                person.updateDisplayName = true;
            }
        }
        if (d->mMatches.contains(uid)) {
            Person p = d->mMatches.value(uid);
            if (p.rootCollection > -1) {
                //two collection with the same uid ?!
                qCWarning(LIBKDEPIM_LOG) << "Two collections match to same person" << p.rootCollection << person.rootCollection;
            } else if (p.mail != person.mail) {
                p.rootCollection = person.rootCollection;
                p.updateDisplayName = person.updateDisplayName;
                updatePersonCollection(p);
            } else {
                d->mMatches.insert(uid, person);
                Q_EMIT personUpdate(person);
            }
        } else {
            d->mMatches.insert(uid, person);
            persons << person;
        }
    }

    if (!persons.isEmpty()) {
        Q_EMIT personsFound(persons);
    }
}

void PersonSearchJob::updatePersonCollection(const Person &person)
{
    Akonadi::Collection c(person.rootCollection);
    Akonadi::CollectionIdentificationAttribute *identification = c.attribute<Akonadi::CollectionIdentificationAttribute>(Akonadi::Collection::AddIfMissing);

    if (person.updateDisplayName) {
        Akonadi::EntityDisplayAttribute *displayname  = c.attribute<Akonadi::EntityDisplayAttribute >(Akonadi::Collection::AddIfMissing);
        displayname->setDisplayName(person.name);
    }

    //identification->setIdentifier("Other Users/" + person.uid);
    identification->setIdentifier(person.name.toUtf8());
    identification->setName(person.name.toUtf8());
    identification->setCollectionNamespace("usertoplevel");
    identification->setMail(person.mail.toUtf8());
    identification->setOu(person.ou.toUtf8());

    Akonadi::CollectionModifyJob *job = new Akonadi::CollectionModifyJob(c, this);
    connect(job, &Akonadi::CollectionModifyJob::result, this, &PersonSearchJob::modifyResult);
}

void PersonSearchJob::onCollectionsFetched(KJob *job)
{
    if (job->error()) {
        qCWarning(LIBKDEPIM_LOG) << job->errorString();
    }
    d->mCollectionSearchDone = true;
    if (d->mLdapSearchDone) {
        emitResult();
    }
}

QList<Person> PersonSearchJob::matches() const
{
    return d->mMatches.values();
}

void PersonSearchJob::modifyResult(KJob *job)
{
    if (job->error()) {
        qCWarning(LIBKDEPIM_LOG) << job->errorString();
        return;
    }

    const Akonadi::CollectionModifyJob *modifyJob = static_cast<Akonadi::CollectionModifyJob *>(job);
    const Akonadi::Collection &col = modifyJob->collection();

    const Akonadi::CollectionIdentificationAttribute *const attr = col.attribute<Akonadi::CollectionIdentificationAttribute>();
    const Akonadi::EntityDisplayAttribute *const displayname = col.attribute<Akonadi::EntityDisplayAttribute>();
    const QString &uid = col.name();
    Person &person = d->mMatches[col.name()];
    person.rootCollection = col.id();
    person.uid = uid;
    if (attr) {
        person.ou = QString::fromUtf8(attr->ou());
        person.mail = QString::fromUtf8(attr->mail());
        person.name = QString::fromUtf8(attr->identifier());
        if (!displayname || displayname->displayName().isEmpty() || displayname->displayName() == person.name) {
            person.updateDisplayName = true;
        }
    }
    qCDebug(LIBKDEPIM_LOG) << "modified person to" << person.uid << person.name << person.rootCollection;

    d->mMatches.insert(person.uid, person);
    Q_EMIT personUpdate(person);
}
