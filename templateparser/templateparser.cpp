/*
 * Copyright (C) 2006 Dmitry Morozhnikov <dmiceman@mail.ru>
 * Copyright (C) 2011 Sudhendu Kumar <sudhendu.kumar.roy@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "templateparser.h"
#include "globalsettings_base.h"
#include "customtemplates_kfg.h"
#include "templatesconfiguration_kfg.h"
#include "templatesconfiguration.h"

#include <messagecore/attachment/attachmentcollector.h>
#include <messagecore/misc/imagecollector.h>
#include <messagecore/utils/stringutil.h>

#include <messageviewer/viewer/objecttreeparser.h>

#include <KPIMIdentities/Identity>
#include <KPIMIdentities/IdentityManager>

#include <KCalendarSystem>
#include <KCharsets>
#include <KGlobal>
#include <KLocalizedString>
#include <KMessageBox>
#include <KProcess>
#include <KShell>
#include <QDebug>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextCodec>
#include <QWebFrame>
#include <QWebPage>
#include <QTextDocument>
#include <KLocalizedString>
#include <QLocale>

namespace TemplateParser {

static const int PipeTimeout = 15 * 1000;

QTextCodec *selectCharset( const QStringList &charsets, const QString &text )
{
    foreach ( const QString &name, charsets ) {
        // We use KCharsets::codecForName() instead of QTextCodec::codecForName() here, because
        // the former knows us-ascii is latin1.
        bool ok = true;
        QTextCodec *codec;
        if ( name == QLatin1String( "locale" ) ) {
            codec = QTextCodec::codecForLocale();
        } else {
            codec = KCharsets::charsets()->codecForName( name, ok );
        }
        if( !ok || !codec ) {
            qWarning() << "Could not get text codec for charset" << name;
            continue;
        }
        if( codec->canEncode( text ) ) {
            // Special check for us-ascii (needed because us-ascii is not exactly latin1).
            if( name == QLatin1String( "us-ascii" ) && !KMime::isUsAscii( text ) ) {
                continue;
            }
            qDebug() << "Chosen charset" << name << codec->name();
            return codec;
        }
    }
    qDebug() << "No appropriate charset found.";
    return KCharsets::charsets()->codecForName( QLatin1String("utf-8") );
}

TemplateParser::TemplateParser( const KMime::Message::Ptr &amsg, const Mode amode ) :
    mMode( amode ), mIdentity( 0 ),
    mAllowDecryption( true ),
    mDebug( false ), mQuoteString( QLatin1String("> ") ), m_identityManager( 0 ),
    mWrap( true ),
    mColWrap( 80 ),
    mQuotes( ReplyAsOriginalMessage ),
    mForceCursorPosition(false)
{
    mMsg = amsg;

    mEmptySource = new MessageViewer::EmptySource;
    mEmptySource->setAllowDecryption( mAllowDecryption );

    mOtp = new MessageViewer::ObjectTreeParser( mEmptySource );
    mOtp->setAllowAsync( false );
}

void TemplateParser::setSelection( const QString &selection )
{
    mSelection = selection;
}

void TemplateParser::setAllowDecryption( const bool allowDecryption )
{
    mAllowDecryption = allowDecryption;
    mEmptySource->setAllowDecryption( mAllowDecryption );
}

bool TemplateParser::shouldStripSignature() const
{
    // Only strip the signature when replying, it should be preserved when forwarding
    return ( mMode == Reply || mMode == ReplyAll ) && GlobalSettings::self()->stripSignature();
}

void TemplateParser::setIdentityManager( KPIMIdentities::IdentityManager *ident )
{
    m_identityManager = ident;
}

void TemplateParser::setCharsets( const QStringList &charsets )
{
    m_charsets = charsets;
}

TemplateParser::~TemplateParser()
{
    delete mEmptySource;
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
    quoteChars.append( QLatin1Char('"') );
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
            if ( c == QLatin1Char('\\') ) {
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
    if ( ( sep_pos = str.indexOf( QLatin1Char( '@' ) ) ) > 0 ) {
        int i;
        for ( i = ( sep_pos - 1 ); i >= 0; --i ) {
            QChar c = str[i];
            if ( c.isLetterOrNumber() ) {
                res.prepend( c );
            } else {
                break;
            }
        }
    } else if ( ( sep_pos = str.indexOf( QLatin1Char( ',' ) ) ) > 0 ) {
        int i;
        bool begin = false;
        const int strLength( str.length() );
        for ( i = sep_pos; i < strLength; ++i ) {
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
        const int strLength( str.length() );
        for ( i = 0; i < strLength; ++i ) {
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
    if ( ( sep_pos = str.indexOf( QLatin1Char( ',' ) ) ) > 0 ) {
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
        if ( ( sep_pos = str.indexOf( QLatin1Char( ' ' ) ) ) > 0 ) {
            bool begin = false;
            const int strLength( str.length() );
            for ( int i = sep_pos; i < strLength; ++i ) {
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

void TemplateParser::process( const KMime::Message::Ptr &aorig_msg,
                              const Akonadi::Collection & afolder )
{
    if( aorig_msg == 0 ) {
        qDebug() << "aorig_msg == 0!";
        return;
    }
    mOrigMsg = aorig_msg;
    mFolder = afolder;
    const QString tmpl = findTemplate();
    if ( tmpl.isEmpty() ) {
        return;
    }
    processWithTemplate( tmpl );
}

void TemplateParser::process( const QString &tmplName, const KMime::Message::Ptr &aorig_msg,
                              const Akonadi::Collection &afolder )
{
    mForceCursorPosition = false;
    mOrigMsg = aorig_msg;
    mFolder = afolder;
    const QString tmpl = findCustomTemplate( tmplName );
    processWithTemplate( tmpl );
}

void TemplateParser::processWithIdentity( uint uoid, const KMime::Message::Ptr &aorig_msg,
                                          const Akonadi::Collection &afolder )
{
    mIdentity = uoid;
    process( aorig_msg, afolder );
}

void TemplateParser::processWithTemplate( const QString &tmpl )
{
    mOtp->parseObjectTree( mOrigMsg.get() );
    const int tmpl_len = tmpl.length();
    QString plainBody, htmlBody;

    bool dnl = false;
    for ( int i = 0; i < tmpl_len; ++i ) {
        QChar c = tmpl[i];
        // qDebug() << "Next char: " << c;
        if ( c == QLatin1Char('%') ) {
            const QString cmd = tmpl.mid( i + 1 );

            if ( cmd.startsWith( QLatin1Char( '-' ) ) ) {
                // dnl
                qDebug() << "Command: -";
                dnl = true;
                i += 1;

            } else if ( cmd.startsWith( QLatin1String( "REM=" ) ) ) {
                // comments
                qDebug() << "Command: REM=";
                QString q;
                int len = parseQuotes( QLatin1String("REM="), cmd, q );
                i += len;

            } else if ( cmd.startsWith( QLatin1String( "INSERT=" ) ) ) {
                // insert content of specified file as is
                qDebug() << "Command: INSERT=";
                QString q;
                int len = parseQuotes( QLatin1String("INSERT="), cmd, q );
                i += len;
                QString path = KShell::tildeExpand( q );
                QFileInfo finfo( path );
                if ( finfo.isRelative() ) {
                    path = QDir::homePath();
                    path += QLatin1Char('/');
                    path += q;
                }
                QFile file( path );
                if ( file.open( QIODevice::ReadOnly ) ) {
                    const QByteArray content = file.readAll();
                    const QString str = QString::fromLocal8Bit( content, content.size() );
                    plainBody.append( str );
                    const QString body = plainToHtml( str );
                    htmlBody.append( body );
                } else if ( mDebug ) {
                    KMessageBox::error(
                                0,
                                i18nc( "@info",
                                       "Cannot insert content from file %1: %2", path, file.errorString() ) );
                }

            } else if ( cmd.startsWith( QLatin1String( "SYSTEM=" ) ) ) {
                // insert content of specified file as is
                qDebug() << "Command: SYSTEM=";
                QString q;
                int len = parseQuotes( QLatin1String("SYSTEM="), cmd, q );
                i += len;
                const QString pipe_cmd = q;
                const QString str = pipe( pipe_cmd, QString() );
                plainBody.append( str );
                const QString body = plainToHtml( str );
                htmlBody.append( body );

            } else if ( cmd.startsWith( QLatin1String( "PUT=" ) ) ) {
                // insert content of specified file as is
                qDebug() << "Command: PUT=";
                QString q;
                int len = parseQuotes( QLatin1String("PUT="), cmd, q );
                i += len;
                QString path = KShell::tildeExpand( q );
                QFileInfo finfo( path );
                if ( finfo.isRelative() ) {
                    path = QDir::homePath();
                    path += QLatin1Char('/');
                    path += q;
                }
                QFile file( path );
                if ( file.open( QIODevice::ReadOnly ) ) {
                    const QByteArray content = file.readAll();
                    plainBody.append( QString::fromLocal8Bit( content, content.size() ) );

                    const QString body = plainToHtml( QString::fromLocal8Bit( content, content.size() ) );
                    htmlBody.append( body );
                } else if ( mDebug ) {
                    KMessageBox::error(
                                0,
                                i18nc( "@info",
                                       "Cannot insert content from file %1: %2", path, file.errorString() ) );
                }

            } else if ( cmd.startsWith( QLatin1String( "QUOTEPIPE=" ) ) ) {
                // pipe message body through command and insert it as quotation
                qDebug() << "Command: QUOTEPIPE=";
                QString q;
                int len = parseQuotes( QLatin1String("QUOTEPIPE="), cmd, q );
                i += len;
                const QString pipe_cmd = q;
                if ( mOrigMsg ) {
                    const QString plainStr =
                            pipe( pipe_cmd, plainMessageText( shouldStripSignature(), NoSelectionAllowed ) );
                    QString plainQuote = quotedPlainText( plainStr );
                    if ( plainQuote.endsWith( QLatin1Char('\n') ) ) {
                        plainQuote.chop( 1 );
                    }
                    plainBody.append( plainQuote );

                    const QString htmlStr =
                            pipe( pipe_cmd, htmlMessageText( shouldStripSignature(), NoSelectionAllowed ) );
                    const QString htmlQuote = quotedHtmlText( htmlStr );
                    htmlBody.append( htmlQuote );
                }

            } else if ( cmd.startsWith( QLatin1String( "QUOTE" ) ) ) {
                qDebug() << "Command: QUOTE";
                i += strlen( "QUOTE" );
                if ( mOrigMsg ) {
                    QString plainQuote =
                            quotedPlainText( plainMessageText( shouldStripSignature(), SelectionAllowed ) );
                    if ( plainQuote.endsWith( QLatin1Char('\n') ) ) {
                        plainQuote.chop( 1 );
                    }
                    plainBody.append( plainQuote );

                    const QString htmlQuote =
                            quotedHtmlText( htmlMessageText( shouldStripSignature(), SelectionAllowed ) );
                    htmlBody.append( htmlQuote );
                }

            } else if ( cmd.startsWith( QLatin1String( "FORCEDPLAIN" ) ) ) {
                qDebug() << "Command: FORCEDPLAIN";
                mQuotes = ReplyAsPlain;
                i += strlen( "FORCEDPLAIN" );

            } else if ( cmd.startsWith( QLatin1String( "FORCEDHTML" ) ) ) {
                qDebug() << "Command: FORCEDHTML";
                mQuotes = ReplyAsHtml;
                i += strlen( "FORCEDHTML" );

            } else if ( cmd.startsWith( QLatin1String( "QHEADERS" ) ) ) {
                qDebug() << "Command: QHEADERS";
                i += strlen( "QHEADERS" );
                if ( mOrigMsg ) {
                    QString plainQuote =
                            quotedPlainText( QString::fromLatin1(MessageCore::StringUtil::headerAsSendableString( mOrigMsg )) );
                    if ( plainQuote.endsWith( QLatin1Char('\n') ) ) {
                        plainQuote.chop( 1 );
                    }
                    plainBody.append( plainQuote );

                    const QString htmlQuote =
                            quotedHtmlText( QString::fromLatin1(MessageCore::StringUtil::headerAsSendableString( mOrigMsg )) );
                    const QString str = plainToHtml( htmlQuote );
                    htmlBody.append( str );
                }

            } else if ( cmd.startsWith( QLatin1String( "HEADERS" ) ) ) {
                qDebug() << "Command: HEADERS";
                i += strlen( "HEADERS" );
                if ( mOrigMsg ) {
                    const QString str = QString::fromLatin1(MessageCore::StringUtil::headerAsSendableString( mOrigMsg ));
                    plainBody.append( str );
                    const QString body = plainToHtml( str );
                    htmlBody.append( body );
                }

            } else if ( cmd.startsWith( QLatin1String( "TEXTPIPE=" ) ) ) {
                // pipe message body through command and insert it as is
                qDebug() << "Command: TEXTPIPE=";
                QString q;
                int len = parseQuotes( QLatin1String("TEXTPIPE="), cmd, q );
                i += len;
                const QString pipe_cmd = q;
                if ( mOrigMsg ) {
                    const QString plainStr =
                            pipe( pipe_cmd, plainMessageText( shouldStripSignature(), NoSelectionAllowed ) );
                    plainBody.append( plainStr );

                    const QString htmlStr =
                            pipe( pipe_cmd, htmlMessageText( shouldStripSignature(), NoSelectionAllowed ) );
                    htmlBody.append( htmlStr );
                }

            } else if ( cmd.startsWith( QLatin1String( "MSGPIPE=" ) ) ) {
                // pipe full message through command and insert result as is
                qDebug() << "Command: MSGPIPE=";
                QString q;
                int len = parseQuotes( QLatin1String("MSGPIPE="), cmd, q );
                i += len;
                if ( mOrigMsg ) {
                    QString pipe_cmd = q;
                    const QString str = pipe( pipe_cmd, QString::fromLatin1(mOrigMsg->encodedContent()) );
                    plainBody.append( str );

                    const QString body = plainToHtml( str );
                    htmlBody.append( body );
                }

            } else if ( cmd.startsWith( QLatin1String( "BODYPIPE=" ) ) ) {
                // pipe message body generated so far through command and insert result as is
                qDebug() << "Command: BODYPIPE=";
                QString q;
                int len = parseQuotes( QLatin1String("BODYPIPE="), cmd, q );
                i += len;
                const QString pipe_cmd = q;
                const QString plainStr = pipe( pipe_cmd, plainBody );
                plainBody.append( plainStr );

                const QString htmlStr = pipe( pipe_cmd, htmlBody );
                const QString body = plainToHtml( htmlStr );
                htmlBody.append( body );

            } else if ( cmd.startsWith( QLatin1String( "CLEARPIPE=" ) ) ) {
                // pipe message body generated so far through command and
                // insert result as is replacing current body
                qDebug() << "Command: CLEARPIPE=";
                QString q;
                int len = parseQuotes( QLatin1String("CLEARPIPE="), cmd, q );
                i += len;
                const QString pipe_cmd = q;
                const QString plainStr = pipe( pipe_cmd, plainBody );
                plainBody = plainStr;

                const QString htmlStr = pipe( pipe_cmd, htmlBody );
                htmlBody = htmlStr;

                KMime::Headers::Generic *header =
                        new KMime::Headers::Generic( "X-KMail-CursorPos", mMsg.get(),
                                                     QString::number( 0 ), "utf-8" );
                mMsg->setHeader( header );

            } else if ( cmd.startsWith( QLatin1String( "TEXT" ) ) ) {
                qDebug() << "Command: TEXT";
                i += strlen( "TEXT" );
                if ( mOrigMsg ) {
                    const QString plainStr = plainMessageText( shouldStripSignature(), NoSelectionAllowed );
                    plainBody.append( plainStr );

                    const QString htmlStr = htmlMessageText( shouldStripSignature(), NoSelectionAllowed );
                    htmlBody.append( htmlStr );
                }

            } else if ( cmd.startsWith( QLatin1String( "OTEXTSIZE" ) ) ) {
                qDebug() << "Command: OTEXTSIZE";
                i += strlen( "OTEXTSIZE" );
                if ( mOrigMsg ) {
                    const QString str = QString::fromLatin1( "%1" ).arg( mOrigMsg->body().length() );
                    plainBody.append( str );
                    const QString body = plainToHtml( str );
                    htmlBody.append( body );
                }

            } else if ( cmd.startsWith( QLatin1String( "OTEXT" ) ) ) {
                qDebug() << "Command: OTEXT";
                i += strlen( "OTEXT" );
                if ( mOrigMsg ) {
                    const QString plainStr = plainMessageText( shouldStripSignature(), NoSelectionAllowed );
                    plainBody.append( plainStr );

                    const QString htmlStr = htmlMessageText( shouldStripSignature(), NoSelectionAllowed );
                    htmlBody.append( htmlStr );
                }

            } else if ( cmd.startsWith( QLatin1String( "OADDRESSEESADDR" ) ) ) {
                qDebug() << "Command: OADDRESSEESADDR";
                i += strlen( "OADDRESSEESADDR" );
                if ( mOrigMsg ) {
                    const QString to = mOrigMsg->to()->asUnicodeString();
                    const QString cc = mOrigMsg->cc()->asUnicodeString();
                    if ( !to.isEmpty() ) {
                        QString toLine =  i18nc( "@item:intext email To", "To:" ) + QLatin1Char( ' ' ) + to;
                        plainBody.append( toLine );
                        const QString body = plainToHtml( toLine );
                        htmlBody.append( body );
                    }
                    if ( !to.isEmpty() && !cc.isEmpty() ) {
                        plainBody.append( QLatin1Char( '\n' ) );
                        const QString str = plainToHtml( QString( QLatin1Char( '\n' ) ) );
                        htmlBody.append( str );
                    }
                    if ( !cc.isEmpty() ) {
                        QString ccLine = i18nc( "@item:intext email CC", "CC:" ) + QLatin1Char( ' ' ) +  cc;
                        plainBody.append( ccLine );
                        const QString str = plainToHtml( ccLine );
                        htmlBody.append( str );
                    }
                }

            } else if ( cmd.startsWith( QLatin1String( "CCADDR" ) ) ) {
                qDebug() << "Command: CCADDR";
                i += strlen( "CCADDR" );
                const QString str = mMsg->cc()->asUnicodeString();
                plainBody.append( str );
                const QString body = plainToHtml( str );
                htmlBody.append( body );

            } else if ( cmd.startsWith( QLatin1String( "CCNAME" ) ) ) {
                qDebug() << "Command: CCNAME";
                i += strlen( "CCNAME" );
                const QString str =
                        MessageCore::StringUtil::stripEmailAddr( mMsg->cc()->asUnicodeString( ) );
                plainBody.append( str );
                const QString body = plainToHtml( str );
                htmlBody.append( body );

            } else if ( cmd.startsWith( QLatin1String( "CCFNAME" ) ) ) {
                qDebug() << "Command: CCFNAME";
                i += strlen( "CCFNAME" );
                const QString str =
                        MessageCore::StringUtil::stripEmailAddr( mMsg->cc()->asUnicodeString( ) );
                plainBody.append( getFName( str ) );
                const QString body = plainToHtml( getFName( str ) );
                htmlBody.append( body );

            } else if ( cmd.startsWith( QLatin1String( "CCLNAME" ) ) ) {
                qDebug() << "Command: CCLNAME";
                i += strlen( "CCLNAME" );
                const QString str =
                        MessageCore::StringUtil::stripEmailAddr( mMsg->cc()->asUnicodeString( ) );
                plainBody.append( getLName( str ) );
                const QString body = plainToHtml( getLName( str ) );
                htmlBody.append( body );

            } else if ( cmd.startsWith( QLatin1String( "TOADDR" ) ) ) {
                qDebug() << "Command: TOADDR";
                i += strlen( "TOADDR" );
                const QString str = mMsg->to()->asUnicodeString();
                plainBody.append( str );
                const QString body = plainToHtml( str );
                htmlBody.append( body );

            } else if ( cmd.startsWith( QLatin1String( "TONAME" ) ) ) {
                qDebug() << "Command: TONAME";
                i += strlen( "TONAME" );
                const QString str =
                        MessageCore::StringUtil::stripEmailAddr( mMsg->to()->asUnicodeString( ) );
                plainBody.append( str );
                const QString body = plainToHtml( str );
                htmlBody.append( body );

            } else if ( cmd.startsWith( QLatin1String( "TOFNAME" ) ) ) {
                qDebug() << "Command: TOFNAME";
                i += strlen( "TOFNAME" );
                const QString str =
                        MessageCore::StringUtil::stripEmailAddr( mMsg->to()->asUnicodeString( ) );
                plainBody.append( getFName( str ) );
                const QString body = plainToHtml( getFName( str ) );
                htmlBody.append( body );

            } else if ( cmd.startsWith( QLatin1String( "TOLNAME" ) ) ) {
                qDebug() << "Command: TOLNAME";
                i += strlen( "TOLNAME" );
                const QString str =
                        MessageCore::StringUtil::stripEmailAddr( mMsg->to()->asUnicodeString( ) );
                plainBody.append( getLName( str ) );
                const QString body = plainToHtml( getLName( str ) );
                htmlBody.append( body );

            } else if ( cmd.startsWith( QLatin1String( "TOLIST" ) ) ) {
                qDebug() << "Command: TOLIST";
                i += strlen( "TOLIST" );
                const QString str = mMsg->to()->asUnicodeString();
                plainBody.append( str );
                const QString body = plainToHtml( str );
                htmlBody.append( body );

            } else if ( cmd.startsWith( QLatin1String( "FROMADDR" ) ) ) {
                qDebug() << "Command: FROMADDR";
                i += strlen( "FROMADDR" );
                const QString str = mMsg->from()->asUnicodeString();
                plainBody.append( str );
                const QString body = plainToHtml( str );
                htmlBody.append( body );

            } else if ( cmd.startsWith( QLatin1String( "FROMNAME" ) ) ) {
                qDebug() << "Command: FROMNAME";
                i += strlen( "FROMNAME" );
                const QString str =
                        MessageCore::StringUtil::stripEmailAddr( mMsg->from()->asUnicodeString( ) );
                plainBody.append( str );
                const QString body = plainToHtml( str );
                htmlBody.append( body );

            } else if ( cmd.startsWith( QLatin1String( "FROMFNAME" ) ) ) {
                qDebug() << "Command: FROMFNAME";
                i += strlen( "FROMFNAME" );
                const QString str =
                        MessageCore::StringUtil::stripEmailAddr( mMsg->from()->asUnicodeString( ) );
                plainBody.append( getFName( str ) );
                const QString body = plainToHtml( getFName( str ) );
                htmlBody.append( body );

            } else if ( cmd.startsWith( QLatin1String( "FROMLNAME" ) ) ) {
                qDebug() << "Command: FROMLNAME";
                i += strlen( "FROMLNAME" );
                const QString str =
                        MessageCore::StringUtil::stripEmailAddr( mMsg->from()->asUnicodeString( ) );
                plainBody.append( getLName( str ) );
                const QString body = plainToHtml( getLName( str ) );
                htmlBody.append( body );

            } else if ( cmd.startsWith( QLatin1String( "FULLSUBJECT" ) ) ) {
                qDebug() << "Command: FULLSUBJECT";
                i += strlen( "FULLSUBJECT" );
                const QString str = mMsg->subject()->asUnicodeString();
                plainBody.append( str );
                const QString body = plainToHtml( str );
                htmlBody.append( body );

            } else if ( cmd.startsWith( QLatin1String( "FULLSUBJ" ) ) ) {
                qDebug() << "Command: FULLSUBJ";
                i += strlen( "FULLSUBJ" );
                const QString str = mMsg->subject()->asUnicodeString();
                plainBody.append( str );
                const QString body = plainToHtml( str );
                htmlBody.append( body );

            } else if ( cmd.startsWith( QLatin1String( "MSGID" ) ) ) {
                qDebug() << "Command: MSGID";
                i += strlen( "MSGID" );
                const QString str = mMsg->messageID()->asUnicodeString();
                plainBody.append( str );
                const QString body = plainToHtml( str );
                htmlBody.append( body );

            } else if ( cmd.startsWith( QLatin1String( "OHEADER=" ) ) ) {
                // insert specified content of header from original message
                qDebug() << "Command: OHEADER=";
                QString q;
                int len = parseQuotes( QLatin1String("OHEADER="), cmd, q );
                i += len;
                if ( mOrigMsg ) {
                    const QString hdr = q;
                    const QString str =
                            mOrigMsg->headerByType( hdr.toLocal8Bit() ) ?
                                mOrigMsg->headerByType( hdr.toLocal8Bit() )->asUnicodeString() :
                                QString();
                    plainBody.append( str );
                    const QString body = plainToHtml( str );
                    htmlBody.append( body );
                }

            } else if ( cmd.startsWith( QLatin1String( "HEADER=" ) ) ) {
                // insert specified content of header from current message
                qDebug() << "Command: HEADER=";
                QString q;
                int len = parseQuotes( QLatin1String("HEADER="), cmd, q );
                i += len;
                const QString hdr = q;
                const QString str =
                        mMsg->headerByType( hdr.toLocal8Bit() ) ?
                            mMsg->headerByType( hdr.toLocal8Bit() )->asUnicodeString() :
                            QString();
                plainBody.append( str );
                const QString body = plainToHtml( str );
                htmlBody.append( body );

            } else if ( cmd.startsWith( QLatin1String( "HEADER( " ) ) ) {
                // insert specified content of header from current message
                qDebug() << "Command: HEADER(";
                QRegExp re = QRegExp( QLatin1String("^HEADER\\((.+)\\)") );
                re.setMinimal( true );
                int res = re.indexIn( cmd );
                if ( res != 0 ) {
                    // something wrong
                    i += strlen( "HEADER( " );
                } else {
                    i += re.matchedLength();
                    const QString hdr = re.cap( 1 );
                    const QString str =
                            mMsg->headerByType( hdr.toLocal8Bit() ) ?
                                mMsg->headerByType( hdr.toLocal8Bit() )->asUnicodeString() :
                                QString();
                    plainBody.append( str );
                    const QString body = plainToHtml( str );
                    htmlBody.append( body );
                }

            } else if ( cmd.startsWith( QLatin1String( "OCCADDR" ) ) ) {
                qDebug() << "Command: OCCADDR";
                i += strlen( "OCCADDR" );
                if ( mOrigMsg ) {
                    const QString str = mOrigMsg->cc()->asUnicodeString();
                    plainBody.append( str );
                    const QString body = plainToHtml( str );
                    htmlBody.append( body );
                }

            } else if ( cmd.startsWith( QLatin1String( "OCCNAME" ) ) ) {
                qDebug() << "Command: OCCNAME";
                i += strlen( "OCCNAME" );
                if ( mOrigMsg ) {
                    const QString str =
                            MessageCore::StringUtil::stripEmailAddr( mOrigMsg->cc()->asUnicodeString( ) );
                    plainBody.append( str );
                    const QString body = plainToHtml( str );
                    htmlBody.append( body );
                }

            } else if ( cmd.startsWith( QLatin1String( "OCCFNAME" ) ) ) {
                qDebug() << "Command: OCCFNAME";
                i += strlen( "OCCFNAME" );
                if ( mOrigMsg ) {
                    const QString str =
                            MessageCore::StringUtil::stripEmailAddr( mOrigMsg->cc()->asUnicodeString( ) );
                    plainBody.append( getFName( str ) );
                    const QString body = plainToHtml( getFName( str ) );
                    htmlBody.append( body );
                }

            } else if ( cmd.startsWith( QLatin1String( "OCCLNAME" ) ) ) {
                qDebug() << "Command: OCCLNAME";
                i += strlen( "OCCLNAME" );
                if ( mOrigMsg ) {
                    const QString str =
                            MessageCore::StringUtil::stripEmailAddr( mOrigMsg->cc()->asUnicodeString( ) );
                    plainBody.append( getLName( str ) );
                    const QString body = plainToHtml( getLName( str ) );
                    htmlBody.append( body );
                }

            } else if ( cmd.startsWith( QLatin1String( "OTOADDR" ) ) ) {
                qDebug() << "Command: OTOADDR";
                i += strlen( "OTOADDR" );
                if ( mOrigMsg ) {
                    const QString str = mOrigMsg->to()->asUnicodeString();
                    plainBody.append( str );
                    const QString body = plainToHtml( str );
                    htmlBody.append( body );
                }

            } else if ( cmd.startsWith( QLatin1String( "OTONAME" ) ) ) {
                qDebug() << "Command: OTONAME";
                i += strlen( "OTONAME" );
                if ( mOrigMsg ) {
                    const QString str =
                            MessageCore::StringUtil::stripEmailAddr( mOrigMsg->to()->asUnicodeString( ) );
                    plainBody.append( str );
                    const QString body = plainToHtml( str );
                    htmlBody.append( body );
                }

            } else if ( cmd.startsWith( QLatin1String( "OTOFNAME" ) ) ) {
                qDebug() << "Command: OTOFNAME";
                i += strlen( "OTOFNAME" );
                if ( mOrigMsg ) {
                    const QString str =
                            MessageCore::StringUtil::stripEmailAddr( mOrigMsg->to()->asUnicodeString( ) );
                    plainBody.append( getFName( str ) );
                    const QString body = plainToHtml( getFName( str ) );
                    htmlBody.append( body );
                }

            } else if ( cmd.startsWith( QLatin1String( "OTOLNAME" ) ) ) {
                qDebug() << "Command: OTOLNAME";
                i += strlen( "OTOLNAME" );
                if ( mOrigMsg ) {
                    const QString str =
                            MessageCore::StringUtil::stripEmailAddr( mOrigMsg->to()->asUnicodeString( ) );
                    plainBody.append( getLName( str ) );
                    const QString body = plainToHtml( getLName( str ) );
                    htmlBody.append( body );
                }

            } else if ( cmd.startsWith( QLatin1String( "OTOLIST" ) ) ) {
                qDebug() << "Command: OTOLIST";
                i += strlen( "OTOLIST" );
                if ( mOrigMsg ) {
                    const QString str = mOrigMsg->to()->asUnicodeString();
                    plainBody.append( str );
                    const QString body = plainToHtml( str );
                    htmlBody.append( body );
                }

            } else if ( cmd.startsWith( QLatin1String( "OTO" ) ) ) {
                qDebug() << "Command: OTO";
                i += strlen( "OTO" );
                if ( mOrigMsg ) {
                    const QString str = mOrigMsg->to()->asUnicodeString();
                    plainBody.append( str );
                    const QString body = plainToHtml( str );
                    htmlBody.append( body );
                }

            } else if ( cmd.startsWith( QLatin1String( "OFROMADDR" ) ) ) {
                qDebug() << "Command: OFROMADDR";
                i += strlen( "OFROMADDR" );
                if ( mOrigMsg ) {
                    const QString str = mOrigMsg->from()->asUnicodeString();
                    plainBody.append( str );
                    const QString body = plainToHtml( str );
                    htmlBody.append( body );
                }

            } else if ( cmd.startsWith( QLatin1String( "OFROMNAME" ) ) ) {
                qDebug() << "Command: OFROMNAME";
                i += strlen( "OFROMNAME" );
                if ( mOrigMsg ) {
                    const QString str =
                            MessageCore::StringUtil::stripEmailAddr( mOrigMsg->from()->asUnicodeString() );
                    plainBody.append( str );
                    const QString body = plainToHtml( str );
                    htmlBody.append( body );
                }

            } else if ( cmd.startsWith( QLatin1String( "OFROMFNAME" ) ) ) {
                qDebug() << "Command: OFROMFNAME";
                i += strlen( "OFROMFNAME" );
                if ( mOrigMsg ) {
                    const QString str =
                            MessageCore::StringUtil::stripEmailAddr( mOrigMsg->from()->asUnicodeString() );
                    plainBody.append( getFName( str ) );
                    const QString body = plainToHtml( getFName( str ) );
                    htmlBody.append( body );
                }

            } else if ( cmd.startsWith( QLatin1String( "OFROMLNAME" ) ) ) {
                qDebug() << "Command: OFROMLNAME";
                i += strlen( "OFROMLNAME" );
                if ( mOrigMsg ) {
                    const QString str =
                            MessageCore::StringUtil::stripEmailAddr( mOrigMsg->from()->asUnicodeString() );
                    plainBody.append( getLName( str ) );
                    const QString body = plainToHtml( getLName( str ) );
                    htmlBody.append( body );
                }

            } else if ( cmd.startsWith( QLatin1String( "OFULLSUBJECT" ) ) ) {
                qDebug() << "Command: OFULLSUBJECT";
                i += strlen( "OFULLSUBJECT" );
                if ( mOrigMsg ) {
                    const QString str = mOrigMsg->subject()->asUnicodeString();
                    plainBody.append( str );
                    const QString body = plainToHtml( str );
                    htmlBody.append( body );
                }

            } else if ( cmd.startsWith( QLatin1String( "OFULLSUBJ" ) ) ) {
                qDebug() << "Command: OFULLSUBJ";
                i += strlen( "OFULLSUBJ" );
                if ( mOrigMsg ) {
                    const QString str = mOrigMsg->subject()->asUnicodeString();
                    plainBody.append( str );
                    const QString body = plainToHtml( str );
                    htmlBody.append( body );
                }

            } else if ( cmd.startsWith( QLatin1String( "OMSGID" ) ) ) {
                qDebug() << "Command: OMSGID";
                i += strlen( "OMSGID" );
                if ( mOrigMsg ) {
                    const QString str = mOrigMsg->messageID()->asUnicodeString();
                    plainBody.append( str );
                    const QString body = plainToHtml( str );
                    htmlBody.append( body );
                }

            } else if ( cmd.startsWith( QLatin1String( "DATEEN" ) ) ) {
                qDebug() << "Command: DATEEN";
                i += strlen( "DATEEN" );
                const QDateTime date = QDateTime::currentDateTime();
                KLocale locale( QLatin1String("C") );
                const QString str = locale.formatDate( date.date(), KLocale::LongDate );
                plainBody.append( str );
                const QString body = plainToHtml( str );
                htmlBody.append( body );

            } else if ( cmd.startsWith( QLatin1String( "DATESHORT" ) ) ) {
                qDebug() << "Command: DATESHORT";
                i += strlen( "DATESHORT" );
                const QDateTime date = QDateTime::currentDateTime();
                const QString str = QLocale().toString( date.date(), QLocale::ShortFormat );
                plainBody.append( str );
                const QString body = plainToHtml( str );
                htmlBody.append( body );

            } else if ( cmd.startsWith( QLatin1String( "DATE" ) ) ) {
                qDebug() << "Command: DATE";
                i += strlen( "DATE" );
                const QDateTime date = QDateTime::currentDateTime();
                const QString str = QLocale().toString( date.date(), QLocale::LongFormat );
                plainBody.append( str );
                const QString body = plainToHtml( str );
                htmlBody.append( body );

            } else if ( cmd.startsWith( QLatin1String( "DOW" ) ) ) {
                qDebug() << "Command: DOW";
                i += strlen( "DOW" );
                const QDateTime date = QDateTime::currentDateTime();
                const QString str = KLocale::global()->calendar()->weekDayName( date.date(),
                                                                                KCalendarSystem::LongDayName );
                plainBody.append( str );
                const QString body = plainToHtml( str );
                htmlBody.append( body );

            } else if ( cmd.startsWith( QLatin1String( "TIMELONGEN" ) ) ) {
                qDebug() << "Command: TIMELONGEN";
                i += strlen( "TIMELONGEN" );
                const QDateTime date = QDateTime::currentDateTime();
                KLocale locale( QLatin1String("C") );
                const QString str = locale.formatTime( date.time(), true );
                plainBody.append( str );
                const QString body = plainToHtml( str );
                htmlBody.append( body );

            } else if ( cmd.startsWith( QLatin1String( "TIMELONG" ) ) ) {
                qDebug() << "Command: TIMELONG";
                i += strlen( "TIMELONG" );
                const QDateTime date = QDateTime::currentDateTime();
                const QString str = KLocale::global()->formatTime( date.time(), true );
                plainBody.append( str );
                const QString body = plainToHtml( str );
                htmlBody.append( body );

            } else if ( cmd.startsWith( QLatin1String( "TIME" ) ) ) {
                qDebug() << "Command: TIME";
                i += strlen( "TIME" );
                const QDateTime date = QDateTime::currentDateTime();
                const QString str = KLocale::global()->formatTime( date.time(), false );
                plainBody.append( str );
                const QString body = plainToHtml( str );
                htmlBody.append( body );

            } else if ( cmd.startsWith( QLatin1String( "ODATEEN" ) ) ) {
                qDebug() << "Command: ODATEEN";
                i += strlen( "ODATEEN" );
                if ( mOrigMsg ) {
                    const QDateTime date = mOrigMsg->date()->dateTime();
                    KLocale locale( QLatin1String("C") );
                    const QString str = locale.formatDate( date.date(), KLocale::LongDate );
                    plainBody.append( str );
                    const QString body = plainToHtml( str );
                    htmlBody.append( body );
                }

            } else if ( cmd.startsWith( QLatin1String( "ODATESHORT" ) ) ) {
                qDebug() << "Command: ODATESHORT";
                i += strlen( "ODATESHORT" );
                if ( mOrigMsg ) {
                    const QDateTime date = mOrigMsg->date()->dateTime();
                    const QString str = QLocale().toString( date.date(), QLocale::ShortFormat );
                    plainBody.append( str );
                    const QString body = plainToHtml( str );
                    htmlBody.append( body );
                }

            } else if ( cmd.startsWith( QLatin1String( "ODATE" ) ) ) {
                qDebug() << "Command: ODATE";
                i += strlen( "ODATE" );
                if ( mOrigMsg ) {
                    const QDateTime date = mOrigMsg->date()->dateTime();
                    const QString str = QLocale().toString( date.date(), QLocale::LongFormat );
                    plainBody.append( str );
                    const QString body = plainToHtml( str );
                    htmlBody.append( body );
                }

            } else if ( cmd.startsWith( QLatin1String( "ODOW" ) ) ) {
                qDebug() << "Command: ODOW";
                i += strlen( "ODOW" );
                if ( mOrigMsg ) {
                    const QDateTime date = mOrigMsg->date()->dateTime();
                    const QString str =
                            KLocale::global()->calendar()->weekDayName( date.date(), KCalendarSystem::LongDayName );
                    plainBody.append( str );
                    const QString body = plainToHtml( str );
                    htmlBody.append( body );
                }

            } else if ( cmd.startsWith( QLatin1String( "OTIMELONGEN" ) ) ) {
                qDebug() << "Command: OTIMELONGEN";
                i += strlen( "OTIMELONGEN" );
                if ( mOrigMsg ) {
                    const QDateTime date = mOrigMsg->date()->dateTime();
                    KLocale locale( QLatin1String("C") );
                    const QString str = locale.formatTime( date.time(), true );
                    plainBody.append( str );
                    const QString body = plainToHtml( str );
                    htmlBody.append( body );
                }

            } else if ( cmd.startsWith( QLatin1String( "OTIMELONG" ) ) ) {
                qDebug() << "Command: OTIMELONG";
                i += strlen( "OTIMELONG" );
                if ( mOrigMsg ) {
                    const QDateTime date = mOrigMsg->date()->dateTime();
                    const QString str = KLocale::global()->formatTime( date.time(), true );
                    plainBody.append( str );
                    const QString body = plainToHtml( str );
                    htmlBody.append( body );
                }

            } else if ( cmd.startsWith( QLatin1String( "OTIME" ) ) ) {
                qDebug() << "Command: OTIME";
                i += strlen( "OTIME" );
                if ( mOrigMsg ) {
                    const QDateTime date = mOrigMsg->date()->dateTime();
                    const QString str = KLocale::global()->formatTime( date.time(), false );
                    plainBody.append( str );
                    const QString body = plainToHtml( str );
                    htmlBody.append( body );
                }

            } else if ( cmd.startsWith( QLatin1String( "BLANK" ) ) ) {
                // do nothing
                qDebug() << "Command: BLANK";
                i += strlen( "BLANK" );

            } else if ( cmd.startsWith( QLatin1String( "NOP" ) ) ) {
                // do nothing
                qDebug() << "Command: NOP";
                i += strlen( "NOP" );

            } else if ( cmd.startsWith( QLatin1String( "CLEAR" ) ) ) {
                // clear body buffer; not too useful yet
                qDebug() << "Command: CLEAR";
                i += strlen( "CLEAR" );
                plainBody.clear();
                htmlBody.clear();
                KMime::Headers::Generic *header =
                        new KMime::Headers::Generic( "X-KMail-CursorPos", mMsg.get(),
                                                     QString::number( 0 ), "utf-8" );
                mMsg->setHeader( header );
            } else if ( cmd.startsWith( QLatin1String( "DEBUGOFF" ) ) ) {
                // turn off debug
                qDebug() << "Command: DEBUGOFF";
                i += strlen( "DEBUGOFF" );
                mDebug = false;

            } else if ( cmd.startsWith( QLatin1String( "DEBUG" ) ) ) {
                // turn on debug
                qDebug() << "Command: DEBUG";
                i += strlen( "DEBUG" );
                mDebug = true;

            } else if ( cmd.startsWith( QLatin1String( "CURSOR" ) ) ) {
                // turn on debug
                qDebug() << "Command: CURSOR";
                int oldI = i;
                i += strlen( "CURSOR" );
                KMime::Headers::Generic *header =
                        new KMime::Headers::Generic( "X-KMail-CursorPos", mMsg.get(),
                                                     QString::number( plainBody.length() ), "utf-8" );
                /* if template is:
         *  FOOBAR
         *  %CURSOR
         *
         * Make sure there is an empty line for the cursor otherwise it will be placed at the end of FOOBAR
         */
                if ( oldI > 0 && tmpl[ oldI - 1 ] == QLatin1Char('\n') && i == tmpl_len - 1 ) {
                    plainBody.append( QLatin1Char('\n') );
                }
                mMsg->setHeader( header );
                mForceCursorPosition = true;
                //FIXME HTML part for header remaining
            } else if ( cmd.startsWith( QLatin1String( "SIGNATURE" ) ) ) {
                qDebug() << "Command: SIGNATURE";
                i += strlen( "SIGNATURE" );
                plainBody.append( getPlainSignature() );
                htmlBody.append( getHtmlSignature() );

            } else {
                // wrong command, do nothing
                plainBody.append( c );
                htmlBody.append( c );
            }

        } else if ( dnl && ( c == QLatin1Char('\n') || c == QLatin1Char('\r') ) ) {
            // skip
            if ( ( tmpl.size() > i+1 ) &&
                 ( ( c == QLatin1Char('\n') && tmpl[i + 1] == QLatin1Char('\r') ) ||
                   ( c == QLatin1Char('\r') && tmpl[i + 1] == QLatin1Char('\n') ) ) ) {
                // skip one more
                i += 1;
            }
            dnl = false;
        } else {
            plainBody.append( c );
            if( c == QLatin1Char('\n') || c == QLatin1Char('\r') ) {
                htmlBody.append( QLatin1String( "<br />" ) );
                htmlBody.append( c );
                if( tmpl.size() > i+1 &&
                        ( ( c == QLatin1Char('\n') && tmpl[i + 1] == QLatin1Char('\r') ) ||
                          ( c == QLatin1Char('\r') && tmpl[i + 1] == QLatin1Char('\n') ) ) ) {
                    htmlBody.append( tmpl[i + 1] );
                    plainBody.append( tmpl[i + 1] );
                    i += 1;
                }
            } else {
                htmlBody.append( c );
            }
        }
    }
    // Clear the HTML body if FORCEDPLAIN has set ReplyAsPlain, OR if,
    // there is no use of FORCED command but a configure setting has ReplyUsingHtml disabled,
    // OR the original mail has no HTML part.
    const KMime::Content *content = mOrigMsg->mainBodyPart( "text/html" );
    if( mQuotes == ReplyAsPlain ||
            ( mQuotes != ReplyAsHtml && !GlobalSettings::self()->replyUsingHtml() ) ||
            (!content || !content->hasContent() ) ) {
        htmlBody.clear();
    } else {
        makeValidHtml( htmlBody );
    }
    addProcessedBodyToMessage( plainBody, htmlBody );
}

