/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/verifychecksumscontroller.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2010 Klarälvdalens Datakonsult AB

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

#include "verifychecksumscontroller.h"

#ifndef QT_NO_DIRMODEL

#include <crypto/gui/verifychecksumsdialog.h>

#include <utils/input.h>
#include <utils/output.h>
#include <utils/classify.h>
#include <utils/kleo_assert.h>

#include <kleo/stl_util.h>
#include <kleo/checksumdefinition.h>

#include <KLocalizedString>

#include <QDebug>
#include <QPointer>
#include <QFileInfo>
#include <QThread>
#include <QMutex>
#include <QProgressDialog>
#include <QDir>
#include <QProcess>

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <gpg-error.h>

#include <deque>
#include <limits>
#include <set>

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace Kleo::Crypto::Gui;
using namespace boost;

#ifdef Q_OS_UNIX
static const bool HAVE_UNIX = true;
#else
static const bool HAVE_UNIX = false;
#endif

static const QLatin1String CHECKSUM_DEFINITION_ID_ENTRY( "checksum-definition-id" );

static const Qt::CaseSensitivity fs_cs = HAVE_UNIX ? Qt::CaseSensitive : Qt::CaseInsensitive ; // can we use QAbstractFileEngine::caseSensitive()?

#if 0
static QStringList fs_sort( QStringList l ) {
    int (*QString_compare)(const QString&,const QString&,Qt::CaseSensitivity) = &QString::compare;
    kdtools::sort( l, boost::bind( QString_compare, _1, _2, fs_cs ) < 0 );
    return l;
}

static QStringList fs_intersect( QStringList l1, QStringList l2 ) {
    int (*QString_compare)(const QString&,const QString&,Qt::CaseSensitivity) = &QString::compare;
    fs_sort( l1 );
    fs_sort( l2 );
    QStringList result;
    std::set_intersection( l1.begin(), l1.end(),
                           l2.begin(), l2.end(),
                           std::back_inserter( result ),
                           boost::bind( QString_compare, _1, _2, fs_cs ) < 0 );
    return result;
}
#endif

static QList<QRegExp> get_patterns( const std::vector< shared_ptr<ChecksumDefinition> > & checksumDefinitions )
{
    QList<QRegExp> result;
    Q_FOREACH( const shared_ptr<ChecksumDefinition> & cd, checksumDefinitions )
        if ( cd )
            Q_FOREACH( const QString & pattern, cd->patterns() )
                result.push_back( QRegExp( pattern, fs_cs ) );
    return result;
}

namespace {
    struct matches_any : std::unary_function<QString,bool> {
        const QList<QRegExp> m_regexps;
        explicit matches_any( const QList<QRegExp> & regexps ) : m_regexps( regexps ) {}
        bool operator()( const QString & s ) const {
            return kdtools::any( m_regexps, boost::bind( &QRegExp::exactMatch, _1, s ) );
        }
    };
    struct matches_none_of : std::unary_function<QString,bool> {
        const QList<QRegExp> m_regexps;
        explicit matches_none_of( const QList<QRegExp> & regexps ) : m_regexps( regexps ) {}
        bool operator()( const QString & s ) const {
            return kdtools::none_of( m_regexps, boost::bind( &QRegExp::exactMatch, _1, s ) );
        }
    };
}

class VerifyChecksumsController::Private : public QThread {
    Q_OBJECT
    friend class ::Kleo::Crypto::VerifyChecksumsController;
    VerifyChecksumsController * const q;
public:
    explicit Private( VerifyChecksumsController * qq );
    ~Private();

Q_SIGNALS:
    void baseDirectories( const QStringList & );
    void progress( int, int, const QString & );
    void status( const QString & file, Kleo::Crypto::Gui::VerifyChecksumsDialog::Status );

private:
    void slotOperationFinished() {
        if ( dialog ) {
            dialog->setProgress( 100, 100 );
            dialog->setErrors( errors );
        }

        if ( !errors.empty() )
            q->setLastError( gpg_error( GPG_ERR_GENERAL ),
                             errors.join( QLatin1String("\n") ) );
        q->emitDoneOrError();
    }

private:
    /* reimp */ void run();

private:
    QPointer<VerifyChecksumsDialog> dialog;
    mutable QMutex mutex;
    const std::vector< shared_ptr<ChecksumDefinition> > checksumDefinitions;
    QStringList files;
    QStringList errors;
    volatile bool canceled;
};

