/**********************************************************************
 *
 *   imapparser.cc  - IMAP4rev1 Parser
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

#include <config.h>

#include "imapparser.h"
#include "imapinfo.h"
#include "mailheader.h"
#include "mimeheader.h"
#include "mailaddress.h"

#include <sys/types.h>

#include <stdlib.h>
#include <unistd.h>
#include <QList>
#include <Q3PtrList>

#ifdef HAVE_LIBSASL2
extern "C" {
#include <sasl/sasl.h>
}
#endif

#include <QRegExp>
#include <QBuffer>
#include <QString>
#include <QStringList>

#include <kascii.h>
#include <kdebug.h>
#include <kcodecs.h>
#include <kglobal.h>
#include <kurl.h>

#include <kimap/rfccodecs.h>
using namespace KIMAP;

#ifdef HAVE_LIBSASL2
static sasl_callback_t callbacks[] = {
    { SASL_CB_ECHOPROMPT, NULL, NULL },
    { SASL_CB_NOECHOPROMPT, NULL, NULL },
    { SASL_CB_GETREALM, NULL, NULL },
    { SASL_CB_USER, NULL, NULL },
    { SASL_CB_AUTHNAME, NULL, NULL },
    { SASL_CB_PASS, NULL, NULL },
    { SASL_CB_CANON_USER, NULL, NULL },
    { SASL_CB_LIST_END, NULL, NULL }
};
#endif

imapParser::imapParser ()
{
  sentQueue.setAutoDelete (false);
  completeQueue.setAutoDelete (true);
  currentState = ISTATE_NO;
  commandCounter = 0;
  lastHandled = 0;
}

imapParser::~imapParser ()
{
  delete lastHandled;
  lastHandled = 0;
}

imapCommand *
imapParser::doCommand (imapCommand * aCmd)
{
  int pl = 0;
  sendCommand (aCmd);
  while (pl != -1 && !aCmd->isComplete ()) {
    while ((pl = parseLoop ()) == 0)
     ;
  }

  return aCmd;
}

imapCommand *
imapParser::sendCommand (imapCommand * aCmd)
{
  aCmd->setId (QString::number(commandCounter++));
  sentQueue.append (aCmd);

  continuation.resize(0);
  const QString& command = aCmd->command();

  if (command == "SELECT" || command == "EXAMINE")
  {
     // we need to know which box we are selecting
    parseString p;
    p.fromString(aCmd->parameter());
    currentBox = parseOneWord(p);
    kDebug(7116) <<"imapParser::sendCommand - setting current box to" << currentBox;
  }
  else if (command == "CLOSE")
  {
     // we no longer have a box open
    currentBox.clear();
  }
  else if (command.contains("SEARCH")
           || command == "GETACL"
           || command == "LISTRIGHTS"
           || command == "MYRIGHTS"
           || command == "GETANNOTATION"
           || command == "NAMESPACE"
           || command == "GETQUOTAROOT"
           || command == "GETQUOTA")
  {
    lastResults.clear ();
  }
  else if (command == "LIST"
           || command == "LSUB")
  {
    listResponses.clear ();
  }
  parseWriteLine (aCmd->getStr ());
  return aCmd;
}

bool
imapParser::clientLogin (const QString & aUser, const QString & aPass,
  QString & resultInfo)
{
  imapCommand *cmd;
  bool retVal = false;

  cmd =
    doCommand (new
               imapCommand ("LOGIN", "\"" + KIMAP::quoteIMAP(aUser)
               + "\" \"" + KIMAP::quoteIMAP(aPass) + "\""));

  if (cmd->result () == "OK")
  {
    currentState = ISTATE_LOGIN;
    retVal = true;
  }
  resultInfo = cmd->resultInfo();
  completeQueue.removeRef (cmd);

  return retVal;
}

#ifdef HAVE_LIBSASL2
static bool sasl_interact( KIO::SlaveBase *slave, KIO::AuthInfo &ai, void *in )
{
  kDebug(7116) <<"sasl_interact";
  sasl_interact_t *interact = ( sasl_interact_t * ) in;

  //some mechanisms do not require username && pass, so it doesn't need a popup
  //window for getting this info
  for ( ; interact->id != SASL_CB_LIST_END; interact++ ) {
    if ( interact->id == SASL_CB_AUTHNAME ||
         interact->id == SASL_CB_PASS ) {

      if ( ai.username.isEmpty() || ai.password.isEmpty() ) {
        if (!slave->openPasswordDialog(ai))
          return false;
      }
      break;
    }
  }

  interact = ( sasl_interact_t * ) in;
  while( interact->id != SASL_CB_LIST_END ) {
    kDebug(7116) <<"SASL_INTERACT id:" << interact->id;
    switch( interact->id ) {
      case SASL_CB_USER:
      case SASL_CB_AUTHNAME:
        kDebug(7116) <<"SASL_CB_[USER|AUTHNAME]: '" << ai.username <<"'";
        interact->result = strdup( ai.username.toUtf8() );
        interact->len = strlen( (const char *) interact->result );
        break;
      case SASL_CB_PASS:
        kDebug(7116) <<"SASL_CB_PASS: [hidden]";
        interact->result = strdup( ai.password.toUtf8() );
        interact->len = strlen( (const char *) interact->result );
        break;
      default:
        interact->result = 0;
        interact->len = 0;
        break;
    }
    interact++;
  }
  return true;
}
#endif

bool
imapParser::clientAuthenticate ( KIO::SlaveBase *slave, KIO::AuthInfo &ai,
  const QString & aFQDN, const QString & aAuth, bool isSSL, QString & resultInfo)
{
  bool retVal = false;
#ifdef HAVE_LIBSASL2
  int result;
  sasl_conn_t *conn = 0;
  sasl_interact_t *client_interact = 0;
  const char *out = 0;
  uint outlen = 0;
  const char *mechusing = 0;
  QByteArray tmp, challenge;

  kDebug(7116) <<"aAuth:" << aAuth <<" FQDN:" << aFQDN <<" isSSL:" << isSSL;

  // see if server supports this authenticator
  if (!hasCapability ("AUTH=" + aAuth))
    return false;

//  result = sasl_client_new( isSSL ? "imaps" : "imap",
  result = sasl_client_new( "imap", /* FIXME: with cyrus-imapd, even imaps' digest-uri
                                       must be 'imap'. I don't know if it's good or bad. */
                       aFQDN.toLatin1(),
                       0, 0, callbacks, 0, &conn );

  if ( result != SASL_OK ) {
    kDebug(7116) <<"sasl_client_new failed with:" << result;
    resultInfo = QString::fromUtf8( sasl_errdetail( conn ) );
    return false;
  }

  do {
    result = sasl_client_start(conn, aAuth.toLatin1(), &client_interact,
                       hasCapability("SASL-IR") ? &out : 0, &outlen, &mechusing);

    if ( result == SASL_INTERACT ) {
      if ( !sasl_interact( slave, ai, client_interact ) ) {
        sasl_dispose( &conn );
        return false;
      }
    }
  } while ( result == SASL_INTERACT );

  if ( result != SASL_CONTINUE && result != SASL_OK ) {
    kDebug(7116) <<"sasl_client_start failed with:" << result;
    resultInfo = QString::fromUtf8( sasl_errdetail( conn ) );
    sasl_dispose( &conn );
    return false;
  }
  imapCommand *cmd;

  tmp = QByteArray::fromRawData( out, outlen );
  KCodecs::base64Encode( tmp, challenge );
  tmp.clear();
  // then lets try it
  QString firstCommand = aAuth;
  if ( !challenge.isEmpty() ) {
    firstCommand += " ";
    firstCommand += QString::fromLatin1( challenge.data(), challenge.size() );
  }
  cmd = sendCommand (new imapCommand ("AUTHENTICATE", firstCommand.toLatin1()));

  while ( true )
  {
    //read the next line
    while (parseLoop() == 0);
    if ( cmd->isComplete() ) break;

    if (!continuation.isEmpty())
    {
//      kDebug(7116) <<"S:" << QCString(continuation.data(),continuation.size()+1);
      if ( continuation.size() > 4 ) {
        tmp = QByteArray::fromRawData( continuation.data() + 2, continuation.size() - 4 );
        KCodecs::base64Decode( tmp, challenge );
//        kDebug(7116) <<"S-1:" << QCString(challenge.data(),challenge.size()+1);
        tmp.clear();
      }

      do {
        result = sasl_client_step(conn, challenge.isEmpty() ? 0 : challenge.data(),
                                  challenge.size(),
                                  &client_interact,
                                  &out, &outlen);

        if (result == SASL_INTERACT) {
          if ( !sasl_interact( slave, ai, client_interact ) ) {
            sasl_dispose( &conn );
            return false;
          }
        }
      } while ( result == SASL_INTERACT );

      if ( result != SASL_CONTINUE && result != SASL_OK ) {
        kDebug(7116) <<"sasl_client_step failed with:" << result;
        resultInfo = QString::fromUtf8( sasl_errdetail( conn ) );
        sasl_dispose( &conn );
        return false;
      }

      tmp = QByteArray::fromRawData( out, outlen );
//      kDebug(7116) <<"C-1:" << QCString(tmp.data(),tmp.size()+1);
      KCodecs::base64Encode( tmp, challenge );
      tmp.clear();
//      kDebug(7116) <<"C:" << QCString(challenge.data(),challenge.size()+1);
      parseWriteLine (challenge);
      continuation.resize(0);
    }
  }

  if (cmd->result () == "OK")
  {
    currentState = ISTATE_LOGIN;
    retVal = true;
  }
  resultInfo = cmd->resultInfo();
  completeQueue.removeRef (cmd);

  sasl_dispose( &conn ); //we don't use sasl_en/decode(), so it's safe to dispose the connection.