QString TemplateParser::getPlainSignature() const
{
    const KPIMIdentities::Identity &identity =
            m_identityManager->identityForUoid( mIdentity );

    if ( identity.isNull() ) {
        return QString();
    }

    KPIMIdentities::Signature signature =
            const_cast<KPIMIdentities::Identity &>( identity ).signature();

    if ( signature.type() == KPIMIdentities::Signature::Inlined &&
         signature.isInlinedHtml() ) {
        return signature.toPlainText();
    } else {
        return signature.rawText();
    }
}
// TODO If %SIGNATURE command is on, then override it with signature from
// "KMail configure->General->identity->signature".
// There should be no two signatures.
QString TemplateParser::getHtmlSignature() const
{
    const KPIMIdentities::Identity &identity =
            m_identityManager->identityForUoid( mIdentity );
    if ( identity.isNull() ) {
        return QString();
    }

    KPIMIdentities::Signature signature =
            const_cast<KPIMIdentities::Identity &>( identity ).signature();

    if ( !signature.isInlinedHtml() ) {
        signature.rawText().toHtmlEscaped();
        return signature.rawText().replace( QRegExp( QLatin1String("\n") ), QLatin1String("<br />") );
    }
    return signature.rawText();
}

void TemplateParser::addProcessedBodyToMessage( const QString &plainBody,
                                                const QString &htmlBody ) const
{
    // Get the attachments of the original mail
    MessageCore::AttachmentCollector ac;
    ac.collectAttachmentsFrom( mOrigMsg.get() );

    MessageCore::ImageCollector ic;
    ic.collectImagesFrom( mOrigMsg.get() );

    // Now, delete the old content and set the new content, which
    // is either only the new text or the new text with some attachments.
    KMime::Content::List parts = mMsg->contents();
    foreach ( KMime::Content *content, parts ) {
        mMsg->removeContent( content, true/*delete*/ );
    }

    // Set To and CC from the template
    if ( !mTo.isEmpty() ) {
        mMsg->to()->fromUnicodeString( mMsg->to()->asUnicodeString() + QLatin1Char(',') + mTo, "utf-8" );
    }

    if ( !mCC.isEmpty() ) {
        mMsg->cc()->fromUnicodeString( mMsg->cc()->asUnicodeString() + QLatin1Char(',') + mCC, "utf-8" );
    }

    mMsg->contentType()->clear(); // to get rid of old boundary

    const QByteArray boundary = KMime::multiPartBoundary();
    KMime::Content *const mainTextPart =
            htmlBody.isEmpty() ?
                createPlainPartContent( plainBody ) :
                createMultipartAlternativeContent( plainBody, htmlBody );
    mainTextPart->assemble();

    KMime::Content *textPart = mainTextPart;
    if ( !ic.images().empty() ) {
        textPart = createMultipartRelated( ic, mainTextPart );
        textPart->assemble();
    }

    // If we have some attachments, create a multipart/mixed mail and
    // add the normal body as well as the attachments
    KMime::Content *mainPart = textPart;
    if ( !ac.attachments().empty() && mMode == Forward ) {
        mainPart = createMultipartMixed( ac, textPart );
        mainPart->assemble();
    }

    mMsg->setBody( mainPart->encodedBody() );
    mMsg->setHeader( mainPart->contentType() );
    mMsg->setHeader( mainPart->contentTransferEncoding() );
    mMsg->assemble();
    mMsg->parse();
}

