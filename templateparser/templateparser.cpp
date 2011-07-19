/*   -*- mode: C++; c-file-style: "gnu" -*-
 *   kmail: KDE mail client
 *   Copyright (C) 2006 Dmitry Morozhnikov <dmiceman@mail.ru>
 *   Copyright (C) 2011 Sudhendu Kumar <sudhendu.kumar.roy@gmail.com>
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
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "templateparser.h"
#ifndef Q_OS_WINCE
#include "customtemplates_kfg.h"
#include "templatesconfiguration_kfg.h"
#include "templatesconfiguration.h"
#endif
#include "globalsettings_base.h"


#include "messagecore/stringutil.h"
#include "messagecore/attachmentcollector.h"
#include "messageviewer/objecttreeparser.h"
#include "messageviewer/nodehelper.h"
#include "messagecomposer/messagehelper.h"

#include <akonadi/collection.h>

#include <kmime/kmime_message.h>
#include <kmime/kmime_content.h>

#include <kpimidentities/identity.h>
#include <kpimidentities/identitymanager.h>

#include <klocale.h>
#include <kcalendarsystem.h>
#include <kglobal.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kshell.h>
#include <kcharsets.h>
#include <QString>
#include <QWebPage>
#include <QWebFrame>
#include <QDateTime>
#include <QRegExp>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextCodec>

namespace TemplateParser {

static const int PipeTimeout = 15 * 1000;

QByteArray selectCharset( const QStringList &charsets, const QString &text )
{
  foreach( const QString &name, charsets ) {
    // We use KCharsets::codecForName() instead of QTextCodec::codecForName() here, because
    // the former knows us-ascii is latin1.
    QTextCodec *codec = KGlobal::charsets()->codecForName( name );
    if( !codec ) {
      kWarning() << "Could not get text codec for charset" << name;
      continue;
    }
    if( codec->canEncode( text ) ) {
      // Special check for us-ascii (needed because us-ascii is not exactly latin1).
      if( name == "us-ascii" && !KMime::isUsAscii( text ) ) {
        continue;
      }
      kDebug() << "Chosen charset" << name;
      return name.toLatin1();
    }
  }
  kDebug() << "No appropriate charset found.";
  return "utf-8";
}

TemplateParser::TemplateParser( const KMime::Message::Ptr &amsg, const Mode amode ) :
  mMode( amode ), mIdentity( 0 ),
  mAllowDecryption( false ),
  mDebug( false ), mQuoteString( "> " ), m_identityManager( 0 ), mWrap( true ), mColWrap( 80 )
{
  mMsg = amsg;
  mRoot = new KMime::Content;
  mOtp = new MessageViewer::ObjectTreeParser( );
  mOtp->setAllowAsync( false );
}

void TemplateParser::setSelection( const QString &selection )
{
  mSelection = selection;
}

void TemplateParser::setAllowDecryption( const bool allowDecryption )
{
  mAllowDecryption = allowDecryption;
}

bool TemplateParser::shouldStripSignature() const
{
  // Only strip the signature when replying, it should be preserved when forwarding
  return ( mMode == Reply || mMode == ReplyAll )
#ifndef Q_OS_WINCE
  && GlobalSettings::self()->stripSignature()
#endif
  ;
}

void TemplateParser::setIdentityManager( KPIMIdentities::IdentityManager* ident)
{
  m_identityManager = ident;
}

void TemplateParser::setCharsets( const QStringList& charsets )
{
  m_charsets = charsets;
}


TemplateParser::~TemplateParser()
{
}

int TemplateParser::parseQuotes( const QString &prefix, const QString &str,
                                 QString &quote ) const
{
  int pos = prefix.length();
  int len;
  int str_len = str.length();

  // Also allow the german lower double-quote sign as quote separator, not only
  // the standard ASCII quote ("). This fixes bug 166728.
  QList< QChar > quoteChars;
  quoteChars.append( '"' );
  quoteChars.append( 0x201C );

  QChar prev( QChar::Null );

  pos++;
  len = pos;

  while ( pos < str_len ) {
    QChar c = str[pos];

    pos++;
    len++;

    if ( !prev.isNull() ) {
      quote.append( c );
      prev = QChar::Null;
    } else {
      if ( c == '\\' ) {
        prev = c;
      } else if ( quoteChars.contains( c ) ) {
        break;
      } else {
        quote.append( c );
      }
    }
  }

  return len;
}

QString TemplateParser::getFName( const QString &str )
{
  // simple logic:
  // if there is ',' in name, than format is 'Last, First'
  // else format is 'First Last'
  // last resort -- return 'name' from 'name@domain'
  int sep_pos;
  QString res;
  if ( ( sep_pos = str.indexOf( '@' ) ) > 0 ) {
    int i;
    for ( i = (sep_pos - 1); i >= 0; --i ) {
      QChar c = str[i];
      if ( c.isLetterOrNumber() ) {
        res.prepend( c );
      } else {
        break;
      }
    }
  } else if ( ( sep_pos = str.indexOf(',') ) > 0 ) {
    int i;
    bool begin = false;
    for ( i = sep_pos; i < str.length(); ++i ) {
      QChar c = str[i];
      if ( c.isLetterOrNumber() ) {
        begin = true;
        res.append( c );
      } else if ( begin ) {
        break;
      }
    }
  } else {
    int i;
    for ( i = 0; i < str.length(); ++i ) {
      QChar c = str[i];
      if ( c.isLetterOrNumber() ) {
        res.append( c );
      } else {
        break;
      }
    }
  }
  return res;
}

QString TemplateParser::getLName( const QString &str )
{
  // simple logic:
  // if there is ',' in name, than format is 'Last, First'
  // else format is 'First Last'
  int sep_pos;
  QString res;
  if ( ( sep_pos = str.indexOf(',') ) > 0 ) {
    int i;
    for ( i = sep_pos; i >= 0; --i ) {
      QChar c = str[i];
      if ( c.isLetterOrNumber() ) {
        res.prepend( c );
      } else {
        break;
      }
    }
  } else {
    if ( ( sep_pos = str.indexOf( ' ' ) ) > 0 ) {
      int i;
      bool begin = false;
      for ( i = sep_pos; i < str.length(); ++i ) {
        QChar c = str[i];
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

void TemplateParser::process( const KMime::Message::Ptr &aorig_msg, const Akonadi::Collection & afolder )
{
  if( aorig_msg == 0 ) {
    kDebug() << "aorig_msg == 0!";
    return;
  }
  mOrigMsg = aorig_msg;
  mFolder = afolder;
  QString tmpl = findTemplate();
  if( tmpl.isEmpty()) return;
  return processWithTemplate( tmpl );
}

void TemplateParser::process( const QString &tmplName, const KMime::Message::Ptr &aorig_msg,
                              const Akonadi::Collection &afolder)
{
  mOrigMsg = aorig_msg;
  mFolder = afolder;
  QString tmpl = findCustomTemplate( tmplName );
  return processWithTemplate( tmpl );
}

void TemplateParser::processWithIdentity( uint uoid, const KMime::Message::Ptr &aorig_msg,
                                          const Akonadi::Collection &afolder)
{
  mIdentity = uoid;
  return process( aorig_msg, afolder );
}

void TemplateParser::processWithTemplate( const QString &tmpl )//TODO mAllowDecryption
{
  mOtp->parseObjectTree( mOrigMsg.get() );

  QString plainBody, htmlBody;

  int tmpl_len = tmpl.length();
  bool dnl = false;
  for ( int i = 0; i < tmpl_len; ++i ) {
    QChar c = tmpl[i];
    // kDebug() << "Next char: " << c;
    if ( c == '%' ) {
      QString cmd = tmpl.mid( i + 1 );

      if ( cmd.startsWith( '-' ) ) {
        // dnl
        kDebug() << "Command: -";
        dnl = true;
        i += 1;

      } else if ( cmd.startsWith( QLatin1String("REM=") ) ) {
        // comments
        kDebug() << "Command: REM=";
        QString q;
        int len = parseQuotes( "REM=", cmd, q );
        i += len;

      } else if ( cmd.startsWith( QLatin1String("INSERT=") ) ) {
        // insert content of specified file as is
        kDebug() << "Command: INSERT=";
        QString q;
        int len = parseQuotes( "INSERT=", cmd, q );
        i += len;
        QString path = KShell::tildeExpand( q );
        QFileInfo finfo( path );
        if (finfo.isRelative() ) {
          path = QDir::homePath();
          path += '/';
          path += q;
        }
        QFile file( path );
        if ( file.open( QIODevice::ReadOnly ) ) {
          QByteArray content = file.readAll();
          QString str = QString::fromLocal8Bit( content, content.size() );
          plainBody.append( str );
          htmlBody.append( str );
        } else if ( mDebug ) {
          KMessageBox::error( 0, i18nc( "@info:status", "Cannot insert content from file %1: %2", path, file.errorString() ) );
        }

      } else if ( cmd.startsWith( QLatin1String("SYSTEM=") ) ) {
        // insert content of specified file as is
        kDebug() << "Command: SYSTEM=";
        QString q;
        int len = parseQuotes( "SYSTEM=", cmd, q );
        i += len;
        QString pipe_cmd = q;
        QString str = pipe( pipe_cmd, "" );
        plainBody.append( str );
        htmlBody.append( str );

      } else if ( cmd.startsWith( QLatin1String("PUT=") ) ) {
        // insert content of specified file as is
        kDebug() << "Command: PUT=";
        QString q;
        int len = parseQuotes( "PUT=", cmd, q );
        i += len;
        QString path = KShell::tildeExpand( q );
        QFileInfo finfo( path );
        if (finfo.isRelative() ) {
          path = QDir::homePath();
          path += '/';
          path += q;
        }
        QFile file( path );
        if ( file.open( QIODevice::ReadOnly ) ) {
          QByteArray content = file.readAll();
          plainBody.append( QString::fromLocal8Bit( content, content.size() ) );
          htmlBody.append( QString::fromLocal8Bit( content, content.size() ) );
        } else if ( mDebug ) {
          KMessageBox::error( 0, i18nc( "@info:status", "Cannot insert content from file %1: %2", path, file.errorString() ));
        }

      } else if ( cmd.startsWith( QLatin1String("QUOTEPIPE=") ) ) {
        // pipe message body through command and insert it as quotation
        kDebug() << "Command: QUOTEPIPE=";
        QString q;
        int len = parseQuotes( "QUOTEPIPE=", cmd, q );
        i += len;
        QString pipe_cmd = q;
        if ( mOrigMsg ) {
          QString plainStr = pipe( pipe_cmd, plainMessageText( shouldStripSignature(), NoSelectionAllowed ) );
          QString plainQuote = quotedPlainText( plainStr );
          if ( plainQuote.endsWith( '\n' ) ) {
            plainQuote.chop( 1 );
          }
          plainBody.append( plainQuote );

          QString htmlStr = pipe( pipe_cmd, htmlMessageText( shouldStripSignature(), NoSelectionAllowed ) );
          QString htmlQuote = quotedHtmlText( htmlStr );
          htmlBody.append( htmlQuote );
        }

      } else if ( cmd.startsWith( QLatin1String("QUOTE") ) ) {
        kDebug() << "Command: QUOTE";
        i += strlen( "QUOTE" );
        if ( mOrigMsg ) {
          QString plainQuote = quotedPlainText( plainMessageText( shouldStripSignature(), SelectionAllowed ) );
          if ( plainQuote.endsWith( '\n' ) ) {
            plainQuote.chop( 1 );
          }
          plainBody.append( plainQuote );

          QString htmlQuote = quotedHtmlText( htmlMessageText( shouldStripSignature(), SelectionAllowed ) );
          htmlBody.append( htmlQuote );
        }

      } else if ( cmd.startsWith( QLatin1String("QHEADERS") ) ) {
        kDebug() << "Command: QHEADERS";
        i += strlen( "QHEADERS" );
        if ( mOrigMsg ) {
          QString plainQuote = quotedPlainText( MessageCore::StringUtil::headerAsSendableString( mOrigMsg ) );
          if ( plainQuote.endsWith( '\n' ) ) {
               plainQuote.chop( 1 );
          }
          plainBody.append( plainQuote );

          QString htmlQuote = quotedHtmlText( MessageCore::StringUtil::headerAsSendableString( mOrigMsg ) );
          htmlBody.append( htmlQuote );
        }

      } else if ( cmd.startsWith( QLatin1String("HEADERS") ) ) {
        kDebug() << "Command: HEADERS";
        i += strlen( "HEADERS" );
        if ( mOrigMsg ) {
          QString str = MessageCore::StringUtil::headerAsSendableString( mOrigMsg );
          plainBody.append( str );
          htmlBody.append( str );
        }

      } else if ( cmd.startsWith( QLatin1String("TEXTPIPE=") ) ) {
        // pipe message body through command and insert it as is
        kDebug() << "Command: TEXTPIPE=";
        QString q;
        int len = parseQuotes( "TEXTPIPE=", cmd, q );
        i += len;
        QString pipe_cmd = q;
        if ( mOrigMsg ) {
          QString plainStr = pipe( pipe_cmd, plainMessageText( shouldStripSignature(), NoSelectionAllowed ) );
          plainBody.append( plainStr );

          QString htmlStr = pipe( pipe_cmd, htmlMessageText( shouldStripSignature(), NoSelectionAllowed ) );
          htmlBody.append( htmlStr );
        }

      } else if ( cmd.startsWith( QLatin1String("MSGPIPE=") ) ) {
        // pipe full message through command and insert result as is
        kDebug() << "Command: MSGPIPE=";
        QString q;
        int len = parseQuotes( "MSGPIPE=", cmd, q );
        i += len;
        QString pipe_cmd = q;
        if ( mOrigMsg ) {
          QString str = pipe( pipe_cmd, mOrigMsg->encodedContent() );
          plainBody.append( str );
          htmlBody.append( str );
        }

      } else if ( cmd.startsWith( QLatin1String("BODYPIPE=") ) ) {
        // pipe message body generated so far through command and insert result as is
        kDebug() << "Command: BODYPIPE=";
        QString q;
        int len = parseQuotes( "BODYPIPE=", cmd, q );
        i += len;
        QString pipe_cmd = q;
        QString plainStr = pipe( pipe_cmd, plainBody );
        plainBody.append( plainStr );

        QString htmlStr = pipe( pipe_cmd, htmlBody );
        htmlBody.append( htmlStr );

      } else if ( cmd.startsWith( QLatin1String("CLEARPIPE=") ) ) {
        // pipe message body generated so far through command and
        // insert result as is replacing current body
        kDebug() << "Command: CLEARPIPE=";
        QString q;
        int len = parseQuotes( "CLEARPIPE=", cmd, q );
        i += len;
        QString pipe_cmd = q;
        QString plainStr = pipe( pipe_cmd, plainBody );
        plainBody = plainStr;

        QString htmlStr = pipe( pipe_cmd, htmlBody );
        htmlBody = htmlStr;

        KMime::Headers::Generic *header = new KMime::Headers::Generic( "X-KMail-CursorPos", mMsg.get(), QString::number( 0 ), "utf-8" );
        mMsg->setHeader( header );

      } else if ( cmd.startsWith( QLatin1String("TEXT") ) ) {
        kDebug() << "Command: TEXT";
        i += strlen( "TEXT" );
        if ( mOrigMsg ) {
          QString plainQuote = plainMessageText( shouldStripSignature(), NoSelectionAllowed );
          plainBody.append( plainQuote );

          QString htmlQuote = htmlMessageText( shouldStripSignature(), NoSelectionAllowed );
          htmlBody.append( htmlQuote );
        }

      } else if ( cmd.startsWith( QLatin1String("OTEXTSIZE") ) ) {
        kDebug() << "Command: OTEXTSIZE";
        i += strlen( "OTEXTSIZE" );
        if ( mOrigMsg ) {
          QString str = QString( "%1" ).arg( mOrigMsg->body().length() );
          plainBody.append( str );
          htmlBody.append( str );
        }

      } else if ( cmd.startsWith( QLatin1String("OTEXT") ) ) {
        kDebug() << "Command: OTEXT";
        i += strlen( "OTEXT" );
        if ( mOrigMsg ) {
          QString plainQuote = plainMessageText( shouldStripSignature(), NoSelectionAllowed );
          plainBody.append( plainQuote );

          QString htmlQuote = htmlMessageText( shouldStripSignature(), NoSelectionAllowed );
          htmlBody.append( htmlQuote );
        }

      } else if ( cmd.startsWith( QLatin1String("OADDRESSEESADDR") ) ) {
        kDebug() << "Command: OADDRESSEESADDR";
        i += strlen( "OADDRESSEESADDR" );
        if ( mOrigMsg ) {
          const QString to = mOrigMsg->to()->asUnicodeString();
          const QString cc = mOrigMsg->cc()->asUnicodeString();
          if ( !to.isEmpty() ) {
            plainBody.append( i18n( "To:" ) + QLatin1Char( ' ' ) + to );
            htmlBody.append( i18n( "To:" ) + QLatin1Char( ' ' ) + to );
          }
          if ( !to.isEmpty() && !cc.isEmpty() ) {
            plainBody.append( QLatin1Char( '\n' ) );
            htmlBody.append( QLatin1Char( '\n' ) );
          }
          if ( !cc.isEmpty() ) {
            plainBody.append( i18n( "CC:" ) + QLatin1Char( ' ' ) +  cc );
            htmlBody.append( i18n( "CC:" ) + QLatin1Char( ' ' ) +  cc );
          }
	}

      } else if ( cmd.startsWith( QLatin1String("CCADDR") ) ) {
        kDebug() << "Command: CCADDR";
        i += strlen( "CCADDR" );
        QString str = mMsg->cc()->asUnicodeString();
        plainBody.append( str );
        htmlBody.append( str );

      } else if ( cmd.startsWith( QLatin1String("CCNAME") ) ) {
        kDebug() << "Command: CCNAME";
        i += strlen( "CCNAME" );
        const QByteArray str = MessageCore::StringUtil::stripEmailAddr( mMsg->cc()->as7BitString( false ) );
        plainBody.append( str );
        htmlBody.append( str );

      } else if ( cmd.startsWith( QLatin1String("CCFNAME") ) ) {
        kDebug() << "Command: CCFNAME";
        i += strlen( "CCFNAME" );
        const QByteArray str = MessageCore::StringUtil::stripEmailAddr( mMsg->cc()->as7BitString( false ) );
        plainBody.append( getFName( str ) );
        htmlBody.append( getFName( str ) );

      } else if ( cmd.startsWith( QLatin1String("CCLNAME") ) ) {
        kDebug() << "Command: CCLNAME";
        i += strlen( "CCLNAME" );
        const QByteArray str = MessageCore::StringUtil::stripEmailAddr( mMsg->cc()->as7BitString( false ) );
        plainBody.append( getLName( str ) );
        htmlBody.append( getLName( str ) );

      } else if ( cmd.startsWith( QLatin1String("TOADDR") ) ) {
        kDebug() << "Command: TOADDR";
        i += strlen( "TOADDR" );
        QString str = mMsg->to()->asUnicodeString();
        plainBody.append( str );
        htmlBody.append( str );

      } else if ( cmd.startsWith( QLatin1String("TONAME") ) ) {
        kDebug() << "Command: TONAME";
        i += strlen( "TONAME" );
        const QByteArray str = MessageCore::StringUtil::stripEmailAddr( mMsg->to()->as7BitString( false ) );
        plainBody.append( str );
        htmlBody.append( str );

      } else if ( cmd.startsWith( QLatin1String("TOFNAME") ) ) {
        kDebug() << "Command: TOFNAME";
        i += strlen( "TOFNAME" );
        const QByteArray str = MessageCore::StringUtil::stripEmailAddr( mMsg->to()->as7BitString( false ) );
        plainBody.append( getFName( str ) );
        htmlBody.append( getFName( str ) );

      } else if ( cmd.startsWith( QLatin1String("TOLNAME") ) ) {
        kDebug() << "Command: TOLNAME";
        i += strlen( "TOLNAME" );
        const QByteArray str = MessageCore::StringUtil::stripEmailAddr( mMsg->to()->as7BitString( false ) );
        plainBody.append( getLName( str ) );
        htmlBody.append( getLName( str ) );

      } else if ( cmd.startsWith( QLatin1String("TOLIST") ) ) {
        kDebug() << "Command: TOLIST";
        i += strlen( "TOLIST" );
        QString str = mMsg->to()->asUnicodeString();
        plainBody.append( str );
        htmlBody.append( str );

      } else if ( cmd.startsWith( QLatin1String("FROMADDR") ) ) {
        kDebug() << "Command: FROMADDR";
        i += strlen( "FROMADDR" );
        QString str = mMsg->from()->asUnicodeString();
        plainBody.append( str );
        htmlBody.append( str );

      } else if ( cmd.startsWith( QLatin1String("FROMNAME") ) ) {
        kDebug() << "Command: FROMNAME";
        i += strlen( "FROMNAME" );
        const QByteArray str = MessageCore::StringUtil::stripEmailAddr( mMsg->from()->as7BitString( false ) );
        plainBody.append( str );
        htmlBody.append( str );

      } else if ( cmd.startsWith( QLatin1String("FROMFNAME") ) ) {
        kDebug() << "Command: FROMFNAME";
        i += strlen( "FROMFNAME" );
        const QByteArray str = MessageCore::StringUtil::stripEmailAddr( mMsg->from()->as7BitString( false ) );
        plainBody.append( getFName( str ) );
        htmlBody.append( getFName( str ) );

      } else if ( cmd.startsWith( QLatin1String("FROMLNAME") ) ) {
        kDebug() << "Command: FROMLNAME";
        i += strlen( "FROMLNAME" );
        const QByteArray str = MessageCore::StringUtil::stripEmailAddr( mMsg->from()->as7BitString( false ) );
        plainBody.append( getLName( str ) );
        htmlBody.append( getLName( str ) );

      } else if ( cmd.startsWith( QLatin1String("FULLSUBJECT") ) ) {
        kDebug() << "Command: FULLSUBJECT";
        i += strlen( "FULLSUBJECT" );
        QString str = mMsg->subject()->asUnicodeString();
        plainBody.append( str );
        htmlBody.append( str );

      } else if ( cmd.startsWith( QLatin1String("FULLSUBJ") ) ) {
        kDebug() << "Command: FULLSUBJ";
        i += strlen( "FULLSUBJ" );
        QString str = mMsg->subject()->asUnicodeString();
        plainBody.append( str );
        htmlBody.append( str );

      } else if ( cmd.startsWith( QLatin1String("MSGID") ) ) {
        kDebug() << "Command: MSGID";
        i += strlen( "MSGID" );
        QString str = mMsg->messageID()->asUnicodeString();
        plainBody.append( str );
        htmlBody.append( str );

      } else if ( cmd.startsWith( QLatin1String("OHEADER=") ) ) {
        // insert specified content of header from original message
        kDebug() << "Command: OHEADER=";
        QString q;
        int len = parseQuotes( "OHEADER=", cmd, q );
        i += len;
        if ( mOrigMsg ) {
          QString hdr = q;
          QString str = mOrigMsg->headerByType(hdr.toLocal8Bit() ) ? mOrigMsg->headerByType(hdr.toLocal8Bit() )->asUnicodeString() : "";
          plainBody.append( str );
          htmlBody.append( str );
        }

      } else if ( cmd.startsWith( QLatin1String("HEADER=") ) ) {
        // insert specified content of header from current message
        kDebug() << "Command: HEADER=";
        QString q;
        int len = parseQuotes( "HEADER=", cmd, q );
        i += len;
        QString hdr = q;
        QString str = mMsg->headerByType(hdr.toLocal8Bit() ) ? mMsg->headerByType(hdr.toLocal8Bit() )->asUnicodeString() : "";
        plainBody.append( str );
        htmlBody.append( str );

      } else if ( cmd.startsWith( QLatin1String("HEADER( ") ) ) {
        // insert specified content of header from current message
        kDebug() << "Command: HEADER(";
        QRegExp re = QRegExp( "^HEADER\\((.+)\\)" );
        re.setMinimal( true );
        int res = re.indexIn( cmd );
        if ( res != 0 ) {
          // something wrong
          i += strlen( "HEADER( " );
        } else {
          i += re.matchedLength();
          QString hdr = re.cap( 1 );
          QString str = mMsg->headerByType( hdr.toLocal8Bit() ) ? mMsg->headerByType( hdr.toLocal8Bit() )->asUnicodeString() : "";
          plainBody.append( str );
          htmlBody.append( str );
        }

      } else if ( cmd.startsWith( QLatin1String("OCCADDR") ) ) {
        kDebug() << "Command: OCCADDR";
        i += strlen( "OCCADDR" );
        if ( mOrigMsg ) {
          QString str = mOrigMsg->cc()->asUnicodeString();
          plainBody.append( str );
          htmlBody.append( str );
        }

      } else if ( cmd.startsWith( QLatin1String("OCCNAME") ) ) {
        kDebug() << "Command: OCCNAME";
        i += strlen( "OCCNAME" );
        if ( mOrigMsg ) {
          const QByteArray str = MessageCore::StringUtil::stripEmailAddr( mOrigMsg->cc()->as7BitString( false ) );
          plainBody.append( str );
          htmlBody.append( str );
        }

      } else if ( cmd.startsWith( QLatin1String("OCCFNAME") ) ) {
        kDebug() << "Command: OCCFNAME";
        i += strlen( "OCCFNAME" );
        if ( mOrigMsg ) {
          const QByteArray str = MessageCore::StringUtil::stripEmailAddr( mOrigMsg->cc()->as7BitString( false ) );
          plainBody.append( getFName( str ) );
          htmlBody.append( getFName( str ) );
        }

      } else if ( cmd.startsWith( QLatin1String("OCCLNAME") ) ) {
        kDebug() << "Command: OCCLNAME";
        i += strlen( "OCCLNAME" );
        if ( mOrigMsg ) {
          const QByteArray str = MessageCore::StringUtil::stripEmailAddr( mOrigMsg->cc()->as7BitString( false ) );
          plainBody.append( getLName( str ) );
          htmlBody.append( getLName( str ) );
        }

      } else if ( cmd.startsWith( QLatin1String("OTOADDR") ) ) {
        kDebug() << "Command: OTOADDR";
        i += strlen( "OTOADDR" );
        if ( mOrigMsg ) {
          QString str = mOrigMsg->to()->asUnicodeString();
          plainBody.append( str );
          htmlBody.append( str );
        }

      } else if ( cmd.startsWith( QLatin1String("OTONAME") ) ) {
        kDebug() << "Command: OTONAME";
        i += strlen( "OTONAME" );
        if ( mOrigMsg ) {
          const QByteArray str = MessageCore::StringUtil::stripEmailAddr( mOrigMsg->to()->as7BitString( false ) );
          plainBody.append( str );
          htmlBody.append( str );
        }

      } else if ( cmd.startsWith( QLatin1String("OTOFNAME") ) ) {
        kDebug() << "Command: OTOFNAME";
        i += strlen( "OTOFNAME" );
        if ( mOrigMsg ) {
          const QByteArray str = MessageCore::StringUtil::stripEmailAddr( mOrigMsg->to()->as7BitString( false ) );
          plainBody.append( getFName( str ) );
          htmlBody.append( getFName( str ) );
        }

      } else if ( cmd.startsWith( QLatin1String("OTOLNAME") ) ) {
        kDebug() << "Command: OTOLNAME";
        i += strlen( "OTOLNAME" );
        if ( mOrigMsg ) {
          const QByteArray str = MessageCore::StringUtil::stripEmailAddr( mOrigMsg->to()->as7BitString( false ) );
          plainBody.append( getLName( str ) );
          htmlBody.append( getLName( str ) );
        }

      } else if ( cmd.startsWith( QLatin1String("OTOLIST") ) ) {
        kDebug() << "Command: OTOLIST";
        i += strlen( "OTOLIST" );
        if ( mOrigMsg ) {
          QString str = mOrigMsg->to()->asUnicodeString();
          plainBody.append( str );
          htmlBody.append( str );
        }

      } else if ( cmd.startsWith( QLatin1String("OTO") ) ) {
        kDebug() << "Command: OTO";
        i += strlen( "OTO" );
        if ( mOrigMsg ) {
          QString str = mOrigMsg->to()->asUnicodeString();
          plainBody.append( str );
          htmlBody.append( str );
        }

      } else if ( cmd.startsWith( QLatin1String("OFROMADDR") ) ) {
        kDebug() << "Command: OFROMADDR";
        i += strlen( "OFROMADDR" );
        if ( mOrigMsg ) {
          QString str = mOrigMsg->from()->asUnicodeString();
          plainBody.append( str );
          htmlBody.append( str );
        }

      } else if ( cmd.startsWith( QLatin1String("OFROMNAME") ) ) {
        kDebug() << "Command: OFROMNAME";
        i += strlen( "OFROMNAME" );
        if ( mOrigMsg ) {
          const QByteArray str = MessageCore::StringUtil::stripEmailAddr( mOrigMsg->from()->as7BitString( false ) );
          plainBody.append( str );
          htmlBody.append( str );
        }

      } else if ( cmd.startsWith( QLatin1String("OFROMFNAME") ) ) {
        kDebug() << "Command: OFROMFNAME";
        i += strlen( "OFROMFNAME" );
        if ( mOrigMsg ) {
          const QByteArray str = MessageCore::StringUtil::stripEmailAddr( mOrigMsg->from()->as7BitString( false ) );
          plainBody.append( getFName( str ) );
          htmlBody.append( getFName( str ) );
        }

      } else if ( cmd.startsWith( QLatin1String("OFROMLNAME") ) ) {
        kDebug() << "Command: OFROMLNAME";
        i += strlen( "OFROMLNAME" );
        if ( mOrigMsg ) {
          const QByteArray str = MessageCore::StringUtil::stripEmailAddr( mOrigMsg->from()->as7BitString( false ) );
          plainBody.append( getLName( str ) );
          htmlBody.append( getLName( str ) );
        }

      } else if ( cmd.startsWith( QLatin1String("OFULLSUBJECT") ) ) {
        kDebug() << "Command: OFULLSUBJECT";
        i += strlen( "OFULLSUBJECT" );
        if ( mOrigMsg ) {
          QString str = mOrigMsg->subject()->asUnicodeString();
          plainBody.append( str );
          htmlBody.append( str );
        }

      } else if ( cmd.startsWith( QLatin1String("OFULLSUBJ") ) ) {
        kDebug() << "Command: OFULLSUBJ";
        i += strlen( "OFULLSUBJ" );
        if ( mOrigMsg ) {
          QString str = mOrigMsg->subject()->asUnicodeString();
          plainBody.append( str );
          htmlBody.append( str );
        }

      } else if ( cmd.startsWith( QLatin1String("OMSGID") ) ) {
        kDebug() << "Command: OMSGID";
        i += strlen( "OMSGID" );
        if ( mOrigMsg ) {
          QString str = mOrigMsg->messageID()->asUnicodeString();
          plainBody.append( str );
          htmlBody.append( str );
        }

      } else if ( cmd.startsWith( QLatin1String("DATEEN") ) ) {
        kDebug() << "Command: DATEEN";
        i += strlen( "DATEEN" );
        QDateTime date = QDateTime::currentDateTime();
        KLocale locale( "C" );
        QString str = locale.formatDate( date.date(), KLocale::LongDate );
        plainBody.append( str );
        htmlBody.append( str );

      } else if ( cmd.startsWith( QLatin1String("DATESHORT") ) ) {
        kDebug() << "Command: DATESHORT";
        i += strlen( "DATESHORT" );
        QDateTime date = QDateTime::currentDateTime();
        QString str = KGlobal::locale()->formatDate( date.date(), KLocale::ShortDate );
        plainBody.append( str );
        htmlBody.append( str );

      } else if ( cmd.startsWith( QLatin1String("DATE") ) ) {
        kDebug() << "Command: DATE";
        i += strlen( "DATE" );
        QDateTime date = QDateTime::currentDateTime();
        QString str = KGlobal::locale()->formatDate( date.date(), KLocale::LongDate );
        plainBody.append( str );
        htmlBody.append( str );

      } else if ( cmd.startsWith( QLatin1String("DOW") ) ) {
        kDebug() << "Command: DOW";
        i += strlen( "DOW" );
        QDateTime date = QDateTime::currentDateTime();
        QString str = KGlobal::locale()->calendar()->weekDayName( date.date(),
                      KCalendarSystem::LongDayName );
        plainBody.append( str );
        htmlBody.append( str );

      } else if ( cmd.startsWith( QLatin1String("TIMELONGEN") ) ) {
        kDebug() << "Command: TIMELONGEN";
        i += strlen( "TIMELONGEN" );
        QDateTime date = QDateTime::currentDateTime();
        KLocale locale( "C");
        QString str = locale.formatTime( date.time(), true );
        plainBody.append( str );
        htmlBody.append( str );

      } else if ( cmd.startsWith( QLatin1String("TIMELONG") ) ) {
        kDebug() << "Command: TIMELONG";
        i += strlen( "TIMELONG" );
        QDateTime date = QDateTime::currentDateTime();
        QString str = KGlobal::locale()->formatTime( date.time(), true );
        plainBody.append( str );
        htmlBody.append( str );

      } else if ( cmd.startsWith( QLatin1String("TIME") ) ) {
        kDebug() << "Command: TIME";
        i += strlen( "TIME" );
        QDateTime date = QDateTime::currentDateTime();
        QString str = KGlobal::locale()->formatTime( date.time(), false );
        plainBody.append( str );
        htmlBody.append( str );

      } else if ( cmd.startsWith( QLatin1String("ODATEEN") ) ) {
        kDebug() << "Command: ODATEEN";
        i += strlen( "ODATEEN" );
        if ( mOrigMsg ) {
          QDateTime date = mOrigMsg->date()->dateTime().dateTime();
          KLocale locale( "C");
          QString str = locale.formatDate( date.date(), KLocale::LongDate );
          plainBody.append( str );
          htmlBody.append( str );
        }

      } else if ( cmd.startsWith( QLatin1String("ODATESHORT")) ) {
        kDebug() << "Command: ODATESHORT";
        i += strlen( "ODATESHORT");
        if ( mOrigMsg ) {
          QDateTime date = mOrigMsg->date()->dateTime().dateTime();
          QString str = KGlobal::locale()->formatDate( date.date(), KLocale::ShortDate );
          plainBody.append( str );
          htmlBody.append( str );
        }

      } else if ( cmd.startsWith( QLatin1String("ODATE")) ) {
        kDebug() << "Command: ODATE";
        i += strlen( "ODATE");
        if ( mOrigMsg ) {
          const QDateTime date = mOrigMsg->date()->dateTime().dateTime();
          const QString str = KGlobal::locale()->formatDate( date.date(), KLocale::LongDate );
          plainBody.append( str );
          htmlBody.append( str );
        }

      } else if ( cmd.startsWith( QLatin1String("ODOW")) ) {
        kDebug() << "Command: ODOW";
        i += strlen( "ODOW");
        if ( mOrigMsg ) {
          const QDateTime date = mOrigMsg->date()->dateTime().dateTime();
          const QString str = KGlobal::locale()->calendar()->weekDayName( date.date(), KCalendarSystem::LongDayName );
          plainBody.append( str );
          htmlBody.append( str );
        }

      } else if ( cmd.startsWith( QLatin1String("OTIMELONGEN")) ) {
        kDebug() << "Command: OTIMELONGEN";
        i += strlen( "OTIMELONGEN");
        if ( mOrigMsg ) {
          const QDateTime date = mOrigMsg->date()->dateTime().dateTime();
          KLocale locale( "C");
          const QString str = locale.formatTime( date.time(), true );
          plainBody.append( str );
          htmlBody.append( str );
        }

      } else if ( cmd.startsWith( QLatin1String("OTIMELONG")) ) {
        kDebug() << "Command: OTIMELONG";
        i += strlen( "OTIMELONG");
        if ( mOrigMsg ) {
          const QDateTime date = mOrigMsg->date()->dateTime().dateTime();
          const QString str = KGlobal::locale()->formatTime( date.time(), true );
          plainBody.append( str );
          htmlBody.append( str );
        }

      } else if ( cmd.startsWith( QLatin1String("OTIME")) ) {
        kDebug() << "Command: OTIME";
        i += strlen( "OTIME");
        if ( mOrigMsg ) {
          const QDateTime date = mOrigMsg->date()->dateTime().dateTime();
          const QString str = KGlobal::locale()->formatTime( date.time(), false );
          plainBody.append( str );
          htmlBody.append( str );
        }

      } else if ( cmd.startsWith( QLatin1String("BLANK") ) ) {
        // do nothing
        kDebug() << "Command: BLANK";
        i += strlen( "BLANK" );

      } else if ( cmd.startsWith( QLatin1String("NOP") ) ) {
        // do nothing
        kDebug() << "Command: NOP";
        i += strlen( "NOP" );

      } else if ( cmd.startsWith( QLatin1String("CLEAR") ) ) {
        // clear body buffer; not too useful yet
        kDebug() << "Command: CLEAR";
        i += strlen( "CLEAR" );
        plainBody = "";
        htmlBody = "";
        KMime::Headers::Generic *header = new KMime::Headers::Generic( "X-KMail-CursorPos", mMsg.get(), QString::number( 0 ), "utf-8" );
        mMsg->setHeader( header );
      } else if ( cmd.startsWith( QLatin1String("DEBUGOFF") ) ) {
        // turn off debug
        kDebug() << "Command: DEBUGOFF";
        i += strlen( "DEBUGOFF" );
        mDebug = false;

      } else if ( cmd.startsWith( QLatin1String("DEBUG") ) ) {
        // turn on debug
        kDebug() << "Command: DEBUG";
        i += strlen( "DEBUG" );
        mDebug = true;

      } else if ( cmd.startsWith( QLatin1String("CURSOR") ) ) {
        // turn on debug
        kDebug() << "Command: CURSOR";
        i += strlen( "CURSOR" );
        KMime::Headers::Generic *header = new KMime::Headers::Generic( "X-KMail-CursorPos", mMsg.get(), QString::number( plainBody.length()-1 ), "utf-8" );
        mMsg->setHeader( header );
        //FIXME HTML part for header remaining
      } else if ( cmd.startsWith( QLatin1String( "SIGNATURE" ) ) ) {
        kDebug() << "Command: SIGNATURE";
        i += strlen( "SIGNATURE" );
        plainBody.append( getPlainSignature() );
        //TODO getHtmlSignature
        //htmlBody.append( getHtmlSignature() );

      } else {
        // wrong command, do nothing
        plainBody.append( c );
        htmlBody.append( c );
      }

    } else if ( dnl && ( c == '\n' || c == '\r') ) {
      // skip
      if ( ( tmpl.size() > i+1 ) &&
           ( ( c == '\n' && tmpl[i + 1] == '\r' ) ||
             ( c == '\r' && tmpl[i + 1] == '\n' ) ) ) {
        // skip one more
        i += 1;
      }
      dnl = false;
    } else {
      plainBody.append( c );
      htmlBody.append( c );
    }
  }
  addProcessedBodyToMessage( plainBody, htmlBody );
}

QString TemplateParser::getPlainSignature() const
{
  const KPIMIdentities::Identity &identity =
    m_identityManager->identityForUoid( mIdentity );
  if ( identity.isNull() )
    return QString();

  KPIMIdentities::Signature signature = const_cast<KPIMIdentities::Identity&>
                                                  ( identity ).signature();
  if ( signature.type() == KPIMIdentities::Signature::Inlined &&
       signature.isInlinedHtml() ) {
    return signature.toPlainText();
  }
  else {
    return signature.rawText();
  }
}
/*
QString TemplateParser::getHtmlSignature() const
{
  const KPIMIdentities::Identity &identity =
    m_identityManager->identityForUoid( mIdentity );
  if ( identity.isNull() )
    return QString();

  KPIMIdentities::Signature signature = const_cast<KPIMIdentities::Identity&>
                                                  ( identity ).signature();
  return signature.rawText();
}
*/
void TemplateParser::addProcessedBodyToMessage( const QString &plainBody, const QString &htmlBody )
{
  // Get the attachments of the original mail
  MessageCore::AttachmentCollector ac;
  ac.collectAttachmentsFrom( mOrigMsg.get() );

  // Now, delete the old content and set the new content, which
  // is either only the new text or the new text with some attachments.
  KMime::Content::List parts = mMsg->contents();
  foreach ( KMime::Content *content, parts )
    mMsg->removeContent( content, true /*delete*/ );

  // Set To and CC from the template
  if ( !mTo.isEmpty() ) {
    mMsg->to()->fromUnicodeString( mMsg->to()->asUnicodeString() + ',' + mTo, "utf-8" );
  }
  if ( !mCC.isEmpty() )
    mMsg->cc()->fromUnicodeString( mMsg->cc()->asUnicodeString() + ',' + mCC, "utf-8" );

  mMsg->contentType()->clear(); // to get rid of old boundary
  // If we have no attachment, simply create a text/plain part or multipart/alternative
  //and set the processed template text as the body
  if ( ac.attachments().empty() || mMode != Forward ) {
    KMime::Content* const mainTextPart = mOtp->htmlContent().isEmpty() ?
      createPlainPartContent( plainBody ) : createMultipartAlternativeContent( plainBody, htmlBody );

    mainTextPart->assemble();
    mMsg->setBody( mainTextPart->encodedBody() );
    mMsg->setHeader( mainTextPart->contentType() );
    mMsg->setHeader( mainTextPart->contentTransferEncoding() );
  }

  // If we have some attachments, create a multipart/mixed mail and
  // add the normal body as well as the attachments
  else
  {
    const QByteArray boundary = KMime::multiPartBoundary();
    mMsg->contentType()->setMimeType( "multipart/mixed" );
    mMsg->contentType()->setBoundary( boundary );

    KMime::Content* const mainTextPart = mOtp->htmlContent().isEmpty() ?
      createPlainPartContent( plainBody ) : createMultipartAlternativeContent( plainBody, htmlBody );

    mMsg->addContent( mainTextPart );

    int attachmentNumber = 1;
    foreach( KMime::Content *attachment, ac.attachments() ) {
      mMsg->addContent( attachment );
      // If the content type has no name or filename parameter, add one, since otherwise the name
      // would be empty in the attachment view of the composer, which looks confusing
      if ( attachment->contentType( false ) ) {
        if ( !attachment->contentType()->hasParameter( "name" ) &&
              !attachment->contentType()->hasParameter( "filename" ) ) {
          attachment->contentType()->setParameter( "name", i18n( "Attachment %1", attachmentNumber ) );
        }
      }
      attachmentNumber++;
    }
  }
  mMsg->assemble();
}