#endif //HAVE_LIBSASL2
  return retVal;
}

void
imapParser::parseUntagged (parseString & result)
{
  //kDebug(7116) <<"imapParser::parseUntagged - '" << result.cstr() <<"'";

  parseOneWord(result);        // *
  QByteArray what = parseLiteral (result); // see whats coming next

  switch (what[0])
  {
    //the status responses
  case 'B':                    // BAD or BYE
    if (qstrncmp(what, "BAD", what.size()) == 0)
    {
      parseResult (what, result);
    }
    else if (qstrncmp(what, "BYE", what.size()) == 0)
    {
      parseResult (what, result);
      if ( sentQueue.count() ) {
        // BYE that interrupts a command -> copy the reason for it
        imapCommand *current = sentQueue.at (0);
        current->setResultInfo(result.cstr());
      }
      currentState = ISTATE_NO;
    }
    break;

  case 'N':                    // NO
    if (what[1] == 'O' && what.size() == 2)
    {
      parseResult (what, result);
    }
    else if (qstrncmp(what, "NAMESPACE", what.size()) == 0)
    {
      parseNamespace (result);
    }
    break;

  case 'O':                    // OK
    if (what[1] == 'K' && what.size() == 2)
    {
      parseResult (what, result);
    }
    break;

  case 'P':                    // PREAUTH
    if (qstrncmp(what, "PREAUTH", what.size()) == 0)
    {
      parseResult (what, result);
      currentState = ISTATE_LOGIN;
    }
    break;

    // parse the other responses
  case 'C':                    // CAPABILITY
    if (qstrncmp(what, "CAPABILITY", what.size()) == 0)
    {
      parseCapability (result);
    }
    break;

  case 'F':                    // FLAGS
    if (qstrncmp(what, "FLAGS", what.size()) == 0)
    {
      parseFlags (result);
    }
    break;

  case 'L':                    // LIST or LSUB or LISTRIGHTS
    if (qstrncmp(what, "LIST", what.size()) == 0)
    {
      parseList (result);
    }
    else if (qstrncmp(what, "LSUB", what.size()) == 0)
    {
      parseLsub (result);
    }
    else if (qstrncmp(what, "LISTRIGHTS", what.size()) == 0)
    {
      parseListRights (result);
    }
    break;

  case 'M': // MYRIGHTS
    if (qstrncmp(what, "MYRIGHTS", what.size()) == 0)
    {
      parseMyRights (result);
    }
    break;
  case 'S':                    // SEARCH or STATUS
    if (qstrncmp(what, "SEARCH", what.size()) == 0)
    {
      parseSearch (result);
    }
    else if (qstrncmp(what, "STATUS", what.size()) == 0)
    {
      parseStatus (result);
    }
    break;

  case 'A': // ACL or ANNOTATION
    if (qstrncmp(what, "ACL", what.size()) == 0)
    {
      parseAcl (result);
    }
    else if (qstrncmp(what, "ANNOTATION", what.size()) == 0)
    {
      parseAnnotation (result);
    }
    break;
  case 'Q': // QUOTA or QUOTAROOT
    if ( what.size() > 5 && qstrncmp(what, "QUOTAROOT", what.size()) == 0)
    {
      parseQuotaRoot( result );
    }
    else if (qstrncmp(what, "QUOTA", what.size()) == 0)
    {
      parseQuota( result );
    }

  default:
    //better be a number
    {
      ulong number;
      bool valid;

      number = what.toUInt(&valid);
      if (valid)
      {
        what = parseLiteral (result);
        switch (what[0])
        {
        case 'E':
          if (qstrncmp(what, "EXISTS", what.size()) == 0)
          {
            parseExists (number, result);
          }
          else if (qstrncmp(what, "EXPUNGE", what.size()) == 0)
          {
            parseExpunge (number, result);
          }
          break;

        case 'F':
          if (qstrncmp(what, "FETCH", what.size()) == 0)
          {
            seenUid.clear();
            parseFetch (number, result);
          }
          break;

        case 'S':
          if (qstrncmp(what, "STORE", what.size()) == 0)  // deprecated store
          {
            seenUid.clear();
            parseFetch (number, result);
          }
          break;

        case 'R':
          if (qstrncmp(what, "RECENT", what.size()) == 0)
          {
            parseRecent (number, result);
          }
          break;
        default:
          break;
        }
      }
    }
    break;
  }                             //switch
}                               //func


