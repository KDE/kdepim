#ifndef _IMAPPARSER_H
#define _IMAPPARSER_H
/**********************************************************************
 *
 *   imapparser.h  - IMAP4rev1 Parser
 *   Copyright (C) 2001-2002 Michael Haeckel <haeckel@kde.org>
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

#include <tqstringlist.h>
#include <tqvaluelist.h>
#include <tqptrlist.h>
#include <tqasciidict.h>

#include <kio/authinfo.h>
#include <kio/slavebase.h>

#include "imaplist.h"
#include "imapcommand.h"
#include "imapinfo.h"

#include "mailheader.h"

class KURL;
class QString;
class mailAddress;
class mimeHeader;


/** @brief a string used during parsing
 * the string allows you to move the effective start of the string using
 * str.pos++ and str.pos--.
 * @bug it is possible to move past the beginning and end of the string
 */
class parseString
{
public:
  parseString() { pos = 0; }
  char operator[](uint i) const { return data[i + pos]; }
  bool isEmpty() const { return pos >= data.size(); }
  TQCString cstr() const
  {
    if (pos >= data.size()) return TQCString();
    return TQCString(data.data() + pos, data.size() - pos + 1);
  }
  int find(char c, int index = 0)
  {
    int res = data.find(c, index + pos);
    return (res == -1) ? res : (res - pos);
  }
  // Warning: does not check for going past end of "data"
  void takeLeft(TQCString& dest, uint len) const
  {
    dest.resize(len + 1);
    qmemmove(dest.data(), data.data() + pos, len);
  }
  // Warning: does not check for going past end of "data"
  void takeLeftNoResize(TQCString& dest, uint len) const
  {
    qmemmove(dest.data(), data.data() + pos, len);
  }
  // Warning: does not check for going past end of "data"
  void takeMid(TQCString& dest, uint start, uint len) const
  {
    dest.resize(len + 1);
    qmemmove(dest.data(), data.data() + pos + start, len);
  }
  // Warning: does not check for going past end of "data"
  void takeMidNoResize(TQCString& dest, uint start, uint len) const
  {
    qmemmove(dest.data(), data.data() + pos + start, len);
  }
  void clear()
  {
    data.resize(0);
    pos = 0;
  }
  uint length()
  {
    return data.size() - pos;
  }
  void fromString(const TQString &s)
  {
    clear();
    data.duplicate(s.latin1(), s.length());
  }
  TQByteArray data;
  uint pos;
};

class imapCache
{
public:
  imapCache ()
  {
    myHeader = NULL;
    mySize = 0;
    myFlags = 0;
    myUid = 0;
  }

  ~imapCache ()
  {
    if (myHeader) delete myHeader;
  }

  mailHeader *getHeader ()
  {
    return myHeader;
  }
  void setHeader (mailHeader * inHeader)
  {
    myHeader = inHeader;
  }

  ulong getSize ()
  {
    return mySize;
  }
  void setSize (ulong inSize)
  {
    mySize = inSize;
  }

  ulong getUid ()
  {
    return myUid;
  }
  void setUid (ulong inUid)
  {
    myUid = inUid;
  }

  ulong getFlags ()
  {
    return myFlags;
  }
  void setFlags (ulong inFlags)
  {
    myFlags = inFlags;
  }

  TQCString getDate ()
  {
    return myDate;
  }
  void setDate (const TQCString & _str)
  {
    myDate = _str;
  }
  void clear()
  {
    if (myHeader) delete myHeader;
    myHeader = NULL;
    mySize = 0;
    myFlags = 0;
    myDate = TQCString();
    myUid = 0;
  }

protected:
  mailHeader * myHeader;
  ulong mySize;
  ulong myFlags;
  ulong myUid;
  TQCString myDate;
};


class imapParser
{

public:

  /** the different states the client can be in */
  enum IMAP_STATE
  {
    ISTATE_NO,       /**< Not connected */
    ISTATE_CONNECT,  /**< Connected but not logged in */
    ISTATE_LOGIN,    /**< Logged in */
    ISTATE_SELECT    /**< A folder is currently selected */
  };

public:
    imapParser ();
    virtual ~ imapParser ();

  /** @brief Get the current state */
  enum IMAP_STATE getState () { return currentState; }
  /** @brief Set the current state */
  void setState(enum IMAP_STATE state) { currentState = state; }

  /* @brief return the currently selected mailbox */
  const TQString getCurrentBox ()
  {
    return rfcDecoder::fromIMAP(currentBox);
  };

