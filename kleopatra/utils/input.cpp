/* -*- mode: c++; c-basic-offset:4 -*-
    utils/input.cpp

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

#include <config-kleopatra.h>

#include "input.h"

#include "detail_p.h"
#include "classify.h"
#include "kdpipeiodevice.h"
#include "log.h"
#include "kleo_assert.h"
#include "cached.h"

#include <kleo/exception.h>

#include <KDebug>
#include <KLocalizedString>

#include <QFile>
#include <QString>
#include <QClipboard>
#include <QApplication>
#include <QByteArray>
#include <QBuffer>
#include <QDir>
#include <QFileInfo>
#include <QProcess>

#include <errno.h>

using namespace Kleo;
using namespace boost;

namespace {

    class Process : public QProcess {
    public:
        explicit Process( QObject * parent=0 )
            : QProcess( parent ) {}
        /* reimp */ void close() { closeReadChannel( StandardOutput ); }
    };
}

namespace {

    class InputImplBase : public Input {
    public:
        InputImplBase() : Input(), m_customLabel(), m_defaultLabel() {}

        /* reimp */ QString label() const { return m_customLabel.isEmpty() ? m_defaultLabel : m_customLabel; }
        void setDefaultLabel( const QString & l ) { m_defaultLabel = l; }
        /* reimp */ void setLabel( const QString & l ) { m_customLabel = l; }
        /* reimp */ QString errorString() const {
            if ( m_errorString.dirty() )
                m_errorString = doErrorString();
            return m_errorString;
        }

    private:
        virtual QString doErrorString() const {
            if ( const shared_ptr<QIODevice> io = ioDevice() )
                return io->errorString();
            else
                return i18n("No input device");
        }

    private:
        QString m_customLabel;
        QString m_defaultLabel;
        mutable cached<QString> m_errorString;
    };



    class PipeInput : public InputImplBase {
    public:
        explicit PipeInput( assuan_fd_t fd );

        /* reimp */ shared_ptr<QIODevice> ioDevice() const { return m_io; }
        /* reimp */ unsigned int classification() const;
        /* reimp */ unsigned long long size() const { return 0; }

    private:
        shared_ptr<QIODevice> m_io;
    };

    class ProcessStdOutInput : public InputImplBase {
    public:
        explicit ProcessStdOutInput( const QString & cmd, const QStringList & args, const QDir & wd, const QByteArray & stdin_=QByteArray() );

        /* reimp */ shared_ptr<QIODevice> ioDevice() const { return m_proc; }
        /* reimp */ unsigned int classification() const { return 0U; } // plain text
        /* reimp */ unsigned long long size() const { return 0; }
        /* reimp */ QString label() const;

    private:
        /* reimp */ QString doErrorString() const;

    private:
        const QString m_command;
        const QStringList m_arguments;
        const shared_ptr<Process> m_proc;
    };

    class FileInput : public InputImplBase {
    public:
        explicit FileInput( const QString & fileName );
        explicit FileInput( const shared_ptr<QFile> & file );

        /* reimp */ QString label() const {
            return m_io ? QFileInfo( m_fileName ).fileName() : InputImplBase::label();
        }
        /* reimp */ shared_ptr<QIODevice> ioDevice() const { return m_io; }
        /* reimp */ unsigned int classification() const;
        /* reimp */ unsigned long long size() const { return QFileInfo( m_fileName ).size(); }

    private:
        shared_ptr<QIODevice> m_io;
        QString m_fileName;
    };

#ifndef QT_NO_CLIPBOARD
    class ClipboardInput : public Input {
    public:
        explicit ClipboardInput( QClipboard::Mode mode );

        /* reimp */ void setLabel( const QString & label );
        /* reimp */ QString label() const;
        /* reimp */ shared_ptr<QIODevice> ioDevice() const { return m_buffer; }
        /* reimp */ unsigned int classification() const;
        /* reimp */ unsigned long long size() const { return m_buffer ? m_buffer->buffer().size() : 0; }
        /* reimp */ QString errorString() const { return QString(); }

    private:
        const QClipboard::Mode m_mode;
        shared_ptr<QBuffer> m_buffer;
    };
#endif // QT_NO_CLIPBOARD

}