void
imapParser::parseResult (QByteArray & result, parseString & rest,
  const QString & command)
{
  if (command == "SELECT")
    selectInfo.setReadWrite(true);

  if (rest[0] == '[')
  {
    rest.pos++;
    QByteArray option = parseOneWord(rest, true);

    switch (option[0])
    {
    case 'A':                  // ALERT
      if (option == "ALERT")
      {
        rest.pos = rest.data.indexOf(']', rest.pos) + 1;
        // The alert text is after [ALERT].
        // Is this correct or do we need to care about litterals?
        selectInfo.setAlert( rest.cstr() );
      }
      break;

    case 'N':                  // NEWNAME
      if (option == "NEWNAME")
      {
      }
      break;

    case 'P':                  //PARSE or PERMANENTFLAGS
      if (option == "PARSE")
      {
      }
      else if (option == "PERMANENTFLAGS")
      {
        uint end = rest.data.indexOf(']', rest.pos);
        QByteArray flags(rest.data.data() + rest.pos, end - rest.pos);
        selectInfo.setPermanentFlags (flags);
        rest.pos = end;
      }
      break;

    case 'R':                  //READ-ONLY or READ-WRITE
      if (option == "READ-ONLY")
      {
        selectInfo.setReadWrite (false);
      }
      else if (option == "READ-WRITE")
      {
        selectInfo.setReadWrite (true);
      }
      break;

    case 'T':                  //TRYCREATE
      if (option == "TRYCREATE")
      {
      }
      break;

    case 'U':                  //UIDVALIDITY or UNSEEN
      if (option == "UIDVALIDITY")
      {
        ulong value;
        if (parseOneNumber (rest, value))
          selectInfo.setUidValidity (value);
      }
      else if (option == "UNSEEN")
      {
        ulong value;
        if (parseOneNumber (rest, value))
          selectInfo.setUnseen (value);
      }
      else if (option == "UIDNEXT")
      {
        ulong value;
        if (parseOneNumber (rest, value))
          selectInfo.setUidNext (value);
      }
      else
      break;

    }
    if (rest[0] == ']')
      rest.pos++; //tie off ]
    skipWS (rest);
  }

  if (command.isEmpty())
  {
    // This happens when parsing an intermediate result line (those that start with '*').
    // No state change involved, so we can stop here.
    return;
  }

  switch (command[0].toLatin1 ())
  {
  case 'A':
    if (command == "AUTHENTICATE")
      if (qstrncmp(result, "OK", result.size()) == 0)
        currentState = ISTATE_LOGIN;
    break;

  case 'L':
    if (command == "LOGIN")
      if (qstrncmp(result, "OK", result.size()) == 0)
        currentState = ISTATE_LOGIN;
    break;

  case 'E':
    if (command == "EXAMINE")
    {
      if (qstrncmp(result, "OK", result.size()) == 0)
        currentState = ISTATE_SELECT;
      else
      {
        if (currentState == ISTATE_SELECT)
          currentState = ISTATE_LOGIN;
        currentBox.clear();
      }
      kDebug(7116) <<"imapParser::parseResult - current box is now" << currentBox;
    }
    break;

  case 'S':
    if (command == "SELECT")
    {
      if (qstrncmp(result, "OK", result.size()) == 0)
        currentState = ISTATE_SELECT;
      else
      {
        if (currentState == ISTATE_SELECT)
          currentState = ISTATE_LOGIN;
        currentBox.clear();
      }
      kDebug(7116) <<"imapParser::parseResult - current box is now" << currentBox;
    }
    break;

  default:
    break;
  }

}

void imapParser::parseCapability (parseString & result)
{
  QByteArray data = result.cstr();
  kAsciiToLower( data.data() );
  imapCapabilities = QString::fromLatin1(data).split ( ' ', QString::SkipEmptyParts );
}

void imapParser::parseFlags (parseString & result)
{
  selectInfo.setFlags(result.cstr());
}

void imapParser::parseList (parseString & result)
{
  imapList this_one;

  if (result[0] != '(')
    return;                     //not proper format for us

  result.pos++; // tie off (

  this_one.parseAttributes( result );

  result.pos++; // tie off )
  skipWS (result);

  this_one.setHierarchyDelimiter(parseLiteral(result));
  this_one.setName (KIMAP::decodeImapFolderName( parseLiteral(result)));  // decode modified UTF7

  listResponses.append (this_one);
}

void imapParser::parseLsub (parseString & result)
{
  imapList this_one (result.cstr(), *this);
  listResponses.append (this_one);
}

void imapParser::parseListRights (parseString & result)
{
  parseOneWord (result); // skip mailbox name
  parseOneWord (result); // skip user id
  while ( true ) {
    const QByteArray word = parseOneWord (result);
    if ( word.isEmpty() )
      break;
    lastResults.append (word);
  }
}

void imapParser::parseAcl (parseString & result)
{
  parseOneWord (result); // skip mailbox name
  // The result is user1 perm1 user2 perm2 etc. The caller will sort it out.
  while ( !result.isEmpty() ) {
    const QByteArray word = parseLiteral(result);
    if ( word.isEmpty() )
      break;
    lastResults.append (word);
  }
}

void imapParser::parseAnnotation (parseString & result)
{
  parseOneWord (result); // skip mailbox name
  skipWS (result);
  parseOneWord (result); // skip entry name (we know it since we don't allow wildcards in it)
  skipWS (result);
  if (result.isEmpty() || result[0] != '(')
    return;
  result.pos++;
  skipWS (result);
  // The result is name1 value1 name2 value2 etc. The caller will sort it out.
  while ( !result.isEmpty() && result[0] != ')' ) {
    const QByteArray word = parseLiteral (result);
    if ( word.isEmpty() )
      break;
    lastResults.append (word);
  }
}