KMime::Content* TemplateParser::createPlainPartContent( const QString& plainBody ) const
{
    KMime::Content *textPart = new KMime::Content( mMsg.get() );
    textPart->contentType()->setMimeType( "text/plain" );
    textPart->fromUnicodeString( plainBody );
    QByteArray charset = selectCharset( m_charsets, plainBody );
    textPart->contentType()->setCharset( charset );
    textPart->contentTransferEncoding()->setEncoding( KMime::Headers::CE7Bit );//FIXME doesn't work
    return textPart;
}

KMime::Content* TemplateParser::createMultipartAlternativeContent( const QString& plainBody, const QString& htmlBody ) const
{
  KMime::Content *multipartAlternative = new KMime::Content( mMsg.get() );
  multipartAlternative->contentType()->setMimeType( "multipart/alternative" );
  const QByteArray boundary = KMime::multiPartBoundary();
  multipartAlternative->contentType()->setBoundary( boundary );

  KMime::Content *textPart = createPlainPartContent( plainBody );
  multipartAlternative->addContent( textPart );

  KMime::Content *htmlPart = new KMime::Content( mMsg.get() );
  htmlPart->contentType()->setMimeType( "text/html" );
  QByteArray charset = selectCharset( m_charsets, htmlBody );
  htmlPart->contentType()->setCharset( charset );
  htmlPart->fromUnicodeString( htmlBody );
  htmlPart->contentTransferEncoding()->setEncoding( KMime::Headers::CE7Bit ); //FIXME doesn't work
  multipartAlternative->addContent( htmlPart );

  return multipartAlternative;
}

