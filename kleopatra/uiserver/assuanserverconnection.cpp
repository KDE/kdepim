/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/assuanserverconnection.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/
#ifndef QT_NO_CAST_TO_ASCII
# define QT_NO_CAST_TO_ASCII
#endif
#ifndef QT_NO_CAST_FROM_ASCII
# define QT_NO_CAST_FROM_ASCII
#endif

#include <config-kleopatra.h>

#include "assuanserverconnection.h"
#include "assuancommand.h"

#include <models/keycache.h> // :(

#include <utils/input.h>
#include <utils/output.h>
#include <utils/gnupg-helper.h>
#include <utils/detail_p.h>
#include <utils/hex.h>
#include <utils/log.h>
#include <utils/exception.h>
#include <utils/kleo_assert.h>
#include <utils/getpid.h>
#include <utils/stl_util.h>

#include <gpgme++/data.h>
#include <gpgme++/key.h>

#include <kmime/kmime_header_parsing.h>

#include <KLocalizedString>
#include <KWindowSystem>
#include <KMessageBox>

#include <QSocketNotifier>
#include <QTimer>
#include <QVariant>
#include <QPointer>
#include <QFileInfo>
#include <QDebug>
#include <QStringList>
#include <QDialog>
#include <QRegExp>

#include <kleo-assuan.h>

#include <boost/type_traits/remove_pointer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/mem_fn.hpp>
#include <boost/mpl/if.hpp>

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <algorithm>

#include <errno.h>

#ifdef __GNUC__
# include <ext/algorithm> // for is_sorted
#endif

#ifdef Q_OS_WIN32
# include <io.h>
# include <process.h>
#else
# include <sys/types.h>
# include <unistd.h>
#endif

#ifdef Q_WS_X11
# include <qx11info_x11.h>
# include <X11/Xlib.h>
#endif

using namespace Kleo;
using namespace boost;

namespace {
    struct IOF {
        QString fileName;
        shared_ptr<QFile> file;
    };

    static void close_all( const std::vector<IOF> & ios ) {
        Q_FOREACH( const IOF & io, ios )
            if ( io.file && io.file->isOpen() )
                io.file->close();
    }

}

static const unsigned int INIT_SOCKET_FLAGS = 3; // says info assuan...
//static int(*USE_DEFAULT_HANDLER)(assuan_context_t,char*) = 0;
static const int FOR_READING = 0;
static const unsigned int MAX_ACTIVE_FDS = 32;

// shared_ptr for assuan_context_t w/ deleter enforced to assuan_deinit_server:
typedef shared_ptr< remove_pointer<assuan_context_t>::type > AssuanContextBase;
struct AssuanContext : AssuanContextBase {
    AssuanContext() : AssuanContextBase() {}
    explicit AssuanContext( assuan_context_t ctx ) : AssuanContextBase( ctx, &assuan_deinit_server ) {}

    void reset( assuan_context_t ctx=0 ) { AssuanContextBase::reset( ctx, &assuan_deinit_server ); }
};

static inline gpg_error_t assuan_process_done_msg( assuan_context_t ctx, gpg_error_t err, const char * err_msg ) {
    return assuan_process_done( ctx, assuan_set_error( ctx, err, err_msg ) );
}

static inline gpg_error_t assuan_process_done_msg( assuan_context_t ctx, gpg_error_t err, const std::string & err_msg ) {
    return assuan_process_done_msg( ctx, err, err_msg.c_str() );
}

static inline gpg_error_t assuan_process_done_msg( assuan_context_t ctx, gpg_error_t err, const QString & err_msg ) {
    return assuan_process_done_msg( ctx, err, err_msg.toUtf8().constData() );
}

static std::map<std::string,std::string> upcase_option( const char * option, std::map<std::string,std::string> options ) {
    std::string value;
    bool value_found = false;
    std::map<std::string,std::string>::iterator it = options.begin();
    while ( it != options.end() )
        if ( qstricmp( it->first.c_str(), option ) == 0 ) {
            value = it->second;
            options.erase( it++ );
            value_found = true;
        } else {
            ++it;
        }
    if ( value_found )
        options[option] = value;
    return options;
}

static std::map<std::string,std::string> parse_commandline( const char * line ) {
    std::map<std::string,std::string> result;
    if ( line ) {
        const char * begin = line;
        const char * lastEQ = 0;
        while ( *line ) {
            if ( *line == ' ' || *line == '\t' ) {
                if ( begin != line ) {
                    if ( begin[0] == '-' && begin[1] == '-' )
                        begin += 2; // skip initial "--"
                    if ( lastEQ && lastEQ > begin )
                        result[ std::string( begin, lastEQ - begin ) ] = hexdecode( std::string( lastEQ+1, line - (lastEQ+1) ) );
                    else
                        result[ std::string( begin,  line  - begin ) ] = std::string();
                }
                begin = line + 1;
            } else if ( *line == '=' ) {
                if ( line == begin )
                    throw Exception( gpg_error( GPG_ERR_ASS_SYNTAX ),
                                     i18n("No option name given") );
                else
                    lastEQ = line;
            }
            ++line;
        }
        if ( begin != line ) {
            if ( begin[0] == '-' && begin[1] == '-' )
                begin += 2; // skip initial "--"
            if ( lastEQ && lastEQ > begin )
                result[ std::string( begin, lastEQ - begin ) ] = hexdecode( std::string( lastEQ+1, line - (lastEQ+1 ) ) );
            else
                result[ begin ] = std::string();
        }
    }

    return result;
}

static WId wid_from_string( const QString & winIdStr, bool * ok=0 ) {
    return
#ifdef Q_OS_WIN32
        reinterpret_cast<WId>
#else
        static_cast<WId>
#endif
             ( winIdStr.toULongLong( ok, 16 ) );
}

static void apply_window_id( QWidget * widget, const QString & winIdStr ) {
    if ( !widget || winIdStr.isEmpty() )
        return;
    bool ok = false;
    const WId wid = wid_from_string( winIdStr, &ok );
    if ( !ok ) {
        qDebug() << "window-id value" << wid << "doesn't look like a number";
        return;
    }
    if ( QWidget * pw = QWidget::find( wid ) )
        widget->setParent( pw, widget->windowFlags() );
    else {
        KWindowSystem::setMainWindow( widget, wid );
    }
}

//
//
// AssuanServerConnection:
//
//

class AssuanServerConnection::Private : public QObject {
    Q_OBJECT
    friend class ::Kleo::AssuanServerConnection;
    friend class ::Kleo::AssuanCommandFactory;
    friend class ::Kleo::AssuanCommand;
    AssuanServerConnection * const q;
public:
    Private( assuan_fd_t fd_, const std::vector< shared_ptr<AssuanCommandFactory> > & factories_, AssuanServerConnection * qq );
    ~Private();

Q_SIGNALS:
    void startKeyManager();

public Q_SLOTS:
    void slotReadActivity( int ) {
        assert( ctx );
        if ( const int err = assuan_process_next( ctx.get() ) ) {
            //if ( err == -1 || gpg_err_code(err) == GPG_ERR_EOF ) {
                topHalfDeletion();
                if ( nohupedCommands.empty() )
                    bottomHalfDeletion();
            //} else {
                //assuan_process_done( ctx.get(), err );
                //return;
            //}
        }
    }

