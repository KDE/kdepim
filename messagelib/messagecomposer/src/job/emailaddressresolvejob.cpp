/*
 * This file is part of KMail.
 *
 * Copyright (c) 2010 KDAB
 *
 * Authors: Tobias Koenig <tokoe@kde.org>
 *         Leo Franchi    <lfranchi@kde.org>
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

#include "job/emailaddressresolvejob.h"

#include "aliasesexpandjob.h"
#include "settings/messagecomposersettings.h"

#include "MessageComposer/Composer"
#include "MessageComposer/InfoPart"

#include <KEmailAddress>

using namespace MessageComposer;

class MessageComposer::EmailAddressResolveJobPrivate
{
public:
    EmailAddressResolveJobPrivate()
        : mJobCount(0)
    {

    }

    int mJobCount;
    QVariantMap mResultMap;
    QString mFrom;
    QStringList mTo;
    QStringList mCc;
    QStringList mBcc;
    QString mDefaultDomainName;
};

EmailAddressResolveJob::EmailAddressResolveJob(QObject *parent)
    : KJob(parent),
      d(new MessageComposer::EmailAddressResolveJobPrivate)
{
}

EmailAddressResolveJob::~EmailAddressResolveJob()
{
    delete d;
}

static inline bool containsAliases(const QString &address)
{
    // an valid email is defined as foo@foo.extension
    return !(address.contains(QLatin1Char('@')) && address.contains(QLatin1Char('.')));
}

static bool containsAliases(const QStringList &addresses)
{
    foreach (const QString &address, addresses) {
        if (containsAliases(address)) {
            return true;
        }
    }

    return false;
}

void EmailAddressResolveJob::setDefaultDomainName(const QString &domainName)
{
    d->mDefaultDomainName = domainName;
}

void EmailAddressResolveJob::start()
{
    QVector<AliasesExpandJob *> jobs;

    if (containsAliases(d->mFrom)) {
        AliasesExpandJob *job = new AliasesExpandJob(d->mFrom, d->mDefaultDomainName, this);
        job->setProperty("id", QStringLiteral("infoPartFrom"));
        connect(job, &AliasesExpandJob::result, this, &EmailAddressResolveJob::slotAliasExpansionDone);
        jobs << job;
    }
    if (containsAliases(d->mTo)) {
        AliasesExpandJob *job = new AliasesExpandJob(d->mTo.join(QStringLiteral(", ")), d->mDefaultDomainName, this);
        job->setProperty("id", QStringLiteral("infoPartTo"));
        connect(job, &AliasesExpandJob::result, this, &EmailAddressResolveJob::slotAliasExpansionDone);
        jobs << job;
    }

    if (containsAliases(d->mCc)) {
        AliasesExpandJob *job = new AliasesExpandJob(d->mCc.join(QStringLiteral(", ")), d->mDefaultDomainName, this);
        job->setProperty("id", QStringLiteral("infoPartCc"));
        connect(job, &AliasesExpandJob::result, this, &EmailAddressResolveJob::slotAliasExpansionDone);
        jobs << job;
    }

    if (containsAliases(d->mBcc)) {
        AliasesExpandJob *job = new AliasesExpandJob(d->mBcc.join(QStringLiteral(", ")), d->mDefaultDomainName, this);
        job->setProperty("id", QStringLiteral("infoPartBcc"));
        connect(job, &AliasesExpandJob::result, this, &EmailAddressResolveJob::slotAliasExpansionDone);
        jobs << job;
    }

    d->mJobCount = jobs.count();

    if (d->mJobCount == 0) {
        emitResult();
    } else {
        foreach (AliasesExpandJob *job, jobs) {
            job->start();
        }
    }
}

void EmailAddressResolveJob::slotAliasExpansionDone(KJob *job)
{
    if (job->error()) {
        setError(job->error());
        setErrorText(job->errorText());
        emitResult();
        return;
    }

    const AliasesExpandJob *expandJob = qobject_cast<AliasesExpandJob *>(job);
    d->mResultMap.insert(expandJob->property("id").toString(), expandJob->addresses());

    d->mJobCount--;
    if (d->mJobCount == 0) {
        emitResult();
    }
}

void EmailAddressResolveJob::setFrom(const QString &from)
{
    d->mFrom = from;
    d->mResultMap.insert(QStringLiteral("infoPartFrom"), from);
}

void EmailAddressResolveJob::setTo(const QStringList &to)
{
    d->mTo = to;
    d->mResultMap.insert(QStringLiteral("infoPartTo"), to.join(QStringLiteral(", ")));
}

void EmailAddressResolveJob::setCc(const QStringList &cc)
{
    d->mCc = cc;
    d->mResultMap.insert(QStringLiteral("infoPartCc"), cc.join(QStringLiteral(", ")));
}

void EmailAddressResolveJob::setBcc(const QStringList &bcc)
{
    d->mBcc = bcc;
    d->mResultMap.insert(QStringLiteral("infoPartBcc"), bcc.join(QStringLiteral(", ")));
}

QString EmailAddressResolveJob::expandedFrom() const
{
    return d->mResultMap.value(QStringLiteral("infoPartFrom")).toString();
}

QStringList EmailAddressResolveJob::expandedTo() const
{
    return KEmailAddress::splitAddressList(d->mResultMap.value(QStringLiteral("infoPartTo")).toString());
}

QStringList EmailAddressResolveJob::expandedCc() const
{
    return KEmailAddress::splitAddressList(d->mResultMap.value(QStringLiteral("infoPartCc")).toString());

}

QStringList EmailAddressResolveJob::expandedBcc() const
{
    return KEmailAddress::splitAddressList(d->mResultMap.value(QStringLiteral("infoPartBcc")).toString());
}