shared_ptr<Input> Input::createFromPipeDevice( assuan_fd_t fd, const QString & label ) {
    shared_ptr<PipeInput> po( new PipeInput( fd ) );
    po->setDefaultLabel( label );
    return po;
}

PipeInput::PipeInput( assuan_fd_t fd )
    : InputImplBase(),
      m_io()
{
    shared_ptr<KDPipeIODevice> kdp( new KDPipeIODevice );
    errno = 0;
    if ( !kdp->open( fd, QIODevice::ReadOnly ) )
        throw Exception( errno ? gpg_error_from_errno( errno ) : gpg_error( GPG_ERR_EIO ),
                         i18n( "Could not open FD %1 for reading",
                               _detail::assuanFD2int( fd ) ) );
    m_io = Log::instance()->createIOLogger( kdp, QLatin1String("pipe-input"), Log::Read );
}


unsigned int PipeInput::classification() const {
    notImplemented();
    return 0;
}


shared_ptr<Input> Input::createFromFile( const QString & fileName, bool ) {
    return shared_ptr<Input>( new FileInput( fileName ) );
}

shared_ptr<Input> Input::createFromFile( const shared_ptr<QFile> & file ) {
    return shared_ptr<Input>( new FileInput( file ) );
}

FileInput::FileInput( const QString & fileName )
    : InputImplBase(),
      m_io(), m_fileName( fileName )
{
    shared_ptr<QFile> file( new QFile( fileName ) );

    errno = 0;
    if ( !file->open( QIODevice::ReadOnly ) )
        throw Exception( errno ? gpg_error_from_errno( errno ) : gpg_error( GPG_ERR_EIO ),
                         i18n( "Could not open file \"%1\" for reading", fileName ) );
    m_io = Log::instance()->createIOLogger( file, QLatin1String("file-in"), Log::Read );

}

FileInput::FileInput( const shared_ptr<QFile> & file )
    : InputImplBase(),
      m_io(), m_fileName( file->fileName() )
{
    kleo_assert( file );
    errno = 0;
    if ( file->isOpen() && !file->isReadable() )
        throw Exception( gpg_error( GPG_ERR_INV_ARG ),
                                i18n( "File \"%1\" is already open, but not for reading", file->fileName() ) );
    if ( !file->isOpen() && !file->open( QIODevice::ReadOnly ) )
        throw Exception( errno ? gpg_error_from_errno( errno ) : gpg_error( GPG_ERR_EIO ),
                         i18n( "Could not open file \"%1\" for reading", m_fileName ) );
    m_io = Log::instance()->createIOLogger( file, QLatin1String("file-in"), Log::Read );
}

unsigned int FileInput::classification() const {
    return classify( m_fileName );
}


shared_ptr<Input> Input::createFromProcessStdOut( const QString & command ) {
    return shared_ptr<Input>( new ProcessStdOutInput( command, QStringList(), QDir::current() ) );
}

shared_ptr<Input> Input::createFromProcessStdOut( const QString & command, const QStringList & args ) {
    return shared_ptr<Input>( new ProcessStdOutInput( command, args, QDir::current() ) );
}

shared_ptr<Input> Input::createFromProcessStdOut( const QString & command, const QStringList & args, const QDir & wd ) {
    return shared_ptr<Input>( new ProcessStdOutInput( command, args, wd ) );
}

shared_ptr<Input> Input::createFromProcessStdOut( const QString & command, const QByteArray & stdin_ ) {
    return shared_ptr<Input>( new ProcessStdOutInput( command, QStringList(), QDir::current(), stdin_ ) );
}

shared_ptr<Input> Input::createFromProcessStdOut( const QString & command, const QStringList & args, const QByteArray & stdin_ ) {
    return shared_ptr<Input>( new ProcessStdOutInput( command, args, QDir::current(), stdin_ ) );
}

shared_ptr<Input> Input::createFromProcessStdOut( const QString & command, const QStringList & args, const QDir & wd, const QByteArray & stdin_ ) {
    return shared_ptr<Input>( new ProcessStdOutInput( command, args, wd, stdin_ ) );
}

