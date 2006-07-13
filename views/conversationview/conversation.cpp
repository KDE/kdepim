/*
 * copyright (c) Aron Bostrom <Aron.Bostrom at gmail.com>, 2006 
 *
 * this library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <klocale.h>

#include "conversation.h"

Conversation::Conversation(QStringList *manyMe, Message *message, QObject *parent) : QObject(parent), listOfMe(manyMe) 
{
  title = message->subject();
  messages << message;
}

Conversation::~Conversation()
{
  Message *tmp;
  foreach (tmp, messages) {
    delete tmp;
  }
}

int Conversation::count() const
{
  return messages.count();
}

QString Conversation::conversationTitle() const
{
  return title;
}

Message* Conversation::message(int messageId) const
{
  if (messageId < 0 || messageId >= messages.count())
    return new Message(true);
  return messages.at(messageId);
}

void Conversation::addMessage(Message *message)
{
  messages << message;
}

QString Conversation::author(int messageId) const
{
  return message(messageId)->author();
}

QString Conversation::content(int messageId) const
{
  return message(messageId)->content();
}

QDateTime Conversation::arrivalTime(int messageId) const
{
  return message(messageId)->arrivalTime();
}

QString Conversation::arrivalTimeInText(int messageId) const
{
  return message(messageId)->arrivalTimeInText();
}

QDateTime Conversation::sendTime(int messageId) const
{
  return message(messageId)->sendTime();
}

QDateTime Conversation::arrivalTime() const
{
  return messages.last()->arrivalTime();
}

QString Conversation::arrivalTimeInText() const
{
  return messages.last()->arrivalTimeInText();
}

QDateTime Conversation::sendTime() const
{
  int max = count();
  QDateTime tmp;
  QDateTime *oldest = new QDateTime(QDate(0, 0, 0), QTime(0, 0));
  for (int count = 0; count < max; ++count) {
    tmp = messages.at(count)->sendTime();
    if (tmp > *oldest)
      oldest = &tmp;
  }
  return *oldest;
}

bool Conversation::operator!=(Conversation &compare) const
{
  return arrivalTime() != compare.arrivalTime();
}

bool Conversation::operator<(Conversation &compare) const
{
  return arrivalTime() < compare.arrivalTime();
}

bool Conversation::operator<=(Conversation &compare) const
{
  return arrivalTime() <= compare.arrivalTime();
}

bool Conversation::operator==(Conversation &compare) const
{
  return arrivalTime() == compare.arrivalTime();
}

bool Conversation::operator>=(Conversation &compare) const
{
  return arrivalTime() >= compare.arrivalTime();
}

bool Conversation::operator>(Conversation &compare) const
{
  return arrivalTime() > compare.arrivalTime();
}

/** 
 * This function generates a list of authors contributing to this conversation. No author is included twice.
 * @return a list of authors contributing to this conversation
 * TODO: feed it with a length to determine how much can be displayed. 
 * TODO: only display two or three authors: first, first unread and last unread
 */
QString Conversation::authors() const
{
  QString text = author(0);
  QString me;
  foreach (me, *listOfMe) {
    text.replace(QRegExp(me), i18n("me"));
  }
  int max = count();
  for (int i = 1; i < max; ++i) {
    QString tmpAuthor = author(i);
    foreach (me, *listOfMe) {
      tmpAuthor.replace(QRegExp(me), i18n("me"));
    }
    if (!text.contains(tmpAuthor)) {
      text.append(", ");
      text.append(tmpAuthor);
    }
  }
  return text;
}

bool Conversation::isUnread() const
{
	Message *tmp;
	foreach (tmp, messages) {
		if (!tmp->isRead())
			return true;
	}
	return false;
}

bool Conversation::numberUnread() const
{
	int count = 0;
	Message *tmp;
	foreach (tmp, messages) {
		if (tmp->isRead())
			++count;
	}
	return false;
}

void Conversation::markAs(bool read)
{
	Message *tmp;
	foreach (tmp, messages) {
		tmp->markAs(read);
	}
}

bool Conversation::isRelated(const Message *message) const
{
  Message *tmp;
  foreach (tmp, messages) {
    if (tmp->isRelated(message))
      return true;
  }
  return false;
}


#include "conversation.moc"
