/**********************************************************************
 *
 *   imapcommand.cc  - IMAP4rev1 command handler
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

#include "imapcommand.h"
#include "rfcdecoder.h"

/*#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <fcntl.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>

#include <tqregexp.h>
#include <tqbuffer.h>

#include <kprotocolmanager.h>
#include <ksock.h>
#include <kdebug.h>
#include <kinstance.h>
#include <kio/connection.h>
#include <kio/slaveinterface.h>
#include <kio/passdlg.h>
#include <klocale.h> */

imapCommand::imapCommand ()
{
  mComplete = false;
  mId = TQString::null;
}

imapCommand::imapCommand (const TQString & command, const TQString & parameter)
//  aCommand(NULL),
//  mResult(NULL),
//  mParameter(NULL)
{
  mComplete = false;
  aCommand = command;
  aParameter = parameter;
  mId = TQString::null;
}

bool
imapCommand::isComplete ()
{
  return mComplete;
}

const TQString &
imapCommand::result ()
{
  return mResult;
}

const TQString &
imapCommand::resultInfo ()
{
  return mResultInfo;
}

const TQString &
imapCommand::id ()
{
  return mId;
}

const TQString &
imapCommand::parameter ()
{
  return aParameter;
}

const TQString &
imapCommand::command ()
{
  return aCommand;
}

void
imapCommand::setId (const TQString & id)
{
  if (mId.isEmpty ())
    mId = id;
}

void
imapCommand::setComplete ()
{
  mComplete = true;
}

void
imapCommand::setResult (const TQString & result)
{
  mResult = result;
}

void
imapCommand::setResultInfo (const TQString & result)
{
  mResultInfo = result;
}

void
imapCommand::setCommand (const TQString & command)
{
  aCommand = command;
}

void
imapCommand::setParameter (const TQString & parameter)
{
  aParameter = parameter;
}

const QString
imapCommand::getStr ()
{
  if (parameter().isEmpty())
    return id() + " " + command() + "\r\n";
  else
    return id() + " " + command() + " " + parameter() + "\r\n";
}

imapCommand *
imapCommand::clientNoop ()
{
  return new imapCommand ("NOOP", "");
}

imapCommand *
imapCommand::clientFetch (ulong uid, const TQString & fields, bool nouid)
{
  return clientFetch (uid, uid, fields, nouid);
}

imapCommand *
imapCommand::clientFetch (ulong fromUid, ulong toUid, const TQString & fields,
                          bool nouid)
{
  TQString uid = TQString::number(fromUid);

  if (fromUid != toUid)
  {
    uid += ":";
    if (toUid < fromUid)
      uid += "*";
    else
      uid += TQString::number(toUid);
  }
  return clientFetch (uid, fields, nouid);
}

imapCommand *
imapCommand::clientFetch (const TQString & sequence, const TQString & fields,
                          bool nouid)
{
  return new imapCommand (nouid ? "FETCH" : "UID FETCH",
                          sequence + " (" + fields + ")");
}

imapCommand *
imapCommand::clientList (const TQString & reference, const TQString & path,
                         bool lsub)
{
  return new imapCommand (lsub ? "LSUB" : "LIST",
                          TQString ("\"") + rfcDecoder::toIMAP (reference) +
                          "\" \"" + rfcDecoder::toIMAP (path) + "\"");
}

imapCommand *
imapCommand::clientSelect (const TQString & path, bool examine)
{
  Q_UNUSED(examine);
  /** @note We use always SELECT, because UW-IMAP doesn't check for new mail, when
     used with the "mbox driver" and the folder is opened with EXAMINE
     and Courier can't append to a mailbox that is in EXAMINE state */
  return new imapCommand ("SELECT",
                          TQString ("\"") + rfcDecoder::toIMAP (path) + "\"");
}

imapCommand *
imapCommand::clientClose()
{
  return new imapCommand("CLOSE", "");
}

imapCommand *
imapCommand::clientCopy (const TQString & box, const TQString & sequence,
                         bool nouid)
{
  return new imapCommand (nouid ? "COPY" : "UID COPY",
                          sequence + " \"" + rfcDecoder::toIMAP (box) + "\"");
}

imapCommand *
imapCommand::clientAppend (const TQString & box, const TQString & flags,
                           ulong size)
{
  return new imapCommand ("APPEND",
                          "\"" + rfcDecoder::toIMAP (box) + "\" " +
                          ((flags.isEmpty()) ? "" : ("(" + flags + ") ")) +
                          "{" + TQString::number(size) + "}");
}

imapCommand *
imapCommand::clientStatus (const TQString & path, const TQString & parameters)
{
  return new imapCommand ("STATUS",
                          TQString ("\"") + rfcDecoder::toIMAP (path) +
                          "\" (" + parameters + ")");
}

imapCommand *
imapCommand::clientCreate (const TQString & path)
{
  return new imapCommand ("CREATE",
                          TQString ("\"") + rfcDecoder::toIMAP (path) + "\"");
}

imapCommand *
imapCommand::clientDelete (const TQString & path)
{
  return new imapCommand ("DELETE",
                          TQString ("\"") + rfcDecoder::toIMAP (path) + "\"");
}