namespace {
    struct Outputter {
        const QByteArray & data;
        explicit Outputter( const QByteArray & data ) : data( data ) {}
    };
    static QDebug operator<<( QDebug s, const Outputter & o ) {
        if ( const quint64 size = o.data.size() )
            s << " << (" << size << "bytes)";
        return s;
    }
}

ProcessStdOutInput::ProcessStdOutInput( const QString & cmd, const QStringList & args, const QDir & wd, const QByteArray & stdin_ )
    : InputImplBase(),
      m_command( cmd ),
      m_arguments( args ),
      m_proc( new Process )
{
    const QIODevice::OpenMode openMode =
        stdin_.isEmpty() ? QIODevice::ReadOnly : QIODevice::ReadWrite ;
    qDebug() << "cd" << wd.absolutePath() << endl << cmd << args << Outputter( stdin_ );
    if ( cmd.isEmpty() )
        throw Exception( gpg_error( GPG_ERR_INV_ARG ),
                         i18n("Command not specified") );
    m_proc->setWorkingDirectory( wd.absolutePath() );
    m_proc->start( cmd, args, openMode );
    if ( !m_proc->waitForStarted() )
        throw Exception( gpg_error( GPG_ERR_EIO ),
                         i18n( "Could not start %1 process: %2", cmd, m_proc->errorString() ) );

    if ( !stdin_.isEmpty() ) {
        if ( m_proc->write( stdin_ ) != stdin_.size() )
            throw Exception( gpg_error( GPG_ERR_EIO ),
                             i18n( "Failed to write input to %1 process: %2", cmd, m_proc->errorString() ) );
        m_proc->closeWriteChannel();
    }
}

QString ProcessStdOutInput::label() const {
    if ( !m_proc )
        return InputImplBase::label();
    // output max. 3 arguments
    const QString cmdline = ( QStringList( m_command ) + m_arguments.mid(0,3) ).join( QLatin1String(" ") );
    if ( m_arguments.size() > 3 )
        return i18nc( "e.g. \"Output of tar xf - file1 ...\"", "Output of %1 ...", cmdline );
    else
        return i18nc( "e.g. \"Output of tar xf - file\"",      "Output of %1",     cmdline );
}

QString ProcessStdOutInput::doErrorString() const {
    kleo_assert( m_proc );
    if ( m_proc->exitStatus() == QProcess::NormalExit && m_proc->exitCode() == 0 )
        return QString();
    if ( m_proc->error() == QProcess::UnknownError )
        return i18n( "Error while running %1:\n%2", m_command,
                     QString::fromLocal8Bit( m_proc->readAllStandardError().trimmed().constData() ) );
    else
        return i18n( "Failed to execute %1: %2", m_command, m_proc->errorString() );
}

#ifndef QT_NO_CLIPBOARD
shared_ptr<Input> Input::createFromClipboard() {
    return shared_ptr<Input>( new ClipboardInput( QClipboard::Clipboard ) );
}

static QByteArray dataFromClipboard( QClipboard::Mode mode )
{
    Q_UNUSED( mode );
    if ( QClipboard * const cb = QApplication::clipboard() )
        return cb->text().toUtf8();
    else
        return QByteArray();
}

ClipboardInput::ClipboardInput( QClipboard::Mode mode )
    : Input(),
      m_mode( mode ),
      m_buffer( new QBuffer )
{
    m_buffer->setData( dataFromClipboard( mode ) );
    if ( !m_buffer->open( QIODevice::ReadOnly ) )
        throw Exception( gpg_error( GPG_ERR_EIO ),
                         i18n( "Could not open clipboard for reading" ) );
}

void ClipboardInput::setLabel( const QString & ) {
    notImplemented();
}

QString ClipboardInput::label() const {
    switch ( m_mode ) {
    case QClipboard::Clipboard:
        return i18n( "Clipboard contents" );
    case QClipboard::FindBuffer:
        return i18n( "FindBuffer contents" );
    case QClipboard::Selection:
        return i18n( "Current selection" );
    };
    return QString();
}

unsigned int ClipboardInput::classification() const {
    return classifyContent( m_buffer->data() );
}
#endif // QT_NO_CLIPBOARD

Input::~Input() {}

void Input::finalize() {
    if ( const shared_ptr<QIODevice> io = ioDevice() )
        if ( io->isOpen() )
            io->close();
}
