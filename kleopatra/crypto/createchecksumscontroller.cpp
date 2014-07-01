/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/createchecksumscontroller.cpp

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

#include "createchecksumscontroller.h"

#include <utils/input.h>
#include <utils/output.h>
#include <utils/classify.h>
#include <utils/kleo_assert.h>

#include <kleo/stl_util.h>
#include <kleo/checksumdefinition.h>

#include <KLocalizedString>
#include <qdebug.h>
#include <QSaveFile>
#include <KConfigGroup>
#include <KSharedConfig>

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>

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
#include <map>
#include <limits>

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace boost;

namespace {

    class ResultDialog : public QDialog {
        Q_OBJECT
    public:
        ResultDialog( const QStringList & created, const QStringList & errors, QWidget * parent=0, Qt::WindowFlags f=0 )
            : QDialog( parent, f ),
              createdLB( created.empty()
                         ? i18nc("@info","No checksum files have been created.")
                         : i18nc("@info","These checksum files have been successfully created:"), this ),
              createdLW( this ),
              errorsLB( errors.empty()
                        ? i18nc("@info","There were no errors.")
                        : i18nc("@info","The following errors were encountered:"), this ),
              errorsLW( this ),
              buttonBox( QDialogButtonBox::Ok, Qt::Horizontal, this ),
              vlay( this )
        {
            KDAB_SET_OBJECT_NAME( createdLB );
            KDAB_SET_OBJECT_NAME( createdLW );
            KDAB_SET_OBJECT_NAME( errorsLB );
            KDAB_SET_OBJECT_NAME( errorsLW );
            KDAB_SET_OBJECT_NAME( buttonBox );
            KDAB_SET_OBJECT_NAME( vlay );

            createdLW.addItems( created );
            QRect r;
            for( int i = 0; i < created.size(); ++i )
                r = r.united( createdLW.visualRect( createdLW.model()->index( 0, i ) ) );
            createdLW.setMinimumWidth( qMin( 1024, r.width() + 4 * createdLW.frameWidth() ) );

            errorsLW.addItems( errors );

            vlay.addWidget( &createdLB );
            vlay.addWidget( &createdLW, 1 );
            vlay.addWidget( &errorsLB );
            vlay.addWidget( &errorsLW, 1 );
            vlay.addWidget( &buttonBox );

            if ( created.empty() )
                createdLW.hide();
            if ( errors.empty() )
                errorsLW.hide();

            connect( &buttonBox, SIGNAL(accepted()), this, SLOT(accept()) );
            connect( &buttonBox, SIGNAL(rejected()), this, SLOT(reject()) );
            readConfig();
        }
        ~ResultDialog()
        {
            writeConfig();
        }

        void readConfig()
        {
            KConfigGroup dialog( KSharedConfig::openConfig(), "ResultDialog" );
            const QSize size = dialog.readEntry( "Size", QSize(600, 400) );
            if ( size.isValid() ) {
                resize( size );
            }
        }
        void writeConfig()
        {
            KConfigGroup dialog( KSharedConfig::openConfig(), "ResultDialog" );
            dialog.writeEntry( "Size",size() );
            dialog.sync();
        }

    private:
        QLabel createdLB;
        QListWidget createdLW;
        QLabel errorsLB;
        QListWidget errorsLW;
        QDialogButtonBox buttonBox;
        QVBoxLayout vlay;
    };

}

#ifdef Q_OS_UNIX
static const bool HAVE_UNIX = true;
#else
static const bool HAVE_UNIX = false;
#endif

static const Qt::CaseSensitivity fs_cs = HAVE_UNIX ? Qt::CaseSensitive : Qt::CaseInsensitive ; // can we use QAbstractFileEngine::caseSensitive()?

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
}

