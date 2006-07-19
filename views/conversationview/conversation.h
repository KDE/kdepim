/*
 * conversation.h
 *
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

#ifndef CONVERSATION_H
#define CONVERSATION_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QDateTime>

#include "message.h"

class Conversation : QObject
{
  Q_OBJECT
public:
  Conversation(QStringList *manyMe, Message *message, QObject *parent = 0);
  ~Conversation();
  int count() const;
  QString subject() const;
  QDateTime arrivalTime() const;
  QString arrivalTimeInText() const;
  QDateTime sendTime() const;
  Message* message(int messageId) const;
  void addMessage(Message *message);
  QString author(int messageId) const;
  QString authors() const;
  QString content(int messageId) const;
  QDateTime arrivalTime(int messageId) const;
  QString arrivalTimeInText(int messageId) const;
  QDateTime sendTime(int messageId) const;

  /**
  * Tells wether this conversation has any unread messages.
  */
  bool isUnread() const;

  /**
  * Returns number of unread messages in this conversation.
  */
  bool numberUnread() const;

  /**
  * Mark this conversation, and all messages in it, as read or unread
  * @param read, true for read, false for unread
  */
  void markAs(bool read);

  /**
  * @return true if this Message is a part of this Conversation
  */
  bool isRelated(const Message *message) const;

  bool operator!=(Conversation &compare) const;
  bool operator<(Conversation &compare) const;
  bool operator<=(Conversation &compare) const;
  bool operator==(Conversation &compare) const;
  bool operator>=(Conversation &compare) const;
  bool operator>(Conversation &compare) const;
  QString snippet() const { return ""; } // not implemented yet

//public signals:
//  void messageAdded();

private:
  QString m_subject;
  QList<Message*> m_messages;
  QStringList *m_listOfMe;
};

#endif
