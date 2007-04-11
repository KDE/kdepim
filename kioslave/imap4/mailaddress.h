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

#include <q3ptrlist.h>
#include <QString>

class mailAddress
{
public:
  mailAddress ();
  ~mailAddress ();
  mailAddress (char *aCStr);
    mailAddress (const mailAddress &);
    mailAddress & operator = (const mailAddress &);

  void setUser (const QByteArray & aUser)
  {
    user = aUser;
  }
  const QByteArray & getUser () const
  {
    return user;
  }
  void setHost (const QByteArray & aHost)
  {
    host = aHost;
  }
  const QByteArray & getHost () const
  {
    return host;
  }

  void setFullName (const QString & aFull);
  const QString getFullName () const;

  void setComment (const QString & aComment);
  void setCommentRaw (const QByteArray &);
  const QString getComment () const;
  const QByteArray & getCommentRaw () const;

  int parseAddress (const char *);
  const QByteArray getStr ();
  bool isEmpty () const;

  static QString emailAddrAsAnchor (const mailAddress &, bool);
  static QString emailAddrAsAnchor (const Q3PtrList < mailAddress > &, bool);

  void clear();

private:
  QByteArray user;
  QByteArray host;
  QByteArray rawFullName;
  QByteArray rawComment;
};

#endif