VerifyChecksumsController::Private::Private( VerifyChecksumsController * qq )
    : q( qq ),
      dialog(),
      mutex(),
      checksumDefinitions( ChecksumDefinition::getChecksumDefinitions() ),
      files(),
      errors(),
      canceled( false )
{
    connect( this, SIGNAL(progress(int,int,QString)),
             q, SIGNAL(progress(int,int,QString)) );
    connect( this, SIGNAL(finished()),
             q, SLOT(slotOperationFinished()) );
}

VerifyChecksumsController::Private::~Private() { qDebug(); }

VerifyChecksumsController::VerifyChecksumsController( QObject * p )
    : Controller( p ), d( new Private( this ) )
{

}

VerifyChecksumsController::VerifyChecksumsController( const shared_ptr<const ExecutionContext> & ctx, QObject * p )
    : Controller( ctx, p ), d( new Private( this ) )
{

}

VerifyChecksumsController::~VerifyChecksumsController() {
    qDebug();
}

void VerifyChecksumsController::setFiles( const QStringList & files ) {
    kleo_assert( !d->isRunning() );
    kleo_assert( !files.empty() );
    const QMutexLocker locker( &d->mutex );
    d->files = files;
}


void VerifyChecksumsController::start() {

    {
        const QMutexLocker locker( &d->mutex );

        d->dialog = new VerifyChecksumsDialog;
        d->dialog->setAttribute( Qt::WA_DeleteOnClose );
        d->dialog->setWindowTitle( i18nc("@title:window","Verify Checksum Results") );

        connect( d->dialog, SIGNAL(canceled()),
                 this, SLOT(cancel()) );
        connect( d.get(), SIGNAL(baseDirectories(QStringList)),
                 d->dialog, SLOT(setBaseDirectories(QStringList)) );
        connect( d.get(), SIGNAL(progress(int,int,QString)),
                 d->dialog, SLOT(setProgress(int,int)) );
        connect( d.get(), SIGNAL(status(QString,Kleo::Crypto::Gui::VerifyChecksumsDialog::Status)),
                 d->dialog, SLOT(setStatus(QString,Kleo::Crypto::Gui::VerifyChecksumsDialog::Status)) );

        d->canceled = false;
        d->errors.clear();
    }

    d->start();

    d->dialog->show();

}

void VerifyChecksumsController::cancel() {
    qDebug();
    const QMutexLocker locker( &d->mutex );
    d->canceled = true;
}

namespace {

    struct SumFile {
        QDir dir;
        QString sumFile;
        quint64 totalSize;
        shared_ptr<ChecksumDefinition> checksumDefinition;
    };

}

static QStringList filter_checksum_files( QStringList l, const QList<QRegExp> & rxs ) {
    l.erase( std::remove_if( l.begin(), l.end(),
                             matches_none_of( rxs ) ),
             l.end() );
    return l;
}


namespace {
    struct File {
        QString name;
        QByteArray checksum;
        bool binary;
    };
}

static QString decode( const QString & encoded ) {
    QString decoded;
    decoded.reserve( encoded.size() );
    bool shift = false;
    Q_FOREACH( const QChar ch, encoded )
        if ( shift ) {
            switch ( ch.toLatin1() ) {
            case '\\': decoded += QLatin1Char( '\\' ); break;
            case 'n':  decoded += QLatin1Char( '\n' ); break;
            default:
                qDebug() << "invalid escape sequence" << '\\' << ch << "(interpreted as '" << ch << "')";
                decoded += ch;
                break;
            }
            shift = false;
        } else {
            if ( ch == QLatin1Char( '\\' ) )
                shift = true;
            else
                decoded += ch;
        }
    return decoded;
}

static std::vector<File> parse_sum_file( const QString & fileName ) {
    std::vector<File> files;
    QFile f( fileName );
    if ( f.open( QIODevice::ReadOnly ) ) {
        QTextStream s( &f );
        QRegExp rx( QLatin1String("(\\?)([a-f0-9A-F]+) ([ *])([^\n]+)\n*") );
        while ( !s.atEnd() ) {
            const QString line = s.readLine();
            if ( rx.exactMatch( line ) ) {
                assert( !rx.cap(4).endsWith( QLatin1Char('\n') ) );
                const File file = {
                    rx.cap( 1 ) == QLatin1String("\\") ? decode( rx.cap( 4 ) ) : rx.cap( 4 ),
                    rx.cap( 2 ).toLatin1(),
                    rx.cap( 3 ) == QLatin1String("*"),
                };
                files.push_back( file );
            }
        }
    }
    return files;
}

