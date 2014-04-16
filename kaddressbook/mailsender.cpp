/*
  This file is part of KAddressBook.

  Copyright (c) 2007 Clément Vannier <clement.vannier@free.fr>

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
#include <QProcess>
#include <QStringList>

MailSender::MailSender(QItemSelectionModel *selection, QObject *parent)
    : QObject(parent), mFetchJobCount(0)
{
    foreach (Akonadi::Item item, Utils::collectSelectedContactsItem(selection)) {
        const KABC::Addressee contact = item.payload<KABC::Addressee>();
        if( ! contact.preferredEmail().isEmpty()){
            mEmailAddresses <<  KPIMUtils::normalizedAddress(contact.formattedName(), contact.preferredEmail());
        }
    }

    foreach (Akonadi::Item item, Utils::collectSelectedGroupItem(selection)) {
        const KABC::ContactGroup group = item.payload<KABC::ContactGroup>();

        for(unsigned int i=0; i<group.dataCount(); ++i){
            mEmailAddresses << KPIMUtils::normalizedAddress(group.data(i).name(), group.data(i).email());
        }

        for(unsigned int i=0; i<group.contactReferenceCount(); ++i){
            KABC::ContactGroup::ContactReference reference = group.contactReference(i);

            Akonadi::Item item;
            if ( !reference.gid().isEmpty() ) {
              item.setGid( reference.gid() );
            } else {
              item.setId( reference.uid().toLongLong() );
            }
            Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( item, this );
            job->fetchScope().fetchFullPayload();

            connect( job, SIGNAL(result(KJob*)), SLOT(fetchJobFinished(KJob*)) );

            ++mFetchJobCount;
        }
    }

    if(mFetchJobCount == 0) {
        finishJob();
    }
}

MailSender::~MailSender()
{

}


void MailSender::fetchJobFinished(KJob *job)
{
    if ( job->error() ) {
      --mFetchJobCount;
      return;
    }

    Akonadi::ItemFetchJob *fetchJob = qobject_cast<Akonadi::ItemFetchJob*>( job );

    if ( fetchJob->items().count() != 1 ) {
      --mFetchJobCount;
      return;
    }

    const Akonadi::Item item = fetchJob->items().first();
    const KABC::Addressee contact = item.payload<KABC::Addressee>();

    if( ! contact.preferredEmail().isEmpty()){
        mEmailAddresses <<  KPIMUtils::normalizedAddress(contact.formattedName(), contact.preferredEmail());
    }

    --mFetchJobCount;

    if(mFetchJobCount == 0) {
        finishJob();
    }
}

void MailSender::finishJob()
{
    if(mEmailAddresses.isEmpty()) {
        return;
    }

    KUrl url;
        url.setProtocol( QLatin1String( "mailto" ) );
        url.setPath( QStringList(mEmailAddresses.toList()).join(QLatin1String(";")) );
        KToolInvocation::invokeMailer( url );
}