KMime::Content *TemplateParser::createMultipartMixed( const MessageCore::AttachmentCollector &ac,
                                                      KMime::Content *textPart ) const
{
    KMime::Content *mixedPart = new KMime::Content( mMsg.get() );
    const QByteArray boundary = KMime::multiPartBoundary();
    mixedPart->contentType()->setMimeType( "multipart/mixed" );
    mixedPart->contentType()->setBoundary( boundary );
    mixedPart->contentTransferEncoding()->setEncoding( KMime::Headers::CE7Bit );
    mixedPart->addContent( textPart );

    int attachmentNumber = 1;
    foreach ( KMime::Content *attachment, ac.attachments() ) {
        mixedPart->addContent( attachment );
        // If the content type has no name or filename parameter, add one, since otherwise the name
        // would be empty in the attachment view of the composer, which looks confusing
        if ( attachment->contentType( false ) ) {
            if ( !attachment->contentType()->hasParameter( QLatin1String("name") ) &&
                 !attachment->contentType()->hasParameter( QLatin1String("filename") ) ) {
                attachment->contentType()->setParameter(
                            QLatin1String("name"), i18nc( "@item:intext", "Attachment %1", attachmentNumber ) );
            }
        }
        attachmentNumber++;
    }
    return mixedPart;
}

