/***************************************************************************
                          mailheader.h  -  description
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

#ifndef MAILHEADER_H
#define MAILHEADER_H

#include "mimeheader.h"
#include "mailaddress.h"
#include "mimeio.h"
#include <kimap/rfccodecs.h>
#include <Q3PtrList>

using namespace KIMAP;

/**
  *@author Sven Carstens
  */

class mailHeader:public mimeHeader
{
public:
  mailHeader ();
  ~mailHeader ();

  virtual void addHdrLine (mimeHdrLine *);
  virtual void outputHeader (mimeIO &);

  void addTo (const mailAddress & _adr)
  {
    toAdr.append (new mailAddress (_adr));
  }
  void addCC (const mailAddress & _adr)
  {
    ccAdr.append (new mailAddress (_adr));
  }
  void addBCC (const mailAddress & _adr)
  {
    bccAdr.append (new mailAddress (_adr));
  }

  void setFrom (const mailAddress & _adr)
  {
    fromAdr = _adr;
  }
  void setSender (const mailAddress & _adr)
  {
    senderAdr = _adr;
  }
  void setReturnPath (const mailAddress & _adr)
  {
    returnpathAdr = _adr;
  }
  void setReplyTo (const mailAddress & _adr)
  {
    replytoAdr = _adr;
  }

  const QByteArray& getMessageId ()
  {
    return messageID;
  }
  void setMessageId (const QByteArray & _str)
  {
    messageID = _str;
  }

  const QByteArray& getInReplyTo ()
  {
    return inReplyTo;
  }
  void setInReplyTo (const QByteArray & _str)
  {
    inReplyTo = _str;
  }

  const QByteArray& getReferences ()
  {
    return references;
  }
  void setReferences (const QByteArray & _str)
  {
    references = _str;
  }

  /**
   * set a unicode subject
   */
  void setSubject (const QString & _str)
  {
    _subject = KIMAP::encodeRFC2047String(_str).toLatin1();
  }
  /**
   * set a encoded subject
   */
  void setSubjectEncoded (const QByteArray & _str)
  {
    _subject = _str.simplified();
  }

  /**
   * get the unicode subject
   */
  const QString getSubject ()
  {
    return KIMAP::decodeRFC2047String(_subject);
  }
  /**
   * get the encoded subject
   */
  const QByteArray& getSubjectEncoded ()
  {
    return _subject;
  }

  /**
   * set the date
   */
  void setDate (const QByteArray & _str)
  {
    mDate = _str;
  }

  /**
   * get the date
   */
  const QByteArray& date ()
  {
    return mDate;
  }

  static int parseAddressList (const char *, Q3PtrList < mailAddress > *);
  static QByteArray getAddressStr (Q3PtrList < mailAddress > *);
  Q3PtrList < mailAddress > &to ()
  {
    return toAdr;
  }
  Q3PtrList < mailAddress > &cc ()
  {
    return ccAdr;
  }
  Q3PtrList < mailAddress > &bcc ()
  {
    return bccAdr;
  }
#ifdef KMAIL_COMPATIBLE
  QString subject ()
  {
    return getSubject ();
  }
  const mailAddress & from ()
  {
    return fromAdr;
  }
  const mailAddress & replyTo ()
  {
    return replytoAdr;
  }
  void readConfig (void)
  {;
  }
#endif

private:
  Q3PtrList < mailAddress > toAdr;
  Q3PtrList < mailAddress > ccAdr;
  Q3PtrList < mailAddress > bccAdr;
  mailAddress fromAdr;
  mailAddress senderAdr;
  mailAddress returnpathAdr;
  mailAddress replytoAdr;
  QByteArray _subject;
  QByteArray mDate;
  int gmt_offset;
  QByteArray messageID;
  QByteArray inReplyTo;
  QByteArray references;
};

#endif
