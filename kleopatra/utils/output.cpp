/* -*- mode: c++; c-basic-offset:4 -*-
    utils/output.cpp

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

#include "output.h"

#include "detail_p.h"
#include "kleo_assert.h"
#include "exception.h"
#include "kdpipeiodevice.h"
#include "log.h"

#include <KLocale>
#include <KMessageBox>
#include <kdebug.h>

#include <QTemporaryFile>
#include <QString>
#include <QClipboard>
#include <QApplication>
#include <QBuffer>
#include <QPointer>

#include <errno.h>

using namespace Kleo;
using namespace Kleo::_detail;
using namespace boost;

namespace {

    class TemporaryFile : public QTemporaryFile {
    public:
        explicit TemporaryFile() : QTemporaryFile() {}
        explicit TemporaryFile( const QString & templateName ) : QTemporaryFile( templateName ) {}
        explicit TemporaryFile( QObject * parent ) : QTemporaryFile( parent ) {}
        explicit TemporaryFile( const QString & templateName, QObject * parent ) : QTemporaryFile( templateName, parent ) {}

        /* reimp */ void close() {
            if ( isOpen() )
                m_oldFileName = fileName();
            QTemporaryFile::close();
        }

        QString oldFileName() const { return m_oldFileName; }

    private:
        QString m_oldFileName;
    };


    template <typename T_IODevice>
    struct inhibit_close : T_IODevice {
        explicit inhibit_close() : T_IODevice() {}
        template <typename T1>
        explicit inhibit_close( T1 & t1 ) : T_IODevice( t1 ) {}

        /* reimp */ void close() {}
        void reallyClose() { T_IODevice::close(); }
    };




    class OutputImplBase : public Output {
    public:
        OutputImplBase()
            : Output(),
              m_label(),
              m_isFinalized( false ),
              m_isFinalizing( false ),
              m_cancelPending( false ),
              m_canceled( false )
        {

        }

        /* reimp */ QString label() const { return m_label; }
        void setLabel( const QString & l ) { m_label = l; }

        /* reimp */ bool isFinalized() const { return m_isFinalized; }
        /* reimp */ void finalize() {
            kDebug() << this;
            if ( m_isFinalized || m_isFinalizing )
                return;
            m_isFinalizing = true;
            try { doFinalize(); } catch ( ... ) { m_isFinalizing = false; throw; }
            m_isFinalizing = false;
            m_isFinalized = true;
            if ( m_cancelPending )
                cancel();
        }

        /* reimp */ void cancel() {
            kDebug() << this;
            if ( m_isFinalizing ) {
                m_cancelPending = true;
            } else if ( !m_canceled ) {
                m_isFinalizing = true;
                try { doCancel(); } catch ( ... ) {}
                m_isFinalizing = false;
                m_isFinalized = true;
                m_canceled = true;
            }
        }
    private:
        virtual void doFinalize() = 0;
        virtual void doCancel() = 0;
    private:
        QString m_label;
        bool m_isFinalized   : 1;
        bool m_isFinalizing  : 1;
        bool m_cancelPending : 1;
        bool m_canceled      : 1;
    };


    class PipeOutput : public OutputImplBase {
    public:
        explicit PipeOutput( assuan_fd_t fd );

        /* reimp */ shared_ptr<QIODevice> ioDevice() const { return m_io; }
        /* reimp */ void doFinalize() { m_io->reallyClose(); }
        /* reimp */ void doCancel() { doFinalize(); }
    private:
        shared_ptr< inhibit_close<KDPipeIODevice> > m_io;
    };


    class FileOutput : public OutputImplBase {
    public:
        explicit FileOutput( const QString & fileName );
        ~FileOutput() { kDebug() << this; }

        enum OverwritePolicy {
            Allow = true,
            Deny = false,
            Ask
        };
        void setOverwritePolicy( OverwritePolicy overwrite ) { m_overwrite = overwrite; }

        /* reimp */ QString label() const { return m_fileName; }
        /* reimp */ shared_ptr<QIODevice> ioDevice() const { return m_tmpFile; }
        /* reimp */ void doFinalize();
        /* reimp */ void doCancel() { kDebug() << this; }
    private:
        const QString m_fileName;
        shared_ptr< TemporaryFile > m_tmpFile;
        OverwritePolicy m_overwrite;
    };

    class ClipboardOutput : public OutputImplBase {
    public:
        explicit ClipboardOutput( QClipboard::Mode mode );

        /* reimp */ QString label() const;
        /* reimp */ shared_ptr<QIODevice> ioDevice() const { return m_buffer; }
        /* reimp */ void doFinalize();
        /* reimp */ void doCancel() {}
    private:
        const QClipboard::Mode m_mode;
        shared_ptr<QBuffer> m_buffer;
    };

}


shared_ptr<Output> Output::createFromPipeDevice( assuan_fd_t fd, const QString & label ) {
    shared_ptr<PipeOutput> po( new PipeOutput( fd ) );
    po->setLabel( label );
    return po;
}

