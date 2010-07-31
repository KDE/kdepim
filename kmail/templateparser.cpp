/*   -*- mode: C++; c-file-style: "gnu" -*-
 *   kmail: KDE mail client
 *   This file: Copyright (C) 2006 Dmitry Morozhnikov <dmiceman@mail.ru>
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
 */

#include <config.h>

#include <tqstring.h>
#include <tqdatetime.h>
#include <klocale.h>
#include <kcalendarsystem.h>
#include <kmime_util.h>
#include <kglobal.h>
#include <kprocess.h>
#include <tqregexp.h>
#include <tqfile.h>
#include <kmessagebox.h>
#include <kshell.h>
#include <tqfileinfo.h>

#include "kmmessage.h"
#include "kmmsgbase.h"
#include "kmfolder.h"
#include "templatesconfiguration.h"
#include "templatesconfiguration_kfg.h"
#include "customtemplates_kfg.h"
#include "globalsettings_base.h"
#include "kmkernel.h"
#include <libkpimidentities/identity.h>
#include <libkpimidentities/identitymanager.h>

#include "templateparser.h"

TemplateParser::TemplateParser( KMMessage *amsg, const Mode amode,
                                const TQString aselection,
                                bool asmartQuote, bool anoQuote,
                                bool aallowDecryption, bool aselectionIsBody ) :
  mMode( amode ), mFolder( 0 ), mIdentity( 0 ), mSelection( aselection ),
  mSmartQuote( asmartQuote ), mNoQuote( anoQuote ),
  mAllowDecryption( aallowDecryption ), mSelectionIsBody( aselectionIsBody ),
  mDebug( false ), mQuoteString( "> " ), mAppend( false )
{
  mMsg = amsg;
}

int TemplateParser::parseQuotes( const TQString &prefix, const TQString &str,
                                 TQString &quote ) const
{
  int pos = prefix.length();
  int len;
  int str_len = str.length();
  TQChar qc = '"';
  TQChar prev = 0;

  pos++;
  len = pos;

  while ( pos < str_len ) {
    TQChar c = str[pos];

    pos++;
    len++;

    if ( prev ) {
      quote.append( c );
      prev = 0;
    } else {
      if ( c == '\\' ) {
        prev = c;
      } else if ( c == qc ) {
        break;
      } else {
        quote.append( c );
      }
    }
  }

  return len;
}

TQString TemplateParser::getFName( const TQString &str )
{
  // simple logic:
  // if there is ',' in name, than format is 'Last, First'
  // else format is 'First Last'
  // last resort -- return 'name' from 'name@domain'
  int sep_pos;
  TQString res;
  if ( ( sep_pos = str.find( '@' ) ) > 0 ) {
    int i;
    for ( i = (sep_pos - 1); i >= 0; --i ) {
      TQChar c = str[i];
      if ( c.isLetterOrNumber() ) {
        res.prepend( c );
      } else {
        break;
      }
    }
  } else if ( ( sep_pos = str.find(',') ) > 0 ) {
    unsigned int i;
    bool begin = false;
    for ( i = sep_pos; i < str.length(); ++i ) {
      TQChar c = str[i];
      if ( c.isLetterOrNumber() ) {
        begin = true;
        res.append( c );
      } else if ( begin ) {
        break;
      }
    }
  } else {
    unsigned int i;
    for ( i = 0; i < str.length(); ++i ) {
      TQChar c = str[i];
      if ( c.isLetterOrNumber() ) {
        res.append( c );
      } else {
        break;
      }
    }
  }
  return res;
}

TQString TemplateParser::getLName( const TQString &str )
{
  // simple logic:
  // if there is ',' in name, than format is 'Last, First'
  // else format is 'First Last'
  int sep_pos;
  TQString res;
  if ( ( sep_pos = str.find(',') ) > 0 ) {
    int i;
    for ( i = sep_pos; i >= 0; --i ) {
      TQChar c = str[i];
      if ( c.isLetterOrNumber() ) {
        res.prepend( c );
      } else {
        break;
      }
    }
  } else {
    if ( ( sep_pos = str.find( ' ' ) ) > 0 ) {
      unsigned int i;
      bool begin = false;
      for ( i = sep_pos; i < str.length(); ++i ) {
        TQChar c = str[i];
        if ( c.isLetterOrNumber() ) {
          begin = true;
          res.append( c );
        } else if ( begin ) {
          break;
        }
      }
    }
  }
  return res;
}

void TemplateParser::process( KMMessage *aorig_msg, KMFolder *afolder, bool append )
{
  mAppend = append;
  mOrigMsg = aorig_msg;
  mFolder = afolder;
  TQString tmpl = findTemplate();
  return processWithTemplate( tmpl );
}