QString TemplateParser::findCustomTemplate( const QString &tmplName )
{
#ifndef Q_OS_WINCE
  CTemplates t( tmplName );
  mTo = t.to();
  mCC = t.cC();
  QString content = t.content();
  if ( !content.isEmpty() ) {
    return content;
  } else {
    return findTemplate();
  }
#else
    return findTemplate();
#endif
}

QString TemplateParser::findTemplate()
{
  // kDebug() << "Trying to find template for mode" << mode;

  QString tmpl;
#ifndef Q_OS_WINCE

#if 0
  if ( !mFolder.isValid() ) { // find folder message belongs to
    mFolder = mMsg->parentCollection();
    if ( !mFolder.isValid() ) {
      if ( mOrigMsg ) {
        mFolder = mOrigMsg->parentCollection();
      }
      if ( !mFolder.isValid() ) {
        kDebug() << "Oops! No folder for message";
      }
    }
  }
#else
  kDebug() << "AKONADI PORT: Disabled code in  " << Q_FUNC_INFO;
#endif
  kDebug() << "Folder found:" << mFolder;
  if ( mFolder.isValid() )  // only if a folder was found
  {
    QString fid = QString::number( mFolder.id() );
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
        kDebug() << "Unknown message mode:" << mMode;
        return "";
      }
      mQuoteString = fconf.quoteString();
      if ( !tmpl.isEmpty() ) {
        return tmpl;  // use folder-specific template
      }
    }
  }

  if ( !mIdentity ) { // find identity message belongs to
       kDebug() << "AKONADI PORT: verify Akonadi::Item() here  " << Q_FUNC_INFO;

    mIdentity = identityUoid( mMsg );
    if ( !mIdentity && mOrigMsg ) {
      kDebug() << "AKONADI PORT: verify Akonadi::Item() here  " << Q_FUNC_INFO;
      mIdentity = identityUoid( mOrigMsg );
    }
    mIdentity = m_identityManager->identityForUoidOrDefault( mIdentity ).uoid();
    if ( !mIdentity ) {
      kDebug() << "Oops! No identity for message";
    }
  }
  kDebug() << "Identity found:" << mIdentity;

  QString iid;
  if ( mIdentity ) {
    iid = TemplatesConfiguration::configIdString( mIdentity );	// templates ID for that identity
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
      kDebug() << "Unknown message mode:" << mMode;
      return "";
    }
    mQuoteString = iconf.quoteString();
    if ( !tmpl.isEmpty() ) {
      return tmpl;  // use identity-specific template
    }
  }
