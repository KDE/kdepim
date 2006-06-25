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

#include "dummykonadiconversation.h"

int DummyKonadiConversation::count() const
{
  return messages.count();
}

QString DummyKonadiConversation::conversationTitle() const
{
  return title;
}

DummyKonadiMessage DummyKonadiConversation::message(int messageId) const
{
  if (messageId < 0 || messageId >= messages.count())
    return DummyKonadiMessage(true);
  return messages.at(messageId);
}

void DummyKonadiConversation::addMessage(DummyKonadiMessage &message)
{
  messages << message;
//  emit messageAdded();
}

QString DummyKonadiConversation::author(int messageId) const
{
  DummyKonadiMessage tmp = message(messageId);
  if (! tmp.isNull())
    return tmp.author();
  return 0;
}

QString DummyKonadiConversation::content(int messageId) const
{
  DummyKonadiMessage tmp = message(messageId);
  if (! tmp.isNull())
    return tmp.content();
  return 0;
}

QDateTime DummyKonadiConversation::arrivalTime(int messageId) const
{
  DummyKonadiMessage tmp = message(messageId);
  return tmp.arrivalTime();
}

QString DummyKonadiConversation::arrivalTimeInText(int messageId) const
{
  DummyKonadiMessage tmp = message(messageId);
  return tmp.arrivalTimeInText();
}

QDateTime DummyKonadiConversation::sendTime(int messageId) const
{
  DummyKonadiMessage tmp = message(messageId);
  return tmp.sendTime();
}

QDateTime DummyKonadiConversation::arrivalTime() const
{
  return messages.at(count()-1).arrivalTime();
}

QString DummyKonadiConversation::arrivalTimeInText() const
{
  return messages.at(count()-1).arrivalTimeInText();
}

QDateTime DummyKonadiConversation::sendTime() const
{
  int max = count();
  QDateTime tmp;
  QDateTime *oldest = new QDateTime(QDate(0, 0, 0), QTime(0, 0));
  for (int count = 0; count < max; ++count) {
    tmp = messages.at(count).sendTime();
    if (tmp > *oldest)
      oldest = &tmp;
  }
  return *oldest;
}
