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

MailboxInfo::MailboxInfo()
  : count_(0),
    recent_(0),
    unseen_(0),
    uidValidity_(0),
    flags_(0),
    permanentFlags_(0),
    readWrite_(false),
    countAvailable_(false),
    recentAvailable_(false),
    unseenAvailable_(false),
    uidValidityAvailable_(false),
    flagsAvailable_(false),
    permanentFlagsAvailable_(false),
    readWriteAvailable_(false)
{
}

MailboxInfo::MailboxInfo(const MailboxInfo & mi)
  : count_(mi.count_),
    recent_(mi.recent_),
    unseen_(mi.unseen_),
    uidValidity_(mi.uidValidity_),
    flags_(mi.flags_),
    permanentFlags_(mi.permanentFlags_),
    readWrite_(mi.readWrite_),
    countAvailable_(mi.countAvailable_),
    recentAvailable_(mi.recentAvailable_),
    unseenAvailable_(mi.unseenAvailable_),
    uidValidityAvailable_(mi.uidValidityAvailable_),
    flagsAvailable_(mi.flagsAvailable_),
    permanentFlagsAvailable_(mi.permanentFlagsAvailable_),
    readWriteAvailable_(mi.readWriteAvailable_)
{
}

  MailboxInfo &
MailboxInfo::operator = (const MailboxInfo & mi)
{
  // Avoid a = a.
  if (this == &mi)
    return *this;

  count_ = mi.count_;
  recent_ = mi.recent_;
  unseen_ = mi.unseen_;
  uidValidity_ = mi.uidValidity_;
  flags_ = mi.flags_;
  permanentFlags_ = mi.permanentFlags_;
  readWrite_ = mi.readWrite_;
  countAvailable_ = mi.countAvailable_;
  recentAvailable_ = mi.recentAvailable_;
  unseenAvailable_ = mi.unseenAvailable_;
  uidValidityAvailable_ = mi.uidValidityAvailable_;
  flagsAvailable_ = mi.flagsAvailable_;
  permanentFlagsAvailable_ = mi.permanentFlagsAvailable_;
  readWriteAvailable_ = mi.readWriteAvailable_;

  return *this;
}

MailboxInfo::MailboxInfo(const QString & s)
  : count_(0),
    recent_(0),
    unseen_(0),
    uidValidity_(0),
    flags_(0),
    permanentFlags_(0),
    readWrite_(false),
    countAvailable_(false),
    recentAvailable_(false),
    unseenAvailable_(false),
    uidValidityAvailable_(false),
    flagsAvailable_(false),
    permanentFlagsAvailable_(false),
    readWriteAvailable_(false)
{
  QStringList list(QStringList::split("\r\n", s));

  for (QStringList::ConstIterator it(list.begin()); it != list.fromLast(); ++it)
  {
    QString line(*it);

    QStringList tokens(QStringList::split(' ', line));

    if (tokens[0] != "*")
      continue;

    if (tokens[2] == "EXISTS")
      setCount(tokens[1].toULong());

    else if (tokens[2] == "RECENT")
      setRecent(tokens[1].toULong());

    else if (tokens[1] == "FLAGS")
    {
      int flagsStart  = line.find('(');
      int flagsEnd    = line.find(')');

      if ((-1 != flagsStart) && (-1 != flagsEnd) && flagsStart < flagsEnd)
        setFlags(_flags(line.mid(flagsStart, flagsEnd)));
    }

    else if (tokens[2] == "[UNSEEN")
      setUnseen(tokens[3].left(tokens[3].length() - 2).toULong());

    else if (tokens[2] == "[UIDVALIDITY")
      setUidValidity(tokens[3].left(tokens[3].length() - 2).toULong());

    else if (tokens[2] == "[PERMANENTFLAGS")
    {
      int flagsStart  = line.find('(');
      int flagsEnd    = line.find(')');

      if ((-1 != flagsStart) && (-1 != flagsEnd) && flagsStart < flagsEnd)
        setPermanentFlags(_flags(line.mid(flagsStart, flagsEnd)));
    }
  }

  setReadWrite(-1 != list.last().find("READ-WRITE"));
}

  ulong
MailboxInfo::_flags(const QString & flagsString) const
{
  ulong flags = 0;

  if (0 != flagsString.contains("\\Seen"))
    flags ^= Seen;
  if (0 != flagsString.contains("\\Answered"))
    flags ^= Answered;
  if (0 != flagsString.contains("\\Flagged"))
    flags ^= Flagged;
  if (0 != flagsString.contains("\\Deleted"))
    flags ^= Deleted;
  if (0 != flagsString.contains("\\Draft"))
    flags ^= Draft;
  if (0 != flagsString.contains("\\Recent"))
    flags ^= Recent;

  return flags;
}


