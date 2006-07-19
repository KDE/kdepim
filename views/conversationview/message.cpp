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

#include <QRegExp>
#include <QDate>
#include <QTime>
#include <QDateTime>

#include "message.h"

Message::Message(bool null) : m_nullContent(null) { m_readStatus = false; }
QString Message::subject() const { return m_subject; }
QString Message::fancySubject() const { return m_fancySubject; }
QDateTime Message::sendTime() const { return m_send; }
QDateTime Message::arrivalTime() const { return m_arrival; }
unsigned long Message::id() const { return m_id; }
unsigned long Message::parentId() const { return m_pid; }
void Message::setArrivalTime(const QDateTime &dateTime) { m_arrival = dateTime; }
void Message::setSendTime(const QDateTime &dateTime) { m_send = dateTime; }
void Message::setSubject(const QString &subject) { m_subject = subject; m_fancySubject = fancify(subject); }
void Message::setId(unsigned long id) { m_id = id; }
void Message::setParentId(unsigned long parentId) { m_pid = parentId; }

QString Message::author() const
{
  if (m_nullContent) return "";
  return m_author;
}

QString Message::content() const
{
  if (m_nullContent) return "";
  return m_content;
}

void Message::setAuthor(const QString& author)
{
  m_author = author;
}

void Message::setContent(const QString& content)
{
  m_content = content;
  m_content.replace(QRegExp("\n"), "<br>");
}

bool Message::isNull() const
{
  return m_nullContent;
}

QString Message::arrivalTimeInText() const
{
  QString ctime = m_arrival.date().toString();
  ctime.append(" ");
  ctime.append(m_arrival.time().toString());
  return ctime;
}

bool Message::operator!=(Message &compare) const
{
  return m_arrival != compare.m_arrival;
}

bool Message::operator<(Message &compare) const
{
  return m_arrival < compare.m_arrival;
}

bool Message::operator<=(Message &compare) const
{
  return m_arrival <= compare.m_arrival;
}

bool Message::operator==(Message &compare) const
{
  return m_arrival == compare.m_arrival;
}

bool Message::operator>=(Message &compare) const
{
  return m_arrival >= compare.m_arrival;
}

bool Message::operator>(Message &compare) const
{
  return m_arrival > compare.m_arrival;
}

bool Message::isRead() const
{
	return m_readStatus;
}

void Message::markAs(bool read) 
{
	m_readStatus = read;
}

bool Message::isRelated(const Message *message) const
{
  if (m_id == message->m_id) return true; //duplicate
  if (m_pid == message->m_pid) return true; //same parent
  if (m_id == message->m_pid) return true; // this is a child of message
  if (m_pid == message->m_id) return true; // this is a parent of message
  if (m_fancySubject == message->m_fancySubject) return true; //fuzzy hit, same subject in this and message
  return false;
}

QString Message::fancify(const QString &subject) const 
{
  return subject;
}