void imapParser::parseQuota (parseString & result)
{
  // quota_response  ::= "QUOTA" SP astring SP quota_list
  // quota_list      ::= "(" #quota_resource ")"
  // quota_resource  ::= atom SP number SP number
  QByteArray root = parseOneWord( result );
  if ( root.isEmpty() ) {
    lastResults.append( "" );
  } else {
    lastResults.append( root );
  }
  if (result.isEmpty() || result[0] != '(')
    return;
  result.pos++;
  skipWS (result);
  QStringList triplet;
  while ( !result.isEmpty() && result[0] != ')' ) {
    const QByteArray word = parseLiteral(result);
    if ( word.isEmpty() )
      break;
    triplet.append(word);
  }
  lastResults.append( triplet.join(" ") );
}

void imapParser::parseQuotaRoot (parseString & result)
{
  //    quotaroot_response
  //         ::= "QUOTAROOT" SP astring *(SP astring)
  parseOneWord (result); // skip mailbox name
  skipWS (result);
  if ( result.isEmpty() )
    return;
  QStringList roots;
  while ( !result.isEmpty() ) {
    const QByteArray word = parseLiteral (result);
    if ( word.isEmpty() )
      break;
    roots.append (word);
  }
  lastResults.append( roots.join(" ") );
}

void imapParser::parseMyRights (parseString & result)
{
  parseOneWord (result); // skip mailbox name
  Q_ASSERT( lastResults.isEmpty() ); // we can only be called once
  lastResults.append (parseOneWord (result) );
}

void imapParser::parseSearch (parseString & result)
{
  ulong value;

  while (parseOneNumber (result, value))
  {
    lastResults.append (QString::number(value));
  }
}

void imapParser::parseStatus (parseString & inWords)
{
  lastStatus = imapInfo ();

  parseLiteral(inWords);       // swallow the box
  if (inWords[0] != '(')
    return;

  inWords.pos++;
  skipWS (inWords);

  while (!inWords.isEmpty() && inWords[0] != ')')
  {
    ulong value;

    QByteArray label = parseOneWord(inWords);
    if (parseOneNumber (inWords, value))
    {
      if (label == "MESSAGES")
        lastStatus.setCount (value);
      else if (label == "RECENT")
        lastStatus.setRecent (value);
      else if (label == "UIDVALIDITY")
        lastStatus.setUidValidity (value);
      else if (label == "UNSEEN")
        lastStatus.setUnseen (value);
      else if (label == "UIDNEXT")
        lastStatus.setUidNext (value);
    }
  }

  if (inWords[0] == ')')
    inWords.pos++;
  skipWS (inWords);
}

void imapParser::parseExists (ulong value, parseString & result)
{
  selectInfo.setCount (value);
  result.pos = result.data.size();
}

void imapParser::parseExpunge (ulong value, parseString & result)
{
  Q_UNUSED(value);
  Q_UNUSED(result);
}

void imapParser::parseAddressList (parseString & inWords, Q3PtrList<mailAddress>& list)
{
  if (inWords[0] != '(')
  {
    parseOneWord (inWords);     // parse NIL
  }
  else
  {
    inWords.pos++;
    skipWS (inWords);

    while (!inWords.isEmpty () && inWords[0] != ')')
    {
      if (inWords[0] == '(') {
        mailAddress *addr = new mailAddress;
        parseAddress(inWords, *addr);
        list.append(addr);
      } else {
        break;
      }
    }

    if (inWords[0] == ')')
      inWords.pos++;
    skipWS (inWords);
  }
}

const mailAddress& imapParser::parseAddress (parseString & inWords, mailAddress& retVal)
{
  inWords.pos++;
  skipWS (inWords);

  retVal.setFullName(KIMAP::quoteIMAP(parseLiteral(inWords)));
  retVal.setCommentRaw(parseLiteral(inWords));
  retVal.setUser(parseLiteral(inWords));
  retVal.setHost(parseLiteral(inWords));

  if (inWords[0] == ')')
    inWords.pos++;
  skipWS (inWords);

  return retVal;
}

mailHeader * imapParser::parseEnvelope (parseString & inWords)
{
  mailHeader *envelope = 0;

  if (inWords[0] != '(')
    return envelope;
  inWords.pos++;
  skipWS (inWords);

  envelope = new mailHeader;

  //date
  envelope->setDate(parseLiteral(inWords));

  //subject
  envelope->setSubject(parseLiteral(inWords));

  Q3PtrList<mailAddress> list;
  list.setAutoDelete(true);

  //from
  parseAddressList(inWords, list);
  if (!list.isEmpty()) {
	  envelope->setFrom(*list.last());
	  list.clear();
  }

  //sender
  parseAddressList(inWords, list);
  if (!list.isEmpty()) {
	  envelope->setSender(*list.last());
	  list.clear();
  }

  //reply-to
  parseAddressList(inWords, list);
  if (!list.isEmpty()) {
	  envelope->setReplyTo(*list.last());
	  list.clear();
  }

  //to
  parseAddressList (inWords, envelope->to());

  //cc
  parseAddressList (inWords, envelope->cc());

  //bcc
  parseAddressList (inWords, envelope->bcc());

  //in-reply-to
  envelope->setInReplyTo(parseLiteral(inWords));

  //message-id
  envelope->setMessageId(parseLiteral(inWords));

  // see if we have more to come
  while (!inWords.isEmpty () && inWords[0] != ')')
  {
    //eat the extensions to this part
    if (inWords[0] == '(')
      parseSentence (inWords);
    else
      parseLiteral (inWords);
  }

  if (inWords[0] == ')')
    inWords.pos++;
  skipWS (inWords);

  return envelope;
}

// parse parameter pairs into a dictionary
// caller must clean up the dictionary items
Q3AsciiDict < QString > imapParser::parseDisposition (parseString & inWords)
{
  QByteArray disposition;
  Q3AsciiDict < QString > retVal (17, false);

  // return value is a shallow copy
  retVal.setAutoDelete (false);

  if (inWords[0] != '(')
  {
    //disposition only
    disposition = parseOneWord (inWords);
  }
  else
  {
    inWords.pos++;
    skipWS (inWords);

    //disposition
    disposition = parseOneWord (inWords);

    retVal = parseParameters (inWords);
    if (inWords[0] != ')')
      return retVal;
    inWords.pos++;
    skipWS (inWords);
  }

  if (!disposition.isEmpty ())
  {
    retVal.insert ("content-disposition", new QString(disposition));
  }

  return retVal;
}

