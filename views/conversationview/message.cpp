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

QString Message::author() const
{
  if (nullContent == true) return "";
  return conversationAuthor;
}

QString Message::content() const
{
  if (nullContent == true) return "";
  return conversationContent;
}

void Message::setAuthor(QString author)
{
  conversationAuthor = author;
}

void Message::setContent(QString content)
{
  content.replace(QRegExp("\n"), "<br>");
  conversationContent = content;
}

bool Message::isNull() const
{
  return nullContent;
}

QString Message::arrivalTimeInText() const
{
  QString ctime = arrival.date().toString();
  ctime.append(" ");
  ctime.append(arrival.time().toString());
  return ctime;
}

bool Message::operator!=(Message &compare) const
{
  return arrival != compare.arrival;
}

bool Message::operator<(Message &compare) const
{
  return arrival < compare.arrival;
}

bool Message::operator<=(Message &compare) const
{
  return arrival <= compare.arrival;
}

bool Message::operator==(Message &compare) const
{
  return arrival == compare.arrival;
}

bool Message::operator>=(Message &compare) const
{
  return arrival >= compare.arrival;
}

bool Message::operator>(Message &compare) const
{
  return arrival > compare.arrival;
}

bool Message::isRead() const
{
	return readStatus;
}

void Message::markAs(bool read) 
{
	readStatus = read;
}
