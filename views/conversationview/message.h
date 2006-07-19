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

#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>
#include <QObject>
#include <QDateTime>

class Message
{
public:
  Message(bool null = false);
  QString author() const;
  QString content() const;
  QString subject() const;
  QString fancySubject() const;
  QDateTime sendTime() const;
  QDateTime arrivalTime() const;
  unsigned long id() const;
  unsigned long parentId() const;
  QString arrivalTimeInText() const;
  void setAuthor(const QString& newAuthor);
  void setContent(const QString& newContent);
  void setArrivalTime(const QDateTime &dateTime);
  void setSendTime(const QDateTime &dateTime);
  void setSubject(const QString &subject);
  void setId(unsigned long id);
  void setParentId(unsigned long parentId);
  bool isNull() const;
  bool isRead() const;

  /** 
  * Marks this message as read or unread. 
  * @param read true marks message as read, false marks message as unread
  */
  void markAs(bool read);

  /**
  * @return true if this message is related (parent, child or sibling) to message.
  */
  bool isRelated(const Message *message) const;

  /**
  * @return true if this two messages doesn't have the same arrival date
  */
  bool operator!=(Message &compare) const;
  bool operator<(Message &compare) const;
  bool operator<=(Message &compare) const;
  bool operator==(Message &compare) const;
  bool operator>=(Message &compare) const;
  bool operator>(Message &compare) const;

private:
  QString fancify(const QString &subject) const;

  QString m_author, m_content, m_subject, m_fancySubject;
  QDateTime m_arrival, m_send;
  bool m_nullContent, m_readStatus;
  unsigned long m_id, m_pid;
};

#endif
