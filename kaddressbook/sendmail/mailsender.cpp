/*
  This file is part of KAddressBook.

  Copyright (c) 2014 Laurent Montel <montel@kde.org>
  based on code from Copyright (c) 2014 Clément Vannier <clement.vannier@free.fr>

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

#include "mailsender.h"
#include "utils.h"
#include "kpimutils/email.h"

#include <KJob>

#include <Akonadi/Item>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>

#include <KABC/Addressee>
#include <KABC/ContactGroup>
#include <KToolInvocation>

#include <QDebug>
#include <QItemSelectionModel>
using namespace KABMailSender;

MailSender::MailSender(const Akonadi::Item::List &listItem, QObject *parent)
    : QObject(parent),
      mListItem(listItem),
      mFetchJobCount(0)
{
}

MailSender::~MailSender()
{

}

void MailSender::start()
{
    Q_FOREACH (const Akonadi::Item &item, mListItem) {
        if (item.hasPayload<KABC::Addressee>()) {
            const KABC::Addressee contact = item.payload<KABC::Addressee>();
            const QString preferredEmail = contact.preferredEmail();
            if( !preferredEmail.isEmpty() && mEmailAddresses.contains(preferredEmail) ){
                mEmailAddresses <<  KPIMUtils::normalizedAddress(contact.formattedName(), contact.preferredEmail());
            }
        } else if (item.hasPayload<KABC::ContactGroup>()) {
            const KABC::ContactGroup group = item.payload<KABC::ContactGroup>();
            for(unsigned int i=0; i<group.dataCount(); ++i){
                const QString email = KPIMUtils::normalizedAddress(group.data(i).name(), group.data(i).email());
                if (!email.isEmpty() && !mEmailAddresses.contains(email)) {
                    mEmailAddresses << email;
                }
            }
            for(unsigned int i=0; i<group.contactReferenceCount(); ++i){
                KABC::ContactGroup::ContactReference reference = group.contactReference(i);

                Akonadi::Item item;
                if ( !reference.gid().isEmpty() ) {
                    item.setGid( reference.gid() );
                } else {
                    item.setId( reference.uid().toLongLong() );
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

void MailSender::fetchNextItem()
{
    if (mFetchJobCount<mItemToFetch.count()) {
        fetchItem(mItemToFetch.at(mFetchJobCount));
        ++mFetchJobCount;
    } else {
        finishJob();
    }
}

void MailSender::fetchItem(const Akonadi::Item &item)
{
    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( item, this );
    job->fetchScope().fetchFullPayload();

    connect( job, SIGNAL(result(KJob*)), SLOT(fetchJobFinished(KJob*)) );
}

void MailSender::fetchJobFinished(KJob *job)
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

    if( !contact.preferredEmail().isEmpty()){
        mEmailAddresses <<  KPIMUtils::normalizedAddress(contact.formattedName(), contact.preferredEmail());
    }
    fetchNextItem();
}

void MailSender::finishJob()
{
#if 0
    if (!mEmailAddresses.isEmpty()) {
        KUrl url;
        url.setProtocol( QLatin1String( "mailto" ) );
        url.setPath( QStringList(mEmailAddresses.toList()).join(QLatin1String(";")) );
        KToolInvocation::invokeMailer( url );
    }
#endif
    if (!mEmailAddresses.isEmpty()) {
        emit sendMails(mEmailAddresses);
    }
    deleteLater();
}