void TemplateParser::process( const TQString &tmplName, KMMessage *aorig_msg,
                              KMFolder *afolder, bool append )
{
  mAppend = append;
  mOrigMsg = aorig_msg;
  mFolder = afolder;
  TQString tmpl = findCustomTemplate( tmplName );
  return processWithTemplate( tmpl );
}

void TemplateParser::processWithTemplate( const TQString &tmpl )
{
  TQString body;
  int tmpl_len = tmpl.length();
  bool dnl = false;
  for ( int i = 0; i < tmpl_len; ++i ) {
    TQChar c = tmpl[i];
    // kdDebug() << "Next char: " << c << endl;
    if ( c == '%' ) {
      TQString cmd = tmpl.mid( i + 1 );

      if ( cmd.startsWith( "-" ) ) {
        // dnl
        kdDebug() << "Command: -" << endl;
        dnl = true;
        i += 1;

      } else if ( cmd.startsWith( "REM=" ) ) {
        // comments
        kdDebug() << "Command: REM=" << endl;
        TQString q;
        int len = parseQuotes( "REM=", cmd, q );
        i += len;

      } else if ( cmd.startsWith( "INSERT=" ) ) {
        // insert content of specified file as is
        kdDebug() << "Command: INSERT=" << endl;
        TQString q;
        int len = parseQuotes( "INSERT=", cmd, q );
        i += len;
        TQString path = KShell::tildeExpand( q );
        TQFileInfo finfo( path );
        if (finfo.isRelative() ) {
          path = KShell::homeDir( "" );
          path += '/';
          path += q;
        }
        TQFile file( path );
        if ( file.open( IO_ReadOnly ) ) {
          TQByteArray content = file.readAll();
          TQString str = TQString::fromLocal8Bit( content, content.size() );
          body.append( str );
        } else if ( mDebug ) {
          KMessageBox::error( 0,
                              i18n( "Cannot insert content from file %1: %2" ).
                              arg( path ).arg( file.errorString() ) );
        }

      } else if ( cmd.startsWith( "SYSTEM=" ) ) {
        // insert content of specified file as is
        kdDebug() << "Command: SYSTEM=" << endl;
        TQString q;
        int len = parseQuotes( "SYSTEM=", cmd, q );
        i += len;
        TQString pipe_cmd = q;
        TQString str = pipe( pipe_cmd, "" );
        body.append( str );

      } else if ( cmd.startsWith( "PUT=" ) ) {
        // insert content of specified file as is
        kdDebug() << "Command: PUT=" << endl;
        TQString q;
        int len = parseQuotes( "PUT=", cmd, q );
        i += len;
        TQString path = KShell::tildeExpand( q );
        TQFileInfo finfo( path );
        if (finfo.isRelative() ) {
          path = KShell::homeDir( "" );
          path += '/';
          path += q;
        }
        TQFile file( path );
        if ( file.open( IO_ReadOnly ) ) {
          TQByteArray content = file.readAll();
          body.append( TQString::fromLocal8Bit( content, content.size() ) );
        } else if ( mDebug ) {
          KMessageBox::error( 0,
                              i18n( "Cannot insert content from file %1: %2").
                              arg( path ).arg(file.errorString() ));
        }

      } else if ( cmd.startsWith( "QUOTEPIPE=" ) ) {
        // pipe message body throw command and insert it as quotation
        kdDebug() << "Command: QUOTEPIPE=" << endl;
        TQString q;
        int len = parseQuotes( "QUOTEPIPE=", cmd, q );
        i += len;
        TQString pipe_cmd = q;
        if ( mOrigMsg && !mNoQuote ) {
          TQString str = pipe( pipe_cmd, mSelection );
          TQString quote = mOrigMsg->asQuotedString( "", mQuoteString, str,
                                                    mSmartQuote, mAllowDecryption );
          body.append( quote );
        }

      } else if ( cmd.startsWith( "QUOTE" ) ) {
        kdDebug() << "Command: QUOTE" << endl;
        i += strlen( "QUOTE" );
        if ( mOrigMsg && !mNoQuote ) {
          TQString quote = mOrigMsg->asQuotedString( "", mQuoteString, mSelection,
                                                    mSmartQuote, mAllowDecryption );
          body.append( quote );
        }

      } else if ( cmd.startsWith( "QHEADERS" ) ) {
        kdDebug() << "Command: QHEADERS" << endl;
        i += strlen( "QHEADERS" );
        if ( mOrigMsg && !mNoQuote ) {
          TQString quote = mOrigMsg->asQuotedString( "", mQuoteString,
                                                    mOrigMsg->headerAsSendableString(),
                                                    mSmartQuote, false );
          body.append( quote );
        }

      } else if ( cmd.startsWith( "HEADERS" ) ) {
        kdDebug() << "Command: HEADERS" << endl;
        i += strlen( "HEADERS" );
        if ( mOrigMsg && !mNoQuote ) {
          TQString str = mOrigMsg->headerAsSendableString();
          body.append( str );
        }

      } else if ( cmd.startsWith( "TEXTPIPE=" ) ) {
        // pipe message body throw command and insert it as is
        kdDebug() << "Command: TEXTPIPE=" << endl;
        TQString q;
        int len = parseQuotes( "TEXTPIPE=", cmd, q );
        i += len;
        TQString pipe_cmd = q;
        if ( mOrigMsg ) {
          TQString str = pipe(pipe_cmd, mSelection );
          body.append( str );
        }

      } else if ( cmd.startsWith( "MSGPIPE=" ) ) {
        // pipe full message throw command and insert result as is
        kdDebug() << "Command: MSGPIPE=" << endl;
        TQString q;
        int len = parseQuotes( "MSGPIPE=", cmd, q );
        i += len;
        TQString pipe_cmd = q;
        if ( mOrigMsg ) {
          TQString str = pipe(pipe_cmd, mOrigMsg->asString() );
          body.append( str );
        }

      } else if ( cmd.startsWith( "BODYPIPE=" ) ) {
        // pipe message body generated so far throw command and insert result as is
        kdDebug() << "Command: BODYPIPE=" << endl;
        TQString q;
        int len = parseQuotes( "BODYPIPE=", cmd, q );
        i += len;
        TQString pipe_cmd = q;
        TQString str = pipe( pipe_cmd, body );
        body.append( str );

      } else if ( cmd.startsWith( "CLEARPIPE=" ) ) {
        // pipe message body generated so far throw command and
        // insert result as is replacing current body
        kdDebug() << "Command: CLEARPIPE=" << endl;
        TQString q;
        int len = parseQuotes( "CLEARPIPE=", cmd, q );
        i += len;
        TQString pipe_cmd = q;
        TQString str = pipe( pipe_cmd, body );
        body = str;
        mMsg->setCursorPos( 0 );

      } else if ( cmd.startsWith( "TEXT" ) ) {
        kdDebug() << "Command: TEXT" << endl;
        i += strlen( "TEXT" );
        if ( mOrigMsg ) {
          TQString quote = mOrigMsg->asPlainText( false, mAllowDecryption );
          body.append( quote );
        }

      } else if ( cmd.startsWith( "OTEXTSIZE" ) ) {
        kdDebug() << "Command: OTEXTSIZE" << endl;
        i += strlen( "OTEXTSIZE" );
        if ( mOrigMsg ) {
          TQString str = TQString( "%1" ).arg( mOrigMsg->body().length() );
          body.append( str );
        }

      } else if ( cmd.startsWith( "OTEXT" ) ) {
        kdDebug() << "Command: OTEXT" << endl;
        i += strlen( "OTEXT" );
        if ( mOrigMsg ) {
          TQString quote = mOrigMsg->asPlainText( false, mAllowDecryption );
          body.append( quote );
        }

      } else if ( cmd.startsWith( "CCADDR" ) ) {
        kdDebug() << "Command: CCADDR" << endl;
        i += strlen( "CCADDR" );
        TQString str = mMsg->cc();
        body.append( str );

      } else if ( cmd.startsWith( "CCNAME" ) ) {
        kdDebug() << "Command: CCNAME" << endl;
        i += strlen( "CCNAME" );
        TQString str = mMsg->ccStrip();
        body.append( str );

      } else if ( cmd.startsWith( "CCFNAME" ) ) {
        kdDebug() << "Command: CCFNAME" << endl;
        i += strlen( "CCFNAME" );
        TQString str = mMsg->ccStrip();
        body.append( getFName( str ) );

      } else if ( cmd.startsWith( "CCLNAME" ) ) {
        kdDebug() << "Command: CCLNAME" << endl;
        i += strlen( "CCLNAME" );
        TQString str = mMsg->ccStrip();
        body.append( getLName( str ) );

      } else if ( cmd.startsWith( "TOADDR" ) ) {
        kdDebug() << "Command: TOADDR" << endl;
        i += strlen( "TOADDR" );
        TQString str = mMsg->to();
        body.append( str );

      } else if ( cmd.startsWith( "TONAME" ) ) {
        kdDebug() << "Command: TONAME" << endl;
        i += strlen( "TONAME" );
        TQString str = mMsg->toStrip();
        body.append( str );

      } else if ( cmd.startsWith( "TOFNAME" ) ) {
        kdDebug() << "Command: TOFNAME" << endl;
        i += strlen( "TOFNAME" );
        TQString str = mMsg->toStrip();
        body.append( getFName( str ) );

      } else if ( cmd.startsWith( "TOLNAME" ) ) {
        kdDebug() << "Command: TOLNAME" << endl;
        i += strlen( "TOLNAME" );
        TQString str = mMsg->toStrip();
        body.append( getLName( str ) );

      } else if ( cmd.startsWith( "TOLIST" ) ) {
        kdDebug() << "Command: TOLIST" << endl;
        i += strlen( "TOLIST" );
        TQString str = mMsg->to();
        body.append( str );

      } else if ( cmd.startsWith( "FROMADDR" ) ) {
        kdDebug() << "Command: FROMADDR" << endl;
        i += strlen( "FROMADDR" );
        TQString str = mMsg->from();
        body.append( str );

      } else if ( cmd.startsWith( "FROMNAME" ) ) {
        kdDebug() << "Command: FROMNAME" << endl;
        i += strlen( "FROMNAME" );
        TQString str = mMsg->fromStrip();
        body.append( str );

      } else if ( cmd.startsWith( "FROMFNAME" ) ) {
        kdDebug() << "Command: FROMFNAME" << endl;
        i += strlen( "FROMFNAME" );
        TQString str = mMsg->fromStrip();
        body.append( getFName( str ) );

      } else if ( cmd.startsWith( "FROMLNAME" ) ) {
        kdDebug() << "Command: FROMLNAME" << endl;
        i += strlen( "FROMLNAME" );
        TQString str = mMsg->fromStrip();
        body.append( getLName( str ) );

      } else if ( cmd.startsWith( "FULLSUBJECT" ) ) {
        kdDebug() << "Command: FULLSUBJECT" << endl;
        i += strlen( "FULLSUBJECT" );
        TQString str = mMsg->subject();
        body.append( str );

      } else if ( cmd.startsWith( "FULLSUBJ" ) ) {
        kdDebug() << "Command: FULLSUBJ" << endl;
        i += strlen( "FULLSUBJ" );
        TQString str = mMsg->subject();
        body.append( str );

      } else if ( cmd.startsWith( "MSGID" ) ) {
        kdDebug() << "Command: MSGID" << endl;
        i += strlen( "MSGID" );
        TQString str = mMsg->id();
        body.append( str );

      } else if ( cmd.startsWith( "OHEADER=" ) ) {
        // insert specified content of header from original message
        kdDebug() << "Command: OHEADER=" << endl;
        TQString q;
        int len = parseQuotes( "OHEADER=", cmd, q );
        i += len;
        if ( mOrigMsg ) {
          TQString hdr = q;
          TQString str = mOrigMsg->headerFields(hdr.local8Bit() ).join( ", " );
          body.append( str );
        }

      } else if ( cmd.startsWith( "HEADER=" ) ) {
        // insert specified content of header from current message
        kdDebug() << "Command: HEADER=" << endl;
        TQString q;
        int len = parseQuotes( "HEADER=", cmd, q );
        i += len;
        TQString hdr = q;
        TQString str = mMsg->headerFields(hdr.local8Bit() ).join( ", " );
        body.append( str );

      } else if ( cmd.startsWith( "HEADER( " ) ) {
        // insert specified content of header from current message
        kdDebug() << "Command: HEADER( " << endl;
        TQRegExp re = TQRegExp( "^HEADER\\((.+)\\)" );
        re.setMinimal( true );
        int res = re.search( cmd );
        if ( res != 0 ) {
          // something wrong
          i += strlen( "HEADER( " );
        } else {
          i += re.matchedLength();
          TQString hdr = re.cap( 1 );
          TQString str = mMsg->headerFields( hdr.local8Bit() ).join( ", " );
          body.append( str );
        }

      } else if ( cmd.startsWith( "OCCADDR" ) ) {
        kdDebug() << "Command: OCCADDR" << endl;
        i += strlen( "OCCADDR" );
        if ( mOrigMsg ) {
          TQString str = mOrigMsg->cc();
          body.append( str );
        }

      } else if ( cmd.startsWith( "OCCNAME" ) ) {
        kdDebug() << "Command: OCCNAME" << endl;
        i += strlen( "OCCNAME" );
        if ( mOrigMsg ) {
          TQString str = mOrigMsg->ccStrip();
          body.append( str );
        }

      } else if ( cmd.startsWith( "OCCFNAME" ) ) {
        kdDebug() << "Command: OCCFNAME" << endl;
        i += strlen( "OCCFNAME" );
        if ( mOrigMsg ) {
          TQString str = mOrigMsg->ccStrip();
          body.append( getFName( str ) );
        }

      } else if ( cmd.startsWith( "OCCLNAME" ) ) {
        kdDebug() << "Command: OCCLNAME" << endl;
        i += strlen( "OCCLNAME" );
        if ( mOrigMsg ) {
          TQString str = mOrigMsg->ccStrip();
          body.append( getLName( str ) );
        }

      } else if ( cmd.startsWith( "OTOADDR" ) ) {
        kdDebug() << "Command: OTOADDR" << endl;
        i += strlen( "OTOADDR" );
        if ( mOrigMsg ) {
          TQString str = mOrigMsg->to();
          body.append( str );
        }

      } else if ( cmd.startsWith( "OTONAME" ) ) {
        kdDebug() << "Command: OTONAME" << endl;
        i += strlen( "OTONAME" );
        if ( mOrigMsg ) {
          TQString str = mOrigMsg->toStrip();
          body.append( str );
        }

      } else if ( cmd.startsWith( "OTOFNAME" ) ) {
        kdDebug() << "Command: OTOFNAME" << endl;
        i += strlen( "OTOFNAME" );
        if ( mOrigMsg ) {
          TQString str = mOrigMsg->toStrip();
          body.append( getFName( str ) );
        }

      } else if ( cmd.startsWith( "OTOLNAME" ) ) {
        kdDebug() << "Command: OTOLNAME" << endl;
        i += strlen( "OTOLNAME" );
        if ( mOrigMsg ) {
          TQString str = mOrigMsg->toStrip();
          body.append( getLName( str ) );
        }

      } else if ( cmd.startsWith( "OTOLIST" ) ) {
        kdDebug() << "Command: OTOLIST" << endl;
        i += strlen( "OTOLIST" );
        if ( mOrigMsg ) {
          TQString str = mOrigMsg->to();
          body.append( str );
        }

      } else if ( cmd.startsWith( "OTO" ) ) {
        kdDebug() << "Command: OTO" << endl;
        i += strlen( "OTO" );
        if ( mOrigMsg ) {
          TQString str = mOrigMsg->to();
          body.append( str );
        }

      } else if ( cmd.startsWith( "OFROMADDR" ) ) {
        kdDebug() << "Command: OFROMADDR" << endl;
        i += strlen( "OFROMADDR" );
        if ( mOrigMsg ) {
          TQString str = mOrigMsg->from();
          body.append( str );
        }

      } else if ( cmd.startsWith( "OFROMNAME" ) ) {
        kdDebug() << "Command: OFROMNAME" << endl;
        i += strlen( "OFROMNAME" );
        if ( mOrigMsg ) {
          TQString str = mOrigMsg->fromStrip();
          body.append( str );
        }

      } else if ( cmd.startsWith( "OFROMFNAME" ) ) {
        kdDebug() << "Command: OFROMFNAME" << endl;
        i += strlen( "OFROMFNAME" );
        if ( mOrigMsg ) {
          TQString str = mOrigMsg->fromStrip();
          body.append( getFName( str ) );
        }

      } else if ( cmd.startsWith( "OFROMLNAME" ) ) {
        kdDebug() << "Command: OFROMLNAME" << endl;
        i += strlen( "OFROMLNAME" );
        if ( mOrigMsg ) {
          TQString str = mOrigMsg->fromStrip();
          body.append( getLName( str ) );
        }

      } else if ( cmd.startsWith( "OFULLSUBJECT" ) ) {
        kdDebug() << "Command: OFULLSUBJECT" << endl;
        i += strlen( "OFULLSUBJECT" );
        if ( mOrigMsg ) {
          TQString str = mOrigMsg->subject();
          body.append( str );
        }

      } else if ( cmd.startsWith( "OFULLSUBJ" ) ) {
        kdDebug() << "Command: OFULLSUBJ" << endl;
        i += strlen( "OFULLSUBJ" );
        if ( mOrigMsg ) {
          TQString str = mOrigMsg->subject();
          body.append( str );
        }

      } else if ( cmd.startsWith( "OMSGID" ) ) {
        kdDebug() << "Command: OMSGID" << endl;
        i += strlen( "OMSGID" );
        if ( mOrigMsg ) {
          TQString str = mOrigMsg->id();
          body.append( str );
        }

      } else if ( cmd.startsWith( "DATEEN" ) ) {
        kdDebug() << "Command: DATEEN" << endl;
        i += strlen( "DATEEN" );
        TQDateTime date = TQDateTime::currentDateTime();
        KLocale locale( "C" );
        TQString str = locale.formatDate( date.date(), false );
        body.append( str );

      } else if ( cmd.startsWith( "DATESHORT" ) ) {
        kdDebug() << "Command: DATESHORT" << endl;
        i += strlen( "DATESHORT" );
        TQDateTime date = TQDateTime::currentDateTime();
        TQString str = KGlobal::locale()->formatDate( date.date(), true );
        body.append( str );

      } else if ( cmd.startsWith( "DATE" ) ) {
        kdDebug() << "Command: DATE" << endl;
        i += strlen( "DATE" );
        TQDateTime date = TQDateTime::currentDateTime();
        TQString str = KGlobal::locale()->formatDate( date.date(), false );
        body.append( str );

      } else if ( cmd.startsWith( "DOW" ) ) {
        kdDebug() << "Command: DOW" << endl;
        i += strlen( "DOW" );
        TQDateTime date = TQDateTime::currentDateTime();
        TQString str = KGlobal::locale()->calendar()->weekDayName( date.date(), false );
        body.append( str );

      } else if ( cmd.startsWith( "TIMELONGEN" ) ) {
        kdDebug() << "Command: TIMELONGEN" << endl;
        i += strlen( "TIMELONGEN" );
        TQDateTime date = TQDateTime::currentDateTime();
        KLocale locale( "C");
        TQString str = locale.formatTime( date.time(), true );
        body.append( str );

      } else if ( cmd.startsWith( "TIMELONG" ) ) {
        kdDebug() << "Command: TIMELONG" << endl;
        i += strlen( "TIMELONG" );
        TQDateTime date = TQDateTime::currentDateTime();
        TQString str = KGlobal::locale()->formatTime( date.time(), true );
        body.append( str );

      } else if ( cmd.startsWith( "TIME" ) ) {
        kdDebug() << "Command: TIME" << endl;
        i += strlen( "TIME" );
        TQDateTime date = TQDateTime::currentDateTime();
        TQString str = KGlobal::locale()->formatTime( date.time(), false );
        body.append( str );

      } else if ( cmd.startsWith( "ODATEEN" ) ) {
        kdDebug() << "Command: ODATEEN" << endl;
        i += strlen( "ODATEEN" );
        if ( mOrigMsg ) {
          TQDateTime date;
          date.setTime_t( mOrigMsg->date() );
          KLocale locale( "C");
          TQString str = locale.formatDate( date.date(), false );
          body.append( str );
        }

      } else if ( cmd.startsWith( "ODATESHORT") ) {
        kdDebug() << "Command: ODATESHORT" << endl;
        i += strlen( "ODATESHORT");
        if ( mOrigMsg ) {
          TQDateTime date;
          date.setTime_t( mOrigMsg->date() );
          TQString str = KGlobal::locale()->formatDate( date.date(), true );
          body.append( str );
        }

      } else if ( cmd.startsWith( "ODATE") ) {
        kdDebug() << "Command: ODATE" << endl;
        i += strlen( "ODATE");
        if ( mOrigMsg ) {
          TQDateTime date;
          date.setTime_t( mOrigMsg->date() );
          TQString str = KGlobal::locale()->formatDate( date.date(), false );
          body.append( str );
        }

      } else if ( cmd.startsWith( "ODOW") ) {
        kdDebug() << "Command: ODOW" << endl;
        i += strlen( "ODOW");
        if ( mOrigMsg ) {
          TQDateTime date;
          date.setTime_t( mOrigMsg->date() );
          TQString str = KGlobal::locale()->calendar()->weekDayName( date.date(), false );
          body.append( str );
        }

      } else if ( cmd.startsWith( "OTIMELONGEN") ) {
        kdDebug() << "Command: OTIMELONGEN" << endl;
        i += strlen( "OTIMELONGEN");
        if ( mOrigMsg ) {
          TQDateTime date;
          date.setTime_t( mOrigMsg->date() );
          KLocale locale( "C");
          TQString str = locale.formatTime( date.time(), true );
          body.append( str );
        }

      } else if ( cmd.startsWith( "OTIMELONG") ) {
        kdDebug() << "Command: OTIMELONG" << endl;
        i += strlen( "OTIMELONG");
        if ( mOrigMsg ) {
          TQDateTime date;
          date.setTime_t( mOrigMsg->date() );
          TQString str = KGlobal::locale()->formatTime( date.time(), true );
          body.append( str );
        }

      } else if ( cmd.startsWith( "OTIME") ) {
        kdDebug() << "Command: OTIME" << endl;
        i += strlen( "OTIME");
        if ( mOrigMsg ) {
          TQDateTime date;
          date.setTime_t( mOrigMsg->date() );
          TQString str = KGlobal::locale()->formatTime( date.time(), false );
          body.append( str );
        }

      } else if ( cmd.startsWith( "BLANK" ) ) {
        // do nothing
        kdDebug() << "Command: BLANK" << endl;
        i += strlen( "BLANK" );

      } else if ( cmd.startsWith( "NOP" ) ) {
        // do nothing
        kdDebug() << "Command: NOP" << endl;
        i += strlen( "NOP" );

      } else if ( cmd.startsWith( "CLEAR" ) ) {
        // clear body buffer; not too useful yet
        kdDebug() << "Command: CLEAR" << endl;
        i += strlen( "CLEAR" );
        body = "";
        mMsg->setCursorPos( 0 );

      } else if ( cmd.startsWith( "DEBUGOFF" ) ) {
        // turn off debug
        kdDebug() << "Command: DEBUGOFF" << endl;
        i += strlen( "DEBUGOFF" );
        mDebug = false;

      } else if ( cmd.startsWith( "DEBUG" ) ) {
        // turn on debug
        kdDebug() << "Command: DEBUG" << endl;
        i += strlen( "DEBUG" );
        mDebug = true;

      } else if ( cmd.startsWith( "CURSOR" ) ) {
        // turn on debug
        kdDebug() << "Command: CURSOR" << endl;
        i += strlen( "CURSOR" );
        mMsg->setCursorPos( body.length() );

      } else {
        // wrong command, do nothing
        body.append( c );
      }

    } else if ( dnl && ( c == '\n' || c == '\r') ) {
      // skip
      if ( ( c == '\n' && tmpl[i + 1] == '\r' ) ||
           ( c == '\r' && tmpl[i + 1] == '\n' ) ) {
        // skip one more
        i += 1;
      }
      dnl = false;
    } else {
      body.append( c );
    }
  }

  // kdDebug() << "Message body: " << body << endl;

  if ( mAppend ) {
    TQCString msg_body = mMsg->body();
    msg_body.append( body.utf8() );
    mMsg->setBody( msg_body );
  } else {
    mMsg->setBodyFromUnicode( body );
  }
}

