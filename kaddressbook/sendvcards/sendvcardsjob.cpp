/*
  This file is part of KAddressBook.

  Copyright (c) 2015 Laurent Montel <montel@kde.org>

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
*/

#include "sendvcardsjob.h"
#include <KABC/Addressee>
#include <KABC/ContactGroup>
#include <Akonadi/Item>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <QDebug>

using namespace KABSendVCards;

SendVcardsJob::SendVcardsJob(const Akonadi::Item::List &listItem, QObject *parent)
    : QObject(parent),
      mListItem(listItem),
      mFetchJobCount(0)
{

}

SendVcardsJob::~SendVcardsJob()
{

}

void SendVcardsJob::start()
{
    if (mListItem.isEmpty()) {
        qDebug()<<" No Item found";
        deleteLater();
        return;
    }

    Q_FOREACH (const Akonadi::Item &item, mListItem) {
        if (item.hasPayload<KABC::Addressee>()) {
            const KABC::Addressee contact = item.payload<KABC::Addressee>();
            //TODO
        } else if (item.hasPayload<KABC::ContactGroup>()) {
            const KABC::ContactGroup group = item.payload<KABC::ContactGroup>();
            unsigned int nbDataCount(group.dataCount());
            for(unsigned int i=0; i<nbDataCount; ++i) {
                const QString currentEmail(group.data(i).email());
                //TODO
            }
            const unsigned int nbContactReference(group.contactReferenceCount());
            for(unsigned int i=0; i<nbContactReference; ++i){
                KABC::ContactGroup::ContactReference reference = group.contactReference(i);

                Akonadi::Item item;
                if (reference.gid().isEmpty()) {
                    item.setId( reference.uid().toLongLong() );
                } else {
                    item.setGid( reference.gid() );
                }
                mItemToFetch << item;
            }
        }
    }

    if(mItemToFetch.isEmpty()) {
        finishJob();
    } else {
        fetchNextItem();
    }
}

void SendVcardsJob::fetchNextItem()
{
    if (mFetchJobCount < mItemToFetch.count()) {
        fetchItem(mItemToFetch.at(mFetchJobCount));
        ++mFetchJobCount;
    } else {
        finishJob();
    }
}

void SendVcardsJob::fetchItem(const Akonadi::Item &item)
{
    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( item, this );
    job->fetchScope().fetchFullPayload();

    connect( job, SIGNAL(result(KJob*)), SLOT(fetchJobFinished(KJob*)) );
}

void SendVcardsJob::fetchJobFinished(KJob *job)
{
    if ( job->error() ) {
        qDebug()<<" error during fetching "<<job->errorString();
        fetchNextItem();
        return;
    }

    Akonadi::ItemFetchJob *fetchJob = qobject_cast<Akonadi::ItemFetchJob*>( job );

    if ( fetchJob->items().count() != 1 ) {
        fetchNextItem();
        return;
    }

    const Akonadi::Item item = fetchJob->items().first();
    const KABC::Addressee contact = item.payload<KABC::Addressee>();
    //TODO
    fetchNextItem();
}

void SendVcardsJob::finishJob()
{
#if 0
    if (!mEmailAddresses.isEmpty()) {
        //emit sendMails(mEmailAddresses);
    }
#endif
    deleteLater();
}