    int startCommandBottomHalf();

private:
    void nohupDone( AssuanCommand * cmd ) {
        const std::vector< shared_ptr<AssuanCommand> >::iterator it
            = std::find_if( nohupedCommands.begin(), nohupedCommands.end(),
                            bind( &shared_ptr<AssuanCommand>::get, _1 ) == cmd );
        assert( it != nohupedCommands.end() );
        nohupedCommands.erase( it );
        if ( nohupedCommands.empty() && closed )
            bottomHalfDeletion();
    }

    void commandDone( AssuanCommand * cmd ) {
        if ( !cmd || cmd != currentCommand.get() )
            return;
        currentCommand.reset();
    }

    void topHalfDeletion() {
        if ( currentCommand )
            currentCommand->canceled();
	if ( fd != ASSUAN_INVALID_FD ) {
#ifdef Q_OS_WIN32
            CloseHandle( fd );
#else
            ::close( fd );
#endif
	}
        notifiers.clear();
        closed = true;
    }

    void bottomHalfDeletion() {
        cleanup();
        const QPointer<Private> that = this;
        emit q->closed( q );
        if ( that ) // still there
            q->deleteLater();
    }

private:
    static void reset_handler( assuan_context_t ctx_ ) {
        assert( assuan_get_pointer( ctx_ ) );

        AssuanServerConnection::Private & conn = *static_cast<AssuanServerConnection::Private*>( assuan_get_pointer( ctx_ ) );

        conn.reset();
    }

    static int option_handler( assuan_context_t ctx_, const char * key, const char * value ) {
        assert( assuan_get_pointer( ctx_ ) );

        AssuanServerConnection::Private & conn = *static_cast<AssuanServerConnection::Private*>( assuan_get_pointer( ctx_ ) );

        if ( key && key[0] == '-' && key[1] == '-' )
            key += 2; // skip "--"
        conn.options[key] = QString::fromUtf8( value );

        return 0;
        //return gpg_error( GPG_ERR_UNKNOWN_OPTION );
    }

    static int session_handler( assuan_context_t ctx_, char * line ) {
        assert( assuan_get_pointer( ctx_ ) );
        AssuanServerConnection::Private & conn = *static_cast<AssuanServerConnection::Private*>( assuan_get_pointer( ctx_ ) );

        const QString str = QString::fromUtf8( line );
        QRegExp rx( QLatin1String( "(?:\\d+)(?:\\s+(.*))?" ) );
        if ( !rx.exactMatch( str ) ) {
            static const QString errorString = i18n("Parse error");
            return assuan_process_done_msg( ctx_, gpg_error( GPG_ERR_ASS_SYNTAX ), errorString );
        }
        if ( !rx.cap( 1 ).isEmpty() )
            conn.sessionTitle = rx.cap( 1 );
        return assuan_process_done( ctx_, 0 );
    }

    static int capabilities_handler( assuan_context_t ctx_, char * line ) {
        if ( !QByteArray( line ).trimmed().isEmpty() ) {
            static const QString errorString = i18n("CAPABILITIES doesn't take arguments");
            return assuan_process_done_msg( ctx_, gpg_error( GPG_ERR_ASS_PARAMETER ), errorString );
        }
        static const char capabilities[] =
            "SENDER=info\n"
            "RECIPIENT=info\n"
            "SESSION\n"
        ;
        return assuan_process_done( ctx_, assuan_send_data( ctx_, capabilities, sizeof capabilities - 1 ) );
    }

    static int getinfo_handler( assuan_context_t ctx_, char * line ) {
        assert( assuan_get_pointer( ctx_ ) );
        AssuanServerConnection::Private & conn = *static_cast<AssuanServerConnection::Private*>( assuan_get_pointer( ctx_ ) );

        if ( qstrcmp( line, "version" ) == 0 ) {
            static const char version[] = "Kleopatra " KLEOPATRA_VERSION_STRING ;
            return assuan_process_done( ctx_, assuan_send_data( ctx_, version, sizeof version - 1 ) );
        }

        QByteArray ba;
        if ( qstrcmp( line, "pid" ) == 0 )
            ba = QByteArray::number( mygetpid() );
        else if ( qstrcmp( line, "options" ) == 0 )
            ba = conn.dumpOptions();
        else if ( qstrcmp( line, "x-mementos" ) == 0 )
            ba = conn.dumpMementos();
        else if ( qstrcmp( line, "senders" ) == 0 )
            ba = conn.dumpSenders();
        else if ( qstrcmp( line, "recipients" ) == 0 )
            ba = conn.dumpRecipients();
        else if ( qstrcmp( line, "x-files" ) == 0 )
            ba = conn.dumpFiles();
        else {
            static const QString errorString = i18n("Unknown value for WHAT");
            return assuan_process_done_msg( ctx_, gpg_error( GPG_ERR_ASS_PARAMETER ), errorString );
        }
        return assuan_process_done( ctx_, assuan_send_data( ctx_, ba.constData(), ba.size() ) );
    }

    static int start_keymanager_handler( assuan_context_t ctx_, char * line ) {
        assert( assuan_get_pointer( ctx_ ) );
        AssuanServerConnection::Private & conn = *static_cast<AssuanServerConnection::Private*>( assuan_get_pointer( ctx_ ) );

        if ( line && *line ) {
            static const QString errorString = i18n("START_KEYMANAGER does not take arguments");
            return assuan_process_done_msg( ctx_, gpg_error( GPG_ERR_ASS_PARAMETER ), errorString );
        }

        emit conn.q->startKeyManagerRequested();

        return assuan_process_done( ctx_, 0 );
    }

    static int start_confdialog_handler( assuan_context_t ctx_, char * line ) {
        assert( assuan_get_pointer( ctx_ ) );
        AssuanServerConnection::Private & conn = *static_cast<AssuanServerConnection::Private*>( assuan_get_pointer( ctx_ ) );

        if ( line && *line ) {
            static const QString errorString = i18n("START_CONFDIALOG does not take arguments");
            return assuan_process_done_msg( ctx_, gpg_error( GPG_ERR_ASS_PARAMETER ), errorString );
        }

        emit conn.q->startConfigDialogRequested();

        return assuan_process_done( ctx_, 0 );
    }

    template <bool in> struct Input_or_Output : mpl::if_c<in,Input,Output> {};