KMime::Content *TemplateParser::createMultipartRelated( const MessageCore::ImageCollector &ic,
                                                        KMime::Content *mainTextPart ) const
{
    KMime::Content *relatedPart = new KMime::Content( mMsg.get() );
    const QByteArray boundary = KMime::multiPartBoundary();
    relatedPart->contentType()->setMimeType( "multipart/related" );
    relatedPart->contentType()->setBoundary( boundary );
    relatedPart->contentTransferEncoding()->setEncoding( KMime::Headers::CE7Bit );
    relatedPart->addContent( mainTextPart );
    foreach ( KMime::Content *image, ic.images() ) {
        qWarning() << "Adding" << image->contentID() << "as an embedded image";
        relatedPart->addContent( image );
    }
    return relatedPart;
}

KMime::Content *TemplateParser::createPlainPartContent( const QString &plainBody ) const
{
    KMime::Content *textPart = new KMime::Content( mMsg.get() );
    textPart->contentType()->setMimeType( "text/plain" );
    QTextCodec *charset = selectCharset( m_charsets, plainBody );
    textPart->contentType()->setCharset( charset->name() );
    textPart->contentTransferEncoding()->setEncoding( KMime::Headers::CE8Bit );
    textPart->fromUnicodeString( plainBody );
    return textPart;
}

