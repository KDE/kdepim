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

#include <message.h>

#include <AkonadiCore/ItemFetchScope>
#include <KMime/Message>

#include <KUrl>

Message::Message(QObject *parent) : QObject(parent), m_error(new Error())
{

}

Message::~Message()
{

}

QString Message::subject() const
{
    return m_subject;
}

QString Message::from() const
{
    return m_from;
}

QString Message::textContent() const
{
    return m_textContent;
}

Error *Message::error() const
{
    return m_error.data();
}

void Message::loadMessage(const QString &id)
{
    m_akonadiId = id;
    Akonadi::ItemFetchJob *fetchJob = new Akonadi::ItemFetchJob(Akonadi::Item::fromUrl(KUrl(m_akonadiId)));
    fetchJob->fetchScope().fetchFullPayload();

    connect(fetchJob, SIGNAL(itemsReceived(Akonadi::Item::List)), this, SLOT(slotItemReceived(Akonadi::Item::List)));
}

void Message::slotItemReceived(const Akonadi::Item::List &itemList)
{
    if (itemList.empty()) {
        //TODO handle Message not found?
        return;
    }

    Akonadi::Item item = itemList.first();

    KMime::Message msg;
    msg.setContent(item.payloadData());
    msg.setFrozen(true);
    msg.parse();

    m_subject = msg.subject()->asUnicodeString();
    m_from = msg.from()->asUnicodeString();
    m_textContent = msg.textContent()->decodedText(true,true);
    emit messageChanged();
}