    // format: TAG (FD|FD=\d+|FILE=...)
    template <bool in, typename T_memptr>
    static int IO_handler( assuan_context_t ctx_, char * line_, T_memptr which ) {
        assert( assuan_get_pointer( ctx_ ) );
        AssuanServerConnection::Private & conn = *static_cast<AssuanServerConnection::Private*>( assuan_get_pointer( ctx_ ) );

        try {

            /*const*/ std::map<std::string,std::string> options = upcase_option( "FD", upcase_option( "FILE", parse_commandline( line_ ) ) );
            if ( options.size() < 1 || options.size() > 2 )
                throw gpg_error( GPG_ERR_ASS_SYNTAX );

            shared_ptr< typename Input_or_Output<in>::type > io;

            if ( options.count( "FD" ) ) {

                if ( options.count( "FILE" ) )
                    throw gpg_error( GPG_ERR_CONFLICT );

                assuan_fd_t fd = ASSUAN_INVALID_FD;

                const std::string fdstr = options["FD"];

                if ( fdstr.empty() ) {
                    if ( const gpg_error_t err = assuan_receivefd( conn.ctx.get(), &fd ) )
                        throw err;
                } else {
#ifdef Q_OS_WIN32
                    fd = (assuan_fd_t)lexical_cast<intptr_t>( fdstr );
#else
                    fd = lexical_cast<assuan_fd_t>( fdstr );
#endif
                }

                io = Input_or_Output<in>::type::createFromPipeDevice( fd, in ? i18n( "Message #%1", (conn.*which).size() + 1 ) : QString() );

                options.erase( "FD" );

            } else if ( options.count( "FILE" ) ) {

                if ( options.count( "FD" ) )
                    throw gpg_error( GPG_ERR_CONFLICT );

                const QString filePath = QFile::decodeName( options["FILE"].c_str() );
                if ( filePath.isEmpty() )
                    throw Exception( gpg_error( GPG_ERR_ASS_SYNTAX ), i18n("Empty file path") );
                const QFileInfo fi( filePath );
                if ( !fi.isAbsolute() )
                    throw Exception( gpg_error( GPG_ERR_INV_ARG ), i18n("Only absolute file paths are allowed") );
                if ( fi.isDir() )
                    io = Input_or_Output<in>::type::createFromDir( fi.absoluteFilePath() );
                else
                    io = Input_or_Output<in>::type::createFromFile( fi.absoluteFilePath(), true );

                options.erase( "FILE" );

            } else {

                throw gpg_error( GPG_ERR_ASS_PARAMETER );

            }

            if ( options.size() )
                throw gpg_error( GPG_ERR_UNKNOWN_OPTION );

            (conn.*which).push_back( io );

            qDebug() << "AssuanServerConnection: added" << io->label();

            return assuan_process_done( conn.ctx.get(), 0 );
        } catch ( const GpgME::Exception & e ) {
            return assuan_process_done_msg( conn.ctx.get(), e.error().encodedError(), e.message().c_str() );
        } catch ( const std::exception & ) {
            return assuan_process_done( conn.ctx.get(), gpg_error( GPG_ERR_ASS_SYNTAX ) );
        } catch ( const gpg_error_t e ) {
            return assuan_process_done( conn.ctx.get(), e );
        } catch ( ... ) {
            return assuan_process_done_msg( conn.ctx.get(), gpg_error( GPG_ERR_UNEXPECTED ), "unknown exception caught" );
        }

    }

    static int input_handler( assuan_context_t ctx, char * line ) {
        return IO_handler<true>( ctx, line, &Private::inputs );
    }

    static int output_handler( assuan_context_t ctx, char * line ) {
        return IO_handler<false>( ctx, line, &Private::outputs );
    }

    static int message_handler( assuan_context_t ctx, char * line ) {
        return IO_handler<true>( ctx, line, &Private::messages );
    }

    static int file_handler( assuan_context_t ctx_, char * line ) {
        assert( assuan_get_pointer( ctx_ ) );
        AssuanServerConnection::Private & conn = *static_cast<AssuanServerConnection::Private*>( assuan_get_pointer( ctx_ ) );

        try {
            const QFileInfo fi( QFile::decodeName( hexdecode( line ).c_str() ) );
            if ( !fi.isAbsolute() )
                throw Exception( gpg_error( GPG_ERR_INV_ARG ), i18n("Only absolute file paths are allowed") );
            const QString filePath = fi.absoluteFilePath();

            if ( fi.exists() && fi.isDir() ) {
                if ( !fi.isReadable() || !fi.isExecutable() )
                    throw Exception( gpg_error( GPG_ERR_INV_ARG  ), i18n("Could not access directory \"%1\" for reading", filePath ) );
                conn.dirs.push_back( filePath );
                return assuan_process_done( conn.ctx.get(), 0 );
            }

            const shared_ptr<QFile> file( new QFile( filePath ) );
            if ( !file->open( QIODevice::ReadOnly ) )
                throw Exception( gpg_error_from_errno( errno ), i18n("Could not open file \"%1\" for reading", filePath) );
            const IOF io = {
                filePath, file
            };
            conn.files.push_back( io );

            return assuan_process_done( conn.ctx.get(), 0 );
        } catch ( const Exception & e ) {
            return assuan_process_done_msg( conn.ctx.get(), e.error().encodedError(), e.message().toUtf8().constData() );
        } catch ( ... ) {
            return assuan_process_done_msg( conn.ctx.get(), gpg_error( GPG_ERR_UNEXPECTED ), i18n("unknown exception caught").toUtf8().constData() );
        }
    }

    static bool parse_informative( const char * & begin, GpgME::Protocol & protocol ) {
        protocol = GpgME::UnknownProtocol;
        bool informative = false;
        const char * pos = begin;
        while ( true ) {
            while ( *pos == ' ' || *pos == '\t' )
                ++pos;
            if ( qstrnicmp( pos, "--info", strlen("--info") ) == 0 ) {
                informative = true;
                pos += strlen("--info");
                if ( *pos == '=' ) {
                    ++pos;
                    break;
                }
            } else if ( qstrnicmp( pos, "--protocol=", strlen("--protocol=") ) == 0 ) {
                pos += strlen("--protocol=");
                if ( qstrnicmp( pos, "OpenPGP", strlen("OpenPGP") ) == 0 ) {
                    protocol = GpgME::OpenPGP;
                    pos += strlen("OpenPGP");
                } else if ( qstrnicmp( pos, "CMS", strlen("CMS") ) == 0 ) {
                    protocol = GpgME::CMS;
                    pos += strlen("CMS");
                } else
                    ;
            } else if ( qstrncmp( pos, "-- ", strlen("-- ") ) == 0 ) {
                pos += 3;
                while ( *pos == ' ' || *pos == '\t' )
                    ++pos;
                break;
            } else
                break;
        }
        begin = pos;
        return informative;
    }

