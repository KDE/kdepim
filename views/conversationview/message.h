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
  Message(bool null = false) : nullContent(null) { readStatus = false; }
  QString author() const;
  QString content() const;
  QString subject() const { return m_subject; }
  QDateTime sendTime() const { return send; }
  QDateTime arrivalTime() const { return arrival; }
  unsigned long id() const { return m_id; }
  unsigned long parentId() const { return m_pid; }
  QString arrivalTimeInText() const;
  void setAuthor(QString newAuthor);
  void setContent(QString newContent);
  void setArrivalTime(const QDateTime &dateTime) { arrival = dateTime; }
  void setSendTime(const QDateTime &dateTime) { send = dateTime; }
  void setSubject(const QString &subject) { m_subject = subject; }
  void setId(unsigned long id) { m_id = id; }
  void setParentId(unsigned long parentId) { m_pid = parentId; }
  bool isNull() const;
  bool isRead() const;
  void markAs(bool read);
  bool operator!=(Message &compare) const;
  bool operator<(Message &compare) const;
  bool operator<=(Message &compare) const;
  bool operator==(Message &compare) const;
  bool operator>=(Message &compare) const;
  bool operator>(Message &compare) const;

private:
  QString conversationAuthor, conversationContent, m_subject;
  QDateTime arrival, send;
  bool nullContent, readStatus;
  unsigned long m_id, m_pid;
};

#endif
