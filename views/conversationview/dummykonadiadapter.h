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

#ifndef DUMMYKONADIADAPTER_H
#define DUMMYKONADIADAPTER_H

#include <QString>
#include <QStringList>
#include <QDateTime>

#include "dummykonadiconversation.h"

class DummyKonadiAdapter
{
public:
  DummyKonadiAdapter();
  ~DummyKonadiAdapter();
  int conversationCount() const;
  QString conversationTitle(int conversationId) const;
  QDateTime conversationSendTime(int conversationId) const;
  QDateTime conversationArrivalTime(int conversationId) const;
  QString conversationArrivalTimeInText(int conversationId) const;
  DummyKonadiConversation conversation(int conversationId) const;
  QString messageContent(int conversationId, int messageId) const;
  QString messageAuthor(int conversationId, int messageId) const;
  QDateTime messageSendTime(int conversationId, int messageId) const;
  QDateTime messageArrivalTime(int conversationId, int messageId) const;
  QString messageArrivalTimeInText(int conversationId, int messageId) const;
  int messageCount(int conversationId) const;
  void sort(bool descending = true);

private:
  bool tryConversationId(int conversationId) const;
  QList<DummyKonadiConversation*> conversations;
};

#endif