TQString TemplateParser::findCustomTemplate( const TQString &tmplName )
{
  CTemplates t( tmplName );
  TQString content = t.content();
  if ( !content.isEmpty() ) {
    return content;
  } else {
    return findTemplate();
  }
}

TQString TemplateParser::findTemplate()
{
  // import 'Phrases' if it not done yet
  if ( !GlobalSettings::self()->phrasesConverted() ) {
    TemplatesConfiguration::importFromPhrases();
  }

  // kdDebug() << "Trying to find template for mode " << mode << endl;

  TQString tmpl;

  if ( !mFolder ) { // find folder message belongs to
    mFolder = mMsg->parent();
    if ( !mFolder ) {
      if ( mOrigMsg ) {
        mFolder = mOrigMsg->parent();
      }
      if ( !mFolder ) {
        kdDebug(5006) << "Oops! No folder for message" << endl;
      }
    }
  }
  kdDebug(5006) << "Folder found: " << mFolder << endl;

  if ( mFolder )  // only if a folder was found
  {
    TQString fid = mFolder->idString();
    Templates fconf( fid );
    if ( fconf.useCustomTemplates() ) {   // does folder use custom templates?
      switch( mMode ) {
      case NewMessage:
        tmpl = fconf.templateNewMessage();
        break;
      case Reply:
        tmpl = fconf.templateReply();
        break;
      case ReplyAll:
        tmpl = fconf.templateReplyAll();
        break;
      case Forward:
        tmpl = fconf.templateForward();
        break;
      default:
        kdDebug(5006) << "Unknown message mode: " << mMode << endl;
        return "";
      }
      mQuoteString = fconf.quoteString();
      if ( !tmpl.isEmpty() ) {
        return tmpl;  // use folder-specific template
      }
    }
  }

  if ( !mIdentity ) { // find identity message belongs to
    mIdentity = mMsg->identityUoid();
    if ( !mIdentity && mOrigMsg ) {
      mIdentity = mOrigMsg->identityUoid();
    }
    mIdentity = kmkernel->identityManager()->identityForUoidOrDefault( mIdentity ).uoid();
    if ( !mIdentity ) {
      kdDebug(5006) << "Oops! No identity for message" << endl;
    }
  }
  kdDebug(5006) << "Identity found: " << mIdentity << endl;

  TQString iid;
  if ( mIdentity ) {
    iid = TQString("IDENTITY_%1").arg( mIdentity );	// templates ID for that identity
  }
  else {
    iid = "IDENTITY_NO_IDENTITY"; // templates ID for no identity
  }

  Templates iconf( iid );
  if ( iconf.useCustomTemplates() ) { // does identity use custom templates?
    switch( mMode ) {
    case NewMessage:
      tmpl = iconf.templateNewMessage();
      break;
    case Reply:
      tmpl = iconf.templateReply();
      break;
    case ReplyAll:
      tmpl = iconf.templateReplyAll();
      break;
    case Forward:
      tmpl = iconf.templateForward();
      break;
    default:
      kdDebug(5006) << "Unknown message mode: " << mMode << endl;
      return "";
    }
    mQuoteString = iconf.quoteString();
    if ( !tmpl.isEmpty() ) {
      return tmpl;  // use identity-specific template
    }
  }

  switch( mMode ) { // use the global template
  case NewMessage:
    tmpl = GlobalSettings::self()->templateNewMessage();
    break;
  case Reply:
    tmpl = GlobalSettings::self()->templateReply();
    break;
  case ReplyAll:
    tmpl = GlobalSettings::self()->templateReplyAll();
    break;
  case Forward:
    tmpl = GlobalSettings::self()->templateForward();
    break;
  default:
    kdDebug(5006) << "Unknown message mode: " << mMode << endl;
    return "";
  }

  mQuoteString = GlobalSettings::self()->quoteString();
  return tmpl;
}