#endif

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
    kDebug() << "Unknown message mode:" << mMode;
    return "";
  }

  mQuoteString = DefaultTemplates::defaultQuoteString();
  return tmpl;
}

QString TemplateParser::pipe( const QString &cmd, const QString &buf )
{
  KProcess process;
  bool success, finished;

  process.setOutputChannelMode( KProcess::SeparateChannels );
  process.setShellCommand( cmd );
  process.start();
  if ( process.waitForStarted( PipeTimeout ) ) {
    if ( !buf.isEmpty() )
      process.write( buf.toAscii() );
    if ( buf.isEmpty() || process.waitForBytesWritten( PipeTimeout ) ) {
      if ( !buf.isEmpty() )
        process.closeWriteChannel();
      if ( process.waitForFinished( PipeTimeout ) ) {
        success = ( process.exitStatus() == QProcess::NormalExit );
        finished = true;
      }
      else {
        finished = false;
        success = false;
      }
    }
    else {
      success = false;
      finished = false;
    }

    // The process has started, but did not finish in time. Kill it.
    if ( !finished )
      process.kill();
  }
  else
    success = false;

  if ( !success && mDebug )
    KMessageBox::error( 0, i18nc( "@info",
                                  "Pipe command <command>%1</command> failed.",
                                  cmd ) );

  if ( success )
    return process.readAllStandardOutput();
  else
    return QString();
}