    template <typename T_memptr, typename T_memptr2>
    static int recipient_sender_handler( T_memptr mp, T_memptr2 info, assuan_context_t ctx, char * line, bool sender=false ) {
        assert( assuan_get_pointer( ctx ) );
        AssuanServerConnection::Private & conn = *static_cast<AssuanServerConnection::Private*>( assuan_get_pointer( ctx ) );

        if ( !line || !*line )
            return assuan_process_done( conn.ctx.get(), gpg_error( GPG_ERR_INV_ARG ) );
        const char * begin     = line;
        const char * const end = begin + qstrlen( line );
        GpgME::Protocol proto = GpgME::UnknownProtocol;
        const bool informative = parse_informative( begin, proto );
        if ( !(conn.*mp).empty() && informative != (conn.*info) )
            return assuan_process_done_msg( conn.ctx.get(), gpg_error( GPG_ERR_CONFLICT ),
                                            i18n("Cannot mix --info with non-info SENDER or RECIPIENT").toUtf8().constData() );
        KMime::Types::Mailbox mb;
        if ( !KMime::HeaderParsing::parseMailbox( begin, end, mb ) )
            return assuan_process_done_msg( conn.ctx.get(), gpg_error( GPG_ERR_INV_ARG ),
                                            i18n("Argument is not a valid RFC-2822 mailbox").toUtf8().constData() );
        if ( begin != end )
            return assuan_process_done_msg( conn.ctx.get(), gpg_error( GPG_ERR_INV_ARG ),
                                            i18n("Garbage after valid RFC-2822 mailbox detected").toUtf8().constData() );
        (conn.*info) = informative;
        (conn.*mp).push_back( mb );
        // ### we should wait for the KeyCache to become ready here,
        // ### but we don't have time anymore to turn SENDER into a
        // ### command, so we just ignore this for now:
        const QString email = mb.addrSpec().asString();
        (void)assuan_write_line( conn.ctx.get(), qPrintable( QString().sprintf( "# ok, parsed as \"%s\"", qPrintable( email ) ) ) );
        if ( sender && !informative ) {
            const std::vector<GpgME::Key> seckeys =
                kdtools::copy_if<std::vector<GpgME::Key> >( KeyCache::instance()->findByEMailAddress( email.toStdString() ),
                                                            mem_fn( &GpgME::Key::hasSecret ) );
            if ( seckeys.empty() )
                (void)assuan_write_line( conn.ctx.get(), "# no matching keys found" );
            else
                Q_FOREACH( const GpgME::Key & key, seckeys )
                    (void)assuan_write_line( conn.ctx.get(), qPrintable( QString().sprintf( "# matching %s key %s", key.protocolAsString(), key.primaryFingerprint() ) ) );
            const bool pgp = proto != GpgME::CMS     && kdtools::any( seckeys, bind( &GpgME::Key::protocol, _1 ) == GpgME::OpenPGP );
            const bool cms = proto != GpgME::OpenPGP && kdtools::any( seckeys, bind( &GpgME::Key::protocol, _1 ) == GpgME::CMS );
            if ( cms != pgp )
                proto = pgp ? GpgME::OpenPGP : GpgME::CMS ;
            if ( cms && pgp )
                if ( conn.bias != GpgME::UnknownProtocol ) {
                    proto = conn.bias;
                } else {
                    if ( conn.options.count("window-id") )
                        proto =
                            KMessageBox::questionYesNoWId( wid_from_string( conn.options["window-id"].toString() ),
                                                           i18nc("@info",
                                                                 "<para>The sender address <email>%1</email> matches more than one cryptographic format.</para>"
                                                                 "<para>Which format do you want to use?</para>", email ),
                                                           i18nc("@title","Format Choice"),
                                                           KGuiItem( i18nc("@action:button","Send OpenPGP-Signed") ),
                                                           KGuiItem( i18nc("@action:button","Send S/MIME-Signed") ),
                                                           QLatin1String("uiserver-sender-ask-protocol") ) == KMessageBox::Yes
                            ? GpgME::OpenPGP
                            : GpgME::CMS ;
                    else
                        proto =
                            KMessageBox::questionYesNo( 0, i18nc("@info",
                                                                 "<para>The sender address <email>%1</email> matches more than one cryptographic format.</para>"
                                                                 "<para>Which format do you want to use?</para>", email ),
                                                        i18nc("@title","Format Choice"),
                                                        KGuiItem( i18nc("@action:button","Send OpenPGP-Signed") ),
                                                        KGuiItem( i18nc("@action:button","Send S/MIME-Signed") ),
                                                        QLatin1String("uiserver-sender-ask-protocol") ) == KMessageBox::Yes
                            ? GpgME::OpenPGP
                            : GpgME::CMS ;
                }
            conn.bias = proto;
            switch ( proto ) {
            case GpgME::OpenPGP:
                (void)assuan_write_status( conn.ctx.get(), "PROTOCOL", "OpenPGP" );
                break;
            case GpgME::CMS:
                (void)assuan_write_status( conn.ctx.get(), "PROTOCOL", "CMS" );
                break;
            case GpgME::UnknownProtocol:
                ; // keep compiler happy
            };
        }
        return assuan_process_done( ctx, 0 );
    }

    static int recipient_handler( assuan_context_t ctx, char * line ) {
        return recipient_sender_handler( &Private::recipients, &Private::informativeRecipients, ctx, line );
    }

    static int sender_handler( assuan_context_t ctx, char * line ) {
        return recipient_sender_handler( &Private::senders, &Private::informativeSenders, ctx, line, true );
    }

    QByteArray dumpOptions() const {
        QByteArray result;
        for ( std::map<std::string,QVariant>::const_iterator it = options.begin(), end = options.end() ; it != end ; ++it )
            result += it->first.c_str() + it->second.toString().toUtf8() + '\n';
        return result;
    }

    static QByteArray dumpStringList( const QStringList & sl ) {
        return sl.join( QLatin1String( "\n" ) ).toUtf8();
    }

    template <typename T_container>
    static QByteArray dumpStringList( const T_container & c ) {
        QStringList sl;
        std::copy( c.begin(), c.end(), std::back_inserter( sl ) );
        return dumpStringList( sl );
    }

    template <typename T_container>
    static QByteArray dumpMailboxes( const T_container & c ) {
        QStringList sl;
        std::transform( c.begin(), c.end(),
                        std::back_inserter( sl ),
                        bind( &KMime::Types::Mailbox::prettyAddress, _1 ) );
        return dumpStringList( sl );
    }

    QByteArray dumpSenders() const {
        return dumpMailboxes( senders );
    }

    QByteArray dumpRecipients() const {
        return dumpMailboxes( recipients );
    }

    QByteArray dumpMementos() const {
        QByteArray result;
        for ( std::map< QByteArray, shared_ptr<AssuanCommand::Memento> >::const_iterator it = mementos.begin(), end = mementos.end() ; it != end ; ++it ) {
            char buf[2 + 2*sizeof(void*) + 2];
            sprintf( buf, "0x%p\n", it->second.get() );
            buf[sizeof(buf)-1] = '\0';
            result += it->first + QByteArray::fromRawData( buf, sizeof buf );
        }
        return result;
    }

    QByteArray dumpFiles() const {
        QStringList sl;
        std::transform( files.begin(), files.end(), std::back_inserter( sl ),
                        bind( &IOF::fileName, _1 ) );
        return dumpStringList( sl );
    }

    void cleanup();
    void reset() {
        options.clear();
        senders.clear();
        informativeSenders = false;
        recipients.clear();
        informativeRecipients = false;
        sessionTitle.clear();
        mementos.clear();
        close_all( files );
        files.clear();
        std::for_each( inputs.begin(), inputs.end(),
                       bind( &Input::finalize, _1 ) );
        inputs.clear();
        std::for_each( outputs.begin(), outputs.end(),
                       bind( &Output::finalize, _1 ) );
        outputs.clear();
        std::for_each( messages.begin(), messages.end(),
                       bind( &Input::finalize, _1 ) );
        messages.clear();
        bias = GpgME::UnknownProtocol;
    }