// parse parameter pairs into a dictionary
// caller must clean up the dictionary items
Q3AsciiDict < QString > imapParser::parseParameters (parseString & inWords)
{
  Q3AsciiDict < QString > retVal (17, false);

  // return value is a shallow copy
  retVal.setAutoDelete (false);

  if (inWords[0] != '(')
  {
    //better be NIL
    parseOneWord (inWords);
  }
  else
  {
    inWords.pos++;
    skipWS (inWords);

    while (!inWords.isEmpty () && inWords[0] != ')')
    {
      const QByteArray l1 = parseLiteral(inWords);
      const QByteArray l2 = parseLiteral(inWords);
      retVal.insert (l1, new QString(l2));
    }

    if (inWords[0] != ')')
      return retVal;
    inWords.pos++;
    skipWS (inWords);
  }

  return retVal;
}

mimeHeader * imapParser::parseSimplePart (parseString & inWords,
  QString & inSection, mimeHeader * localPart)
{
  QByteArray subtype;
  QByteArray typeStr;
  Q3AsciiDict < QString > parameters (17, false);
  ulong size;

  parameters.setAutoDelete (true);

  if (inWords[0] != '(')
    return 0;

  if (!localPart)
    localPart = new mimeHeader;

  localPart->setPartSpecifier (inSection);

  inWords.pos++;
  skipWS (inWords);

  //body type
  typeStr = parseLiteral(inWords);

  //body subtype
  subtype = parseLiteral(inWords);

  localPart->setType (typeStr + "/" + subtype);

  //body parameter parenthesized list
  parameters = parseParameters (inWords);
  {
    Q3AsciiDictIterator < QString > it (parameters);

    while (it.current ())
    {
      localPart->setTypeParm (it.currentKey (), *(it.current ()));
      ++it;
    }
    parameters.clear ();
  }

  //body id
  localPart->setID (parseLiteral(inWords));

  //body description
  localPart->setDescription (parseLiteral(inWords));

  //body encoding
  localPart->setEncoding (parseLiteral(inWords));

  //body size
  if (parseOneNumber (inWords, size))
    localPart->setLength (size);

  // type specific extensions
  if (localPart->getType().toUpper() == "MESSAGE/RFC822")
  {
    //envelope structure
    mailHeader *envelope = parseEnvelope (inWords);

    //body structure
    parseBodyStructure (inWords, inSection, envelope);

    localPart->setNestedMessage (envelope);

    //text lines
    ulong lines;
    parseOneNumber (inWords, lines);
  }
  else
  {
    if (typeStr ==  "TEXT")
    {
      //text lines
      ulong lines;
      parseOneNumber (inWords, lines);
    }

    // md5
    parseLiteral(inWords);

    // body disposition
    parameters = parseDisposition (inWords);
    {
      QString *disposition = parameters["content-disposition"];

      if (disposition)
        localPart->setDisposition (disposition->toAscii ());
      parameters.remove ("content-disposition");
      Q3AsciiDictIterator < QString > it (parameters);
      while (it.current ())
      {
        localPart->setDispositionParm (it.currentKey (),
                                       *(it.current ()));
        ++it;
      }
      parameters.clear ();
    }

    // body language
    parseSentence (inWords);
  }

  // see if we have more to come
  while (!inWords.isEmpty () && inWords[0] != ')')
  {
    //eat the extensions to this part
    if (inWords[0] == '(')
      parseSentence (inWords);
    else
      parseLiteral(inWords);
  }

  if (inWords[0] == ')')
    inWords.pos++;
  skipWS (inWords);

  return localPart;
}

mimeHeader * imapParser::parseBodyStructure (parseString & inWords,
  QString & inSection, mimeHeader * localPart)
{
  bool init = false;
  if (inSection.isEmpty())
  {
    // first run
    init = true;
    // assume one part
    inSection = "1";
  }
  int section = 0;

  if (inWords[0] != '(')
  {
    // skip ""
    parseOneWord (inWords);
    return 0;
  }
  inWords.pos++;
  skipWS (inWords);

  if (inWords[0] == '(')
  {
    QByteArray subtype;
    Q3AsciiDict < QString > parameters (17, false);
    QString outSection;
    parameters.setAutoDelete (true);
    if (!localPart)
      localPart = new mimeHeader;
    else
    {
      // might be filled from an earlier run
      localPart->clearNestedParts ();
      localPart->clearTypeParameters ();
      localPart->clearDispositionParameters ();
      // an envelope was passed in so this is the multipart header
      outSection = inSection + ".HEADER";
    }
    if (inWords[0] == '(' && init)
      inSection = "0";

    // set the section
    if ( !outSection.isEmpty() ) {
      localPart->setPartSpecifier(outSection);
    } else {
      localPart->setPartSpecifier(inSection);
    }

    // is multipart (otherwise its a simplepart and handled later)
    while (inWords[0] == '(')
    {
      outSection = QString::number(++section);
      if (!init)
        outSection = inSection + "." + outSection;
      mimeHeader *subpart = parseBodyStructure (inWords, outSection, 0);
      localPart->addNestedPart (subpart);
    }

    // fetch subtype
    subtype = parseOneWord (inWords);

    localPart->setType ("MULTIPART/" + subtype);

    // fetch parameters
    parameters = parseParameters (inWords);
    {
      Q3AsciiDictIterator < QString > it (parameters);

      while (it.current ())
      {
        localPart->setTypeParm (it.currentKey (), *(it.current ()));
        ++it;
      }
      parameters.clear ();
    }

    // body disposition
    parameters = parseDisposition (inWords);
    {
      QString *disposition = parameters["content-disposition"];

      if (disposition)
        localPart->setDisposition (disposition->toAscii ());
      parameters.remove ("content-disposition");
      Q3AsciiDictIterator < QString > it (parameters);
      while (it.current ())
      {
        localPart->setDispositionParm (it.currentKey (),
                                       *(it.current ()));
        ++it;
      }
      parameters.clear ();
    }

    // body language
    parseSentence (inWords);

  }
  else
  {
    // is simple part
    inWords.pos--;
    inWords.data[inWords.pos] = '('; //fake a sentence
    if ( localPart )
      inSection = inSection + ".1";
    localPart = parseSimplePart (inWords, inSection, localPart);
    inWords.pos--;
    inWords.data[inWords.pos] = ')'; //remove fake
  }

  // see if we have more to come
  while (!inWords.isEmpty () && inWords[0] != ')')
  {
    //eat the extensions to this part
    if (inWords[0] == '(')
      parseSentence (inWords);
    else
      parseLiteral(inWords);
  }

  if (inWords[0] == ')')
    inWords.pos++;
  skipWS (inWords);

  return localPart;
}

