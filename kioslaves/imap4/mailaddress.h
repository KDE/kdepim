#ifndef _MAILADDRESS_H
#define _MAILADDRESS_H
/**********************************************************************
 *
 *   mailaddress.h - mail address handler
 *   Copyright (C) 2000 s.carstens@gmx.de
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *   Send comments and bug fixes to s.carstens@gmx.de
 *
 *********************************************************************/

#include <tqptrlist.h>
#include <tqstring.h>
#include <tqcstring.h>
#include "rfcdecoder.h"

class mailAddress
{
public:
  mailAddress ();
  ~mailAddress ();
  mailAddress (char *aCStr);
    mailAddress (const mailAddress &);
    mailAddress & operator = (const mailAddress &);

  void setUser (const TQCString & aUser)
  {
    user = aUser;
  }
  const TQCString & getUser () const
  {
    return user;
  }
  void setHost (const TQCString & aHost)
  {
    host = aHost;
  }
  const TQCString & getHost () const
  {
    return host;
  }

  void setFullName (const TQString & aFull);
  const TQString getFullName () const;

  void setComment (const TQString & aComment);
  void setCommentRaw (const TQCString &);
  const TQString getComment () const;
  const TQCString & getCommentRaw () const;

  int parseAddress (char *);
  const TQCString getStr ();
  bool isEmpty () const;

  static TQString emailAddrAsAnchor (const mailAddress &, bool);
  static TQString emailAddrAsAnchor (const TQPtrList < mailAddress > &, bool);

  void clear();

private:
  TQCString user;
  TQCString host;
  TQCString rawFullName;
  TQCString rawComment;
};

#endif