void TemplateParser::setWordWrap(bool wrap, int wrapColWidth)
{
  mWrap = wrap;
  mColWrap = wrapColWidth;
}

QString TemplateParser::plainMessageText( bool aStripSignature, AllowSelection isSelectionAllowed ) const
{
  if ( !mSelection.isEmpty() && ( isSelectionAllowed == SelectionAllowed ) ) {
    return mSelection;
  }

  if ( !mOrigMsg ) {
    return QString();
  }

  QString result = mOtp->plainTextContent();

  if ( result.isEmpty() ) { //HTML-only mails
    result = mOtp->convertedTextContent();
  }

  if ( aStripSignature ) {
    result = MessageCore::StringUtil::stripSignature( result );
  }

  return result;
}

QString TemplateParser::htmlMessageText( bool aStripSignature, AllowSelection isSelectionAllowed ) const
{
  const QString htmlElement = mOtp->htmlContent();

  QWebPage page;
  //TODO to be tested/verified if this is not an issue
  page.settings()->setAttribute( QWebSettings::JavascriptEnabled, false );
  page.settings()->setAttribute( QWebSettings::JavaEnabled, false );
  page.settings()->setAttribute( QWebSettings::PluginsEnabled, false );

  page.currentFrame()->setHtml( htmlElement );

  page.settings()->setAttribute( QWebSettings::JavascriptEnabled, true );
  const QString bodyElement = page.currentFrame()->evaluateJavaScript(
    "document.getElementsByTagName('body')[0].innerHTML()").toString();

  page.settings()->setAttribute( QWebSettings::JavascriptEnabled, false );

  //TODO: Extract any style sheet in the head part
  if( !bodyElement.isEmpty() && ( isSelectionAllowed == NoSelectionAllowed ) ) {
    return bodyElement;
  }

  if ( aStripSignature ) {
    return MessageCore::StringUtil::stripSignature( htmlElement );
  }

  return htmlElement;
}

