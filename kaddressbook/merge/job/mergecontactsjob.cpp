/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include "mergecontactsjob.h"
#include "merge/mergecontacts.h"
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemDeleteJob>

#include <KABC/Address>
#include <KABC/Addressee>

#include <QDebug>

using namespace KABMergeContacts;

MergeContactsJob::MergeContactsJob(QObject *parent)
    : QObject(parent)
{
}

MergeContactsJob::~MergeContactsJob()
{

}

bool MergeContactsJob::canStart()
{
    if (!mCollection.isValid()) {
        qDebug()<<" mCollection is not valid !";
        return false;
    }
    if (mListItem.isEmpty()) {
        qDebug()<<" list item is empty !";
        return false;
    }
    return true;

}

void MergeContactsJob::start()
{
    if (!canStart()) {
        Q_EMIT finished(mCreatedContact);
        deleteLater();
        return;
    }
    generateMergedContact();
}

void MergeContactsJob::generateMergedContact()
{
    MergeContacts mergeContact(mListItem);
    KABC::Addressee newContact = mergeContact.mergedContact();
    if (newContact.isEmpty()) {
        Q_EMIT finished(mCreatedContact);
        deleteLater();
        return;
    }
    createMergedContact(newContact);
}

void MergeContactsJob::setListItem(const Akonadi::Item::List &lstItem)
{
    mListItem = lstItem;
}

void MergeContactsJob::setDestination(const Akonadi::Collection &collection)
{
    mCollection = collection;
}

void MergeContactsJob::createMergedContact(const KABC::Addressee &addressee)
{
    Akonadi::Item item;
    item.setMimeType( KABC::Addressee::mimeType() );
    item.setPayload<KABC::Addressee>( addressee );

    Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob(item, mCollection, this);
    connect(job, SIGNAL(result(KJob*)), SLOT(slotCreateMergedContactFinished(KJob*)) );
}

void MergeContactsJob::slotCreateMergedContactFinished(KJob *job)
{
    if (job->error()) {
        qDebug() << job->errorString();
        Q_EMIT finished(mCreatedContact);
        deleteLater();
        return;
    }
    Akonadi::ItemCreateJob *createdJob = qobject_cast<Akonadi::ItemCreateJob *>(job);
    mCreatedContact = createdJob->item();

    Akonadi::ItemDeleteJob *deleteJob = new Akonadi::ItemDeleteJob(mListItem, this);
    connect(deleteJob, SIGNAL(result(KJob*)), SLOT(slotDeleteContactsFinished(KJob*)) );
}

void MergeContactsJob::slotDeleteContactsFinished(KJob *job)
{
    if (job->error()) {
        qDebug() << job->errorString();
    }
    Q_EMIT finished(mCreatedContact);
    deleteLater();
}