KMime::Content *TemplateParser::createMultipartAlternativeContent( const QString &plainBody,
                                                                   const QString &htmlBody ) const
{
    KMime::Content *multipartAlternative = new KMime::Content( mMsg.get() );
    multipartAlternative->contentType()->setMimeType( "multipart/alternative" );
    const QByteArray boundary = KMime::multiPartBoundary();
    multipartAlternative->contentType()->setBoundary( boundary );

    KMime::Content *textPart = createPlainPartContent( plainBody );
    multipartAlternative->addContent( textPart );

    KMime::Content *htmlPart = new KMime::Content( mMsg.get() );
    htmlPart->contentType()->setMimeType( "text/html" );
    QTextCodec *charset = selectCharset( m_charsets, htmlBody );
    htmlPart->contentType()->setCharset( charset->name() );
    htmlPart->contentTransferEncoding()->setEncoding( KMime::Headers::CE8Bit );
    htmlPart->fromUnicodeString( htmlBody );
    multipartAlternative->addContent( htmlPart );

    return multipartAlternative;
}

QString TemplateParser::findCustomTemplate( const QString &tmplName )
{
    CTemplates t( tmplName );
    mTo = t.to();
    mCC = t.cC();
    const QString content = t.content();
    if ( !content.isEmpty() ) {
        return content;
    } else {
        return findTemplate();
    }
}

