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

QAsciiDict<ulong> * Response::statusCodeDict_ = 0L;

Response::Response(const QString & data)
  : allData_(data),
    responseType_(ResponseTypeUnknown),
    statusCode_(StatusCodeUnknown)
{
  int startOfLastLine = allData_.findRev("\n");

  if (-1 != startOfLastLine)
    startOfLastLine = allData_.findRev("\n", startOfLastLine - 1);

  QString lastLine;

  if (-1 == startOfLastLine)
    lastLine = allData_;
  else
    lastLine = allData_.mid(startOfLastLine + 1);

  qDebug("Last line: %s", lastLine.ascii());

  QStringList tokens(QStringList::split(' ', lastLine));

  QString token0 = tokens[0];

  if (token0 == "*")
    responseType_ = ResponseTypeStatus;
  else if (token0 == "+")
    responseType_ = ResponseTypeContinuationRequest;
  else
    responseType_ = ResponseTypeServerData;

  statusCode_ = _statusCode(tokens[1]);

  data_ = allData_;
}

  void
Response::cleanup()
{
  delete statusCodeDict_;
  statusCodeDict_ = 0;
}

  Response::ResponseType
Response::type() const
{
  return responseType_;
}

  Response::StatusCode
Response::statusCode() const
{
  return statusCode_;
}

  QString
Response::data() const
{
  return data_;
}

  QString
Response::allData() const
{
  return allData_;
}

  Response::StatusCode
Response::_statusCode(const QString & key)
{
  if (0 == statusCodeDict_) {

    statusCodeDict_ = new QAsciiDict<ulong>(23);

    statusCodeDict_->insert("ALERT",          new ulong(StatusCodeAlert));
    statusCodeDict_->insert("NEWNAME",        new ulong(StatusCodeNewName));
    statusCodeDict_->insert("PARSE",          new ulong(StatusCodeParse));
    statusCodeDict_->insert("PERMANENTFLAGS", new ulong(StatusCodePermanentFlags));
    statusCodeDict_->insert("READ-ONLY",      new ulong(StatusCodeReadOnly));
    statusCodeDict_->insert("READ-WRITE",     new ulong(StatusCodeReadWrite));
    statusCodeDict_->insert("TRYCREATE",      new ulong(StatusCodeTryCreate));
    statusCodeDict_->insert("UIDVALIDITY",    new ulong(StatusCodeUIDValidity));
    statusCodeDict_->insert("UNSEEN",         new ulong(StatusCodeUnseen));
    statusCodeDict_->insert("OK",             new ulong(StatusCodeOk));
    statusCodeDict_->insert("NO",             new ulong(StatusCodeNo));
    statusCodeDict_->insert("BAD",            new ulong(StatusCodeBad));
    statusCodeDict_->insert("PREAUTH",        new ulong(StatusCodePreAuth));
    statusCodeDict_->insert("CAPABILITY",     new ulong(StatusCodeCapability));
    statusCodeDict_->insert("LIST",           new ulong(StatusCodeList));
    statusCodeDict_->insert("LSUB",           new ulong(StatusCodeLsub));
    statusCodeDict_->insert("STATUS",         new ulong(StatusCodeStatus));
    statusCodeDict_->insert("SEARCH",         new ulong(StatusCodeSearch));
    statusCodeDict_->insert("FLAGS",          new ulong(StatusCodeFlags));
    statusCodeDict_->insert("EXISTS",         new ulong(StatusCodeExists));
    statusCodeDict_->insert("RECENT",         new ulong(StatusCodeRecent));
    statusCodeDict_->insert("EXPUNGE",        new ulong(StatusCodeExpunge));
    statusCodeDict_->insert("FETCH",          new ulong(StatusCodeFetch));
  }

  if (!key)
    return StatusCodeUnknown;

  ulong * l = statusCodeDict_->find(key.ascii());

  if (0 == l)
    return StatusCodeUnknown;
  else
    return StatusCode(*l);
}

