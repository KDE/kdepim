/*
   RFC 2060 (IMAP4rev1) client.

   Copyright (C) 2000 Rik Hemsley (rikkus) <rik@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/ 

#include "IMAPClient.h"

using namespace IMAP;

StatusInfo::StatusInfo(const StatusInfo & si)
  : messageCount_(si.messageCount_),
    recentCount_(si.recentCount_),
    nextUID_(si.nextUID_),
    uidValidity_(si.uidValidity_),
    unseenCount_(si.unseenCount_),
    hasMessageCount_(si.hasMessageCount_),
    hasRecentCount_(si.hasRecentCount_),
    hasNextUID_(si.hasNextUID_),
    hasUIDValidity_(si.hasUIDValidity_),
    hasUnseenCount_(si.hasUnseenCount_)
{
}

  StatusInfo &
StatusInfo::operator = (const StatusInfo & si)
{
  // Avoid a = a.
  if (this == &si)
    return *this;

  messageCount_ = si.messageCount_;
  recentCount_ = si.recentCount_;
  nextUID_ = si.nextUID_;
  uidValidity_ = si.uidValidity_;
  unseenCount_ = si.unseenCount_;
  hasMessageCount_ = si.hasMessageCount_;
  hasRecentCount_ = si.hasRecentCount_;
  hasNextUID_ = si.hasNextUID_;
  hasUIDValidity_ = si.hasUIDValidity_;
  hasUnseenCount_ = si.hasUnseenCount_;

  return *this;
}


StatusInfo::StatusInfo(const QString & s)
  : messageCount_(0),
    recentCount_(0),
    nextUID_(0),
    uidValidity_(0),
    unseenCount_(0),
    hasMessageCount_(false),
    hasRecentCount_(false),
    hasNextUID_(false),
    hasUIDValidity_(false),
    hasUnseenCount_(false)
{
  int openBrace(s.find('('));
  int closeBrace(s.find(')'));

  if ((-1 != openBrace) || (-1 != closeBrace) || (openBrace <= closeBrace))
    return;

  QString _s(s.mid(openBrace + 1, closeBrace - openBrace));

  QStringList tokens(QStringList::split(' ', _s));

  QStringList::ConstIterator it(tokens.begin());

  for (; it != tokens.end(); ++it) {

    QString s1(*it);
    QString s2(*(++it));

    if (s1 == "MESSAGES")
      setMessageCount(s2.toULong());

    else if (s1 == "RECENT")
      setRecentCount(s2.toULong());

    else if (s1 == "UIDNEXT")
      setNextUID(s2.toULong());

    else if (s1 == "UIDVALIDITY")
      setUIDValidity(s2.toULong());

    else if (s1 == "UNSEEN")
      setUnseenCount(s2.toULong());
  }
}