  /**
   * @brief do setup and send the command to parseWriteLine
   * @param aCmd The command to perform
   * @return The completed command
   */
  imapCommand *sendCommand (imapCommand * aCmd);
  /**
   * @brief perform a command and wait to parse the result
   * @param aCmd The command to perform
   * @return The completed command
   */
  imapCommand *doCommand (imapCommand * aCmd);


  /**
   * @brief plaintext login
   * @param aUser Username
   * @param aPass Password
   * @param resultInfo The resultinfo from the command
   * @return success or failure
   */
  bool clientLogin (const TQString & aUser, const TQString & aPass, TQString & resultInfo);
  /**
   * @brief non-plaintext login
   * @param aUser Username
   * @param aPass Password
   * @param aAuth authentication method
   * @param isSSL are we using SSL
   * @param resultInfo The resultinfo from the command
   * @return success or failure
   */
  bool clientAuthenticate (KIO::SlaveBase *slave, KIO::AuthInfo &ai, const TQString & aFQDN,
    const TQString & aAuth, bool isSSL, TQString & resultInfo);

  /**
   * main loop for the parser
   * reads one line and dispatches it to the appropriate sub parser
   */
  int parseLoop ();

  /**
   * @brief parses all untagged responses and passes them on to the
   * following parsers
   */
  void parseUntagged (parseString & result);

  /** @brief parse a RECENT line */
  void parseRecent (ulong value, parseString & result);
  /** @brief parse a RESULT line */
  void parseResult (TQByteArray & result, parseString & rest,
                    const TQString & command = TQString::null);
  /** @brief parse a CAPABILITY line */
  void parseCapability (parseString & result);
  /** @brief parse a FLAGS line */
  void parseFlags (parseString & result);
  /** @brief parse a LIST line */
  void parseList (parseString & result);
  /** @brief parse a LSUB line */
  void parseLsub (parseString & result);
  /** @brief parse a LISTRIGHTS line */
  void parseListRights (parseString & result);
  /** @brief parse a MYRIGHTS line */
  void parseMyRights (parseString & result);
  /** @brief parse a SEARCH line */
  void parseSearch (parseString & result);
  /** @brief parse a STATUS line */
  void parseStatus (parseString & result);
  /** @brief parse a EXISTS line */
  void parseExists (ulong value, parseString & result);
  /** @brief parse a EXPUNGE line */
  void parseExpunge (ulong value, parseString & result);
  /** @brief parse a ACL line */
  void parseAcl (parseString & result);
  /** @brief parse a ANNOTATION line */
  void parseAnnotation (parseString & result);
  /** @brief parse a NAMESPACE line */
  void parseNamespace (parseString & result);
  /** @brief parse a QUOTAROOT line */
  void parseQuotaRoot (parseString & result);
  /** @brief parse a QUOTA line */
  void parseQuota (parseString & result);
  /** @brief parse a custom command line */
  void parseCustom (parseString & result);
  /** @brief parse a OTHER-USER line */
  void parseOtherUser (parseString & result);
  /** @brief parse a DELEGATE line */
  void parseDelegate (parseString & result);
  /** @brief parse a OUT-OF-OFFICE line */
  void parseOutOfOffice (parseString & result);

  /**
   * parses the results of a fetch command
   * processes it with the following sub parsers
   */
  void parseFetch (ulong value, parseString & inWords);

  /** read a envelope from imap and parse the addresses */
  mailHeader *parseEnvelope (parseString & inWords);
  /** @brief parse an address list and return a list of addresses */
  void parseAddressList (parseString & inWords, TQPtrList<mailAddress>& list);
  /** @brief parse an address and return the ref again */
  const mailAddress& parseAddress (parseString & inWords, mailAddress& buffer);

  /** parse the result of the body command */
  void parseBody (parseString & inWords);

  /** parse the body structure recursively */
  mimeHeader *parseBodyStructure (parseString & inWords,
    TQString & section, mimeHeader * inHeader = 0);

  /** parse only one not nested part */
  mimeHeader *parseSimplePart (parseString & inWords, TQString & section,
      mimeHeader * localPart = 0);

  /** parse a parameter list (name value pairs) */
  TQAsciiDict < TQString > parseParameters (parseString & inWords);

  /**
   * parse the disposition list (disposition (name value pairs))
   * the disposition has the key 'content-disposition'
   */
  TQAsciiDict < TQString > parseDisposition (parseString & inWords);

  // reimplement these