QString TemplateParser::quotedPlainText( const QString& selection /*.clear() */) const
{
  QString content = selection;
  // Remove blank lines at the beginning:
  const int firstNonWS = content.indexOf( QRegExp( "\\S" ) );
  const int lineStart = content.lastIndexOf( '\n', firstNonWS );
  if ( lineStart >= 0 )
    content.remove( 0, static_cast<unsigned int>( lineStart ) );

  const QString indentStr = MessageCore::StringUtil::formatString( mQuoteString, mOrigMsg->from()->asUnicodeString() );
#ifndef Q_OS_WINCE
  if ( GlobalSettings::self()->smartQuote() && mWrap) {
    content = MessageCore::StringUtil::smartQuote( content, mColWrap - indentStr.length() );
  }
#endif
  content.replace( '\n', '\n' + indentStr );
  content.prepend( indentStr );
  content += '\n';

  return content;
}

QString TemplateParser::quotedHtmlText( const QString& selection /*.clear() */) const
{
  QString content = selection;
//TODO look for all the variations of <br>  and remove the blank lines
//TODO implement vertical bar for quoted HTML mail
  return content;
}

uint TemplateParser::identityUoid( const KMime::Message::Ptr &msg ) const
{
  QString idString;
  if ( msg->headerByType("X-KMail-Identity") )
    idString = msg->headerByType("X-KMail-Identity")->asUnicodeString().trimmed();
  bool ok = false;
  int id = idString.toUInt( &ok );

  if ( !ok || id == 0 )
    id = m_identityManager->identityForAddress( msg->to()->asUnicodeString() + ", " + msg->cc()->asUnicodeString() ).uoid();

  return id;
}


} // namespace TemplateParser

#include "templateparser.moc"