class CreateChecksumsController::Private : public QThread {
    Q_OBJECT
    friend class ::Kleo::Crypto::CreateChecksumsController;
    CreateChecksumsController * const q;
public:
    explicit Private( CreateChecksumsController * qq );
    ~Private();

Q_SIGNALS:
    void progress( int, int, const QString & );

private:
    void slotOperationFinished() {
#ifndef QT_NO_PROGRESSDIALOG
        if ( progressDialog ) {
            progressDialog->setValue( progressDialog->maximum() );
            progressDialog->close();
        }
#endif // QT_NO_PROGRESSDIALOG
        ResultDialog * const dlg = new ResultDialog( created, errors );
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        q->bringToForeground( dlg );
        if ( !errors.empty() )
            q->setLastError( gpg_error( GPG_ERR_GENERAL ),
                             errors.join( QLatin1String("\n") ) );
        q->emitDoneOrError();
    }
    void slotProgress( int current, int total, const QString & what ) {
        qDebug() << "progress: " << current << "/" << total << ": " << qPrintable( what );
#ifndef QT_NO_PROGRESSDIALOG
        if ( !progressDialog )
            return;
        progressDialog->setMaximum( total );
        progressDialog->setValue( current );
        progressDialog->setLabelText( what );
#endif // QT_NO_PROGRESSDIALOG
    }

private:
    /* reimp */ void run();

private:
#ifndef QT_NO_PROGRESSDIALOG
    QPointer<QProgressDialog> progressDialog;
#endif
    mutable QMutex mutex;
    const std::vector< shared_ptr<ChecksumDefinition> > checksumDefinitions;
    shared_ptr<ChecksumDefinition> checksumDefinition;
    QStringList files;
    QStringList errors, created;
    bool allowAddition;
    volatile bool canceled;
};

CreateChecksumsController::Private::Private( CreateChecksumsController * qq )
    : q( qq ),
#ifndef QT_NO_PROGRESSDIALOG
      progressDialog(),
#endif
      mutex(),
      checksumDefinitions( ChecksumDefinition::getChecksumDefinitions() ),
      checksumDefinition( ChecksumDefinition::getDefaultChecksumDefinition( checksumDefinitions ) ),
      files(),
      errors(),
      created(),
      allowAddition( false ),
      canceled( false )
{
    connect( this, SIGNAL(progress(int,int,QString)),
             q, SLOT(slotProgress(int,int,QString)) );
    connect( this, SIGNAL(progress(int,int,QString)),
             q, SIGNAL(progress(int,int,QString)) );
    connect( this, SIGNAL(finished()),
             q, SLOT(slotOperationFinished()) );
}

CreateChecksumsController::Private::~Private() { qDebug(); }

CreateChecksumsController::CreateChecksumsController( QObject * p )
    : Controller( p ), d( new Private( this ) )
{

}

CreateChecksumsController::CreateChecksumsController( const shared_ptr<const ExecutionContext> & ctx, QObject * p )
    : Controller( ctx, p ), d( new Private( this ) )
{

}

CreateChecksumsController::~CreateChecksumsController() {
    qDebug();
}

void CreateChecksumsController::setFiles( const QStringList & files ) {
    kleo_assert( !d->isRunning() );
    kleo_assert( !files.empty() );
    const QList<QRegExp> patterns = get_patterns( d->checksumDefinitions );
    if ( !kdtools::all( files, matches_any( patterns ) ) &&
         !kdtools::none_of( files, matches_any( patterns ) ) )
        throw Exception( gpg_error( GPG_ERR_INV_ARG ), i18n("Create Checksums: input files must be either all checksum files or all files to be checksummed, not a mixture of both.") );
    const QMutexLocker locker( &d->mutex );
    d->files = files;
}

void CreateChecksumsController::setAllowAddition( bool allow ) {
    kleo_assert( !d->isRunning() );
    const QMutexLocker locker( &d->mutex );
    d->allowAddition = allow;
}

bool CreateChecksumsController::allowAddition() const {
    const QMutexLocker locker( &d->mutex );
    return d->allowAddition;
}

void CreateChecksumsController::start() {

    {
        const QMutexLocker locker( &d->mutex );

#ifndef QT_NO_PROGRESSDIALOG
        d->progressDialog = new QProgressDialog( i18n("Initializing..."), i18n("Cancel"), 0, 0 );
        applyWindowID( d->progressDialog );
        d->progressDialog->setAttribute( Qt::WA_DeleteOnClose );
        d->progressDialog->setMinimumDuration( 1000 );
        d->progressDialog->setWindowTitle( i18nc("@title:window","Create Checksum Progress") );
        connect( d->progressDialog, SIGNAL(canceled()), this, SLOT(cancel()) );
#endif // QT_NO_PROGRESSDIALOG

        d->canceled = false;
        d->errors.clear();
        d->created.clear();
    }

    d->start();

}

void CreateChecksumsController::cancel() {
    qDebug();
    const QMutexLocker locker( &d->mutex );
    d->canceled = true;
}

namespace {

