/* -*- mode: c++; c-basic-offset:4 -*-
    checksumdefinition.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2010 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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

#include "checksumdefinition.h"

#include "exception.h"
#include "cryptobackendfactory.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KConfig>
#include <KShell>
#include <KStandardDirs>

#include <QString>
#include <QStringList>
#include <QDebug>
#include <QFileInfo>
#include <QProcess>
#include <QByteArray>
#include <QMutex>
#include <QCoreApplication>

#include <boost/shared_ptr.hpp>
#include <KSharedConfig>

#ifdef stdin
# undef stdin // pah..
#endif

using namespace Kleo;
using namespace boost;

static QMutex installPathMutex;
Q_GLOBAL_STATIC( QString, _installPath )
QString ChecksumDefinition::installPath() {
    const QMutexLocker locker( &installPathMutex );
    QString * const ip = _installPath();
    if ( ip->isEmpty() ) {
        if ( QCoreApplication::instance() ) {
            *ip = QCoreApplication::applicationDirPath();
        } else {
            qWarning( "checksumdefinition.cpp: installPath() called before QCoreApplication was constructed" );
        }
    }
    return *ip;
}
void ChecksumDefinition::setInstallPath( const QString & ip ) {
    const QMutexLocker locker( &installPathMutex );
    *_installPath() =ip;
}
    

// Checksum Definition #N groups
static const QLatin1String ID_ENTRY( "id" );
static const QLatin1String NAME_ENTRY( "Name" );
static const QLatin1String CREATE_COMMAND_ENTRY( "create-command" );
static const QLatin1String VERIFY_COMMAND_ENTRY( "verify-command" );
static const QLatin1String FILE_PATTERNS_ENTRY( "file-patterns" );
static const QLatin1String OUTPUT_FILE_ENTRY( "output-file" );
static const QLatin1String FILE_PLACEHOLDER( "%f" );
static const QLatin1String INSTALLPATH_PLACEHOLDER( "%I" );
static const QLatin1String NULL_SEPARATED_STDIN_INDICATOR( "0|" );
static const QLatin1Char   NEWLINE_SEPARATED_STDIN_INDICATOR( '|' );

// ChecksumOperations group
static const QLatin1String CHECKSUM_DEFINITION_ID_ENTRY( "checksum-definition-id" );


namespace {

    class ChecksumDefinitionError : public Kleo::Exception {
        const QString m_id;
    public:
        ChecksumDefinitionError( const QString & id, const QString & message )
            : Kleo::Exception( GPG_ERR_INV_PARAMETER, i18n("Error in checksum definition %1: %2", id, message ), MessageOnly ),
              m_id( id )
        {

        }
        ~ChecksumDefinitionError() throw() {}

        const QString & checksumDefinitionId() const { return m_id; }
    };

}

static QString try_extensions( const QString & path ) {
    static const char exts[][4] = {
        "", "exe", "bat", "bin", "cmd",
    };
    static const size_t numExts = sizeof exts / sizeof *exts ;
    for ( unsigned int i = 0 ; i < numExts ; ++i ) {
        const QFileInfo fi( path + QLatin1Char('.') + QLatin1String( exts[i] ) );
        if ( fi.exists() )
            return fi.filePath();
    }
    return QString();
}

static void parse_command( QString cmdline, const QString & id, const QString & whichCommand,
                           QString * command, QStringList * prefix, QStringList * suffix, ChecksumDefinition::ArgumentPassingMethod * method )
{
    assert( prefix );
    assert( suffix );
    assert( method );

    KShell::Errors errors;
    QStringList l;

    if ( cmdline.startsWith( NULL_SEPARATED_STDIN_INDICATOR ) ) {
        *method = ChecksumDefinition::NullSeparatedInputFile;
        cmdline.remove( 0, 2 );
    } else if ( cmdline.startsWith( NEWLINE_SEPARATED_STDIN_INDICATOR ) ) {
        *method = ChecksumDefinition::NewlineSeparatedInputFile;
        cmdline.remove( 0, 1 );
    } else {
        *method = ChecksumDefinition::CommandLine;
    }
    if ( *method != ChecksumDefinition::CommandLine && cmdline.contains( FILE_PLACEHOLDER ) )
        throw ChecksumDefinitionError( id, i18n("Cannot use both %f and | in '%1'", whichCommand) );
    cmdline.replace( FILE_PLACEHOLDER,        QLatin1String("__files_go_here__")  )
           .replace( INSTALLPATH_PLACEHOLDER, QLatin1String("__path_goes_here__") );
    l = KShell::splitArgs( cmdline, KShell::AbortOnMeta|KShell::TildeExpand, &errors );
    l = l.replaceInStrings( QLatin1String("__files_go_here__"), FILE_PLACEHOLDER );
    if ( l.indexOf( QRegExp( QLatin1String(".*__path_goes_here__.*") ) ) >= 0 )
        l = l.replaceInStrings( QLatin1String("__path_goes_here__"), ChecksumDefinition::installPath() );
    if ( errors == KShell::BadQuoting )
        throw ChecksumDefinitionError( id, i18n("Quoting error in '%1' entry", whichCommand) );
    if ( errors == KShell::FoundMeta )
        throw ChecksumDefinitionError( id, i18n("'%1' too complex (would need shell)", whichCommand) );
    qDebug() << "ChecksumDefinition[" << id << ']' << l;
    if ( l.empty() )
        throw ChecksumDefinitionError( id, i18n("'%1' entry is empty/missing", whichCommand) );
    const QFileInfo fi1( l.front() );
    if ( fi1.isAbsolute() )
        *command = try_extensions( l.front() );
    else
        *command = KStandardDirs::findExe( fi1.fileName() );
    if ( command->isEmpty() )
        throw ChecksumDefinitionError( id, i18n("'%1' empty or not found", whichCommand) );
    const int idx1 = l.indexOf( FILE_PLACEHOLDER );
    if ( idx1 < 0 ) {
        // none -> append
        *prefix = l.mid( 1 );
    } else {
        *prefix = l.mid( 1, idx1-1 );
        *suffix = l.mid( idx1+1 );
    }
    switch ( *method ) {
    case ChecksumDefinition::CommandLine:
        qDebug() << "ChecksumDefinition[" << id << ']' << *command << *prefix << FILE_PLACEHOLDER << *suffix;
        break;
    case ChecksumDefinition::NewlineSeparatedInputFile:
        qDebug() << "ChecksumDefinition[" << id << ']' << "find | " << *command << *prefix;
        break;
    case ChecksumDefinition::NullSeparatedInputFile:
        qDebug() << "ChecksumDefinition[" << id << ']' << "find -print0 | " << *command << *prefix;
        break;
    case ChecksumDefinition::NumArgumentPassingMethods:
        assert( !"Should not happen" );
        break;
    }
}

namespace {

    class KConfigBasedChecksumDefinition : public ChecksumDefinition {
    public:
        explicit KConfigBasedChecksumDefinition( const KConfigGroup & group )
            : ChecksumDefinition( group.readEntryUntranslated( ID_ENTRY ),
                                  group.readEntry( NAME_ENTRY ),
                                  group.readEntry( OUTPUT_FILE_ENTRY ),
                                  group.readEntry( FILE_PATTERNS_ENTRY, QStringList() ) )
        {
            if ( id().isEmpty() )
                throw ChecksumDefinitionError( group.name(), i18n("'id' entry is empty/missing") );
            if ( outputFileName().isEmpty() )
                throw ChecksumDefinitionError( id(), i18n("'output-file' entry is empty/missing") );
            if ( patterns().empty() )
                throw ChecksumDefinitionError( id(), i18n("'file-patterns' entry is empty/missing") );

            // create-command
            ArgumentPassingMethod method;
            parse_command( group.readEntry( CREATE_COMMAND_ENTRY ), id(), CREATE_COMMAND_ENTRY,
                           &m_createCommand, &m_createPrefixArguments, &m_createPostfixArguments, &method );
            setCreateCommandArgumentPassingMethod( method );

            // verify-command
            parse_command( group.readEntry( VERIFY_COMMAND_ENTRY ), id(), VERIFY_COMMAND_ENTRY,
                           &m_verifyCommand, &m_verifyPrefixArguments, &m_verifyPostfixArguments, &method );
            setVerifyCommandArgumentPassingMethod( method );
        }

    private:
        /* reimp */ QString doGetCreateCommand() const { return m_createCommand; }
        /* reimp */ QStringList doGetCreateArguments( const QStringList & files ) const {
            return m_createPrefixArguments + files + m_createPostfixArguments;
        }
        /* reimp */ QString doGetVerifyCommand() const { return m_verifyCommand; }
        /* reimp */ QStringList doGetVerifyArguments( const QStringList & files ) const {
            return m_verifyPrefixArguments + files + m_verifyPostfixArguments;
        }

    private:
        QString m_createCommand, m_verifyCommand;
        QStringList m_createPrefixArguments, m_createPostfixArguments;
        QStringList m_verifyPrefixArguments, m_verifyPostfixArguments;
    };

}

