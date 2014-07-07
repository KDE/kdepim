/* -*- mode: c++; c-basic-offset:4 -*-
    utils/archivedefinition.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2009, 2010 Klarälvdalens Datakonsult AB

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

#include "archivedefinition.h"

#include <utils/input.h>
#include <utils/output.h>
#include <utils/path-helper.h>
#include <utils/kleo_assert.h>

#include <kleo/exception.h>
#include <kleo/cryptobackendfactory.h>

#include <KConfigGroup>
#include <KDebug>
#include <KLocalizedString>
#include <KConfig>
#include <KShell>
#include <KStandardDirs>

#include <QString>
#include <QStringList>
#include <QDir>
#include <QMutex>
#include <QCoreApplication>

#include <boost/shared_ptr.hpp>

using namespace GpgME;
using namespace Kleo;
using namespace boost;

static QMutex installPathMutex;
Q_GLOBAL_STATIC( QString, _installPath )
QString ArchiveDefinition::installPath() {
    const QMutexLocker locker( &installPathMutex );
    QString * const ip = _installPath();
    if ( ip->isEmpty() )
        if ( QCoreApplication::instance() )
            *ip = QCoreApplication::applicationDirPath();
        else
            qWarning() << "called before QCoreApplication was constructed";
    return *ip;
}
void ArchiveDefinition::setInstallPath( const QString & ip ) {
    const QMutexLocker locker( &installPathMutex );
    *_installPath() =ip;
}


// Archive Definition #N groups
static const QLatin1String ID_ENTRY( "id" );
static const QLatin1String NAME_ENTRY( "Name" );
static const QLatin1String PACK_COMMAND_ENTRY( "pack-command" );
static const QLatin1String PACK_COMMAND_OPENPGP_ENTRY( "pack-command-openpgp" );
static const QLatin1String PACK_COMMAND_CMS_ENTRY( "pack-command-cms" );
static const QLatin1String UNPACK_COMMAND_ENTRY( "unpack-command" );
static const QLatin1String UNPACK_COMMAND_OPENPGP_ENTRY( "unpack-command-openpgp" );
static const QLatin1String UNPACK_COMMAND_CMS_ENTRY( "unpack-command-cms" );
static const QLatin1String EXTENSIONS_ENTRY( "extensions" );
static const QLatin1String EXTENSIONS_OPENPGP_ENTRY( "extensions-openpgp" );
static const QLatin1String EXTENSIONS_CMS_ENTRY( "extensions-cms" );
static const QLatin1String FILE_PLACEHOLDER( "%f" );
static const QLatin1String INSTALLPATH_PLACEHOLDER( "%I" );
static const QLatin1String NULL_SEPARATED_STDIN_INDICATOR( "0|" );
static const QLatin1Char   NEWLINE_SEPARATED_STDIN_INDICATOR( '|' );

namespace {

    class ArchiveDefinitionError : public Kleo::Exception {
        const QString m_id;
    public:
        ArchiveDefinitionError( const QString & id, const QString & message )
            : Kleo::Exception( GPG_ERR_INV_PARAMETER, i18n("Error in archive definition %1: %2", id, message ), MessageOnly ),
              m_id( id )
        {

        }
        ~ArchiveDefinitionError() throw() {}

        const QString & archiveDefinitionId() const { return m_id; }
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
                           QString * command, QStringList * prefix, QStringList * suffix, ArchiveDefinition::ArgumentPassingMethod * method, bool parseFilePlaceholder )
{
    assert( prefix );
    assert( suffix );
    assert( method );

    KShell::Errors errors;
    QStringList l;

    if ( cmdline.startsWith( NULL_SEPARATED_STDIN_INDICATOR ) ) {
        *method = ArchiveDefinition::NullSeparatedInputFile;
        cmdline.remove( 0, 2 );
    } else if ( cmdline.startsWith( NEWLINE_SEPARATED_STDIN_INDICATOR ) ) {
        *method = ArchiveDefinition::NewlineSeparatedInputFile;
        cmdline.remove( 0, 1 );
    } else {
        *method = ArchiveDefinition::CommandLine;
    }
    if ( *method != ArchiveDefinition::CommandLine && cmdline.contains( FILE_PLACEHOLDER ) )
        throw ArchiveDefinitionError( id, i18n("Cannot use both %f and | in '%1'", whichCommand) );
    cmdline.replace( FILE_PLACEHOLDER,        QLatin1String("__files_go_here__")  )
           .replace( INSTALLPATH_PLACEHOLDER, QLatin1String("__path_goes_here__") );
    l = KShell::splitArgs( cmdline, KShell::AbortOnMeta|KShell::TildeExpand, &errors );
    l = l.replaceInStrings( QLatin1String("__files_go_here__"), FILE_PLACEHOLDER );
    if ( l.indexOf( QRegExp( QLatin1String(".*__path_goes_here__.*") ) ) >= 0 )
        l = l.replaceInStrings( QLatin1String("__path_goes_here__"), ArchiveDefinition::installPath() );
    if ( errors == KShell::BadQuoting )
        throw ArchiveDefinitionError( id, i18n("Quoting error in '%1' entry", whichCommand) );
    if ( errors == KShell::FoundMeta )
        throw ArchiveDefinitionError( id, i18n("'%1' too complex (would need shell)", whichCommand) );
    qDebug() << "ArchiveDefinition[" << id << ']' << l;
    if ( l.empty() )
        throw ArchiveDefinitionError( id, i18n("'%1' entry is empty/missing", whichCommand) );
    const QFileInfo fi1( l.front() );
    if ( fi1.isAbsolute() )
        *command = try_extensions( l.front() );
    else
        *command = QStandardPaths::findExecutable( fi1.fileName() );
    if ( command->isEmpty() )
        throw ArchiveDefinitionError( id, i18n("'%1' empty or not found", whichCommand) );
    if ( parseFilePlaceholder ) {
        const int idx1 = l.indexOf( FILE_PLACEHOLDER );
        if ( idx1 < 0 ) {
            // none -> append
            *prefix = l.mid( 1 );
        } else {
            *prefix = l.mid( 1, idx1-1 );
            *suffix = l.mid( idx1+1 );
        }
    } else {
        *prefix = l.mid( 1 );
    }
    switch ( *method ) {
    case ArchiveDefinition::CommandLine:
        qDebug() << "ArchiveDefinition[" << id << ']' << *command << *prefix << FILE_PLACEHOLDER << *suffix;
        break;
    case ArchiveDefinition::NewlineSeparatedInputFile:
        qDebug() << "ArchiveDefinition[" << id << ']' << "find | " << *command << *prefix;
        break;
    case ArchiveDefinition::NullSeparatedInputFile:
        qDebug() << "ArchiveDefinition[" << id << ']' << "find -print0 | " << *command << *prefix;
        break;
    case ArchiveDefinition::NumArgumentPassingMethods:
        assert( !"Should not happen" );
        break;
    }
}

namespace {

    class KConfigBasedArchiveDefinition : public ArchiveDefinition {
    public:
        explicit KConfigBasedArchiveDefinition( const KConfigGroup & group )
            : ArchiveDefinition( group.readEntryUntranslated( ID_ENTRY ),
                                 group.readEntry( NAME_ENTRY ) )
        {
            if ( id().isEmpty() )
                throw ArchiveDefinitionError( group.name(), i18n("'%1' entry is empty/missing", ID_ENTRY ) );

            QStringList extensions;
            QString extensionsKey;

            // extensions(-openpgp)
            if ( group.hasKey( EXTENSIONS_OPENPGP_ENTRY ) )
                extensionsKey = EXTENSIONS_OPENPGP_ENTRY;
            else
                extensionsKey = EXTENSIONS_ENTRY;
            extensions = group.readEntry( extensionsKey, QStringList() );
            if ( extensions.empty() )
                throw ArchiveDefinitionError( id(), i18n("'%1' entry is empty/missing", extensionsKey ) );
            setExtensions( OpenPGP, extensions );

            // extensions(-cms)
            if ( group.hasKey( EXTENSIONS_CMS_ENTRY ) )
                extensionsKey = EXTENSIONS_CMS_ENTRY;
            else
                extensionsKey = EXTENSIONS_ENTRY;
            extensions = group.readEntry( extensionsKey, QStringList() );
            if ( extensions.empty() )
                throw ArchiveDefinitionError( id(), i18n("'%1' entry is empty/missing", extensionsKey ) );
            setExtensions( CMS, extensions );

            ArgumentPassingMethod method;

            // pack-command(-openpgp)
            if ( group.hasKey( PACK_COMMAND_OPENPGP_ENTRY ) )
                parse_command( group.readEntry( PACK_COMMAND_OPENPGP_ENTRY ), id(), PACK_COMMAND_OPENPGP_ENTRY,
                               &m_packCommand[OpenPGP], &m_packPrefixArguments[OpenPGP], &m_packPostfixArguments[OpenPGP], &method, true );
            else
                parse_command( group.readEntry( PACK_COMMAND_ENTRY ), id(), PACK_COMMAND_ENTRY,
                               &m_packCommand[OpenPGP], &m_packPrefixArguments[OpenPGP], &m_packPostfixArguments[OpenPGP], &method, true );
            setPackCommandArgumentPassingMethod( OpenPGP, method );

            // pack-command(-cms)
            if ( group.hasKey( PACK_COMMAND_CMS_ENTRY ) )
                parse_command( group.readEntry( PACK_COMMAND_CMS_ENTRY ), id(), PACK_COMMAND_CMS_ENTRY,
                               &m_packCommand[CMS], &m_packPrefixArguments[CMS], &m_packPostfixArguments[CMS], &method, true );
            else
                parse_command( group.readEntry( PACK_COMMAND_ENTRY ), id(), PACK_COMMAND_ENTRY,
                               &m_packCommand[CMS], &m_packPrefixArguments[CMS], &m_packPostfixArguments[CMS], &method, true );
            setPackCommandArgumentPassingMethod( CMS, method );

            QStringList dummy;

            // unpack-command(-openpgp)
            if ( group.hasKey( UNPACK_COMMAND_OPENPGP_ENTRY ) )
                parse_command( group.readEntry( UNPACK_COMMAND_OPENPGP_ENTRY ), id(), UNPACK_COMMAND_OPENPGP_ENTRY,
                               &m_unpackCommand[OpenPGP], &m_unpackArguments[OpenPGP], &dummy, &method, false );
            else
                parse_command( group.readEntry( UNPACK_COMMAND_ENTRY ), id(), UNPACK_COMMAND_ENTRY,
                               &m_unpackCommand[OpenPGP], &m_unpackArguments[OpenPGP], &dummy, &method, false );
            if ( method != CommandLine )
                throw ArchiveDefinitionError( id(), i18n("cannot use argument passing on standard input for unpack-command") );

            // unpack-command(-cms)
            if ( group.hasKey( UNPACK_COMMAND_CMS_ENTRY ) )
                parse_command( group.readEntry( UNPACK_COMMAND_CMS_ENTRY ), id(), UNPACK_COMMAND_CMS_ENTRY,
                               &m_unpackCommand[CMS], &m_unpackArguments[CMS], &dummy, &method, false );
            else
                parse_command( group.readEntry( UNPACK_COMMAND_ENTRY ), id(), UNPACK_COMMAND_ENTRY,
                               &m_unpackCommand[CMS], &m_unpackArguments[CMS], &dummy, &method, false );
            if ( method != CommandLine )
                throw ArchiveDefinitionError( id(), i18n("cannot use argument passing on standard input for unpack-command") );
        }

    private:
        /* reimp */ QString doGetPackCommand( Protocol p ) const { return m_packCommand[p]; }
        /* reimp */ QString doGetUnpackCommand( Protocol p ) const { return m_unpackCommand[p]; }
        /* reimp */ QStringList doGetPackArguments( Protocol p, const QStringList & files ) const {
            return m_packPrefixArguments[p] + files + m_packPostfixArguments[p];
        }
        /* reimp */ QStringList doGetUnpackArguments( Protocol p, const QString & file ) const {
            QStringList copy = m_unpackArguments[p];
            copy.replaceInStrings( FILE_PLACEHOLDER, file );
            return copy;
        }

    private:
        QString m_packCommand[2], m_unpackCommand[2];
        QStringList m_packPrefixArguments[2], m_packPostfixArguments[2];
        QStringList m_unpackArguments[2];
    };

}

