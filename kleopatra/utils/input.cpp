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

#include <errno.h>

using namespace Kleo;
using namespace Kleo::_detail;
using namespace boost;

namespace {

    class InputImplBase : public Input {
    public:
        InputImplBase() : Input(), m_label() {}

        /* reimp */ QString label() const { return m_label; }
        void setLabel( const QString & l ) { m_label = l; }

    private:
        QString m_label;
    };



#ifdef HAVE_USABLE_ASSUAN
    class PipeInput : public InputImplBase {
    public:
        explicit PipeInput( assuan_fd_t fd );

        /* reimp */ shared_ptr<QIODevice> ioDevice() const { return m_io; }
        /* reimp */ unsigned int classification() const;
    private:
        shared_ptr<QIODevice> m_io;
    };
#endif // HAVE_USABLE_ASSUAN

    class FileInput : public InputImplBase {
    public:
        explicit FileInput( const QString & fileName );
        explicit FileInput( const shared_ptr<QFile> & file );

        /* reimp */ QString label() const { return m_io ? m_fileName : InputImplBase::label() ; }
        /* reimp */ shared_ptr<QIODevice> ioDevice() const { return m_io; }
        /* reimp */ unsigned int classification() const;
    private:
        shared_ptr<QIODevice> m_io;
        QString m_fileName;
    };

#if 0
    class ClipboardInput : public Input {
    public:
        explicit ClipboardInput( QClipboard::Mode mode );
        ~ClipboardInput();

        /* reimp */ QString label() const;
        /* reimp */ shared_ptr<QIODevice> ioDevice() const;
        /* reimp */ unsigned int classification() const;
    };
#endif

}

#ifdef HAVE_USABLE_ASSUAN
shared_ptr<Input> Input::createFromPipeDevice( assuan_fd_t fd, const QString & label ) {
    shared_ptr<PipeInput> po( new PipeInput( fd ) );
    po->setLabel( label );
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
                         i18n( "Couldn't open FD %1 for reading",
                               assuanFD2int( fd ) ) );
    m_io = Log::instance()->createIOLogger( kdp, "pipe-input", Log::Read );
}


unsigned int PipeInput::classification() const {
    notImplemented();
    return 0;
}

#endif // HAVE_USABLE_ASSUAN

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
                         i18n( "Couldn't open file \"%1\" for reading", fileName ) );
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
                                i18n( "File \"%1\" is already open, but not for reading" ) );
    if ( !file->isOpen() && !file->open( QIODevice::ReadOnly ) )
        throw Exception( errno ? gpg_error_from_errno( errno ) : gpg_error( GPG_ERR_EIO ),
                         i18n( "Couldn't open file \"%1\" for reading", m_fileName ) );
    m_io = Log::instance()->createIOLogger( file, "file-in", Log::Read );
}

unsigned int FileInput::classification() const {
    return classify( m_fileName );
}

#if 0
shared_ptr<Input> Input::createFromClipboard( QClipboard::Mode mode ) {
    notImplemented();
}
#endif

Input::~Input() {}

void Input::finalize() {
    if ( const shared_ptr<QIODevice> io = ioDevice() )
        if ( io->isOpen() )
            io->close();
}