void imapParser::parseBody (parseString & inWords)
{
  // see if we got a part specifier
  if (inWords[0] == '[')
  {
    QByteArray specifier;
    QByteArray label;
    inWords.pos++;

    specifier = parseOneWord (inWords, true);

    if (inWords[0] == '(')
    {
      inWords.pos++;

      while (!inWords.isEmpty () && inWords[0] != ')')
      {
        label = parseOneWord (inWords);
      }

      if (inWords[0] == ')')
        inWords.pos++;
    }
    if (inWords[0] == ']')
      inWords.pos++;
    skipWS (inWords);

    // parse the header
    if (qstrncmp(specifier, "0", specifier.size()) == 0)
    {
      mailHeader *envelope = 0;
      if (lastHandled)
        envelope = lastHandled->getHeader ();

      if (!envelope || seenUid.isEmpty ())
      {
        kDebug(7116) <<"imapParser::parseBody - discarding" << envelope << seenUid.toAscii ();
        // don't know where to put it, throw it away
        parseLiteral(inWords, true);
      }
      else
      {
        kDebug(7116) <<"imapParser::parseBody - reading" << envelope << seenUid.toAscii ();
        // fill it up with data
        QString theHeader = parseLiteral(inWords, true);
        mimeIOQString myIO;

        myIO.setString (theHeader);
        envelope->parseHeader (myIO);

      }
    }
    else if (qstrncmp(specifier, "HEADER.FIELDS", specifier.size()) == 0)
    {
      // BODY[HEADER.FIELDS (References)] {n}
      //kDebug(7116) <<"imapParser::parseBody - HEADER.FIELDS:"
      // << QCString(label.data(), label.size()+1);
      if (qstrncmp(label, "REFERENCES", label.size()) == 0)
      {
       mailHeader *envelope = 0;
       if (lastHandled)
         envelope = lastHandled->getHeader ();

       if (!envelope || seenUid.isEmpty ())
       {
         kDebug(7116) <<"imapParser::parseBody - discarding" << envelope << seenUid.toAscii ();
         // don't know where to put it, throw it away
         parseLiteral (inWords, true);
       }
       else
       {
         QByteArray references = parseLiteral(inWords, true);
         int start = references.indexOf ('<');
         int end = references.lastIndexOf ('>');
         if (start < end)
           references = references.mid (start, end - start + 1);
         envelope->setReferences(references.simplified());
       }
      }
      else
      { // not a header we care about throw it away
        parseLiteral(inWords, true);
      }
    }
    else
    {
      if (specifier.contains(".MIME") )
      {
        mailHeader *envelope = new mailHeader;
        QString theHeader = parseLiteral(inWords, false);
        mimeIOQString myIO;
        myIO.setString (theHeader);
        envelope->parseHeader (myIO);
        if (lastHandled)
          lastHandled->setHeader (envelope);
        return;
      }
      // throw it away
      kDebug(7116) <<"imapParser::parseBody - discarding" << seenUid.toAscii ();
      parseLiteral(inWords, true);
    }

  }
  else // no part specifier
  {
    mailHeader *envelope = 0;
    if (lastHandled)
      envelope = lastHandled->getHeader ();

    if (!envelope || seenUid.isEmpty ())
    {
      kDebug(7116) <<"imapParser::parseBody - discarding" << envelope << seenUid.toAscii ();
      // don't know where to put it, throw it away
      parseSentence (inWords);
    }
    else
    {
      kDebug(7116) <<"imapParser::parseBody - reading" << envelope << seenUid.toAscii ();
      // fill it up with data
      QString section;
      mimeHeader *body = parseBodyStructure (inWords, section, envelope);
      if (body != envelope)
        delete body;
    }
  }
}

void imapParser::parseFetch (ulong /* value */, parseString & inWords)
{
  if (inWords[0] != '(')
    return;
  inWords.pos++;
  skipWS (inWords);

  delete lastHandled;
  lastHandled = 0;

  while (!inWords.isEmpty () && inWords[0] != ')')
  {
    if (inWords[0] == '(')
      parseSentence (inWords);
    else
    {
      const QByteArray word = parseLiteral(inWords, false, true);

      switch (word[0])
      {
      case 'E':
        if (word == "ENVELOPE")
        {
          mailHeader *envelope = 0;

          if (lastHandled)
            envelope = lastHandled->getHeader ();
          else
            lastHandled = new imapCache();

          if (envelope && !envelope->getMessageId ().isEmpty ())
          {
            // we have seen this one already
            // or don't know where to put it
            parseSentence (inWords);
          }
          else
          {
            envelope = parseEnvelope (inWords);
            if (envelope)
            {
              envelope->setPartSpecifier (seenUid + ".0");
              lastHandled->setHeader (envelope);
              lastHandled->setUid (seenUid.toULong ());
            }
          }
        }
        break;

      case 'B':
        if (word == "BODY")
        {
          parseBody (inWords);
        }
        else if (word == "BODY[]" )
        {
          // Do the same as with "RFC822"
          parseLiteral(inWords, true);
        }
        else if (word == "BODYSTRUCTURE")
        {
          mailHeader *envelope = 0;

          if (lastHandled)
            envelope = lastHandled->getHeader ();

          // fill it up with data
          QString section;
          mimeHeader *body =
            parseBodyStructure (inWords, section, envelope);
          QByteArray data;
          QDataStream stream( &data, QIODevice::WriteOnly );
          body->serialize(stream);
          parseRelay(data);

          delete body;
        }
        break;

      case 'U':
        if (word == "UID")
        {
          seenUid = parseOneWord(inWords);
          mailHeader *envelope = 0;
          if (lastHandled)
            envelope = lastHandled->getHeader ();
          else
            lastHandled = new imapCache();

          if (seenUid.isEmpty ())
          {
            // unknown what to do
            kDebug(7116) <<"imapParser::parseFetch - UID empty";
          }
          else
          {
            lastHandled->setUid (seenUid.toULong ());
          }
          if (envelope)
            envelope->setPartSpecifier (seenUid);
        }
        break;

      case 'R':
        if (word == "RFC822.SIZE")
        {
          ulong size;
          parseOneNumber (inWords, size);

          if (!lastHandled) lastHandled = new imapCache();
          lastHandled->setSize (size);
        }
        else if (word.startsWith("RFC822"))
        {
          // might be RFC822 RFC822.TEXT RFC822.HEADER
          parseLiteral(inWords, true);
        }
        break;

      case 'I':
        if (word == "INTERNALDATE")
        {
          const QByteArray date = parseOneWord(inWords);
          if (!lastHandled) lastHandled = new imapCache();
          lastHandled->setDate(date);
        }
        break;

      case 'F':
        if (word == "FLAGS")
        {
	  //kDebug(7116) <<"GOT FLAGS" << inWords.cstr();
          if (!lastHandled) lastHandled = new imapCache();
          lastHandled->setFlags (imapInfo::_flags (inWords.cstr()));
        }
        break;

      default:
        parseLiteral(inWords);
        break;
      }
    }
  }

  // see if we have more to come
  while (!inWords.isEmpty () && inWords[0] != ')')
  {
    //eat the extensions to this part
    if (inWords[0] == '(')
      parseSentence (inWords);
    else
      parseLiteral(inWords);
  }

  if (inWords[0] != ')')
    return;
  inWords.pos++;
  skipWS (inWords);
}