static quint64 aggregate_size( const QDir & dir, const QStringList & files ) {
    quint64 n = 0;
    Q_FOREACH( const QString & file, files )
        n += QFileInfo( dir.absoluteFilePath( file ) ).size();
    return n;
}

static shared_ptr<ChecksumDefinition> filename2definition( const QString & fileName,
                                                           const std::vector< shared_ptr<ChecksumDefinition> > & checksumDefinitions )
{
    Q_FOREACH( const shared_ptr<ChecksumDefinition> & cd, checksumDefinitions )
        if ( cd )
            Q_FOREACH( const QString & pattern, cd->patterns() )
                if ( QRegExp( pattern, fs_cs ).exactMatch( fileName ) )
                    return cd;
    return shared_ptr<ChecksumDefinition>();
}

namespace {
    struct less_dir : std::binary_function<QDir,QDir,bool> {
        bool operator()( const QDir & lhs, const QDir & rhs ) const {
            return QString::compare( lhs.absolutePath(), rhs.absolutePath(), fs_cs ) < 0 ;
        }
    };
    struct less_file : std::binary_function<QString,QString,bool> {
        bool operator()( const QString & lhs, const QString & rhs ) const {
            return QString::compare( lhs, rhs, fs_cs ) < 0 ;
        }
    };
    struct sumfile_contains_file : std::unary_function<QString,bool> {
        const QDir dir;
        const QString fileName;
        sumfile_contains_file( const QDir & dir_, const QString & fileName_ )
            : dir( dir_ ), fileName( fileName_ ) {}
        bool operator()( const QString & sumFile ) const {
            const std::vector<File> files = parse_sum_file( dir.absoluteFilePath( sumFile ) );
            qDebug() << "find_sums_by_input_files:      found " << files.size()
                     << " files listed in " << qPrintable( dir.absoluteFilePath( sumFile ) );
            Q_FOREACH( const File & file, files ) {
                const bool isSameFileName = ( QString::compare( file.name, fileName, fs_cs ) == 0 );
                qDebug() << "find_sums_by_input_files:        "
                         << qPrintable( file.name ) << " == "
                         << qPrintable( fileName )  << " ? "
                         << isSameFileName;
                if ( isSameFileName )
                    return true;
            }
            return false;
        }
    };

}

// IF is_dir(file)
//   add all sumfiles \in dir(file)
//   inputs.prepend( all dirs \in dir(file) )
// ELSE IF is_sum_file(file)
//   add
// ELSE IF \exists sumfile in dir(file) \where sumfile \contains file
//   add sumfile
// ELSE
//   error: no checksum found for "file"

static QStringList find_base_directiories( const QStringList & files ) {

    // Step 1: find base dirs:

    std::set<QDir,less_dir> dirs;
    Q_FOREACH( const QString & file, files ) {
        const QFileInfo fi( file );
        const QDir dir = fi.isDir() ? QDir( file ) : fi.dir() ;
        dirs.insert( dir );
    }

    // Step 1a: collapse direct child directories

    bool changed;
    do {
        changed = false;
        std::set<QDir,less_dir>::iterator it = dirs.begin();
        while ( it != dirs.end() ) {
            QDir dir = *it;
            if ( dir.cdUp() && dirs.count( dir ) ) {
                dirs.erase( it++ );
                changed = true;
            } else {
                ++it;
            }
        }
    } while ( changed );

    return kdtools::transform<QStringList>( dirs, mem_fn( &QDir::absolutePath ) );
}