    struct Dir {
        QDir dir;
        QString sumFile;
        QStringList inputFiles;
        quint64 totalSize;
        shared_ptr<ChecksumDefinition> checksumDefinition;
    };

}

static QStringList remove_checksum_files( QStringList l, const QList<QRegExp> & rxs ) {
    QStringList::iterator end = l.end();
    Q_FOREACH( const QRegExp & rx, rxs )
        end = std::remove_if( l.begin(), end,
                              boost::bind( &QRegExp::exactMatch, rx, _1 ) );
    l.erase( end, l.end() );
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

static std::vector<Dir> find_dirs_by_sum_files( const QStringList & files, bool allowAddition,
                                                const function<void(int)> & progress,
                                                const std::vector< shared_ptr<ChecksumDefinition> > & checksumDefinitions )
{

    const QList<QRegExp> patterns = get_patterns( checksumDefinitions );

    std::vector<Dir> dirs;
    dirs.reserve( files.size() );

    int i = 0;

    Q_FOREACH( const QString & file, files ) {

        const QFileInfo fi( file );
        const QDir dir = fi.dir();
        const QStringList entries = remove_checksum_files( dir.entryList( QDir::Files ), patterns );

        QStringList inputFiles;
        if ( allowAddition ) {
            inputFiles = entries;
        } else {
            const std::vector<File> parsed = parse_sum_file( fi.absoluteFilePath() );
            const QStringList oldInputFiles =
                kdtools::transform<QStringList>( parsed, mem_fn( &File::name ) );
            inputFiles = fs_intersect( oldInputFiles, entries );
        }

        const Dir item = {
            dir,
            fi.fileName(),
            inputFiles,
            aggregate_size( dir, inputFiles ),
            filename2definition( fi.fileName(), checksumDefinitions )
        };

        dirs.push_back( item );

        if ( !progress.empty() )
            progress( ++i );

    }
    return dirs;
}

namespace {
    struct less_dir : std::binary_function<QDir,QDir,bool> {
        bool operator()( const QDir & lhs, const QDir & rhs ) const {
            return QString::compare( lhs.absolutePath(), rhs.absolutePath(), fs_cs ) < 0 ;
        }
    };
}

static std::vector<Dir> find_dirs_by_input_files( const QStringList & files, const shared_ptr<ChecksumDefinition> & checksumDefinition, bool allowAddition,
                                                  const function<void(int)> & progress,
                                                  const std::vector< shared_ptr<ChecksumDefinition> > & checksumDefinitions )
{
    Q_UNUSED( allowAddition );
    if ( !checksumDefinition )
        return std::vector<Dir>();

    const QList<QRegExp> patterns = get_patterns( checksumDefinitions );

    std::map<QDir,QStringList,less_dir> dirs2files;

    // Step 1: sort files by the dir they're contained in:

    std::deque<QString> inputs( files.begin(), files.end() );

    int i = 0;
    while ( !inputs.empty() ) {
        const QString file = inputs.front();
        inputs.pop_front();
        const QFileInfo fi( file );
        if ( fi.isDir() ) {
            QDir dir( file );
            dirs2files[ dir ] = remove_checksum_files( dir.entryList( QDir::Files ), patterns );
            kdtools::transform( dir.entryList( QDir::Dirs|QDir::NoDotAndDotDot ),
                                std::inserter( inputs, inputs.begin() ),
                                boost::bind( &QDir::absoluteFilePath, cref(dir), _1 ) );
        } else {
            dirs2files[fi.dir()].push_back( file );
        }
        if ( !progress.empty() )
            progress( ++i );
    }

    // Step 2: convert into vector<Dir>:

    std::vector<Dir> dirs;
    dirs.reserve( dirs2files.size() );

    for ( std::map<QDir,QStringList,less_dir>::const_iterator it = dirs2files.begin(), end = dirs2files.end() ; it != end ; ++it ) {

        const QStringList inputFiles = remove_checksum_files( it->second, patterns );
        if ( inputFiles.empty() )
            continue;

        const Dir dir = {
            it->first,
            checksumDefinition->outputFileName(),
            inputFiles,
            aggregate_size( it->first, inputFiles ),
            checksumDefinition
        };
        dirs.push_back( dir );

        if ( !progress.empty() )
            progress( ++i );

    }
    return dirs;
}

static QString process( const Dir & dir, bool * fatal ) {
    const QString absFilePath = dir.dir.absoluteFilePath( dir.sumFile );
    QSaveFile file( absFilePath );
    if ( !file.open(QIODevice::ReadWrite) )
        return i18n( "Failed to open file \"%1\" for reading and writing: %2",
                     dir.dir.absoluteFilePath( file.fileName() ),
                     file.errorString() );
    QProcess p;
    p.setWorkingDirectory( dir.dir.absolutePath() );
    p.setStandardOutputFile( dir.dir.absoluteFilePath( file.fileName() /*!sic*/ ) );
    const QString program = dir.checksumDefinition->createCommand();
    dir.checksumDefinition->startCreateCommand( &p, dir.inputFiles );
    p.waitForFinished();
    qDebug() << "[" << &p << "] Exit code " << p.exitCode();

    if ( p.exitStatus() != QProcess::NormalExit || p.exitCode() != 0 ) {
        file.cancelWriting();
        if ( fatal && p.error() == QProcess::FailedToStart )
            *fatal = true;
        if ( p.error() == QProcess::UnknownError )
            return i18n( "Error while running %1: %2", program,
                         QString::fromLocal8Bit( p.readAllStandardError().trimmed().constData() ) );
        else
            return i18n( "Failed to execute %1: %2", program, p.errorString() );
    }

    if ( !file.commit() )
        return i18n( "Failed to move file %1 to its final destination, %2: %3",
                     file.fileName(), dir.sumFile, file.errorString() );

    return QString();
}

namespace {
    static QDebug operator<<( QDebug s, const Dir & dir ) {
        return s << "Dir(" << dir.dir << "->" << dir.sumFile << "<-(" << dir.totalSize << ')' << dir.inputFiles << ")\n";
    }
}

void CreateChecksumsController::Private::run() {

    QMutexLocker locker( &mutex );

    const QStringList files = this->files;
    const std::vector< shared_ptr<ChecksumDefinition> > checksumDefinitions = this->checksumDefinitions;
    const shared_ptr<ChecksumDefinition> checksumDefinition = this->checksumDefinition;
    const bool allowAddition = this->allowAddition;

    locker.unlock();

    QStringList errors;
    QStringList created;

    if ( !checksumDefinition ) {
        errors.push_back( i18n("No checksum programs defined.") );
        locker.relock();
        this->errors = errors;
        return;
    } else {
        qDebug() << "using checksum-definition" << checksumDefinition->id();
    }

    //
    // Step 1: build a list of work to do (no progress):
    //

    const QString scanning = i18n("Scanning directories...");
    emit progress( 0, 0, scanning );

    const bool haveSumFiles
        = kdtools::all( files, matches_any( get_patterns( checksumDefinitions ) ) );
    const function<void(int)> progressCb = boost::bind( &Private::progress, this, _1, 0, scanning );
    const std::vector<Dir> dirs = haveSumFiles
        ? find_dirs_by_sum_files( files, allowAddition, progressCb, checksumDefinitions )
        : find_dirs_by_input_files( files, checksumDefinition, allowAddition, progressCb, checksumDefinitions ) ;

    Q_FOREACH( const Dir & dir, dirs )
        qDebug() << dir;

    if ( !canceled ) {

        emit progress( 0, 0, i18n("Calculating total size...") );

        const quint64 total
            = kdtools::accumulate_transform( dirs, mem_fn( &Dir::totalSize ), Q_UINT64_C(0) );

        if ( !canceled ) {

    //
    // Step 2: perform work (with progress reporting):
    //

            // re-scale 'total' to fit into ints (wish QProgressDialog would use quint64...)
            const quint64 factor = total / std::numeric_limits<int>::max() + 1 ;

            quint64 done = 0;
            Q_FOREACH( const Dir & dir, dirs ) {
                emit progress( done/factor, total/factor,
                               i18n("Checksumming (%2) in %1", dir.checksumDefinition->label(), dir.dir.path() ) );
                bool fatal = false;
                const QString error = process( dir, &fatal );
                if ( !error.isEmpty() )
                    errors.push_back( error );
                else
                    created.push_back( dir.dir.absoluteFilePath( dir.sumFile ) );
                done += dir.totalSize;
                if ( fatal || canceled )
                    break;
            }
            emit progress( done/factor, total/factor, i18n("Done.") );

        }
    }

    locker.relock();

    this->errors = errors;
    this->created = created;

    // mutex unlocked by QMutexLocker

}

#include "moc_createchecksumscontroller.cpp"
#include "createchecksumscontroller.moc"
