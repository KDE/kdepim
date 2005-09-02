/*  -*- c++ -*-
    kmime_mdn.cpp

    This file is part of KMime, the KDE internet mail/usenet news message library.
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

    KMime is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMime is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this library with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "kmime_mdn.h"

#include "kmime_version.h"
#include "kmime_util.h"

#include <klocale.h>
#include <kdebug.h>

#include <q3cstring.h>
//Added by qt3to4:
#include <Q3ValueList>

#include <unistd.h> // gethostname

namespace KMime {

namespace MDN {

  static const struct {
    DispositionType dispositionType;
    const char * string;
    const char * description;
  } dispositionTypes[] = {
    { Displayed, "displayed",
      I18N_NOOP("The message sent on ${date} to ${to} with subject "
		"\"${subject}\" has been displayed. This is no guarantee that "
		"the message has been read or understood.") },
    { Deleted, "deleted",
      I18N_NOOP("The message sent on ${date} to ${to} with subject "
		"\"${subject}\" has been deleted unseen. This is no guarantee "
		"that the message will not be \"undeleted\" and nonetheless "
		"read later on.") },
    { Dispatched, "dispatched",
      I18N_NOOP("The message sent on ${date} to ${to} with subject "
		"\"${subject}\" has been dispatched. This is no guarantee "
		"that the message will not be read later on.") },
    { Processed, "processed",
      I18N_NOOP("The message sent on ${date} to ${to} with subject "
		"\"${subject}\" has been processed by some automatic means.") },
    { Denied, "denied",
      I18N_NOOP("The message sent on ${date} to ${to} with subject "
		"\"${subject}\" has been acted upon. The sender does not wish "
		"to disclose more details to you than that.") },
    { Failed, "failed",
      I18N_NOOP("Generation of a Message Disposition Notification for the "
		"message sent on ${date} to ${to} with subject \"${subject}\" "
		"failed. Reason is given in the Failure: header field below.") }
  };

  static const int numDispositionTypes
           = sizeof dispositionTypes / sizeof *dispositionTypes;


  static const char * stringFor( DispositionType d ) {
    for ( int i = 0 ; i < numDispositionTypes ; ++i )
      if ( dispositionTypes[i].dispositionType == d )
	return dispositionTypes[i].string;
    return 0;
  }


  //
  // disposition-modifier
  //
  static const struct {
    DispositionModifier dispositionModifier;
    const char * string;
  } dispositionModifiers[] = {
    { Error, "error" },
    { Warning, "warning" },
    { Superseded, "superseded" },
    { Expired, "expired" },
    { MailboxTerminated, "mailbox-terminated" }
  };

  static const int numDispositionModifiers
       = sizeof dispositionModifiers / sizeof * dispositionModifiers;


  static const char * stringFor( DispositionModifier m ) {
    for ( int i = 0 ; i < numDispositionModifiers ; ++i )
      if ( dispositionModifiers[i].dispositionModifier == m )
	return dispositionModifiers[i].string;
    return 0;
  }

  //
  // action-mode (part of disposition-mode)
  //

  static const struct {
    ActionMode actionMode;
    const char * string;
  } actionModes[] = {
    { ManualAction, "manual-action" },
    { AutomaticAction, "automatic-action" }
  };

  static const int numActionModes = sizeof actionModes / sizeof *actionModes;

  static const char * stringFor( ActionMode a ) {
    for ( int i = 0 ; i < numActionModes ; ++i )
      if ( actionModes[i].actionMode == a )
	return actionModes[i].string;
    return 0;
  }


  //
  // sending-mode (part of disposition-mode)
  //

  static const struct {
    SendingMode sendingMode;
    const char * string;
  } sendingModes[] = {
    { SentManually, "MDN-sent-manually" },
    { SentAutomatically, "MDN-sent-automatically" }
  };

  static const int numSendingModes = sizeof sendingModes / sizeof *sendingModes;

  static const char * stringFor( SendingMode s ) {
    for ( int i = 0 ; i < numSendingModes ; ++i )
      if ( sendingModes[i].sendingMode == s )
	return sendingModes[i].string;
    return 0;
  }

  static Q3CString dispositionField( DispositionType d, ActionMode a, SendingMode s,
				    const Q3ValueList<DispositionModifier> & m ) {

    // mandatory parts: Disposition: foo/baz; bar
    Q3CString result = "Disposition: ";
    result += stringFor( a );
    result += "/";
    result += stringFor( s );
    result += "; ";
    result += stringFor( d );

    // optional parts: Disposition: foo/baz; bar/mod1,mod2,mod3
    bool first = true;
    for ( Q3ValueList<DispositionModifier>::const_iterator mt = m.begin() ;
	  mt != m.end() ; ++mt ) {
      if ( first ) {
	result += "/";
	first = false;
      } else {
	result += ",";
      }
      result += stringFor( *mt );
    }
    return result + "\n";
  }

  static Q3CString finalRecipient( const QString & recipient ) {
    if ( recipient.isEmpty() )
      return Q3CString();
    else
      return "Final-Recipient: rfc822; "
	+ encodeRFC2047String( recipient, "utf-8" ) + "\n";
  }

  static Q3CString orginalRecipient( const Q3CString & recipient ) {
    if ( recipient.isEmpty() )
      return Q3CString();
    else
      return "Original-Recipient: " + recipient + "\n";
  }

  static Q3CString originalMessageID( const Q3CString & msgid ) {
    if ( msgid.isEmpty() )
      return Q3CString();
    else
      return "Original-Message-ID: " + msgid + "\n";
  }

  static Q3CString reportingUAField() {
    char hostName[256];
    if ( gethostname( hostName, 255 ) )
      hostName[0] = '\0'; // gethostname failed: pretend empty string
    else
      hostName[255] = '\0'; // gethostname may have returned 255 chars (man page)
    return Q3CString("Reporting-UA: ") + Q3CString( hostName )
      + Q3CString( "; KMime " KMIME_VERSION_STRING "\n" );
  }

  Q3CString dispositionNotificationBodyContent( const QString & r,
					       const Q3CString & o,
					       const Q3CString & omid,
					       DispositionType d,
					       ActionMode a,
					       SendingMode s,
					       const Q3ValueList<DispositionModifier> & m,
					       const QString & special )
  {
    // in Perl: chomp(special) 
    QString spec;
    if ( special.endsWith("\n") )
      spec = special.left( special.length() - 1 );
    else
      spec = special;

    // std headers:
    Q3CString result = reportingUAField();
    result += orginalRecipient( o );
    result += finalRecipient( r );
    result += originalMessageID( omid );
    result += dispositionField( d, a, s, m );

    // headers that are only present for certain disposition {types,modifiers}:
    if ( d == Failed )
      result += "Failure: " + encodeRFC2047String( spec, "utf-8" ) + "\n";
    else if ( m.contains( Error ) )
      result += "Error: " + encodeRFC2047String( spec, "utf-8" ) + "\n";
    else if ( m.contains( Warning ) )
      result += "Warning: " + encodeRFC2047String( spec, "utf-8" ) + "\n";

    return result;
  }

  QString descriptionFor( DispositionType d,
			  const Q3ValueList<DispositionModifier> & ) {
    for ( int i = 0 ; i < numDispositionTypes ; ++i )
      if ( dispositionTypes[i].dispositionType == d )
	return i18n( dispositionTypes[i].description );
    kdWarning() << "KMime::MDN::descriptionFor(): No such disposition type: "
		<< (int)d << endl;
    return QString::null;
  }

} // namespace MDN
} // namespace KMime
