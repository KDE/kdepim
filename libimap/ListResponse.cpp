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

ListResponse::ListResponse()
  : noInferiors_(false),
    noSelect_(false),
    marked_(false),
    unmarked_(false)
{
}

ListResponse::ListResponse(const ListResponse & lr)
  : hierarchyDelimiter_(lr.hierarchyDelimiter_),
    name_(lr.name_),
    noInferiors_(lr.noInferiors_),
    noSelect_(lr.noSelect_),
    marked_(lr.marked_),
    unmarked_(lr.unmarked_)
{
}

  ListResponse &
ListResponse::operator = (const ListResponse & lr)
{
  // Avoid a = a.
  if (this == &lr)
    return *this;

  hierarchyDelimiter_ = lr.hierarchyDelimiter_;
  name_ = lr.name_;
  noInferiors_ = lr.noInferiors_;
  noSelect_ = lr.noSelect_;
  marked_ = lr.marked_;
  unmarked_ = lr.unmarked_;

  return *this;
}

ListResponse::ListResponse(const QString & s)
  : noInferiors_(false),
    noSelect_(false),
    marked_(false),
    unmarked_(false)
{
  QStringList l(QStringList::split(' ', s));

  if ((l[0] != "*") || (l[1] != "LIST"))
    return;

  int openBrace = s.find('(');
  int closeBrace = s.find(')');

  if ((-1 == openBrace) || (-1 == closeBrace) || (openBrace >= closeBrace))
    return;

  QString attributes(s.mid(openBrace, closeBrace - openBrace));

  noInferiors_  = -1 != attributes.find("\\Noinferiors");
  noSelect_     = -1 != attributes.find("\\Noselect");
  marked_       = -1 != attributes.find("\\Marked");
  unmarked_     = -1 != attributes.find("\\Unmarked");

  int startQuote = s.find("\"");
  int endQuote = s.find("\"", startQuote + 1);

  if ((-1 == startQuote) || (-1 == endQuote) || (startQuote >= endQuote))
    return;

  hierarchyDelimiter_ = s.mid(startQuote + 1, endQuote - startQuote - 1);

  startQuote = s.find("\"", endQuote + 1);
  endQuote = s.find("\"", startQuote + 1);

  name_ = s.mid(startQuote + 1, endQuote - startQuote - 1);
}

