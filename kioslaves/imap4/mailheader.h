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
#include "rfcdecoder.h"

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

  const TQCString& getMessageId ()
  {
    return messageID;
  }
  void setMessageId (const TQCString & _str)
  {
    messageID = _str;
  }

  const TQCString& getInReplyTo ()
  {
    return inReplyTo;
  }
  void setInReplyTo (const TQCString & _str)
  {
    inReplyTo = _str;
  }

  const TQCString& getReferences ()
  {
    return references;
  }
  void setReferences (const TQCString & _str)
  {
    references = _str;
  }

  /**
   * set a unicode subject
   */
  void setSubject (const TQString & _str)
  {
    _subject = rfcDecoder::encodeRFC2047String(_str).latin1();
  }
  /** 
   * set a encoded subject
   */
  void setSubjectEncoded (const TQCString & _str)
  {
    _subject = _str.simplifyWhiteSpace();
  }

  /** 
   * get the unicode subject
   */
  const TQString getSubject ()
  {
    return rfcDecoder::decodeRFC2047String(_subject);
  }
  /**
   * get the encoded subject
   */
  const TQCString& getSubjectEncoded ()
  {
    return _subject;
  }

  /**
   * set the date
   */
  void setDate (const TQCString & _str)
  {
    mDate = _str;
  }

  /**
   * get the date
   */
  const TQCString& date ()
  {
    return mDate;
  }

  static int parseAddressList (const char *, TQPtrList < mailAddress > *);
  static TQCString getAddressStr (TQPtrList < mailAddress > *);
  TQPtrList < mailAddress > &to ()
  {
    return toAdr;
  }
  TQPtrList < mailAddress > &cc ()
  {
    return ccAdr;
  }
  TQPtrList < mailAddress > &bcc ()
  {
    return bccAdr;
  }
#ifdef KMAIL_COMPATIBLE
  TQString subject ()
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
  TQPtrList < mailAddress > toAdr;
  TQPtrList < mailAddress > ccAdr;
  TQPtrList < mailAddress > bccAdr;
  mailAddress fromAdr;
  mailAddress senderAdr;
  mailAddress returnpathAdr;
  mailAddress replytoAdr;
  TQCString _subject;
  TQCString mDate;
  int gmt_offset;
  TQCString messageID;
  TQCString inReplyTo;
  TQCString references;
};

#endif
