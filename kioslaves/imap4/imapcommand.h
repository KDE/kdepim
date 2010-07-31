#ifndef _IMAPCOMMAND_H
#define _IMAPCOMMAND_H
/**********************************************************************
 *
 *   imapcommand.h  - IMAP4rev1 command handler
 *   Copyright (C) 2000 Sven Carstens
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
 *   Send comments and bug fixes to
 *
 *********************************************************************/

#include <tqstringlist.h>
#include <tqstring.h>
#include <tqmap.h>

/**
 *  @brief encapulate a IMAP command
 *  @author Svenn Carstens
 *  @date 2000
 *  @todo fix the documentation
 */

class imapCommand
{
public:

   /**
    * @brief Constructor
    */
  imapCommand ();
   /**
    * @fn imapCommand (const TQString & command, const TQString & parameter);
    * @brief Constructor
    * @param command Imap command
    * @param parameter Parameters to the command
    * @return none
    */
  imapCommand (const TQString & command, const TQString & parameter);
  /**
   * @fn bool isComplete ();
   * @brief is it complete?
   * @return whether the command is completed
   */
  bool isComplete ();
  /**
   * @fn const TQString & result ();
   * @brief get the result of the command
   * @return The result, i.e. first word of the result line, like OK
   */
  const TQString & result ();
  /**
   * @fn const TQString & resultInfo ();
   * @brief get information about the result
   * @return Information about the result, i.e. the rest of the result line
   */
  const TQString & resultInfo ();
  /**
   * @fn const TQString & parameter ();
   * @brief get the parameter
   * @return the parameter
   */
  const TQString & parameter ();
  /**
   * @fn const TQString & command ();
   * @brief get the command
   * @return the command
   */
  const TQString & command ();
  /**
   * @fn const TQString & id ();
   * @brief get the id
   * @return the id
   */
  const TQString & id ();

  /**
   * @fn void setId (const TQString &);
   * @brief set the id
   * @param id the id used by the command
   * @return none
   */
  void setId (const TQString &);
  /**
   * @fn void setComplete ();
   * @brief set the completed state
   * @return none
   */
  void setComplete ();
  /**
   * @fn void setResult (const TQString &);
   * @brief set the completed state
   * @param result the command result
   * @return none
   */
  void setResult (const TQString &);
  /**
   * @fn void setResultInfo (const TQString &);
   * @brief set the completed state
   * @param result the command result information
   * @return none
   */
  void setResultInfo (const TQString &);
  /**
   * @fn void setCommand (const TQString &);
   * @brief set the command
   * @param command the imap command
   * @return none
   */
  void setCommand (const TQString &);
  /**
   * @fn void setParameter (const TQString &);
   * @brief set the command parameter(s)
   * @param parameter the comand parameter(s)
   * @return none
   */
  void setParameter (const TQString &);
  /**
   * @fn const TQString getStr ();
   * @brief returns the data to send to the server
   * The function returns the complete data to be sent to
   * the server (\<id\> \<command\> [\<parameter\>])
   * @return the data to send to the server
   * @todo possibly rename function to be clear of it's purpose
   */
  const TQString getStr ();

