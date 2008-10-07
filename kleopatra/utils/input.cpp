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
#include "exception.h"

#include <KLocale>

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

    class InputImplBase : public Input {
    public:
        InputImplBase() : Input(), m_customLabel(), m_defaultLabel() {}

        /* reimp */ QString label() const { return m_customLabel.isEmpty() ? m_defaultLabel : m_customLabel; }
        void setDefaultLabel( const QString & l ) { m_defaultLabel = l; }
        /* reimp */ void setLabel( const QString & l ) { m_customLabel = l; }

    private:
        QString m_customLabel;
        QString m_defaultLabel;
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

    class DirInput : public InputImplBase {
    public:
        explicit DirInput( const QString & path );

        /* reimp */ shared_ptr<QIODevice> ioDevice() const { return m_proc; }
        /* reimp */ unsigned int classification() const;
        /* reimp */ unsigned long long size() const { return 0; }
        /* reimp */ QString label() const {
            return m_proc ? QFileInfo( m_path ).fileName() : InputImplBase::label();
        }

    private:
        QString m_path;
        shared_ptr<QProcess> m_proc;
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

    class ClipboardInput : public Input {
    public:
        explicit ClipboardInput( QClipboard::Mode mode );

        /* reimp */ void setLabel( const QString & label );
        /* reimp */ QString label() const;
        /* reimp */ shared_ptr<QIODevice> ioDevice() const { return m_buffer; }
        /* reimp */ unsigned int classification() const;
        /* reimp */ unsigned long long size() const { return m_buffer ? m_buffer->buffer().size() : 0; }

    private:
        const QClipboard::Mode m_mode;
        shared_ptr<QBuffer> m_buffer;
    };

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
    m_io = Log::instance()->createIOLogger( kdp, "pipe-input", Log::Read );
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
    m_io = Log::instance()->createIOLogger( file, "file-in", Log::Read );

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
    m_io = Log::instance()->createIOLogger( file, "file-in", Log::Read );
}

unsigned int FileInput::classification() const {
    return classify( m_fileName );
}


shared_ptr<Input> Input::createFromDir( const QString & path ) {
    return shared_ptr<DirInput>( new DirInput( path ) );
}

DirInput::DirInput( const QString & path )
    : InputImplBase(),
      m_path( path ),
      m_proc() {
    if ( !QFileInfo( path ).isDir() )
        throw Exception( gpg_error( GPG_ERR_EIO ),
                         i18n( "\"%1\" is not a valid directory", path ) );

    m_proc.reset( new QProcess );
    QDir parentDir( path );
    if ( !parentDir.cdUp() )
        throw Exception( gpg_error( GPG_ERR_EIO ),
                         i18n( "Could not determine parent directory of \"%1\"", path ) );

    m_proc->setWorkingDirectory( parentDir.absolutePath() );
    m_proc->start( "zip", QStringList() << "-r" << "-" << QFileInfo( path ).fileName(), QIODevice::ReadOnly );
    if ( !m_proc->waitForStarted() )
        throw Exception( gpg_error( GPG_ERR_EIO ),
                         i18n( "Could not start zip process" ) );
}

unsigned int DirInput::classification() const {
    notImplemented();
    return 0;
}

shared_ptr<Input> Input::createFromClipboard() {
    return shared_ptr<Input>( new ClipboardInput( QClipboard::Clipboard ) );
}

static QByteArray dataFromClipboard( QClipboard::Mode mode ) {
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

Input::~Input() {}

void Input::finalize() {
    if ( const shared_ptr<QIODevice> io = ioDevice() )
        if ( io->isOpen() )
            io->close();
}