ChecksumDefinition::ChecksumDefinition( const QString & id, const QString & label, const QString & outputFileName, const QStringList & patterns )
    : m_id( id ),
      m_label( label.isEmpty() ? id : label ),
      m_outputFileName( outputFileName ),
      m_patterns( patterns ),
      m_createMethod( CommandLine ),
      m_verifyMethod( CommandLine )
{

}

ChecksumDefinition::~ChecksumDefinition() {}

QString ChecksumDefinition::createCommand() const {
    return doGetCreateCommand();
}

QString ChecksumDefinition::verifyCommand() const {
    return doGetVerifyCommand();
}

#if 0
QStringList ChecksumDefinition::createCommandArguments( const QStringList & files ) const {
    return doGetCreateArguments( files );
}

QStringList ChecksumDefinition::verifyCommandArguments( const QStringList & files ) const {
    return doGetVerifyArguments( files );
}
#endif

static QByteArray make_input( const QStringList & files, char sep ) {
    QByteArray result;
    Q_FOREACH( const QString & file, files ) {
        result += QFile::encodeName( file );
        result += sep;
    }
    return result;
}

static bool start_command( QProcess * p, const char * functionName,
                           const QString & cmd, const QStringList & args,
                           const QStringList & files, ChecksumDefinition::ArgumentPassingMethod method )
{
    if ( !p ) {
        qWarning( "%s: process == NULL", functionName );
        return false;
    }

    switch ( method ) {

    case ChecksumDefinition::NumArgumentPassingMethods:
        assert( !"Should not happen" );

    case ChecksumDefinition::CommandLine:
        qDebug( "[%p] Starting %s %s", p, qPrintable( cmd ), qPrintable( args.join(QLatin1String(" ")) ) );
        p->start( cmd, args, QIODevice::ReadOnly );
        return true;

    case ChecksumDefinition::NewlineSeparatedInputFile:
    case ChecksumDefinition::NullSeparatedInputFile:
        p->start( cmd, args, QIODevice::ReadWrite );
        if ( !p->waitForStarted() )
            return false;
        const char sep =
            method == ChecksumDefinition::NewlineSeparatedInputFile ? '\n' :
            /* else */                            '\0' ;
        const QByteArray stdin = make_input( files, sep );
        if ( p->write( stdin ) != stdin.size() )
            return false;
        p->closeWriteChannel();
        return true;
    }

    return false; // make compiler happy

}