static std::vector<SumFile> find_sums_by_input_files( const QStringList & files, QStringList & errors,
                                                      const function<void(int)> & progress,
                                                      const std::vector< shared_ptr<ChecksumDefinition> > & checksumDefinitions )
{
    const QList<QRegExp> patterns = get_patterns( checksumDefinitions );

    const matches_any is_sum_file( patterns );

    std::map<QDir,std::set<QString,less_file>,less_dir> dirs2sums;

    // Step 1: find the sumfiles we need to check:

    std::deque<QString> inputs( files.begin(), files.end() );

    int i = 0;
    while ( !inputs.empty() ) {
        const QString file = inputs.front();
        qDebug() << "find_sums_by_input_files: considering " << qPrintable( file );
        inputs.pop_front();
        const QFileInfo fi( file );
        const QString fileName = fi.fileName();
        if ( fi.isDir() ) {
            qDebug() << "find_sums_by_input_files:   it's a directory";
            QDir dir( file );
            const QStringList sumfiles = filter_checksum_files( dir.entryList( QDir::Files ), patterns );
            qDebug() << "find_sums_by_input_files:   found " << sumfiles.size()
                     << " sum files: " << qPrintable( sumfiles.join(QLatin1String(", ")) );
            dirs2sums[ dir ].insert( sumfiles.begin(), sumfiles.end() );
            const QStringList dirs = dir.entryList( QDir::Dirs|QDir::NoDotAndDotDot );
            qDebug() << "find_sums_by_input_files:   found " << dirs.size()
                     << " subdirs, prepending";
            kdtools::transform( dirs,
                                std::inserter( inputs, inputs.begin() ),
                                boost::bind( &QDir::absoluteFilePath, cref(dir), _1 ) );
        } else if ( is_sum_file( fileName ) ) {
            qDebug() << "find_sums_by_input_files:   it's a sum file";
            dirs2sums[fi.dir()].insert( fileName );
        } else {
            qDebug() << "find_sums_by_input_files:   it's something else; checking whether we'll find a sumfile for it...";
            const QDir dir = fi.dir();
            const QStringList sumfiles = filter_checksum_files( dir.entryList( QDir::Files ), patterns );
            qDebug() << "find_sums_by_input_files:   found " << sumfiles.size()
                     << " potential sumfiles: " << qPrintable( sumfiles.join(QLatin1String(", ")) );
            const QStringList::const_iterator it = kdtools::find_if( sumfiles, sumfile_contains_file( dir, fileName ) );
            if ( it == sumfiles.end() )
                errors.push_back( i18n( "Cannot find checksums file for file %1", file ) );
            else
                dirs2sums[dir].insert( *it );
        }
        if ( !progress.empty() )
            progress( ++i );
    }

    // Step 2: convert into vector<SumFile>:

    std::vector<SumFile> sumfiles;
    sumfiles.reserve( dirs2sums.size() );

    for ( std::map<QDir,std::set<QString,less_file>,less_dir>::const_iterator it = dirs2sums.begin(), end = dirs2sums.end() ; it != end ; ++it ) {

        if ( it->second.empty() )
            continue;

        const QDir & dir = it->first;

        Q_FOREACH( const QString & sumFileName, it->second ) {

            const std::vector<File> summedfiles = parse_sum_file( dir.absoluteFilePath( sumFileName ) );

            const SumFile sumFile = {
                it->first,
                sumFileName,
                aggregate_size( it->first, kdtools::transform<QStringList>( summedfiles, mem_fn( &File::name ) ) ),
                filename2definition( sumFileName, checksumDefinitions ),
            };
            sumfiles.push_back( sumFile );

        }

        if ( !progress.empty() )
            progress( ++i );

    }
    return sumfiles;
}

static QStringList c_lang_environment() {
    QStringList env = QProcess::systemEnvironment();
    env.erase( std::remove_if( env.begin(), env.end(),
                               boost::bind( &QRegExp::exactMatch,
                                     QRegExp( QLatin1String("^LANG=.*"), fs_cs ), _1 ) ),
               env.end() );
    env.push_back( QLatin1String("LANG=C") );
    return env;
}

static const struct {
    const char * string;
    VerifyChecksumsDialog::Status status;
} statusStrings[] = {
    { "OK",     VerifyChecksumsDialog::OK     },
    { "FAILED", VerifyChecksumsDialog::Failed },
};
static const size_t numStatusStrings = sizeof statusStrings / sizeof *statusStrings ;

static VerifyChecksumsDialog::Status string2status( const QByteArray & str ) {
    for ( unsigned int i = 0 ; i < numStatusStrings ; ++i )
        if ( str == statusStrings[i].string )
            return statusStrings[i].status;
    return VerifyChecksumsDialog::Unknown;
}

