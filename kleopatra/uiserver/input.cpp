/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/input.cpp

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
#include "kleo-assuan.h"

#include <utils/classify.h>
#include <utils/kdpipeiodevice.h>

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



    class PipeInput : public InputImplBase {
    public:
        explicit PipeInput( assuan_fd_t fd );

        /* reimp */ shared_ptr<QIODevice> ioDevice() const { return m_io; }
        /* reimp */ unsigned int classification() const;
    private:
        shared_ptr<KDPipeIODevice> m_io;
    };

    class FileInput : public InputImplBase {
    public:
        explicit FileInput( const QString & fileName );
        explicit FileInput( const shared_ptr<QFile> & file );

        /* reimp */ QString label() const { return m_file ? m_file->fileName() : InputImplBase::label() ; }
        /* reimp */ shared_ptr<QIODevice> ioDevice() const { return m_file; }
        /* reimp */ unsigned int classification() const;
    private:
        shared_ptr<QFile> m_file;
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

shared_ptr<Input> Input::createFromPipeDevice( assuan_fd_t fd, const QString & label ) {
    shared_ptr<PipeInput> po( new PipeInput( fd ) );
    po->setLabel( label );
    return po;
}

PipeInput::PipeInput( assuan_fd_t fd )
    : InputImplBase(),
      m_io( new KDPipeIODevice )
{
    errno = 0;
    if ( !m_io->open( fd, QIODevice::ReadOnly ) )
        throw assuan_exception( errno ? gpg_error_from_errno( errno ) : gpg_error( GPG_ERR_EIO ),
                                i18n( "Couldn't open FD %1 for writing",
                                      assuanFD2int( fd ) ) );
}

unsigned int PipeInput::classification() const {
    notImplemented();
}

shared_ptr<Input> Input::createFromFile( const QString & fileName, bool ) {
    return shared_ptr<Input>( new FileInput( fileName ) );
}

shared_ptr<Input> Input::createFromFile( const shared_ptr<QFile> & file ) {
    return shared_ptr<Input>( new FileInput( file ) );
}

FileInput::FileInput( const QString & fileName )
    : InputImplBase(),
      m_file( new QFile( fileName ) )
{
    errno = 0;
    if ( !m_file->open( QIODevice::ReadOnly ) )
        throw assuan_exception( errno ? gpg_error_from_errno( errno ) : gpg_error( GPG_ERR_EIO ),
                                i18n( "Couldn't open file \"%1\" for reading", fileName ) );
}

FileInput::FileInput( const shared_ptr<QFile> & file )
    : InputImplBase(),
      m_file( file )
{
    assuan_assert( file );
    errno = 0;
    if ( m_file->isOpen() && !m_file->isReadable() )
        throw assuan_exception( gpg_error( GPG_ERR_INV_ARG ),
                                i18n( "File \"%1\" is already open, but not for reading" ) );
    if ( !m_file->isOpen() && !m_file->open( QIODevice::ReadOnly ) )
        throw assuan_exception( errno ? gpg_error_from_errno( errno ) : gpg_error( GPG_ERR_EIO ),
                                i18n( "Couldn't open file \"%1\" for reading", m_file->fileName() ) );
}

unsigned int FileInput::classification() const {
    return classify( m_file->fileName() );
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
