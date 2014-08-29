/*
 Copyright 2014  Michael Bohlender michael.bohlender@kdemail.net

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation; either version 2 of
 the License or (at your option) version 3 or any later version
 accepted by the membership of KDE e.V. (or its successor approved
 by the membership of KDE e.V.), which shall act as a proxy
 defined in Section 14 of version 3 of the license.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "maillist.h"

#include <AkonadiCore/ItemFetchJob>
#include <AkonadiCore/ItemFetchScope>
#include <AkonadiCore/Collection>
#include <Akonadi/KMime/MessageParts>

MailList::MailList( QObject *parent ) : QObject( parent ), m_model( new MailListModel )
{
}

MailListModel* MailList::model() const
{
    return m_model.data();
}

void MailList::loadCollection( const QUrl &url )
{
    m_model->clearMails();

    Akonadi::Collection collection = Akonadi::Collection::fromUrl( url );

    Akonadi::ItemFetchJob *fetchJob = new Akonadi::ItemFetchJob( collection );
    fetchJob->fetchScope().fetchPayloadPart(Akonadi::MessagePart::Header);

    connect( fetchJob, SIGNAL(itemsReceived(Akonadi::Item::List)), this, SLOT(slotItemsReceived(Akonadi::Item::List)));
}

void MailList::slotItemsReceived( const Akonadi::Item::List& itemList )
{
    m_model->addMails( itemList );
}
