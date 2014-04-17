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
        deleteLater();
        qDebug()<<" mCollection is not valid !";
        return;
    }
    if (mListItem.isEmpty()) {
        deleteLater();
        qDebug()<<" list item is empty !";
        return;
    }
    generateMergedContact();
}

void MergeContactsJob::generateMergedContact()
{

}

void MergeContactsJob::setListItem(const Akonadi::Item::List &lstItem)
{
    mListItem = lstItem;
}

void MergeContactsJob::setDestination(const Akonadi::Collection &collection)
{
    mCollection = collection;
}

void MergeContactsJob::createMergedContact(const KABC::Address &address)
{
    Akonadi::Item item;
    item.setMimeType( KABC::Addressee::mimeType() );
    //FIXME
    //item.setPayload<KABC::Address>(address);
    Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob(item, mCollection, this);
    connect(job, SIGNAL(result(KJob*)), SLOT(slotCreateMergedContactFinished(KJob*)) );
}

void MergeContactsJob::slotCreateMergedContactFinished(KJob *job)
{
    //TODO
    //TODO delete old contact
    Akonadi::ItemDeleteJob *deleteJob = new Akonadi::ItemDeleteJob(mListItem, this);
    connect(deleteJob, SIGNAL(result(KJob*)), SLOT(slotDeleteContactsFinished(KJob*)) );
}

void MergeContactsJob::slotDeleteContactsFinished(KJob *job)
{
    //TODO
    Q_EMIT finished();
    deleteLater();
}