QString TemplateParser::findTemplate()
{
    // qDebug() << "Trying to find template for mode" << mode;

    QString tmpl;

#if 0
    if ( !mFolder.isValid() ) { // find folder message belongs to
        mFolder = mMsg->parentCollection();
        if ( !mFolder.isValid() ) {
            if ( mOrigMsg ) {
                mFolder = mOrigMsg->parentCollection();
            }
            if ( !mFolder.isValid() ) {
                qDebug() << "Oops! No folder for message";
            }
        }
    }
#else
    qDebug() << "AKONADI PORT: Disabled code in  " << Q_FUNC_INFO;
    qDebug() << "Folder found:" << mFolder;
    if ( mFolder.isValid() ) { // only if a folder was found
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
                qDebug() << "Unknown message mode:" << mMode;
                return QString();
            }
            mQuoteString = fconf.quoteString();
            if ( !tmpl.isEmpty() ) {
                return tmpl;  // use folder-specific template
            }
        }
    }

    if ( !mIdentity ) { // find identity message belongs to
        qDebug() << "AKONADI PORT: verify Akonadi::Item() here  " << Q_FUNC_INFO;

        mIdentity = identityUoid( mMsg );
        if ( !mIdentity && mOrigMsg ) {
            qDebug() << "AKONADI PORT: verify Akonadi::Item() here  " << Q_FUNC_INFO;
            mIdentity = identityUoid( mOrigMsg );
        }
        mIdentity = m_identityManager->identityForUoidOrDefault( mIdentity ).uoid();
        if ( !mIdentity ) {
            qDebug() << "Oops! No identity for message";
        }
    }
    qDebug() << "Identity found:" << mIdentity;

    QString iid;
    if ( mIdentity ) {
        iid = TemplatesConfiguration::configIdString( mIdentity );        // templates ID for that identity
    } else {
        iid = QLatin1String("IDENTITY_NO_IDENTITY"); // templates ID for no identity
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
            qDebug() << "Unknown message mode:" << mMode;
            return QString();
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
        qDebug() << "Unknown message mode:" << mMode;
        return QString();
    }

    mQuoteString = GlobalSettings::self()->quoteString();
    return tmpl;
}