imapCommand *
imapCommand::clientSubscribe (const TQString & path)
{
  return new imapCommand ("SUBSCRIBE",
                          TQString ("\"") + rfcDecoder::toIMAP (path) + "\"");
}

imapCommand *
imapCommand::clientUnsubscribe (const TQString & path)
{
  return new imapCommand ("UNSUBSCRIBE",
                          TQString ("\"") + rfcDecoder::toIMAP (path) + "\"");
}

imapCommand *
imapCommand::clientExpunge ()
{
  return new imapCommand ("EXPUNGE", TQString (""));
}

imapCommand *
imapCommand::clientRename (const TQString & src, const TQString & dest)
{
  return new imapCommand ("RENAME",
                          TQString ("\"") + rfcDecoder::toIMAP (src) +
                          "\" \"" + rfcDecoder::toIMAP (dest) + "\"");
}

imapCommand *
imapCommand::clientSearch (const TQString & search, bool nouid)
{
  return new imapCommand (nouid ? "SEARCH" : "UID SEARCH", search);
}

imapCommand *
imapCommand::clientStore (const TQString & set, const TQString & item,
                          const TQString & data, bool nouid)
{
  return new imapCommand (nouid ? "STORE" : "UID STORE",
                          set + " " + item + " (" + data + ")");
}

imapCommand *
imapCommand::clientLogout ()
{
  return new imapCommand ("LOGOUT", "");
}

imapCommand *
imapCommand::clientStartTLS ()
{
  return new imapCommand ("STARTTLS", "");
}

imapCommand *
imapCommand::clientSetACL( const TQString& box, const TQString& user, const TQString& acl )
{
  return new imapCommand ("SETACL", TQString("\"") + rfcDecoder::toIMAP (box)
                          + "\" \"" + rfcDecoder::toIMAP (user)
                          + "\" \"" + rfcDecoder::toIMAP (acl) + "\"");
}

imapCommand *
imapCommand::clientDeleteACL( const TQString& box, const TQString& user )
{
  return new imapCommand ("DELETEACL", TQString("\"") + rfcDecoder::toIMAP (box)
                          + "\" \"" + rfcDecoder::toIMAP (user)
                          + "\"");
}

imapCommand *
imapCommand::clientGetACL( const TQString& box )
{
  return new imapCommand ("GETACL", TQString("\"") + rfcDecoder::toIMAP (box)
                          + "\"");
}

imapCommand *
imapCommand::clientListRights( const TQString& box, const TQString& user )
{
  return new imapCommand ("LISTRIGHTS", TQString("\"") + rfcDecoder::toIMAP (box)
                          + "\" \"" + rfcDecoder::toIMAP (user)
                          + "\"");
}

imapCommand *
imapCommand::clientMyRights( const TQString& box )
{
  return new imapCommand ("MYRIGHTS", TQString("\"") + rfcDecoder::toIMAP (box)
                          + "\"");
}

imapCommand *
imapCommand::clientSetAnnotation( const TQString& box, const TQString& entry, const TQMap<TQString, TQString>& attributes )
{
  TQString parameter = TQString("\"") + rfcDecoder::toIMAP (box)
                      + "\" \"" + rfcDecoder::toIMAP (entry)
                      + "\" (";
  for( TQMap<TQString,TQString>::ConstIterator it = attributes.begin(); it != attributes.end(); ++it ) {
    parameter += "\"";
    parameter += rfcDecoder::toIMAP (it.key());
    parameter += "\" \"";
    parameter += rfcDecoder::toIMAP (it.data());
    parameter += "\" ";
  }
  // Turn last space into a ')'
  parameter[parameter.length()-1] = ')';

  return new imapCommand ("SETANNOTATION", parameter);
}

imapCommand *
imapCommand::clientGetAnnotation( const TQString& box, const TQString& entry, const TQStringList& attributeNames )
{
  TQString parameter = TQString("\"") + rfcDecoder::toIMAP (box)
                          + "\" \"" + rfcDecoder::toIMAP (entry)
                          + "\" ";
  if ( attributeNames.count() == 1 )
    parameter += "\"" + rfcDecoder::toIMAP (attributeNames.first()) + '"';
  else {
    parameter += '(';
    for( TQStringList::ConstIterator it = attributeNames.begin(); it != attributeNames.end(); ++it ) {
      parameter += "\"" + rfcDecoder::toIMAP (*it) + "\" ";
    }
    // Turn last space into a ')'
    parameter[parameter.length()-1] = ')';
  }
  return new imapCommand ("GETANNOTATION", parameter);
}

imapCommand *
imapCommand::clientNamespace()
{
  return new imapCommand("NAMESPACE", "");
}

imapCommand *
imapCommand::clientGetQuotaroot( const TQString& box )
{
  TQString parameter = TQString("\"") + rfcDecoder::toIMAP (box) + '"';
  return new imapCommand ("GEQUOTAROOT", parameter);
}

imapCommand *
imapCommand::clientCustom( const TQString& command, const TQString& arguments )
{
  return new imapCommand (command, arguments);
}