ArchiveDefinition::ArchiveDefinition( const QString & id, const QString & label  )
    : m_id( id ),
      m_label( label )
{
    m_packCommandMethod[OpenPGP]   = m_packCommandMethod[CMS] = CommandLine;
}

ArchiveDefinition::~ArchiveDefinition() {}

static QByteArray make_input( const QStringList & files, char sep ) {
    QByteArray result;
    Q_FOREACH( const QString & file, files ) {
        result += QFile::encodeName( file );
        result += sep;
    }
    return result;
}

shared_ptr<Input> ArchiveDefinition::createInputFromPackCommand( GpgME::Protocol p, const QStringList & files ) const {
    checkProtocol( p );
    const QString base = heuristicBaseDirectory( files );
    if ( base.isEmpty() )
        throw Kleo::Exception( GPG_ERR_CONFLICT, i18n("Cannot find common base directory for these files:\n%1", files.join( QLatin1String("\n") ) ) );
    qDebug() << "heuristicBaseDirectory(" << files << ") ->" << base;
    const QStringList relative = makeRelativeTo( base, files );
    qDebug() << "relative" << relative;
    switch ( m_packCommandMethod[p] ) {
    case CommandLine:
        return Input::createFromProcessStdOut( doGetPackCommand( p ),
                                               doGetPackArguments( p, relative ),
                                               QDir( base ) );
    case NewlineSeparatedInputFile:
        return Input::createFromProcessStdOut( doGetPackCommand( p ),
                                               doGetPackArguments( p, QStringList() ),
                                               QDir( base ),
                                               make_input( relative, '\n' ) );
    case NullSeparatedInputFile:
        return Input::createFromProcessStdOut( doGetPackCommand( p ),
                                               doGetPackArguments( p, QStringList() ),
                                               QDir( base ),
                                               make_input( relative, '\0' ) );
    case NumArgumentPassingMethods:
        assert( !"Should not happen" );
    }
    return shared_ptr<Input>(); // make compiler happy
}