    assuan_fd_t fd;
    AssuanContext ctx;
    bool closed                : 1;
    bool cryptoCommandsEnabled : 1;
    bool commandWaitingForCryptoCommandsEnabled : 1;
    bool currentCommandIsNohup : 1;
    bool informativeSenders;    // address taken, so no : 1
    bool informativeRecipients; // address taken, so no : 1
    GpgME::Protocol bias;
    QString sessionTitle;
    std::vector< shared_ptr<QSocketNotifier> > notifiers;
    std::vector< shared_ptr<AssuanCommandFactory> > factories; // sorted: _detail::ByName<std::less>
    shared_ptr<AssuanCommand> currentCommand;
    std::vector< shared_ptr<AssuanCommand> > nohupedCommands;
    std::map<std::string,QVariant> options;
    std::vector<KMime::Types::Mailbox> senders, recipients;
    std::vector< shared_ptr<Input> > inputs, messages;
    std::vector< shared_ptr<Output> > outputs;
    std::vector<IOF> files;
    QStringList dirs;
    std::map< QByteArray, shared_ptr<AssuanCommand::Memento> > mementos;
};

void AssuanServerConnection::Private::cleanup() {
    assert( nohupedCommands.empty() );
    reset();
    currentCommand.reset();
    currentCommandIsNohup = false;
    commandWaitingForCryptoCommandsEnabled = false;
    notifiers.clear();
    ctx.reset();
    fd = ASSUAN_INVALID_FD;
}

AssuanServerConnection::Private::Private( assuan_fd_t fd_, const std::vector< shared_ptr<AssuanCommandFactory> > & factories_, AssuanServerConnection * qq )
    : QObject(),
      q( qq ),
      fd( fd_ ),
      closed( false ),
      cryptoCommandsEnabled( false ),
      commandWaitingForCryptoCommandsEnabled( false ),
      currentCommandIsNohup( false ),
      informativeSenders( false ),
      informativeRecipients( false ),
      bias( GpgME::UnknownProtocol ),
      factories( factories_ )
{
#ifdef __GNUC__
    assert( __gnu_cxx::is_sorted( factories_.begin(), factories_.end(), _detail::ByName<std::less>() ) );
#endif

    if ( fd == ASSUAN_INVALID_FD )
        throw Exception( gpg_error( GPG_ERR_INV_ARG ), "pre-assuan_init_socket_server_ext" );

    assuan_context_t naked_ctx = 0;
    if ( const gpg_error_t err = assuan_init_socket_server_ext( &naked_ctx, fd, INIT_SOCKET_FLAGS ) )
        throw Exception( err, "assuan_init_socket_server_ext" );

    ctx.reset( naked_ctx ); naked_ctx = 0;

    // for callbacks, associate the context with this connection:
    assuan_set_pointer( ctx.get(), this );

    FILE* const logFile = Log::instance()->logFile();
    assuan_set_log_stream( ctx.get(), logFile ? logFile : stderr );

    // register FDs with the event loop:
    assuan_fd_t fds[MAX_ACTIVE_FDS];
    const int numFDs = assuan_get_active_fds( ctx.get(), FOR_READING, fds, MAX_ACTIVE_FDS );
    assert( numFDs != -1 ); // == 1

    if ( !numFDs || fds[0] != fd ) {
        const shared_ptr<QSocketNotifier> sn( new QSocketNotifier( (int)fd, QSocketNotifier::Read ), mem_fn( &QObject::deleteLater ) );
        connect( sn.get(), SIGNAL(activated(int)), this, SLOT(slotReadActivity(int)) );
        notifiers.push_back( sn );
    }

    notifiers.reserve( notifiers.size() + numFDs );
    for ( int i = 0 ; i < numFDs ; ++i ) {
        const shared_ptr<QSocketNotifier> sn( new QSocketNotifier( (int)fds[i], QSocketNotifier::Read ), mem_fn( &QObject::deleteLater ) );
        connect( sn.get(), SIGNAL(activated(int)), this, SLOT(slotReadActivity(int)) );
        notifiers.push_back( sn );
    }


    // register our INPUT/OUTPUT/MESSGAE/FILE handlers:
    if ( const gpg_error_t err = assuan_register_command( ctx.get(), "INPUT",  input_handler ) )
        throw Exception( err, "register \"INPUT\" handler" );
    if ( const gpg_error_t err = assuan_register_command( ctx.get(), "MESSAGE",  message_handler ) )
        throw Exception( err, "register \"MESSAGE\" handler" );
    if ( const gpg_error_t err = assuan_register_command( ctx.get(), "OUTPUT", output_handler ) )
        throw Exception( err, "register \"OUTPUT\" handler" );
    if ( const gpg_error_t err = assuan_register_command( ctx.get(), "FILE", file_handler ) )
        throw Exception( err, "register \"FILE\" handler" );


    // register user-defined commands:
    Q_FOREACH( shared_ptr<AssuanCommandFactory> fac, factories )
        if ( const gpg_error_t err = assuan_register_command( ctx.get(), fac->name(), fac->_handler() ) )
            throw Exception( err, std::string( "register \"" ) + fac->name() + "\" handler" );

    if ( const gpg_error_t err = assuan_register_command( ctx.get(), "GETINFO", getinfo_handler ) )
        throw Exception( err, "register \"GETINFO\" handler" );
    if ( const gpg_error_t err = assuan_register_command( ctx.get(), "START_KEYMANAGER", start_keymanager_handler ) )
        throw Exception( err, "register \"START_KEYMANAGER\" handler" );
    if ( const gpg_error_t err = assuan_register_command( ctx.get(), "START_CONFDIALOG", start_confdialog_handler ) )
        throw Exception( err, "register \"START_CONFDIALOG\" handler" );
    if ( const gpg_error_t err = assuan_register_command( ctx.get(), "RECIPIENT", recipient_handler ) )
        throw Exception( err, "register \"RECIPIENT\" handler" );
    if ( const gpg_error_t err = assuan_register_command( ctx.get(), "SENDER", sender_handler ) )
        throw Exception( err, "register \"SENDER\" handler" );
    if ( const gpg_error_t err = assuan_register_command( ctx.get(), "SESSION", session_handler ) )
        throw Exception( err, "register \"SESSION\" handler" );
    if ( const gpg_error_t err = assuan_register_command( ctx.get(), "CAPABILITIES", capabilities_handler ) )
        throw Exception( err, "register \"CAPABILITIES\" handler" );

    assuan_set_hello_line( ctx.get(), "GPG UI server (Kleopatra/" KLEOPATRA_VERSION_STRING ") ready to serve" );
    //assuan_set_hello_line( ctx.get(), GPG UI server (qApp->applicationName() + " v" + kapp->applicationVersion() + "ready to serve" )


    // some notifiers we're interested in:
    if ( const gpg_error_t err = assuan_register_reset_notify( ctx.get(), reset_handler ) )
        throw Exception( err, "register reset notify" );
    if ( const gpg_error_t err = assuan_register_option_handler( ctx.get(), option_handler ) )
        throw Exception( err, "register option handler" );

    // and last, we need to call assuan_accept, which doesn't block
    // (d/t INIT_SOCKET_FLAGS), but performs vital connection
    // establishing handling:
    if ( const gpg_error_t err = assuan_accept( ctx.get() ) )
        throw Exception( err, "assuan_accept" );
}

AssuanServerConnection::Private::~Private() {
    cleanup();
}

AssuanServerConnection::AssuanServerConnection( assuan_fd_t fd, const std::vector< shared_ptr<AssuanCommandFactory> > & factories, QObject * p )
    : QObject( p ), d( new Private( fd, factories, this ) )
{

}

