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

#ifndef DUMMYKONADIMESSAGE_H
#define DUMMYKONADIMESSAGE_H

#include <QString>
#include <QObject>
#include <QDateTime>

class DummyKonadiMessage
{
public:
  DummyKonadiMessage(bool null = false) : nullContent(null) {}
  QString author() const;
  QString content() const;
  QDateTime sendTime() const { return send; }
  QDateTime arrivalTime() const { return arrival; }
  QString arrivalTimeInText() const;
  void setAuthor(QString newAuthor);
  void setContent(QString newContent);
  void setArrivalTime(QDateTime dateTime) { arrival = dateTime; }
  void setSendTime(QDateTime dateTime) { send = dateTime; }
  bool isNull();
  bool operator!=(DummyKonadiMessage &compare) const;
  bool operator<(DummyKonadiMessage &compare) const;
  bool operator<=(DummyKonadiMessage &compare) const;
  bool operator==(DummyKonadiMessage &compare) const;
  bool operator>=(DummyKonadiMessage &compare) const;
  bool operator>(DummyKonadiMessage &compare) const;

private:
  QString conversationAuthor, conversationContent;
  QDateTime arrival, send;
  bool nullContent;
};

#endif
