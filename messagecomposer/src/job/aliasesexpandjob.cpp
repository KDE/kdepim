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

#include "aliasesexpandjob.h"
#include "distributionlistexpandjob.h"

#include <Akonadi/Contact/ContactSearchJob>
#include <Akonadi/Contact/ContactGroupExpandJob>
#include <KEmailAddress>

#include <MessageCore/StringUtil>

using namespace MessageComposer;

AliasesExpandJob::AliasesExpandJob(const QString &recipients, const QString &defaultDomain, QObject *parent)
    : KJob(parent),
      mRecipients(KEmailAddress::splitAddressList(recipients)),
      mDefaultDomain(defaultDomain),
      mDistributionListExpansionJobs(0),
      mNicknameExpansionJobs(0)
{
}

AliasesExpandJob::~AliasesExpandJob()
{
}

void AliasesExpandJob::start()
{
    // At first we try to expand the recipient to a distribution list
    // or nick name and save the results in a map for later lookup
    foreach (const QString &recipient, mRecipients) {

        // speedup: assume aliases and list names don't contain '@'
        if (recipient.isEmpty() || recipient.contains(QLatin1Char('@'))) {
            continue;
        }

        // check for distribution list
        DistributionListExpandJob *expandJob = new DistributionListExpandJob(recipient, this);
        expandJob->setProperty("recipient", recipient);
        connect(expandJob, &Akonadi::ContactGroupExpandJob::result, this, &AliasesExpandJob::slotDistributionListExpansionDone);
        mDistributionListExpansionJobs++;
        expandJob->start();

        // check for nick name
        Akonadi::ContactSearchJob *searchJob = new Akonadi::ContactSearchJob(this);
        searchJob->setProperty("recipient", recipient);
        searchJob->setQuery(Akonadi::ContactSearchJob::NickName, recipient.toLower());
        connect(searchJob, &Akonadi::ContactSearchJob::result, this, &AliasesExpandJob::slotNicknameExpansionDone);
        mNicknameExpansionJobs++;
        searchJob->start();
    }

    if (mDistributionListExpansionJobs == 0 && mNicknameExpansionJobs == 0) {
        emitResult();
    }
}

QString AliasesExpandJob::addresses() const
{
    return mEmailAddresses;
}

QStringList AliasesExpandJob::emptyDistributionLists() const
{
    return mEmptyDistributionLists;
}

void AliasesExpandJob::slotDistributionListExpansionDone(KJob *job)
{
    if (job->error()) {
        setError(job->error());
        setErrorText(job->errorText());
        emitResult();
        return;
    }

    const DistributionListExpandJob *expandJob = qobject_cast<DistributionListExpandJob *>(job);
    const QString recipient = expandJob->property("recipient").toString();

    DistributionListExpansionResult result;
    result.addresses = expandJob->addresses();
    result.isEmpty = expandJob->isEmpty();

    mDistListExpansionResults.insert(recipient, result);

    mDistributionListExpansionJobs--;
    if (mDistributionListExpansionJobs == 0 && mNicknameExpansionJobs == 0) {
        finishExpansion();
    }
}

void AliasesExpandJob::slotNicknameExpansionDone(KJob *job)
{
    if (job->error()) {
        setError(job->error());
        setErrorText(job->errorText());
        emitResult();
        return;
    }

    const Akonadi::ContactSearchJob *searchJob = qobject_cast<Akonadi::ContactSearchJob *>(job);
    const KContacts::Addressee::List contacts = searchJob->contacts();
    const QString recipient = searchJob->property("recipient").toString();

    foreach (const KContacts::Addressee &contact, contacts) {
        if (contact.nickName().toLower() == recipient.toLower()) {
            mNicknameExpansionResults.insert(recipient, contact.fullEmail());
            break;
        }
    }

    mNicknameExpansionJobs--;
    if (mDistributionListExpansionJobs == 0 && mNicknameExpansionJobs == 0) {
        finishExpansion();
    }
}

void AliasesExpandJob::finishExpansion()
{
    foreach (const QString &recipient, mRecipients) {
        if (recipient.isEmpty()) {
            continue;
        }
        if (!mEmailAddresses.isEmpty()) {
            mEmailAddresses += QLatin1String(", ");
        }

        const QString receiver = recipient.trimmed();

        // take prefetched expand distribution list results
        const DistributionListExpansionResult result = mDistListExpansionResults.value(recipient);

        if (result.isEmpty) {
            mEmailAddresses += receiver;
            mEmptyDistributionLists << receiver;
            continue;
        }

        if (!result.addresses.isEmpty()) {
            mEmailAddresses += result.addresses;
            continue;
        }

        // take prefetched expand nick name results
        const QString recipientValue = mNicknameExpansionResults.value(recipient);
        if (!recipientValue.isEmpty()) {
            mEmailAddresses += recipientValue;
            continue;
        }

        // check whether the address is missing the domain part
        QString displayName, addrSpec, comment;
        KEmailAddress::splitAddress(receiver, displayName, addrSpec, comment);
        if (!addrSpec.contains(QLatin1Char('@'))) {
            if (!mDefaultDomain.isEmpty())
                mEmailAddresses += KEmailAddress::normalizedAddress(displayName, addrSpec + QLatin1Char('@') +
                                   mDefaultDomain, comment);
            else {
                mEmailAddresses += MessageCore::StringUtil::guessEmailAddressFromLoginName(addrSpec);
            }
        } else {
            mEmailAddresses += receiver;
        }
    }

    emitResult();
}