AssuanServerConnection::~AssuanServerConnection() {}

void AssuanServerConnection::enableCryptoCommands( bool on ) {
    if ( on == d->cryptoCommandsEnabled )
        return;
    d->cryptoCommandsEnabled = on;
    if ( d->commandWaitingForCryptoCommandsEnabled )
        QTimer::singleShot( 0, d.get(), SLOT(startCommandBottomHalf()) );
}


//
//
// AssuanCommand:
//
//

namespace Kleo {

class InquiryHandler : public QObject {
    Q_OBJECT
public:

#ifdef HAVE_ASSUAN_INQUIRE_EXT
    explicit InquiryHandler( const char * keyword_, QObject * p=0 )
        : QObject( p ),
# ifndef HAVE_NEW_STYLE_ASSUAN_INQUIRE_EXT
          buffer( 0 ),
          buflen( 0 ),
# endif
          keyword( keyword_ )
    {

    }

# ifdef HAVE_NEW_STYLE_ASSUAN_INQUIRE_EXT
    static int handler( void * cb_data, int rc, unsigned char * buffer, size_t buflen )
    {
        assert( cb_data );
        InquiryHandler * this_ = static_cast<InquiryHandler*>(cb_data);
        emit this_->signal( rc, QByteArray::fromRawData( reinterpret_cast<const char*>(buffer), buflen ), this_->keyword );
        std::free( buffer );
        delete this_;
        return 0;
    }
# else
    static int handler( void * cb_data, int rc )
    {
        assert( cb_data );
        InquiryHandler * this_ = static_cast<InquiryHandler*>(cb_data);
        emit this_->signal( rc, QByteArray::fromRawData( reinterpret_cast<const char*>(this_->buffer), this_->buflen ), this_->keyword );
        std::free( this_->buffer );
        delete this_;
        return 0;
    }
# endif

private:
# ifndef HAVE_NEW_STYLE_ASSUAN_INQUIRE_EXT
    friend class ::Kleo::AssuanCommand;
    unsigned char * buffer;
    size_t buflen;
# endif
    const char * keyword;
#endif // HAVE_ASSUAN_INQUIRE_EXT

Q_SIGNALS:
    void signal( int rc, const QByteArray & data, const QByteArray & keyword );
};

} // namespace Kleo

class AssuanCommand::Private {
public:
    Private()
        : informativeRecipients( false ),
          informativeSenders( false ),
          bias( GpgME::UnknownProtocol ),
          done( false ),
          nohup( false )
    {

    }

    std::map<std::string,QVariant> options;
    std::vector< shared_ptr<Input> > inputs, messages;
    std::vector< shared_ptr<Output> > outputs;
    std::vector<IOF> files;
    std::vector<KMime::Types::Mailbox> recipients, senders;
    bool informativeRecipients, informativeSenders;
    GpgME::Protocol bias;
    QString sessionTitle;
    QByteArray utf8ErrorKeepAlive;
    AssuanContext ctx;
    bool done;
    bool nohup;
};

AssuanCommand::AssuanCommand()
    : d( new Private )
{

}

AssuanCommand::~AssuanCommand() {

}

int AssuanCommand::start() {
    try {
        if ( const int err = doStart() )
            if ( !d->done )
                done( err );
        return 0;
    } catch ( const Exception & e ) {
        if ( !d->done )
            done( e.error_code(), e.message() );
        return 0;
    } catch ( const GpgME::Exception & e ) {
        if ( !d->done )
            done( e.error(), QString::fromLocal8Bit( e.message().c_str() ) );
        return 0;
    } catch ( const std::exception & e ) {
        if ( !d->done )
            done( makeError( GPG_ERR_INTERNAL ), i18n("Caught unexpected exception: %1", QString::fromLocal8Bit( e.what() ) ) );
        return 0;
    } catch ( ... ) {
        if ( !d->done )
            done( makeError( GPG_ERR_INTERNAL ), i18n("Caught unknown exception - fix the program!" ) );
        return 0;
    }
}

void AssuanCommand::canceled() {
    d->done = true;
    doCanceled();
}

// static
int AssuanCommand::makeError( int code ) {
    return makeGnuPGError( code );
}

bool AssuanCommand::hasOption( const char * opt ) const {
    return d->options.count( opt );
}

QVariant AssuanCommand::option( const char * opt ) const {
    const std::map<std::string,QVariant>::const_iterator it = d->options.find( opt );
    if ( it == d->options.end() )
        return QVariant();
    else
        return it->second;
}

const std::map<std::string,QVariant> & AssuanCommand::options() const {
    return d->options;
}

namespace {
    template <typename U, typename V>
    std::vector<U> keys( const std::map<U,V> & map ) {
        std::vector<U> result;
        result.resize( map.size() );
        for ( typename std::map<U,V>::const_iterator it = map.begin(), end = map.end() ; it != end ; ++it )
            result.push_back( it->first );
        return result;
    }
}

const std::map< QByteArray, shared_ptr<AssuanCommand::Memento> > & AssuanCommand::mementos() const {
    // oh, hack :(
    assert( assuan_get_pointer( d->ctx.get() ) );
    const AssuanServerConnection::Private & conn = *static_cast<AssuanServerConnection::Private*>( assuan_get_pointer( d->ctx.get() ) );
    return conn.mementos;
}

bool AssuanCommand::hasMemento( const QByteArray & tag ) const {
    return mementos().count( tag );
}

shared_ptr<AssuanCommand::Memento> AssuanCommand::memento( const QByteArray & tag ) const {
    const std::map< QByteArray, shared_ptr<Memento> >::const_iterator it = mementos().find( tag );
    if ( it == mementos().end() )
        return shared_ptr<Memento>();
    else
        return it->second;
}

QByteArray AssuanCommand::registerMemento( const shared_ptr<Memento> & mem ) {
    const QByteArray tag = QByteArray::number( reinterpret_cast<qulonglong>( mem.get() ), 36 );
    return registerMemento( tag, mem );
}

QByteArray AssuanCommand::registerMemento( const QByteArray & tag, const shared_ptr<Memento> & mem ) {
    // oh, hack :(
    assert( assuan_get_pointer( d->ctx.get() ) );
    AssuanServerConnection::Private & conn = *static_cast<AssuanServerConnection::Private*>( assuan_get_pointer( d->ctx.get() ) );

    conn.mementos[tag] = mem;
    return tag;
}

void AssuanCommand::removeMemento( const QByteArray & tag ) {
    // oh, hack :(
    assert( assuan_get_pointer( d->ctx.get() ) );
    AssuanServerConnection::Private & conn = *static_cast<AssuanServerConnection::Private*>( assuan_get_pointer( d->ctx.get() ) );

    conn.mementos.erase( tag );
}

const std::vector< shared_ptr<Input> > & AssuanCommand::inputs() const {
    return d->inputs;
}

const std::vector< shared_ptr<Input> > & AssuanCommand::messages() const {
    return d->messages;
}

const std::vector< shared_ptr<Output> > & AssuanCommand::outputs() const {
    return d->outputs;
}