QString TemplateParser::pipe( const QString &cmd, const QString &buf )
{
    KProcess process;
    bool success;

    process.setOutputChannelMode( KProcess::SeparateChannels );
    process.setShellCommand( cmd );
    process.start();
    if ( process.waitForStarted( PipeTimeout ) ) {
        bool finished = false;
        if ( !buf.isEmpty() ) {
            process.write( buf.toLatin1() );
        }
        if ( buf.isEmpty() || process.waitForBytesWritten( PipeTimeout ) ) {
            if ( !buf.isEmpty() ) {
                process.closeWriteChannel();
            }
            if ( process.waitForFinished( PipeTimeout ) ) {
                success = ( process.exitStatus() == QProcess::NormalExit );
                finished = true;
            } else {
                finished = false;
                success = false;
            }
        } else {
            success = false;
            finished = false;
        }

        // The process has started, but did not finish in time. Kill it.
        if ( !finished ) {
            process.kill();
        }
    } else {
        success = false;
    }

    if ( !success && mDebug ) {
        KMessageBox::error(
                    0,
                    xi18nc( "@info",
                           "Pipe command <command>%1</command> failed.", cmd ) );
    }

    if ( success ) {
        return QString::fromLatin1(process.readAllStandardOutput());
    } else {
        return QString();
    }
}

