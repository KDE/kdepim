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
#include "kdpipeiodevice.h"
#include "log.h"
#include "cached.h"

#include <kleo/exception.h>

#include <KLocale>
#include <KMessageBox>
#include <kdebug.h>

#include <QFileInfo>
#include <QTemporaryFile>
#include <QString>
#include <QClipboard>
#include <QApplication>
#include <QBuffer>
#include <QPointer>
#include <QWidget>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QTimer>

#ifdef Q_OS_WIN
# include <windows.h>
#endif

#include <errno.h>

using namespace Kleo;
using namespace Kleo::_detail;
using namespace boost;

static const int PROCESS_MAX_RUNTIME_TIMEOUT = -1;     // no timeout
static const int PROCESS_TERMINATE_TIMEOUT   = 5*1000; // 5s

class OverwritePolicy::Private {
public:
    Private( QWidget* p, OverwritePolicy::Policy pol ) : policy( pol ), widget( p ) {}
    OverwritePolicy::Policy policy;
    QWidget* widget;
};

OverwritePolicy::OverwritePolicy( QWidget * parent, Policy initialPolicy ) : d( new Private( parent, initialPolicy ) ) {
}

OverwritePolicy::~OverwritePolicy() {}

OverwritePolicy::Policy OverwritePolicy::policy() const {
    return d->policy;
}

void OverwritePolicy::setPolicy( Policy policy ) {
    d->policy = policy;
}