QStringList AssuanCommand::fileNames() const {
    QStringList result;
    Q_FOREACH( const IOF & io, d->files )
        result.push_back( io.fileName );
    return result;
}

std::vector< shared_ptr<QFile> > AssuanCommand::files() const {
    std::vector< shared_ptr<QFile> > result;
    Q_FOREACH( const IOF & io, d->files )
        result.push_back( io.file );
    return result;
}

unsigned int AssuanCommand::numFiles() const {
    return d->files.size();
}

void AssuanCommand::releaseFiles() {
    d->files.clear();
}

#if 0
QString AssuanCommand::bulkInputDeviceFileName( unsigned int idx ) const {
    return d->inputs.at( idx ).fileName;
}

shared_ptr<QIODevice> AssuanCommand::bulkInputDevice( unsigned int idx ) const {
    return d->inputs.at( idx ).iodev;
}

unsigned int AssuanCommand::numBulkInputDevices() const {
    return d->inputs.size();
}


QString AssuanCommand::bulkMessageDeviceFileName( unsigned int idx ) const {
    return d->messages.at( idx ).fileName;
}

shared_ptr<QIODevice> AssuanCommand::bulkMessageDevice( unsigned int idx ) const {
    return d->messages.at( idx ).iodev;
}

unsigned int AssuanCommand::numBulkMessageDevices() const {
    return d->messages.size();
}


QString AssuanCommand::bulkOutputDeviceFileName( unsigned int idx ) const {
    return d->outputs.at( idx ).fileName;
}

shared_ptr<QIODevice> AssuanCommand::bulkOutputDevice( unsigned int idx ) const {
    return d->outputs.at( idx ).iodev;
}

unsigned int AssuanCommand::numBulkOutputDevices() const {
    return d->outputs.size();
}
#endif

void AssuanCommand::sendStatus( const char * keyword, const QString & text ) {
    sendStatusEncoded( keyword, text.toUtf8().constData() );
}

void AssuanCommand::sendStatusEncoded( const char * keyword, const std::string & text ) {
    if ( d->nohup )
        return;
    if ( const int err = assuan_write_status( d->ctx.get(), keyword, text.c_str() ) )
        throw Exception( err, i18n( "Can't send \"%1\" status", QString::fromLatin1( keyword ) ) );
}

void  AssuanCommand::sendData( const QByteArray & data, bool moreToCome ) {
    if ( d->nohup )
        return;
    if ( const gpg_error_t err = assuan_send_data( d->ctx.get(), data.constData(), data.size() ) )
        throw Exception( err, i18n( "Can't send data" ) );
    if ( !moreToCome )
        if ( const gpg_error_t err = assuan_send_data( d->ctx.get(), 0, 0 ) ) // flush
            throw Exception( err, i18n( "Can't flush data" ) );
}

int AssuanCommand::inquire( const char * keyword, QObject * receiver, const char * slot, unsigned int maxSize ) {
    assert( keyword );
    assert( receiver );
    assert( slot );

    if ( d->nohup )
        return makeError( GPG_ERR_INV_OP );

#ifdef HAVE_ASSUAN_INQUIRE_EXT
    std::auto_ptr<InquiryHandler> ih( new InquiryHandler( keyword, receiver ) );
    receiver->connect( ih.get(), SIGNAL(signal(int,QByteArray,QByteArray)), slot );
    if ( const gpg_error_t err = assuan_inquire_ext( d->ctx.get(), keyword,
# ifndef HAVE_NEW_STYLE_ASSUAN_INQUIRE_EXT
                                                     &ih->buffer, &ih->buflen,
# endif
                                                     maxSize, InquiryHandler::handler, ih.get() ) )
         return err;
    ih.release();
    return 0;
#else
    return makeError( GPG_ERR_NOT_SUPPORTED ); // libassuan too old
#endif // HAVE_ASSUAN_INQUIRE_EXT
}

void AssuanCommand::done( const GpgME::Error& err, const QString & details ) {
    if ( d->ctx && !d->done && !details.isEmpty() ) {
	qDebug() << "AssuanCommand::done(): Error: " << details;
        d->utf8ErrorKeepAlive = details.toUtf8();
        if ( !d->nohup )
            assuan_set_error( d->ctx.get(), err.encodedError(), d->utf8ErrorKeepAlive.constData() );
    }
    done( err );
}

void AssuanCommand::done( const GpgME::Error& err ) {
    if ( !d->ctx ) {
        qDebug( "AssuanCommand::done( %s ): called with NULL ctx.", err.asString() );
        return;
    }
    if ( d->done ) {
        qDebug( "AssuanCommand::done( %s ): called twice!", err.asString() );
        return;
    }

    d->done = true;

    std::for_each( d->messages.begin(), d->messages.end(),
                   bind( &Input::finalize, _1 ) );
    std::for_each( d->inputs.begin(), d->inputs.end(),
                   bind( &Input::finalize, _1 ) );
    std::for_each( d->outputs.begin(), d->outputs.end(),
                   bind( &Output::finalize, _1 ) );
    d->messages.clear();
    d->inputs.clear();
    d->outputs.clear();
    close_all( d->files ); // ### ???
    d->files.clear();

    // oh, hack :(
    assert( assuan_get_pointer( d->ctx.get() ) );
    AssuanServerConnection::Private & conn = *static_cast<AssuanServerConnection::Private*>( assuan_get_pointer( d->ctx.get() ) );

    if ( d->nohup ) {
        conn.nohupDone( this );
        return;
    }

    const gpg_error_t rc = assuan_process_done( d->ctx.get(), err.encodedError() );
    if ( gpg_err_code( rc ) != GPG_ERR_NO_ERROR )
        qFatal( "AssuanCommand::done: assuan_process_done returned error %d (%s)",
                static_cast<int>(rc), gpg_strerror(rc) );

    d->utf8ErrorKeepAlive.clear();

    conn.commandDone( this );
}


void AssuanCommand::setNohup( bool nohup ) {
    d->nohup = nohup;
}

bool AssuanCommand::isNohup() const {
    return d->nohup;
}

bool AssuanCommand::isDone() const {
    return d->done;
}

QString AssuanCommand::sessionTitle() const {
    return d->sessionTitle;
}

bool AssuanCommand::informativeSenders() const {
    return d->informativeSenders;
}

bool AssuanCommand::informativeRecipients() const {
    return d->informativeRecipients;
}

const std::vector<KMime::Types::Mailbox> & AssuanCommand::recipients() const {
    return d->recipients;
}

const std::vector<KMime::Types::Mailbox> & AssuanCommand::senders() const {
    return d->senders;
}