void TemplateParser::setWordWrap( bool wrap, int wrapColWidth )
{
    mWrap = wrap;
    mColWrap = wrapColWidth;
}

QString TemplateParser::plainMessageText( bool aStripSignature,
                                          AllowSelection isSelectionAllowed ) const
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

QString TemplateParser::htmlMessageText( bool aStripSignature, AllowSelection isSelectionAllowed )
{
    if( !mSelection.isEmpty() && ( isSelectionAllowed == SelectionAllowed ) ) {
        //TODO implement mSelection for HTML
        return mSelection;
    }

    QString htmlElement = mOtp->htmlContent();

    if ( htmlElement.isEmpty() ) { //plain mails only
        htmlElement = mOtp->convertedHtmlContent();
    }

    QWebPage page;
    page.settings()->setAttribute( QWebSettings::JavascriptEnabled, false );
    page.settings()->setAttribute( QWebSettings::JavaEnabled, false );
    page.settings()->setAttribute( QWebSettings::PluginsEnabled, false );
    page.settings()->setAttribute( QWebSettings::AutoLoadImages, false );

    page.currentFrame()->setHtml( htmlElement );

    //TODO to be tested/verified if this is not an issue
    page.settings()->setAttribute( QWebSettings::JavascriptEnabled, true );
    const QString bodyElement = page.currentFrame()->evaluateJavaScript(
                QLatin1String("document.getElementsByTagName('body')[0].innerHTML") ).toString();

    mHeadElement = page.currentFrame()->evaluateJavaScript(
                QLatin1String("document.getElementsByTagName('head')[0].innerHTML") ).toString();

    page.settings()->setAttribute( QWebSettings::JavascriptEnabled, false );

    if( !bodyElement.isEmpty() ) {
        if ( aStripSignature ) {
            //FIXME strip signature works partially for HTML mails
            return MessageCore::StringUtil::stripSignature( bodyElement );
        }
        return bodyElement;
    }

    if ( aStripSignature ) {
        //FIXME strip signature works partially for HTML mails
        return MessageCore::StringUtil::stripSignature( htmlElement );
    }
    return htmlElement;
}

QString TemplateParser::quotedPlainText( const QString &selection ) const
{
    QString content = selection;
    // Remove blank lines at the beginning:
    const int firstNonWS = content.indexOf( QRegExp( QLatin1String("\\S") ) );
    const int lineStart = content.lastIndexOf( QLatin1Char('\n'), firstNonWS );
    if ( lineStart >= 0 ) {
        content.remove( 0, static_cast<unsigned int>( lineStart ) );
    }

    const QString indentStr =
            MessageCore::StringUtil::formatString( mQuoteString, mOrigMsg->from()->asUnicodeString() );
    if ( GlobalSettings::self()->smartQuote() && mWrap ) {
        content = MessageCore::StringUtil::smartQuote( content, mColWrap - indentStr.length() );
    }
    content.replace( QLatin1Char('\n'), QLatin1Char('\n') + indentStr );
    content.prepend( indentStr );
    content += QLatin1Char('\n');

    return content;
}

QString TemplateParser::quotedHtmlText( const QString &selection ) const
{
    QString content = selection;
    //TODO 1) look for all the variations of <br>  and remove the blank lines
    //2) implement vertical bar for quoted HTML mail.
    //3) After vertical bar is implemented, If a user wants to edit quoted message,
    // then the <blockquote> tags below should open and close as when required.

    //Add blockquote tag, so that quoted message can be differentiated from normal message
    content = QLatin1String("<blockquote>") + content + QLatin1String("</blockquote>");
    return content;
}

uint TemplateParser::identityUoid( const KMime::Message::Ptr &msg ) const
{
    QString idString;
    if ( msg->headerByType( "X-KMail-Identity" ) ) {
        idString = msg->headerByType( "X-KMail-Identity" )->asUnicodeString().trimmed();
    }
    bool ok = false;
    int id = idString.toUInt( &ok );

    if ( !ok || id == 0 ) {
        id = m_identityManager->identityForAddress(
                    msg->to()->asUnicodeString() + QLatin1String(", ") + msg->cc()->asUnicodeString() ).uoid();
    }

    return id;
}

bool TemplateParser::isHtmlSignature() const
{
    const KPIMIdentities::Identity &identity =
            m_identityManager->identityForUoid( mIdentity );

    if ( identity.isNull() ) {
        return false;
    }

    const KPIMIdentities::Signature signature =
            const_cast<KPIMIdentities::Identity &>( identity ).signature();

    return signature.isInlinedHtml();
}

QString TemplateParser::plainToHtml( const QString &body ) const
{
    QString str = body;
    str = str.toHtmlEscaped();
    str.replace( QRegExp( QLatin1String("\n") ), QLatin1String("<br />\n") );
    return str;
}

//TODO implement this function using a DOM tree parser
void TemplateParser::makeValidHtml( QString &body )
{
    QRegExp regEx;
    regEx.setMinimal( true );
    regEx.setPattern( QLatin1String("<html.*>") );

    if ( !body.isEmpty() && !body.contains( regEx ) ) {
        regEx.setPattern( QLatin1String("<body.*>") );
        if ( !body.contains( regEx ) ) {
            body = QLatin1String("<body>") + body + QLatin1String("<br/></body>");
        }
        regEx.setPattern( QLatin1String("<head.*>") );
        if ( !body.contains( regEx ) ) {
            body = QLatin1String("<head>") + mHeadElement + QLatin1String("</head>") + body;
        }
        body = QLatin1String("<html>") + body + QLatin1String("</html>");
    }
}

bool TemplateParser::cursorPositionWasSet() const
{
    return mForceCursorPosition;
}

}