  /**
   * @fn static imapCommand *clientNoop ();
   * @brief Create a NOOP command
   * @return a NOOP imapCommand
   */
  static imapCommand *clientNoop ();
  /**
   * @fn static imapCommand *clientFetch (ulong uid, const TQString & fields, bool nouid = false);
   * @brief Create a FETCH command
   * @param uid Uid of the message to fetch
   * @param fields options to pass to the server
   * @param nouid Perform a FETCH or UID FETCH command
   * @return a FETCH imapCommand
   * Fetch a single uid
   */
  static imapCommand *clientFetch (ulong uid, const TQString & fields,
                                   bool nouid = false);
  /**
   * @fn static imapCommand *clientFetch (ulong fromUid, ulong toUid, const TQString & fields, bool nouid = false);
   * @brief Create a FETCH command
   * @param fromUid start uid of the messages to fetch
   * @param toUid last uid of the messages to fetch
   * @param fields options to pass to the server
   * @param nouid Perform a FETCH or UID FETCH command
   * @return a FETCH imapCommand
   * Fetch a range of uids
   */
  static imapCommand *clientFetch (ulong fromUid, ulong toUid,
                                   const TQString & fields, bool nouid =
                                   false);
  /**
   * @fn static imapCommand *clientFetch (const TQString & sequence, const TQString & fields, bool nouid = false);
   * @brief Create a FETCH command
   * @param sequence a IMAP FETCH sequence string
   * @param fields options to pass to the server
   * @param nouid Perform a FETCH or UID FETCH command
   * @return a FETCH imapCommand
   * Fetch a range of uids. The other clientFetch functions are just
   * wrappers around this function.
   */
  static imapCommand *clientFetch (const TQString & sequence,
                                   const TQString & fields, bool nouid =
                                   false);
  /**
   * @fn static imapCommand *clientList (const TQString & reference, const TQString & path, bool lsub = false);
   * @brief Create a LIST command
   * @param reference
   * @param path The path to list
   * @param lsub Perform a LIST or a LSUB command
   * @return a LIST imapCommand
   */
  static imapCommand *clientList (const TQString & reference,
                                  const TQString & path, bool lsub = false);
  /**
   * @fn static imapCommand *clientSelect (const TQString & path, bool examine = false);
   * @brief Create a SELECT command
   * @param path The path to select
   * @param lsub Perform a SELECT or a EXAMINE command
   * @return a SELECT imapCommand
   */
  static imapCommand *clientSelect (const TQString & path, bool examine =
                                    false);
  /**
   * @fn static imapCommand *clientClose();
   * @brief Create a CLOSE command
   * @return a CLOSE imapCommand
   */
  static imapCommand *clientClose();
  /**
   * @brief Create a STATUS command
   * @param path
   * @param parameters
   * @return a STATUS imapCommand
   */
  static imapCommand *clientStatus (const TQString & path,
                                    const TQString & parameters);
  /**
   * @brief Create a COPY command
   * @param box
   * @param sequence
   * @param nouid Perform a COPY or UID COPY command
   * @return a COPY imapCommand
   */
  static imapCommand *clientCopy (const TQString & box,
                                  const TQString & sequence, bool nouid =
                                  false);
  /**
   * @brief Create a APPEND command
   * @param box
   * @param flags
   * @param size
   * @return a APPEND imapCommand
   */
  static imapCommand *clientAppend (const TQString & box,
                                    const TQString & flags, ulong size);
  /**
   * @brief Create a CREATE command
   * @param path
   * @return a CREATE imapCommand
   */
  static imapCommand *clientCreate (const TQString & path);
  /**
   * @brief Create a DELETE command
   * @param path
   * @return a DELETE imapCommand
   */
  static imapCommand *clientDelete (const TQString & path);
  /**
   * @brief Create a SUBSCRIBE command
   * @param path
   * @return a SUBSCRIBE imapCommand
   */
  static imapCommand *clientSubscribe (const TQString & path);
  /**
   * @brief Create a UNSUBSCRIBE command
   * @param path
   * @return a UNSUBSCRIBE imapCommand
   */
  static imapCommand *clientUnsubscribe (const TQString & path);
  /**
   * @brief Create a EXPUNGE command
   * @return a EXPUNGE imapCommand
   */
  static imapCommand *clientExpunge ();
  /**
   * @brief Create a RENAME command
   * @param src Source
   * @param dest Destination
   * @return a RENAME imapCommand
   */
  static imapCommand *clientRename (const TQString & src,
                                    const TQString & dest);
  /**
   * @brief Create a SEARCH command
   * @param search
   * @param nouid Perform a UID SEARCH or a SEARCH command
   * @return a SEARCH imapCommand
   */
  static imapCommand *clientSearch (const TQString & search, bool nouid =
                                    false);
  /**
   * @brief Create a STORE command
   * @param set
   * @param item
   * @param data
   * @param nouid Perform a UID STORE or a STORE command
   * @return a STORE imapCommand
   */
  static imapCommand *clientStore (const TQString & set, const TQString & item,
                                   const TQString & data, bool nouid = false);
  /**
   * @brief Create a LOGOUT command
   * @return a LOGOUT imapCommand
   */
  static imapCommand *clientLogout ();
  /**
   * @brief Create a STARTTLS command
   * @return a STARTTLS imapCommand
   */
  static imapCommand *clientStartTLS ();

  //////////// ACL support (RFC 2086) /////////////
  /**
   * @brief Create a SETACL command
   * @param box mailbox name
   * @param user authentication identifier
   * @param acl access right modification (starting with optional +/-)
   * @return a SETACL imapCommand
   */
  static imapCommand *clientSetACL ( const TQString& box, const TQString& user, const TQString& acl );

  /**
   * @brief Create a DELETEACL command
   * @param box mailbox name
   * @param user authentication identifier
   * @return a DELETEACL imapCommand
   */
  static imapCommand *clientDeleteACL ( const TQString& box, const TQString& user );

  /**
   * @brief Create a GETACL command
   * @param box mailbox name
   * @return a GETACL imapCommand
   */
  static imapCommand *clientGetACL ( const TQString& box );

  /**
   * @brief Create a LISTRIGHTS command
   * @param box mailbox name
   * @param user authentication identifier
   * @return a LISTRIGHTS imapCommand
   */
  static imapCommand *clientListRights ( const TQString& box, const TQString& user );

  /**
   * @brief Create a MYRIGHTS command
   * @param box mailbox name
   * @return a MYRIGHTS imapCommand
   */
  static imapCommand *clientMyRights ( const TQString& box );

  //////////// ANNOTATEMORE support /////////////
  /**
   * @brief Create a SETANNOTATION command
   * @param box mailbox name
   * @param entry entry specifier
   * @param attributes map of attribute names + values
   * @return a SETANNOTATION imapCommand
   */
  static imapCommand *clientSetAnnotation ( const TQString& box, const TQString& entry, const TQMap<TQString, TQString>& attributes );

  /**
   * @brief Create a GETANNOTATION command
   * @param box mailbox name
   * @param entry entry specifier
   * @param attributeNames attribute specifier
   * @return a GETANNOTATION imapCommand
   */
  static imapCommand *clientGetAnnotation ( const TQString& box, const TQString& entry, const TQStringList& attributeNames );

  /**
   * @brief Create a NAMESPACE command
   * @return a NAMESPACE imapCommand
   */
  static imapCommand *clientNamespace ();

  /**
   * @brief Create a GEQUOTAROOT command
   * @param box mailbox name
   * @return a GEQUOTAROOT imapCommand
   */
  static imapCommand *clientGetQuotaroot ( const TQString& box );

  /**
   * @brief Create a custom command
   * @param command The custom command
   * @param arguments The custom arguments
   * @return a custom imapCommand
   */
  static imapCommand *clientCustom ( const TQString& command, const TQString& arguments );

protected:
  TQString aCommand;
  TQString mId;
  bool mComplete;
  TQString aParameter;
  TQString mResult;
  TQString mResultInfo;

private:
  imapCommand & operator = (const imapCommand &);
};

#endif