TQString TemplateParser::pipe( const TQString &cmd, const TQString &buf )
{
  mPipeOut = "";
  mPipeErr = "";
  mPipeRc = 0;

  KProcess proc;
  TQCString data = buf.local8Bit();

  // kdDebug() << "Command data: " << data << endl;

  proc << KShell::splitArgs( cmd, KShell::TildeExpand );
  proc.setUseShell( true );
  connect( &proc, TQT_SIGNAL( receivedStdout( KProcess *, char *, int ) ),
           this, TQT_SLOT( onReceivedStdout( KProcess *, char *, int ) ) );
  connect( &proc, TQT_SIGNAL( receivedStderr( KProcess *, char *, int ) ),
           this, TQT_SLOT( onReceivedStderr( KProcess *, char *, int ) ) );
  connect( &proc, TQT_SIGNAL( wroteStdin( KProcess * ) ),
           this, TQT_SLOT( onWroteStdin( KProcess * ) ) );

  if ( proc.start( KProcess::NotifyOnExit, KProcess::All ) ) {

    bool pipe_filled = proc.writeStdin( data, data.length() );
    if ( pipe_filled ) {
      proc.closeStdin();

      bool exited = proc.wait( PipeTimeout );
      if ( exited ) {

        if ( proc.normalExit() ) {

          mPipeRc = proc.exitStatus();
          if ( mPipeRc != 0 && mDebug ) {
            if ( mPipeErr.isEmpty() ) {
              KMessageBox::error( 0,
                                  i18n( "Pipe command exit with status %1: %2").
                                  arg( mPipeRc ).arg( cmd ) );
            } else {
              KMessageBox::detailedError( 0,
                                          i18n( "Pipe command exit with status %1: %2" ).
                                          arg( mPipeRc ).arg( cmd ), mPipeErr );
            }
          }

        } else {

          mPipeRc = -( proc.exitSignal() );
          if ( mPipeRc != 0 && mDebug ) {
            if ( mPipeErr.isEmpty() ) {
              KMessageBox::error( 0,
                                  i18n( "Pipe command killed by signal %1: %2" ).
                                  arg( -(mPipeRc) ).arg( cmd ) );
            } else {
              KMessageBox::detailedError( 0,
                                          i18n( "Pipe command killed by signal %1: %2" ).
                                          arg( -(mPipeRc) ).arg( cmd ), mPipeErr );
            }
          }
        }

      } else {
        // process does not exited after TemplateParser::PipeTimeout seconds, kill it
        proc.kill();
        proc.detach();
        if ( mDebug ) {
          KMessageBox::error( 0,
                              i18n( "Pipe command did not finish within %1 seconds: %2" ).
                              arg( PipeTimeout ).arg( cmd ) );
        }
      }

    } else {
      // can`t write to stdin of process
      proc.kill();
      proc.detach();
      if ( mDebug ) {
        if ( mPipeErr.isEmpty() ) {
          KMessageBox::error( 0,
                              i18n( "Cannot write to process stdin: %1" ).arg( cmd ) );
        } else {
          KMessageBox::detailedError( 0,
                                      i18n( "Cannot write to process stdin: %1" ).
                                      arg( cmd ), mPipeErr );
        }
      }
    }

  } else if ( mDebug ) {
    KMessageBox::error( 0,
                        i18n( "Cannot start pipe command from template: %1" ).
                        arg( cmd ) );
  }

  return mPipeOut;
}

void TemplateParser::onProcessExited( KProcess *proc )
{
  Q_UNUSED( proc );
  // do nothing for now
}

void TemplateParser::onReceivedStdout( KProcess *proc, char *buffer, int buflen )
{
  Q_UNUSED( proc );
  mPipeOut += TQString::fromLocal8Bit( buffer, buflen );
}

void TemplateParser::onReceivedStderr( KProcess *proc, char *buffer, int buflen )
{
  Q_UNUSED( proc );
  mPipeErr += TQString::fromLocal8Bit( buffer, buflen );
}

void TemplateParser::onWroteStdin( KProcess *proc )
{
  proc->closeStdin();
}

#include "templateparser.moc"