QWidget * OverwritePolicy::parentWidget() const {
    return d->widget;
}

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

        bool openNonInheritable() {
            if ( !QTemporaryFile::open() )
                return false;
#if defined(Q_OS_WIN) && !defined(_WIN32_WCE)
            //QTemporaryFile (tested with 4.3.3) creates the file handle as inheritable.
            //The handle is then inherited by gpgsm, which prevents deletion of the temp file
            //in FileOutput::doFinalize()
            //There are no inheritable handles under wince
            return SetHandleInformation( (HANDLE)_get_osfhandle( handle() ), HANDLE_FLAG_INHERIT, 0 );
#endif
            return true;
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

    template <typename T_IODevice>
    struct redirect_close : T_IODevice {
        explicit redirect_close() : T_IODevice(), m_closed( false ) {}
        template <typename T1>
        explicit redirect_close( T1 & t1 ) : T_IODevice( t1 ), m_closed( false ) {}

        /* reimp */ void close() { this->closeWriteChannel(); m_closed = true; }

        bool isClosed() const { return m_closed; }
    private:
        bool m_closed;
    };


    class OutputImplBase : public Output {
    public:
        OutputImplBase()
            : Output(),
              m_defaultLabel(),
              m_customLabel(),
              m_errorString(),
              m_isFinalized( false ),
              m_isFinalizing( false ),
              m_cancelPending( false ),
              m_canceled( false ),
              m_binaryOpt( false )
        {

        }

        /* reimp */ QString label() const { return m_customLabel.isEmpty() ? m_defaultLabel : m_customLabel; }
        /* reimp */ void setLabel( const QString & label ) { m_customLabel = label; }
        void setDefaultLabel( const QString & l ) { m_defaultLabel = l; }
        /* reimp */ void setBinaryOpt( bool value ) { m_binaryOpt = value; }
        /* reimp */ bool binaryOpt() const { return m_binaryOpt; }

        /* reimp */ QString errorString() const {
            if ( m_errorString.dirty() )
                m_errorString = doErrorString();
            return m_errorString;
        }

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
        virtual QString doErrorString() const {
            if ( shared_ptr<QIODevice> io = ioDevice() )
                return io->errorString();
            else
                return i18n("No output device");
        }
        virtual void doFinalize() = 0;
        virtual void doCancel() = 0;
    private:
        QString m_defaultLabel;
        QString m_customLabel;
        mutable cached<QString> m_errorString;
        bool m_isFinalized   : 1;
        bool m_isFinalizing  : 1;
        bool m_cancelPending : 1;
        bool m_canceled      : 1;
        bool m_binaryOpt     : 1;
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


    class ProcessStdInOutput : public OutputImplBase {
    public:
        explicit ProcessStdInOutput( const QString & cmd, const QStringList & args, const QDir & wd );

        /* reimp */ shared_ptr<QIODevice> ioDevice() const { return m_proc; }
        /* reimp */ void doFinalize() {
            /*
              Make sure the data is written in the output here. If this
              is not done the output will be written in small chunks
              trough the eventloop causing an uncessary delay before
              the process has even a chance to work and finish.
              This delay is mainly noticabe on Windows where it can
              take ~30 seconds to write out a 10MB file in the 512 byte
              chunks gpgme serves. */
            kDebug(5151) << "Waiting for " << m_proc->bytesToWrite()
                         << " Bytes to be written";
            while ( m_proc->waitForBytesWritten( PROCESS_MAX_RUNTIME_TIMEOUT ) );

            if ( !m_proc->isClosed() ) {
                m_proc->close();
            }
            m_proc->waitForFinished( PROCESS_MAX_RUNTIME_TIMEOUT );
        }
        /* reimp */ void doCancel() {
            m_proc->terminate();
            QTimer::singleShot( PROCESS_TERMINATE_TIMEOUT, m_proc.get(), SLOT(kill()) );
        }
        /* reimp */ QString label() const;

    private:
        /* reimp */ QString doErrorString() const;

    private:
        const QString m_command;
        const QStringList m_arguments;
        const shared_ptr< redirect_close<QProcess> > m_proc;
    };


    class FileOutput : public OutputImplBase {
    public:
        explicit FileOutput( const QString & fileName, const shared_ptr<OverwritePolicy> & policy );
        ~FileOutput() { kDebug() << this; }

        /* reimp */ QString label() const { return QFileInfo( m_fileName ).fileName(); }
        /* reimp */ shared_ptr<QIODevice> ioDevice() const { return m_tmpFile; }
        /* reimp */ void doFinalize();
        /* reimp */ void doCancel() { kDebug() << this; }
    private:
        bool obtainOverwritePermission();

    private:
        const QString m_fileName;
        shared_ptr< TemporaryFile > m_tmpFile;
        const shared_ptr<OverwritePolicy> m_policy;
    };

#ifndef QT_NO_CLIPBOARD
    class ClipboardOutput : public OutputImplBase {
    public:
        explicit ClipboardOutput( QClipboard::Mode mode );

        /* reimp */ QString label() const;
        /* reimp */ shared_ptr<QIODevice> ioDevice() const { return m_buffer; }
        /* reimp */ void doFinalize();
        /* reimp */ void doCancel() {}

    private:
        /* reimp */ QString doErrorString() const { return QString(); }
    private:
        const QClipboard::Mode m_mode;
        shared_ptr<QBuffer> m_buffer;
    };
#endif // QT_NO_CLIPBOARD

}


shared_ptr<Output> Output::createFromPipeDevice( assuan_fd_t fd, const QString & label ) {
    shared_ptr<PipeOutput> po( new PipeOutput( fd ) );
    po->setDefaultLabel( label );
    return po;
}

PipeOutput::PipeOutput( assuan_fd_t fd )
    : OutputImplBase(),
      m_io( new inhibit_close<KDPipeIODevice> )
{
    errno = 0;
    if ( !m_io->open( fd, QIODevice::WriteOnly ) )
        throw Exception( errno ? gpg_error_from_errno( errno ) : gpg_error( GPG_ERR_EIO ),
                         i18n( "Could not open FD %1 for writing",
                               assuanFD2int( fd ) ) );
}


shared_ptr<Output> Output::createFromFile( const QString & fileName, bool forceOverwrite ) {
    return createFromFile( fileName, shared_ptr<OverwritePolicy>( new OverwritePolicy( 0, forceOverwrite ? OverwritePolicy::Allow : OverwritePolicy::Deny ) ) );

}
shared_ptr<Output> Output::createFromFile( const QString & fileName, const shared_ptr<OverwritePolicy> & policy ) {
    shared_ptr<FileOutput> fo( new FileOutput( fileName, policy ) );
    kDebug() << fo.get();
    return fo;
}

FileOutput::FileOutput( const QString & fileName, const shared_ptr<OverwritePolicy> & policy )
    : OutputImplBase(),
      m_fileName( fileName ),
      m_tmpFile( new TemporaryFile( fileName ) ),
      m_policy( policy )
{
    assert( m_policy );
    errno = 0;
    if ( !m_tmpFile->openNonInheritable() )
        throw Exception( errno ? gpg_error_from_errno( errno ) : gpg_error( GPG_ERR_EIO ),
                         i18n( "Could not create temporary file for output \"%1\"", fileName ) );
}

bool FileOutput::obtainOverwritePermission() {
    if ( m_policy->policy() != OverwritePolicy::Ask )
        return m_policy->policy() == OverwritePolicy::Allow;
    const int sel = KMessageBox::questionYesNoCancel( m_policy->parentWidget(), i18n("The file <b>%1</b> already exists.\n"
                                                                                      "Overwrite?", m_fileName ),
                                                      i18n("Overwrite Existing File?"),
                                                      KStandardGuiItem::overwrite(),
                                                      KGuiItem( i18n( "Overwrite All" ) ),
                                                      KStandardGuiItem::cancel() );
    if ( sel == KMessageBox::No ) //Overwrite All
        m_policy->setPolicy( OverwritePolicy::Allow );
    return sel == KMessageBox::Yes || sel == KMessageBox::No;
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

    if ( !obtainOverwritePermission() )
        throw Exception( gpg_error( GPG_ERR_CANCELED ),
                         i18n( "Overwriting declined" ) );

    kDebug() << this << "going to overwrite" << m_fileName ;

    if ( !QFile::remove( m_fileName ) )
        throw Exception( errno ? gpg_error_from_errno( errno ) : gpg_error( GPG_ERR_EIO ),
                         i18n("Could not remove file \"%1\" for overwriting.", m_fileName ) );

    kDebug() << this << "succeeded, renaming " << tmpFileName << "->" << m_fileName;

    if ( QFile::rename( tmpFileName, m_fileName ) ) {
        kDebug() << this << "succeeded";
        return;
    }

    kDebug() << this << "failed";

    throw Exception( errno ? gpg_error_from_errno( errno ) : gpg_error( GPG_ERR_EIO ),
                     i18n( "Could not rename file \"%1\" to \"%2\"",
                           tmpFileName, m_fileName ) );
}


shared_ptr<Output> Output::createFromProcessStdIn( const QString & command ) {
    return shared_ptr<Output>( new ProcessStdInOutput( command, QStringList(), QDir::current() ) );
}

shared_ptr<Output> Output::createFromProcessStdIn( const QString & command, const QStringList & args ) {
    return shared_ptr<Output>( new ProcessStdInOutput( command, args, QDir::current() ) );
}

shared_ptr<Output> Output::createFromProcessStdIn( const QString & command, const QStringList & args, const QDir & wd ) {
    return shared_ptr<Output>( new ProcessStdInOutput( command, args, wd ) );
}

ProcessStdInOutput::ProcessStdInOutput( const QString & cmd, const QStringList & args, const QDir & wd )
    : OutputImplBase(),
      m_command( cmd ),
      m_arguments( args ),
      m_proc( new redirect_close<QProcess> )
{
    kDebug() << "cd" << wd.absolutePath() << endl << cmd << args;
    if ( cmd.isEmpty() )
        throw Exception( gpg_error( GPG_ERR_INV_ARG ),
                         i18n("Command not specified") );
    m_proc->setWorkingDirectory( wd.absolutePath() );
    m_proc->start( cmd, args );
    m_proc->setReadChannel( QProcess::StandardError );
    if ( !m_proc->waitForStarted() )
        throw Exception( gpg_error( GPG_ERR_EIO ),
                         i18n( "Could not start %1 process: %2", cmd, m_proc->errorString() ) );
}

QString ProcessStdInOutput::label() const {
    if ( !m_proc )
        return OutputImplBase::label();
    // output max. 3 arguments
    const QString cmdline = ( QStringList( m_command ) + m_arguments.mid(0,3) ).join( " " );
    if ( m_arguments.size() > 3 )
        return i18nc( "e.g. \"Input to tar xf - file1 ...\"", "Input to %1 ...", cmdline );
    else
        return i18nc( "e.g. \"Input to tar xf - file\"",      "Input to %1",     cmdline );
}

QString ProcessStdInOutput::doErrorString() const {
    kleo_assert( m_proc );
    if ( m_proc->exitStatus() == QProcess::NormalExit && m_proc->exitCode() == 0 )
        return QString();
    if ( m_proc->error() == QProcess::UnknownError )
        return i18n( "Error while running %1: %2", m_command,
                     QString::fromLocal8Bit( m_proc->readAllStandardError().trimmed().constData() ) );
    else
        return i18n( "Failed to execute %1: %2", m_command, m_proc->errorString() );
}

#ifndef QT_NO_CLIPBOARD
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
                         i18n( "Could not write to clipboard" ) );
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
                         i18n( "Could not find clipboard" ) );
}
#endif // QT_NO_CLIPBOARD



Output::~Output() {}