// default parser
void imapParser::parseSentence (parseString & inWords)
{
  bool first = true;
  int stack = 0;

  //find the first nesting parentheses

  while (!inWords.isEmpty () && (stack != 0 || first))
  {
    first = false;
    skipWS (inWords);

    unsigned char ch = inWords[0];
    switch (ch)
    {
    case '(':
      inWords.pos++;
      ++stack;
      break;
    case ')':
      inWords.pos++;
      --stack;
      break;
    case '[':
      inWords.pos++;
      ++stack;
      break;
    case ']':
      inWords.pos++;
      --stack;
      break;
    default:
      parseLiteral(inWords);
      skipWS (inWords);
      break;
    }
  }
  skipWS (inWords);
}

void imapParser::parseRecent (ulong value, parseString & result)
{
  selectInfo.setRecent (value);
  result.pos = result.data.size();
}

void imapParser::parseNamespace (parseString & result)
{
  if ( result[0] != '(' )
    return;

  QString delimEmpty;
  if ( namespaceToDelimiter.contains( QString() ) )
    delimEmpty = namespaceToDelimiter[QString()];

  namespaceToDelimiter.clear();
  imapNamespaces.clear();

  // remember what section we're in (user, other users, shared)
  int ns = -1;
  bool personalAvailable = false;
  while ( !result.isEmpty() )
  {
    if ( result[0] == '(' )
    {
      result.pos++; // tie off (
      if ( result[0] == '(' )
      {
        // new namespace section
        result.pos++; // tie off (
        ++ns;
      }
      // namespace prefix
      QString prefix = QString::fromLatin1( parseOneWord( result ) );
      // delimiter
      QString delim = QString::fromLatin1( parseOneWord( result ) );
      kDebug(7116) <<"imapParser::parseNamespace ns='" << prefix <<"',delim='" << delim <<"'";
      if ( ns == 0 )
      {
        // at least one personal ns
        personalAvailable = true;
      }
      QString nsentry = QString::number( ns ) + "=" + prefix + "=" + delim;
      imapNamespaces.append( nsentry );
      if ( prefix.right( 1 ) == delim ) {
        // strip delimiter to get a correct entry for comparisons
        prefix.resize( prefix.length() );
      }
      namespaceToDelimiter[prefix] = delim;

      result.pos++; // tie off )
      skipWS( result );
    } else if ( result[0] == ')' )
    {
      result.pos++; // tie off )
      skipWS( result );
    } else if ( result[0] == 'N' )
    {
      // drop NIL
      ++ns;
      parseOneWord( result );
    } else {
      // drop whatever it is
      parseOneWord( result );
    }
  }
  if ( !delimEmpty.isEmpty() ) {
    // remember default delimiter
    namespaceToDelimiter[QString()] = delimEmpty;
    if ( !personalAvailable )
    {
      // at least one personal ns would be nice
      kDebug(7116) <<"imapParser::parseNamespace - registering own personal ns";
      QString nsentry = "0==" + delimEmpty;
      imapNamespaces.append( nsentry );
    }
  }
}

int imapParser::parseLoop ()
{
  parseString result;

  if (!parseReadLine(result.data)) return -1;

  //kDebug(7116) << result.cstr(); // includes \n

  if (result.data.isEmpty())
    return 0;
  if (!sentQueue.count ())
  {
    // maybe greeting or BYE everything else SHOULD not happen, use NOOP or IDLE
    kDebug(7116) <<"imapParser::parseLoop - unhandledResponse:" << result.cstr();
    unhandled << result.cstr();
  }
  else
  {
    imapCommand *current = sentQueue.at (0);
    switch (result[0])
    {
    case '*':
      result.data.resize(result.data.size() - 2);  // tie off CRLF
      parseUntagged (result);
      break;
    case '+':
      continuation = result.data;
      break;
    default:
      {
        QByteArray tag = parseLiteral(result);
        if (current->id() == tag.data())
        {
          result.data.resize(result.data.size() - 2);  // tie off CRLF
          QByteArray resultCode = parseLiteral (result); //the result
          current->setResult (resultCode);
          current->setResultInfo(result.cstr());
          current->setComplete ();

          sentQueue.removeRef (current);
          completeQueue.append (current);
          if (result.length())
            parseResult (resultCode, result, current->command());
        }
        else
        {
          kDebug(7116) <<"imapParser::parseLoop - unknown tag '" << tag <<"'";
          QByteArray cstr = tag + " " + result.cstr();
          result.data = cstr;
          result.pos = 0;
          result.data.resize(cstr.length());
        }
      }
      break;
    }
  }

  return 1;
}

void
imapParser::parseRelay (const QByteArray & buffer)
{
  Q_UNUSED(buffer);
  qWarning
    ("imapParser::parseRelay - virtual function not reimplemented - data lost");
}

void
imapParser::parseRelay (ulong len)
{
  Q_UNUSED(len);
  qWarning
    ("imapParser::parseRelay - virtual function not reimplemented - announcement lost");
}

bool imapParser::parseRead (QByteArray & buffer, long len, long relay)
{
  Q_UNUSED(buffer);
  Q_UNUSED(len);
  Q_UNUSED(relay);
  qWarning
    ("imapParser::parseRead - virtual function not reimplemented - no data read");
  return false;
}

bool imapParser::parseReadLine (QByteArray & buffer, long relay)
{
  Q_UNUSED(buffer);
  Q_UNUSED(relay);
  qWarning
    ("imapParser::parseReadLine - virtual function not reimplemented - no data read");
  return false;
}

void
imapParser::parseWriteLine (const QString & str)
{
  Q_UNUSED(str);
  qWarning
    ("imapParser::parseWriteLine - virtual function not reimplemented - no data written");
}

