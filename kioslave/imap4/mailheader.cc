/***************************************************************************
                          mailheader.cc  -  description
                             -------------------
    begin                : Tue Oct 24 2000
    copyright            : (C) 2000 by Sven Carstens
    email                : s.carstens@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mailheader.h"
#include <Q3PtrList>

mailHeader::mailHeader ()
{
  toAdr.setAutoDelete (true);
  ccAdr.setAutoDelete (true);
  bccAdr.setAutoDelete (true);
  setType ("text/plain");
  gmt_offset = 0;
}

mailHeader::~mailHeader ()
{
}

void
mailHeader::addHdrLine (mimeHdrLine * inLine)
{
  mimeHdrLine *addLine = new mimeHdrLine (inLine);

  const QByteArray label(addLine->getLabel());
  const QByteArray value(addLine->getValue());

  if (!qstricmp (label, "Return-Path")) {
	returnpathAdr.parseAddress (value.data ());
	goto out;
  }
  if (!qstricmp (label, "Sender")) {
	senderAdr.parseAddress (value.data ());
	goto out;
  }
  if (!qstricmp (label, "From")) {
	fromAdr.parseAddress (value.data ());
	goto out;
  }
  if (!qstricmp (label, "Reply-To")) {
	replytoAdr.parseAddress (value.data ());
	goto out;
  }
  if (!qstricmp (label, "To")) {
	mailHeader::parseAddressList (value, &toAdr);
	goto out;
  }
  if (!qstricmp (label, "CC")) {
	mailHeader::parseAddressList (value, &ccAdr);
	goto out;
  }
  if (!qstricmp (label, "BCC")) {
	mailHeader::parseAddressList (value, &bccAdr);
	goto out;
  }
  if (!qstricmp (label, "Subject")) {
	_subject = value.simplified();
	goto out;
  }
  if (!qstricmp (label.data (), "Date")) {
	mDate = value;
	goto out;
  }
  if (!qstricmp (label.data (), "Message-ID")) {
      int start = value.lastIndexOf ('<');
      int end = value.lastIndexOf ('>');
      if (start < end)
          messageID = value.mid (start, end - start + 1);
      else {
	  qWarning("bad Message-ID");
          /* messageID = value; */
      }
      goto out;
  }
  if (!qstricmp (label.data (), "In-Reply-To")) {
      int start = value.lastIndexOf ('<');
      int end = value.lastIndexOf ('>');
      if (start < end)
        inReplyTo = value.mid (start, end - start + 1);
      goto out;
  }

  // everything else is handled by mimeHeader
  mimeHeader::addHdrLine (inLine);
  delete addLine;
  return;

 out:
//  cout << label.data() << ": '" << value.data() << "'" << endl;

  //need only to add this line if not handled by mimeHeader
  originalHdrLines.append (addLine);
}

void
mailHeader::outputHeader (mimeIO & useIO)
{
  static const QByteArray __returnPath("Return-Path: ");
  static const QByteArray __from      ("From: ");
  static const QByteArray __sender    ("Sender: ");
  static const QByteArray __replyTo   ("Reply-To: ");
  static const QByteArray __to        ("To: ");
  static const QByteArray __cc        ("CC: ");
  static const QByteArray __bcc       ("BCC: ");
  static const QByteArray __subject   ("Subject: ");
  static const QByteArray __messageId ("Message-ID: ");
  static const QByteArray __inReplyTo ("In-Reply-To: ");
  static const QByteArray __references("References: ");
  static const QByteArray __date      ("Date: ");

  if (!returnpathAdr.isEmpty())
    useIO.outputMimeLine(__returnPath + returnpathAdr.getStr());
  if (!fromAdr.isEmpty())
    useIO.outputMimeLine(__from + fromAdr.getStr());
  if (!senderAdr.isEmpty())
    useIO.outputMimeLine(__sender + senderAdr.getStr());
  if (!replytoAdr.isEmpty())
    useIO.outputMimeLine(__replyTo + replytoAdr.getStr());

  if (toAdr.count())
    useIO.outputMimeLine(mimeHdrLine::truncateLine(__to +
                                    mailHeader::getAddressStr(&toAdr)));
  if (ccAdr.count())
    useIO.outputMimeLine(mimeHdrLine::truncateLine(__cc +
                                    mailHeader::getAddressStr(&ccAdr)));
  if (bccAdr.count())
    useIO.outputMimeLine(mimeHdrLine::truncateLine(__bcc +
                                    mailHeader::getAddressStr(&bccAdr)));
  if (!_subject.isEmpty())
    useIO.outputMimeLine(mimeHdrLine::truncateLine(__subject + _subject));
  if (!messageID.isEmpty())
    useIO.outputMimeLine(mimeHdrLine::truncateLine(__messageId + messageID));
  if (!inReplyTo.isEmpty())
    useIO.outputMimeLine(mimeHdrLine::truncateLine(__inReplyTo + inReplyTo));
  if (!references.isEmpty())
    useIO.outputMimeLine(mimeHdrLine::truncateLine(__references + references));

  if (!mDate.isEmpty())
    useIO.outputMimeLine(__date + mDate);
  mimeHeader::outputHeader(useIO);
}

int
mailHeader::parseAddressList (const char *inCStr,
                              Q3PtrList < mailAddress > *aList)
{
  int advance = 0;
  int skip = 1;
  char *aCStr = (char *) inCStr;

  if (!aCStr || !aList)
    return 0;
  while (skip > 0)
  {
    mailAddress *aAddress = new mailAddress;
    skip = aAddress->parseAddress (aCStr);
    if (skip)
    {
      aCStr += skip;
      if (skip < 0)
        advance -= skip;
      else
        advance += skip;
      aList->append (aAddress);
    }
    else
    {
      delete aAddress;
      break;
    }
  }
  return advance;
}

QByteArray
mailHeader::getAddressStr (Q3PtrList < mailAddress > *aList)
{
  QByteArray retVal;

  Q3PtrListIterator < mailAddress > it = Q3PtrListIterator < mailAddress > (*aList);
  while (it.current ())
  {
    retVal += it.current ()->getStr ();
    ++it;
    if (it.current ())
      retVal += ", ";
  }
  return retVal;
}