  /** relay hook to send the fetched data directly to an upper level */
  virtual void parseRelay (const TQByteArray & buffer);

  /** relay hook to announce the fetched data directly to an upper level
   */
  virtual void parseRelay (ulong);

  /** read at least len bytes */
  virtual bool parseRead (TQByteArray & buffer, ulong len, ulong relay = 0);

  /** read at least a line (up to CRLF) */
  virtual bool parseReadLine (TQByteArray & buffer, ulong relay = 0);

  /** write argument to server */
  virtual void parseWriteLine (const TQString &);

  // generic parser routines

  /** parse a parenthesized list */
  void parseSentence (parseString & inWords);

  /** parse a literal or word, may require more data */
  TQCString parseLiteralC(parseString & inWords, bool relay = false,
                           bool stopAtBracket = false, int *outlen = 0);
  inline TQByteArray parseLiteral (parseString & inWords, bool relay = false,
                           bool stopAtBracket = false) {
    int len = 0; // string size
    // Choice: we can create an extra TQCString, or we can get the buffer in
    // the wrong size to start.  Let's try option b.
    TQCString tmp = parseLiteralC(inWords, relay, stopAtBracket, &len);
    return TQByteArray().duplicate(tmp.data(), len);
  }

  // static parser routines, can be used elsewhere

  static TQCString b2c(const TQByteArray &ba)
  { return TQCString(ba.data(), ba.size() + 1); }

  /** parse one word (maybe quoted) upto next space " ) ] } */
  static TQCString parseOneWordC (parseString & inWords,
    bool stopAtBracket = FALSE, int *len = 0);

  /** parse one number using parseOneWord */
  static bool parseOneNumber (parseString & inWords, ulong & num);

  /** extract the box,section,list type, uid, uidvalidity,info from an url */
  static void parseURL (const KURL & _url, TQString & _box, TQString & _section,
                        TQString & _type, TQString & _uid, TQString & _validity,
                        TQString & _info);


 /** @brief return the last handled foo
  * @todo work out what a foo is
  */
  imapCache *getLastHandled ()
  {
    return lastHandled;
  };

/** @brief return the last results */
  const TQStringList & getResults ()
  {
    return lastResults;
  };

  /** @brief return the last status code */
  const imapInfo & getStatus ()
  {
    return lastStatus;
  };
  /** return the select info */
  const imapInfo & getSelected ()
  {
    return selectInfo;
  };

  const TQByteArray & getContinuation ()
  {
    return continuation;
  };

  /** @brief see if server has a capability */
  bool hasCapability (const TQString &);

  void removeCapability (const TQString & cap);

  static inline void skipWS (parseString & inWords)
  {
    char c;
    while (!inWords.isEmpty() &&
      ((c = inWords[0]) == ' ' || c == '\t' || c == '\r' || c == '\n'))
    {
      inWords.pos++;
    }
  }

  /** @brief find the namespace for the given box */
  TQString namespaceForBox( const TQString & box );


protected:

  /** the current state we're in */
  enum IMAP_STATE currentState;

  /** the box selected */
  TQString currentBox;

  /** @brief here we store the result from select/examine and unsolicited updates */
  imapInfo selectInfo;

  /** @brief the results from the last status command */
  imapInfo lastStatus;

  /** @brief the results from the capabilities, split at ' ' */
  TQStringList imapCapabilities;

  /** @brief the results from list/lsub/listrights commands */
  TQValueList < imapList > listResponses;

  /** @brief queues handling the running commands */
  TQPtrList < imapCommand > sentQueue;  // no autodelete
  TQPtrList < imapCommand > completeQueue;  // autodelete !!

  /**
   * everything we didn't handle, everything but the greeting is bogus
   */
  TQStringList unhandled;

  /** the last continuation request (there MUST not be more than one pending) */
  TQByteArray continuation;

  /** the last uid seen while a fetch */
  TQString seenUid;
  imapCache *lastHandled;

  ulong commandCounter;

  /** @brief the results from search/acl commands */
  TQStringList lastResults;

  /**
   * @brief namespace prefix - delimiter association
   * The namespace is cleaned before so that it does not contain the delimiter
   */
  TQMap<TQString, TQString> namespaceToDelimiter;

  /**
   * @brief list of namespaces in the form: section=namespace=delimiter
   * section is 0 (personal), 1 (other users) or 2 (shared)
   */
  TQStringList imapNamespaces;

private:

  /** we don't want to be able to copy this object */
  imapParser & operator = (const imapParser &); // hide the copy ctor

};
#endif