PipeOutput::PipeOutput( assuan_fd_t fd )
    : OutputImplBase(),
      m_io( new inhibit_close<KDPipeIODevice> )
{
    errno = 0;
    if ( !m_io->open( fd, QIODevice::WriteOnly ) )
        throw Exception( errno ? gpg_error_from_errno( errno ) : gpg_error( GPG_ERR_EIO ),
                         i18n( "Couldn't open FD %1 for writing",
                               assuanFD2int( fd ) ) );
}


shared_ptr<Output> Output::createFromFile( const QString & fileName, bool allowOverwrite ) {
    shared_ptr<FileOutput> fo( new FileOutput( fileName ) );
    if ( allowOverwrite )
        fo->setOverwritePolicy( FileOutput::Allow );
    kDebug() << fo.get();
    return fo;
}

FileOutput::FileOutput( const QString & fileName )
    : OutputImplBase(),
      m_fileName( fileName ),
      m_tmpFile( new TemporaryFile( fileName ) ),
      m_overwrite( Ask )
{
    errno = 0;
    if ( !m_tmpFile->open() )
        throw Exception( errno ? gpg_error_from_errno( errno ) : gpg_error( GPG_ERR_EIO ),
                         i18n( "Couldn't create temporary file for output \"%1\"", fileName ) );
}

static bool obtainOverwritePermission( const QString & fileName, QWidget * parent ) {
    return KMessageBox::questionYesNo( parent, i18n("The file <b>%1</b> already exists.\n"
                                                    "Overwrite?", fileName ),
                                       i18n("Overwrite Existing File?"), KStandardGuiItem::overwrite(), KStandardGuiItem::cancel() ) == KMessageBox::Yes ;
}

void FileOutput::doFinalize() {
    kDebug() << this;

    struct Remover {
        QString file;
        ~Remover() { if ( QFile::exists( file ) ) QFile::remove( file ); }
    } remover;

    kleo_assert( m_tmpFile );

    if ( m_tmpFile->isOpen() )
        m_tmpFile->close();

    const QString tmpFileName = remover.file = m_tmpFile->oldFileName();

    m_tmpFile->setAutoRemove( false );
    QPointer<QObject> guard = m_tmpFile.get();
    m_tmpFile.reset(); // really close the file - needed on Windows for renaming :/
    kleo_assert( !guard ); // if this triggers, we need to audit for holder of shared_ptr<QIODevice>s.

    kDebug() << this << " renaming " << tmpFileName << "->" << m_fileName ;

    if ( QFile::rename( tmpFileName, m_fileName ) ) {
        kDebug() << this << "succeeded";
        return;
    }

    kDebug() << this << "failed";

    if ( !obtainOverwritePermission( m_fileName, 0 ) )
        throw Exception( gpg_error( GPG_ERR_CANCELED ),
                         i18n( "Overwriting declined" ) );

    kDebug() << this << "going to overwrite" << m_fileName ;

    if ( !QFile::remove( m_fileName ) )
        throw Exception( errno ? gpg_error_from_errno( errno ) : gpg_error( GPG_ERR_EIO ),
                         i18n("Couldn't remove file \"%1\" for overwriting.", m_fileName ) );

    kDebug() << this << "succeeded, renaming " << tmpFileName << "->" << m_fileName;

    if ( QFile::rename( tmpFileName, m_fileName ) ) {
        kDebug() << this << "succeeded";
        return;
    }

    kDebug() << this << "failed";

    throw Exception( errno ? gpg_error_from_errno( errno ) : gpg_error( GPG_ERR_EIO ),
                     i18n( "Couldn't rename file \"%1\" to \"%2\"",
                           tmpFileName, m_fileName ) );
}

shared_ptr<Output> Output::createFromClipboard() {
    return shared_ptr<Output>( new ClipboardOutput( QClipboard::Clipboard ) );
}

ClipboardOutput::ClipboardOutput( QClipboard::Mode mode )
    : OutputImplBase(),
      m_mode( mode ),
      m_buffer( new QBuffer )
{
    errno = 0;
    if ( !m_buffer->open( QIODevice::WriteOnly ) )
        throw Exception( errno ? gpg_error_from_errno( errno ) : gpg_error( GPG_ERR_EIO ),
                         i18n( "Couldn't write to clipboard" ) );
}

QString ClipboardOutput::label() const {
    switch ( m_mode ) {
    case QClipboard::Clipboard:
        return i18n( "Clipboard" );
    case QClipboard::FindBuffer:
        return i18n( "Find buffer" );
    case QClipboard::Selection:
        return i18n( "Selection" );
    }
    return QString();
}

void ClipboardOutput::doFinalize() {
    if ( m_buffer->isOpen() )
        m_buffer->close();
    if ( QClipboard * const cb = QApplication::clipboard() )
        cb->setText( QString::fromUtf8( m_buffer->data() ) );
    else
        throw Exception( gpg_error( GPG_ERR_EIO ),
                         i18n( "Couldn't find clipboard" ) );
}



Output::~Output() {}
