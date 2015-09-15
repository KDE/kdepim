/*
 * This file is part of KMail.
 *
 * Copyright (c) 2010 KDAB
 *
 * Author: Tobias Koenig <tokoe@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "distributionlistexpandjob.h"

#include <akonadi/contact/contactgroupexpandjob.h>
#include <akonadi/contact/contactgroupsearchjob.h>

using namespace MessageComposer;

DistributionListExpandJob::DistributionListExpandJob(const QString &name, QObject *parent)
    : KJob(parent), mListName(name), mIsEmpty(false)
{
}

DistributionListExpandJob::~DistributionListExpandJob()
{
}

void DistributionListExpandJob::start()
{
    Akonadi::ContactGroupSearchJob *job = new Akonadi::ContactGroupSearchJob(this);
    job->setQuery(Akonadi::ContactGroupSearchJob::Name, mListName);
    connect(job, &KJob::result, this, &DistributionListExpandJob::slotSearchDone);
}

QString DistributionListExpandJob::addresses() const
{
    return mEmailAddresses.join(QStringLiteral(", "));
}

bool DistributionListExpandJob::isEmpty() const
{
    return mIsEmpty;
}

void DistributionListExpandJob::slotSearchDone(KJob *job)
{
    if (job->error()) {
        setError(job->error());
        setErrorText(job->errorText());
        emitResult();
        return;
    }

    const Akonadi::ContactGroupSearchJob *searchJob = qobject_cast<Akonadi::ContactGroupSearchJob *>(job);

    const KContacts::ContactGroup::List groups = searchJob->contactGroups();
    if (groups.isEmpty()) {
        emitResult();
        return;
    }

    Akonadi::ContactGroupExpandJob *expandJob = new Akonadi::ContactGroupExpandJob(groups.first());
    connect(expandJob, &KJob::result, this, &DistributionListExpandJob::slotExpansionDone);
    expandJob->start();
}

void DistributionListExpandJob::slotExpansionDone(KJob *job)
{
    if (job->error()) {
        setError(job->error());
        setErrorText(job->errorText());
        emitResult();
        return;
    }

    const Akonadi::ContactGroupExpandJob *expandJob = qobject_cast<Akonadi::ContactGroupExpandJob *>(job);

    const KContacts::Addressee::List contacts = expandJob->contacts();

    foreach (const KContacts::Addressee &contact, contacts) {
        mEmailAddresses << contact.fullEmail();
    }

    mIsEmpty = mEmailAddresses.isEmpty();

    emitResult();
}