int AssuanCommandFactory::_handle( assuan_context_t ctx, char * line, const char * commandName ) {
    assert( assuan_get_pointer( ctx ) );
    AssuanServerConnection::Private & conn = *static_cast<AssuanServerConnection::Private*>( assuan_get_pointer( ctx ) );

    try {

        const std::vector< shared_ptr<AssuanCommandFactory> >::const_iterator it
            = std::lower_bound( conn.factories.begin(), conn.factories.end(), commandName, _detail::ByName<std::less>() );
        kleo_assert( it != conn.factories.end() );
        kleo_assert( *it );
        kleo_assert( qstricmp( (*it)->name(), commandName ) == 0 );

        const shared_ptr<AssuanCommand> cmd = (*it)->create();
        kleo_assert( cmd );

        cmd->d->ctx     = conn.ctx;
        cmd->d->options = conn.options;
        cmd->d->inputs.swap( conn.inputs );     kleo_assert( conn.inputs.empty() );
        cmd->d->messages.swap( conn.messages ); kleo_assert( conn.messages.empty() );
        cmd->d->outputs.swap( conn.outputs );   kleo_assert( conn.outputs.empty() );
        cmd->d->files.swap( conn.files );       kleo_assert( conn.files.empty() );
        cmd->d->senders.swap( conn.senders );   kleo_assert( conn.senders.empty() );
        cmd->d->recipients.swap( conn.recipients ); kleo_assert( conn.recipients.empty() );
        cmd->d->informativeRecipients = conn.informativeRecipients;
        cmd->d->informativeSenders    = conn.informativeSenders;
        cmd->d->bias                  = conn.bias;
        cmd->d->sessionTitle          = conn.sessionTitle;

        const std::map<std::string,std::string> cmdline_options = parse_commandline( line );
        for ( std::map<std::string,std::string>::const_iterator it = cmdline_options.begin(), end = cmdline_options.end() ; it != end ; ++it )
            cmd->d->options[it->first] = QString::fromUtf8( it->second.c_str() );

        bool nohup = false;
        if ( cmd->d->options.count( "nohup" ) ) {
            if ( !cmd->d->options["nohup"].toString().isEmpty() )
                return assuan_process_done_msg( conn.ctx.get(), gpg_error( GPG_ERR_ASS_PARAMETER ), "--nohup takes no argument" );
            nohup = true;
            cmd->d->options.erase( "nohup" );
        }

        conn.currentCommand = cmd;
        conn.currentCommandIsNohup = nohup;

        QTimer::singleShot( 0, &conn, SLOT(startCommandBottomHalf()) );

        return 0;

    } catch ( const Exception & e ) {
        return assuan_process_done_msg( conn.ctx.get(), e.error_code(), e.message() );
    } catch ( const std::exception & e ) {
        return assuan_process_done_msg( conn.ctx.get(), gpg_error( GPG_ERR_UNEXPECTED ), e.what() );
    } catch ( ... ) {
        return assuan_process_done_msg( conn.ctx.get(), gpg_error( GPG_ERR_UNEXPECTED ), i18n("Caught unknown exception") );
    }
}

int AssuanServerConnection::Private::startCommandBottomHalf() {

    commandWaitingForCryptoCommandsEnabled = currentCommand && !cryptoCommandsEnabled;

    if ( !cryptoCommandsEnabled )
        return 0;

    const shared_ptr<AssuanCommand> cmd = currentCommand;
    if ( !cmd )
        return 0;

    currentCommand.reset();

    const bool nohup = currentCommandIsNohup;
    currentCommandIsNohup = false;

    try {

        if ( const int err = cmd->start() )
            if ( cmd->isDone() )
                return err;
            else
                return assuan_process_done( ctx.get(), err );

        if ( cmd->isDone() )
            return 0;

        if ( nohup ) {
            cmd->setNohup( true );
            nohupedCommands.push_back( cmd );
            return assuan_process_done_msg( ctx.get(), 0, "Command put in the background to continue executing after connection end." );
        } else {
            currentCommand = cmd;
            return 0;
        }

    } catch ( const Exception & e ) {
        return assuan_process_done_msg( ctx.get(), e.error_code(), e.message() );
    } catch ( const std::exception & e ) {
        return assuan_process_done_msg( ctx.get(), gpg_error( GPG_ERR_UNEXPECTED ), e.what() );
    } catch ( ... ) {
        return assuan_process_done_msg( ctx.get(), gpg_error( GPG_ERR_UNEXPECTED ), i18n("Caught unknown exception") );
    }

}

//
//
// AssuanCommand convenience methods
//
//

/*!
  Checks the \c --mode parameter.

  \returns The parameter as an AssuanCommand::Mode enum value.

  If no \c --mode was given, or it's value wasn't recognized, throws
  an Kleo::Exception.
*/
AssuanCommand::Mode AssuanCommand::checkMode() const {
    if ( !hasOption( "mode" ) )
        throw Exception( makeError( GPG_ERR_MISSING_VALUE ), i18n( "Required --mode option missing" ) );

    const QString modeString = option("mode").toString().toLower();
    if ( modeString == QLatin1String( "filemanager" ) )
        return FileManager;
    if ( modeString == QLatin1String( "email" ) )
        return EMail;
    throw Exception( makeError( GPG_ERR_INV_ARG ), i18n( "invalid mode: \"%1\"", modeString ) );
}

/*!
  Checks the \c --protocol parameter.

  \returns The parameter as a GpgME::Protocol enum value.

  If \c --protocol was given, but has an invalid value, throws an
  Kleo::Exception.

  If no \c --protocol was given, checks the connection bias, if
  available, otherwise, in FileManager mode, returns
  GpgME::UnknownProtocol, but if \a mode == \c EMail, throws an
  Kleo::Exception instead.
*/
GpgME::Protocol AssuanCommand::checkProtocol( Mode mode, int options ) const {
    if ( !hasOption("protocol") )
        if ( d->bias != GpgME::UnknownProtocol )
            return d->bias;
        else if ( mode == AssuanCommand::EMail && ( options & AllowProtocolMissing ) == 0 )
            throw Exception( makeError( GPG_ERR_MISSING_VALUE ), i18n( "Required --protocol option missing" ) );
        else
            return GpgME::UnknownProtocol;
    else
        if ( mode == AssuanCommand::FileManager )
            throw Exception( makeError( GPG_ERR_INV_FLAG ), i18n("--protocol is not allowed here") );

    const QString protocolString = option("protocol").toString().toLower();
    if ( protocolString == QLatin1String( "openpgp" ) )
        return GpgME::OpenPGP;
    if ( protocolString == QLatin1String( "cms" ) )
        return GpgME::CMS;
    throw Exception( makeError( GPG_ERR_INV_ARG ), i18n( "invalid protocol \"%1\"", protocolString ) );
}

void AssuanCommand::doApplyWindowID( QWidget * widget ) const {
    if ( !widget || !hasOption( "window-id" ) )
        return;
    apply_window_id( widget, option("window-id").toString() );
}

WId AssuanCommand::parentWId() const {
    return wid_from_string( option("window-id").toString() );
}

static QString commonPrefix( const QString & s1, const QString & s2 ) {
    return QString( s1.data(), std::mismatch( s1.data(), s1.data() + std::min( s1.size(), s2.size() ), s2.data() ).first - s1.data() );
}

static QString longestCommonPrefix( const QStringList & sl ) {
    if ( sl.empty() )
        return QString();
    QString result = sl.front();
    Q_FOREACH( const QString & s, sl )
        result = commonPrefix( s, result );
    return result;
}

QString AssuanCommand::heuristicBaseDirectory() const {
    const QString candidate = longestCommonPrefix( fileNames() );
    const QFileInfo fi( candidate );
    if ( fi.isDir() )
        return candidate;
    else
        return fi.absolutePath();
}

#include "assuanserverconnection.moc"
#include "moc_assuanserverconnection.cpp"
