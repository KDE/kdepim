/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

void MergeContactsJob::start()
{
    if (!mCollection.isValid()) {
        qDebug()<<" mCollection is not valid !";
        Q_EMIT finished(mCreatedContact);
        deleteLater();
        return;
    }
    if (mListItem.isEmpty()) {
        qDebug()<<" list item is empty !";
        Q_EMIT finished(mCreatedContact);
        deleteLater();
        return;
    }
    generateMergedContact();
}

void MergeContactsJob::generateMergedContact()
{
    KABC::Addressee newContact;
    bool firstAddress = true;
    Q_FOREACH (const Akonadi::Item &item, mListItem) {
        KABC::Addressee address = item.payload<KABC::Addressee>();
        if (firstAddress) {
            firstAddress = false;
            newContact.setName(address.name());
            newContact.setFamilyName(address.familyName());
            newContact.setFormattedName(address.formattedName());
        }
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
    //TODO
    Q_EMIT finished(mCreatedContact);
    deleteLater();
}