void
imapParser::parseURL (const KUrl & _url, QString & _box, QString & _section,
                      QString & _type, QString & _uid, QString & _validity, QString & _info)
{
  QStringList parameters;

  _box = _url.path ();
  kDebug(7116) <<"imapParser::parseURL" << _box;
  int paramStart = _box.indexOf("/;");
  if ( paramStart > -1 )
  {
    QString paramString = _box.right( _box.length() - paramStart-2 );
    parameters = paramString.split (';', QString::SkipEmptyParts);  //split parameters
    _box.truncate( paramStart ); // strip parameters
  }
  // extract parameters
  for (QStringList::ConstIterator it (parameters.begin ());
       it != parameters.end (); ++it)
  {
    QString temp = (*it);

    // if we have a '/' separator we'll just nuke it
    int pt = temp.indexOf ('/');
    if (pt > 0)
      temp.truncate(pt);
    if (temp.startsWith("section=", Qt::CaseInsensitive))
      _section = temp.right (temp.length () - 8);
    else if (temp.startsWith("type=", Qt::CaseInsensitive))
      _type = temp.right (temp.length () - 5);
    else if (temp.startsWith("uid=", Qt::CaseInsensitive))
      _uid = temp.right (temp.length () - 4);
    else if (temp.startsWith("uidvalidity=", Qt::CaseInsensitive))
      _validity = temp.right (temp.length () - 12);
    else if (temp.startsWith("info=", Qt::CaseInsensitive))
      _info = temp.right (temp.length () - 5);
  }
//  kDebug(7116) <<"URL: section=" << _section <<", type=" << _type <<", uid=" << _uid;
//  kDebug(7116) <<"URL: user()" << _url.user();
//  kDebug(7116) <<"URL: path()" << _url.path();
//  kDebug(7116) <<"URL: encodedPathAndQuery()" << _url.encodedPathAndQuery();

  if (!_box.isEmpty ())
  {
    // strip /
    if (_box[0] == '/')
      _box = _box.right (_box.length () - 1);
    if (!_box.isEmpty () && _box[_box.length () - 1] == '/')
      _box.truncate(_box.length() - 1);
  }
  kDebug(7116) <<"URL: box=" << _box <<", section=" << _section <<", type="
    << _type << ", uid=" << _uid << ", validity=" << _validity << ", info=" << _info;
}


QByteArray imapParser::parseLiteral(parseString & inWords, bool relay, bool stopAtBracket) {

  if (inWords[0] == '{')
  {
    QByteArray retVal;
    int runLen = inWords.find ('}', 1);
    if (runLen > 0)
    {
      bool proper;
      long runLenSave = runLen + 1;
      QByteArray tmpstr;
      tmpstr.resize(runLen);
      inWords.takeMidNoResize(tmpstr, 1, runLen - 1);
      runLen = tmpstr.toULong (&proper);
      inWords.pos += runLenSave;
      if (proper)
      {
        //now get the literal from the server
        if (relay)
          parseRelay (runLen);
        QByteArray rv;
        parseRead (rv, runLen, relay ? runLen : 0);
        rv.resize(qMax(runLen, rv.size())); // what's the point?
        retVal = rv;
        inWords.clear();
        parseReadLine (inWords.data); // must get more

        // no duplicate data transfers
        relay = false;
      }
      else
      {
        kDebug(7116) <<"imapParser::parseLiteral - error parsing {} -" /*<< strLen*/;
      }
    }
    else
    {
      inWords.clear();
      kDebug(7116) <<"imapParser::parseLiteral - error parsing unmatched {";
    }
    skipWS (inWords);
    return retVal;
  }

  return parseOneWord(inWords, stopAtBracket);
}

// does not know about literals ( {7} literal )
QByteArray imapParser::parseOneWord (parseString & inWords, bool stopAtBracket)
{
  uint len = inWords.length();
  if (len == 0) {
    return QByteArray();
  }

  if (len > 0 && inWords[0] == '"')
  {
    unsigned int i = 1;
    bool quote = false;
    while (i < len && (inWords[i] != '"' || quote))
    {
      if (inWords[i] == '\\') quote = !quote;
      else quote = false;
      i++;
    }
    if (i < len)
    {
      QByteArray retVal;
      retVal.resize(i);
      inWords.pos++;
      inWords.takeLeftNoResize(retVal, i - 1);
      len = i - 1;
      int offset = 0;
      for (unsigned int j = 0; j <= len; j++) {
        if (retVal[j] == '\\') {
          offset++;
          j++;
        }
        retVal[j - offset] = retVal[j];
      }
      retVal.resize( len - offset );
      inWords.pos += i;
      skipWS (inWords);
      return retVal;
    }
    else
    {
      kDebug(7116) <<"imapParser::parseOneWord - error parsing unmatched \"";
      QByteArray retVal = inWords.cstr();
      inWords.clear();
      return retVal;
    }
  }
  else
  {
    // not quoted
    unsigned int i;
    // search for end
    for (i = 0; i < len; ++i) {
        char ch = inWords[i];
        if (ch <= ' ' || ch == '(' || ch == ')' ||
            (stopAtBracket && (ch == '[' || ch == ']')))
            break;
    }

    QByteArray retVal;
    retVal.resize(i);
    inWords.takeLeftNoResize(retVal, i);
    inWords.pos += i;

    if (retVal == "NIL") {
      retVal.truncate(0);
    }
    skipWS (inWords);
    return retVal;
  }
}

bool imapParser::parseOneNumber (parseString & inWords, ulong & num)
{
  bool valid;
  num = parseOneWord(inWords, true).toULong(&valid);
  return valid;
}

bool imapParser::hasCapability (const QString & cap)
{
  QString c = cap.toLower();
//  kDebug(7116) <<"imapParser::hasCapability - Looking for '" << cap <<"'";
  for (QStringList::ConstIterator it = imapCapabilities.begin ();
       it != imapCapabilities.end (); ++it)
  {
//    kDebug(7116) <<"imapParser::hasCapability - Examining '" << (*it) <<"'";
    if ( !(kasciistricmp(c.toAscii(), (*it).toAscii())) )
    {
      return true;
    }
  }
  return false;
}

void imapParser::removeCapability (const QString & cap)
{
  imapCapabilities.removeAll(cap.toLower());
}

QString imapParser::namespaceForBox( const QString & box )
{
  kDebug(7116) <<"imapParse::namespaceForBox" << box;
  QString myNamespace;
  if ( !box.isEmpty() )
  {
    QList<QString> list = namespaceToDelimiter.keys();
    QString cleanPrefix;
    for ( QList<QString>::Iterator it = list.begin(); it != list.end(); ++it )
    {
      if ( !(*it).isEmpty() && box.contains( *it ) )
        return (*it);
    }
  }
  return myNamespace;
}