shared_ptr<Output> ArchiveDefinition::createOutputFromUnpackCommand( GpgME::Protocol p, const QString & file, const QDir & wd ) const {
    checkProtocol( p );
    const QFileInfo fi( file );
    return Output::createFromProcessStdIn( doGetUnpackCommand( p ),
                                           doGetUnpackArguments( p, fi.absoluteFilePath() ),
                                           wd );
}

// static
std::vector< shared_ptr<ArchiveDefinition> > ArchiveDefinition::getArchiveDefinitions() {
    QStringList errors;
    return getArchiveDefinitions( errors );
}

// static
std::vector< shared_ptr<ArchiveDefinition> > ArchiveDefinition::getArchiveDefinitions( QStringList & errors ) {
    std::vector< shared_ptr<ArchiveDefinition> > result;
    if ( KConfig * config = CryptoBackendFactory::instance()->configObject() ) {
        const QStringList groups = config->groupList().filter( QRegExp(QLatin1String("^Archive Definition #")) );
        result.reserve( groups.size() );
        Q_FOREACH( const QString & group, groups )
            try {
                const shared_ptr<ArchiveDefinition> ad( new KConfigBasedArchiveDefinition( KConfigGroup( config, group ) ) );
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

void ArchiveDefinition::checkProtocol( Protocol p ) const {
    kleo_assert( p == OpenPGP || p == CMS );
}