static QString process( const SumFile & sumFile, bool * fatal, const QStringList & env,
                        const function<void(const QString&,VerifyChecksumsDialog::Status)> & status )
{
    QProcess p;
    p.setEnvironment( env );
    p.setWorkingDirectory( sumFile.dir.absolutePath() );
    p.setReadChannel( QProcess::StandardOutput );

    const QString absFilePath = sumFile.dir.absoluteFilePath( sumFile.sumFile );

    const QString program = sumFile.checksumDefinition->verifyCommand();
    sumFile.checksumDefinition->startVerifyCommand( &p, QStringList( absFilePath ) );

    QByteArray remainder; // used for filenames with newlines in them
    while ( p.state() != QProcess::NotRunning ) {
        p.waitForReadyRead();
        while ( p.canReadLine() ) {
            const QByteArray line = p.readLine();
            const int colonIdx = line.lastIndexOf( ':' );
            if ( colonIdx < 0 ) {
                remainder += line; // no colon -> probably filename with a newline
                continue;
            }
            const QString file = QFile::decodeName( remainder + line.left( colonIdx ) );
            remainder.clear();
            const VerifyChecksumsDialog::Status result = string2status( line.mid( colonIdx+1 ).trimmed() );
            status( sumFile.dir.absoluteFilePath( file ), result );
        }
    }
    qDebug() << "[" << &p << "] Exit code " << p.exitCode();

    if ( p.exitStatus() != QProcess::NormalExit || p.exitCode() != 0 ) {
        if ( fatal && p.error() == QProcess::FailedToStart )
            *fatal = true;
        if ( p.error() == QProcess::UnknownError )
            return i18n( "Error while running %1: %2", program,
                         QString::fromLocal8Bit( p.readAllStandardError().trimmed().constData() ) );
        else
            return i18n( "Failed to execute %1: %2", program, p.errorString() );
    }

    return QString();
}

namespace {
    static QDebug operator<<( QDebug s, const SumFile & sum ) {
        return s << "SumFile(" << sum.dir << "->" << sum.sumFile << "<-(" << sum.totalSize << ')' << ")\n";
    }
}

void VerifyChecksumsController::Private::run() {

    QMutexLocker locker( &mutex );

    const QStringList files = this->files;
    const std::vector< shared_ptr<ChecksumDefinition> > checksumDefinitions = this->checksumDefinitions;

    locker.unlock();

    QStringList errors;

    //
    // Step 0: find base directories:
    //

    emit baseDirectories( find_base_directiories( files ) );

    //
    // Step 1: build a list of work to do (no progress):
    //

    const QString scanning = i18n("Scanning directories...");
    emit progress( 0, 0, scanning );

    const function<void(int)> progressCb = boost::bind( &Private::progress, this, _1, 0, scanning );
    const function<void(const QString&,VerifyChecksumsDialog::Status)>
        statusCb = boost::bind( &Private::status, this, _1, _2 );
    const std::vector<SumFile> sumfiles = find_sums_by_input_files( files, errors, progressCb, checksumDefinitions );

    Q_FOREACH( const SumFile & sumfile, sumfiles )
        qDebug() << sumfile;

    if ( !canceled ) {

        emit progress( 0, 0, i18n("Calculating total size...") );

        const quint64 total
            = kdtools::accumulate_transform( sumfiles, mem_fn( &SumFile::totalSize ), Q_UINT64_C(0) );

        if ( !canceled ) {

    //
    // Step 2: perform work (with progress reporting):
    //

            const QStringList env = c_lang_environment();

            // re-scale 'total' to fit into ints (wish QProgressDialog would use quint64...)
            const quint64 factor = total / std::numeric_limits<int>::max() + 1 ;

            quint64 done = 0;
            Q_FOREACH( const SumFile & sumFile, sumfiles ) {
                emit progress( done/factor, total/factor,
                               i18n("Verifying checksums (%2) in %1", sumFile.checksumDefinition->label(), sumFile.dir.path() ) );
                bool fatal = false;
                const QString error = process( sumFile, &fatal, env, statusCb );
                if ( !error.isEmpty() )
                    errors.push_back( error );
                done += sumFile.totalSize;
                if ( fatal || canceled )
                    break;
            }
            emit progress( done/factor, total/factor, i18n("Done.") );

        }
    }

    locker.relock();

    this->errors = errors;

    // mutex unlocked by QMutexLocker

}

#include "moc_verifychecksumscontroller.cpp"
#include "verifychecksumscontroller.moc"

#endif // QT_NO_DIRMODEL
