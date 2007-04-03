#ifndef _IMAP4_H
#define _IMAP4_H
/**********************************************************************
 *
 *   imap4.h  - IMAP4rev1 KIOSlave
 *   Copyright (C) 2001-2002  Michael Haeckel <haeckel@kde.org>
 *   Copyright (C) 1999  John Corey
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
 *   Send comments and bug fixes to jcorey@fruity.ath.cx
 *
 *********************************************************************/

#include "imapparser.h"
#include "mimeio.h"

#include <kio/tcpslavebase.h>
#include <QBuffer>
#include <QDateTime>
//Added by qt3to4:
#include <QByteArray>

#define IMAP_BUFFER 8192

/** @brief type of object the url refers too */
enum IMAP_TYPE
{
  ITYPE_UNKNOWN, /*< unknown type */
  ITYPE_DIR,     /*< Object is a directory. i.e. does not contain message, just mailboxes */
  ITYPE_BOX,     /*< Object is a mailbox. i.e. contains mails */
  ITYPE_DIR_AND_BOX, /*< Object contains both mails and mailboxes */
  ITYPE_MSG,   /*< Object is a mail */
  ITYPE_ATTACH   /*< Object is an attachment */
};

/** @brief IOSlave derived class */
class IMAP4Protocol:public
  KIO::TCPSlaveBase,
  public
  imapParser,
  public
  mimeIO
{

public:

  // reimplement the TCPSlave
  IMAP4Protocol (const QByteArray & pool, const QByteArray & app, bool isSSL);
  virtual ~IMAP4Protocol ();

  virtual void openConnection();
  virtual void closeConnection();

  virtual void setHost (const QString & _host, int _port, const QString & _user,
    const QString & _pass);
  /**
   * @brief get a message or part of a message
   * the data is normally sent as we get it from the server
   * if you want the slave to decode the content (e.g. for attachments)
   * then append an additional INFO=DECODE to the URL
   */
  virtual void get (const KUrl & _url);
  /**
   * @brief stat a mailbox, message, attachment
   */
  virtual void stat (const KUrl & _url);
  virtual void slave_status ();
  /**
   * @brief delete a mailbox
   */
  virtual void del (const KUrl & _url, bool isFile);
  /**
   * @brief Capabilites, NOOP, (Un)subscribe, Change status,
   * Change ACL
   */
  virtual void special (const QByteArray & data);
  /**
   * @brief list a directory/mailbox
   */
  virtual void listDir (const KUrl & _url);
  virtual void setSubURL (const KUrl & _url);
  virtual void dispatch (int command, const QByteArray & data);
  /**
   * @brief create a mailbox
   */
  virtual void mkdir (const KUrl & url, int permissions);
  virtual void put (const KUrl & url, int permissions, bool overwrite,
    bool resume);
  virtual void rename (const KUrl & src, const KUrl & dest, bool overwrite);
  virtual void copy (const KUrl & src, const KUrl & dest, int permissions,
    bool overwrite);

  /** @brief reimplement the parser
   * relay hook to send the fetched data directly to an upper level
   */
  virtual void parseRelay (const QByteArray & buffer);

  /** @brief reimplement the parser
   * relay hook to announce the fetched data directly to an upper level
   */
  virtual void parseRelay (ulong);

  /** @brief reimplement the parser
   * read at least len bytes */
  virtual bool parseRead (QByteArray &buffer, long len, long relay=0);

  /** @brief reimplement the parser
   * @brief read at least a line (up to CRLF) */
  virtual bool parseReadLine (QByteArray & buffer, long relay = 0);

  /** @brief reimplement the parser
   * @brief write argument to the server */
  virtual void parseWriteLine (const QString &);

  /** @brief reimplement the mimeIO */
  virtual int outputLine (const QByteArray & _str, int len = -1);

  /** @brief send out cached data to the application */
  virtual void flushOutput(QString contentEncoding = QString());

protected:

  // select or examine the box if needed
  bool assureBox (const QString & aBox, bool readonly);

  ssize_t myRead(void *data, ssize_t len);

  /**
   * @brief Parses the given URL
   * The return values are set by parsing the URL and querying the server
   *
   * If you set caching to true the server is not queried but the type is always
   * set to ITYPE_DIR_AND_BOX
   */
  enum IMAP_TYPE
  parseURL (const KUrl & _url, QString & _box, QString & _section,
            QString & _type, QString & _uid, QString & _validity,
            QString & _hierarchyDelimiter, QString & _info,
            bool cache = false);
  QString getMimeType (enum IMAP_TYPE);

  bool makeLogin ();

  void outputLineStr (const QString & _str)
  {
    outputLine (_str.toLatin1 (), _str.length());
  }
  void doListEntry (const KUrl & _url, int stretch, imapCache * cache = NULL,
    bool withFlags = false, bool withSubject = false);

  /**
   * Send a list entry (folder) to the application
   * If @p appendPath is true the foldername will be appended
   * to the path of @p url
   */
  void doListEntry (const KUrl & url, const QString & myBox,
                    const imapList & item, bool appendPath = true);

  /** Send an ACL command which is identified by @p command */
  void specialACLCommand( int command, QDataStream& stream );

  /** Send an annotation command which is identified by @p command */
  void specialAnnotateMoreCommand( int command, QDataStream& stream );
  void specialQuotaCommand( int command, QDataStream& stream );

  /** Search current folder, the search string is passed as SECTION */
  void specialSearchCommand( QDataStream& );

private:

  // This method behaves like the above method but takes an already encoded url,
  // so you don't have to call KUrl::url() for every mail.
  void doListEntry (const QString & encodedUrl, int stretch, imapCache * cache = NULL,
    bool withFlags = false, bool withSubject = false);

  QString myHost, myUser, myPass, myAuth, myTLS;
  int myPort;
  bool mySSL;

  bool relayEnabled, cacheOutput, decodeContent;
  QByteArray outputCache;
  QBuffer outputBuffer;
  int outputBufferIndex;
  KIO::filesize_t mProcessedSize;

  char readBuffer[IMAP_BUFFER];
  ssize_t readBufferLen;
  int readSize;
  QDateTime mTimeOfLastNoop;
};

#endif