bool ChecksumDefinition::startCreateCommand( QProcess * p, const QStringList & files ) const {
    return start_command( p, Q_FUNC_INFO,
                          doGetCreateCommand(),
                          m_createMethod == CommandLine ? doGetCreateArguments( files ) :
                          /* else */                      doGetCreateArguments( QStringList() ),
                          files, m_createMethod );
}

bool ChecksumDefinition::startVerifyCommand( QProcess * p, const QStringList & files ) const {
    return start_command( p, Q_FUNC_INFO,
                          doGetVerifyCommand(),
                          m_verifyMethod == CommandLine ? doGetVerifyArguments( files ) :
                          /* else */                      doGetVerifyArguments( QStringList() ),
                          files, m_verifyMethod );
}

// static
std::vector< shared_ptr<ChecksumDefinition> > ChecksumDefinition::getChecksumDefinitions() {
    QStringList errors;
    return getChecksumDefinitions( errors );
}

// static
std::vector< shared_ptr<ChecksumDefinition> > ChecksumDefinition::getChecksumDefinitions( QStringList & errors ) {
    std::vector< shared_ptr<ChecksumDefinition> > result;
    if ( KConfig * config = CryptoBackendFactory::instance()->configObject() ) {
        const QStringList groups = config->groupList().filter( QRegExp(QLatin1String("^Checksum Definition #")) );
        result.reserve( groups.size() );
        Q_FOREACH( const QString & group, groups )
            try {
                const shared_ptr<ChecksumDefinition> ad( new KConfigBasedChecksumDefinition( KConfigGroup( config, group ) ) );
                result.push_back( ad );
            } catch ( const std::exception & e ) {
                qDebug() << e.what();
                errors.push_back( QString::fromLocal8Bit( e.what() ) );
            } catch ( ... ) {
                errors.push_back( i18n("Caught unknown exception in group %1", group ) );
            }
    }
    return result;
}

// static
shared_ptr<ChecksumDefinition> ChecksumDefinition::getDefaultChecksumDefinition( const std::vector< shared_ptr<ChecksumDefinition> > & checksumDefinitions ) {
    const KConfigGroup group( KSharedConfig::openConfig(), "ChecksumOperations" );
    const QString checksumDefinitionId = group.readEntry( CHECKSUM_DEFINITION_ID_ENTRY );
    if ( !checksumDefinitionId.isEmpty() )
        Q_FOREACH( const shared_ptr<ChecksumDefinition> & cd, checksumDefinitions )
            if ( cd && cd->id() == checksumDefinitionId )
                return cd;
    if ( !checksumDefinitions.empty() )
        return checksumDefinitions.front();
    else
        return shared_ptr<ChecksumDefinition>();
}

// static
void ChecksumDefinition::setDefaultChecksumDefinition( const shared_ptr<ChecksumDefinition> & checksumDefinition ) {
    if ( !checksumDefinition )
        return;
    KConfigGroup group( KSharedConfig::openConfig(), "ChecksumOperations" );
    group.writeEntry( CHECKSUM_DEFINITION_ID_ENTRY, checksumDefinition->id() );
    group.sync();
}

