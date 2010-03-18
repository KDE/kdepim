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
#include <KGlobal>
#include <KConfig>
#include <KShell>
#include <KStandardDirs>

#include <QString>
#include <QStringList>
#include <QDebug>
#include <QFileInfo>

#include <boost/shared_ptr.hpp>

using namespace Kleo;
using namespace boost;

// Checksum Definition #N groups
static const QLatin1String ID_ENTRY( "id" );
static const QLatin1String NAME_ENTRY( "Name" );
static const QLatin1String CREATE_COMMAND_ENTRY( "create-command" );
static const QLatin1String VERIFY_COMMAND_ENTRY( "verify-command" );
static const QLatin1String FILE_PATTERNS_ENTRY( "file-patterns" );
static const QLatin1String OUTPUT_FILE_ENTRY( "output-file" );
static const QLatin1String FILE_PLACEHOLDER( "%f" );

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
            KShell::Errors errors;
            QString cmdline;
            QStringList l;

            // create-command
            cmdline = group.readEntry( CREATE_COMMAND_ENTRY );
            cmdline.replace( FILE_PLACEHOLDER, QLatin1String("__files_go_here__") );
            l = KShell::splitArgs( cmdline, KShell::AbortOnMeta|KShell::TildeExpand, &errors );
            l = l.replaceInStrings( QLatin1String("__files_go_here__"), FILE_PLACEHOLDER );
            if ( errors == KShell::BadQuoting )
                throw ChecksumDefinitionError( id(), i18n("Quoting error in 'create-command' entry") );
            if ( errors == KShell::FoundMeta )
                throw ChecksumDefinitionError( id(), i18n("'create-command' too complex (would need shell)") );
            qDebug() << "ChecksumDefinition[" << id() << ']' << l;
            if ( l.empty() )
                throw ChecksumDefinitionError( id(), i18n("'create-command' entry is empty/missing") );
            const QFileInfo fi1( l.front() );
            if ( fi1.isAbsolute() )
                if ( !fi1.exists() )
                    throw ChecksumDefinitionError( id(), i18n("'create-command' not found in filesystem") );
                else
                    m_createCommand = l.front();
            else
                m_createCommand = KStandardDirs::findExe( fi1.fileName() );
            if ( m_createCommand.isEmpty() )
                throw ChecksumDefinitionError( id(), i18n("'create-command' empty or not found") );
            const int idx1 = l.indexOf( FILE_PLACEHOLDER );
            if ( idx1 < 0 ) {
                // none -> append
                m_createPrefixArguments = l.mid( 1 );
            } else {
                m_createPrefixArguments = l.mid( 1, idx1-1 );
                m_createPostfixArguments = l.mid( idx1+1 );
            }
            qDebug() << "ChecksumDefinition[" << id() << ']' << m_createCommand << m_createPrefixArguments << FILE_PLACEHOLDER << m_createPostfixArguments;

            // verify-command
            cmdline = group.readEntry( VERIFY_COMMAND_ENTRY );
            cmdline.replace( FILE_PLACEHOLDER, QLatin1String("__files_go_here__") );
            l = KShell::splitArgs( cmdline, KShell::AbortOnMeta|KShell::TildeExpand, &errors );
            l = l.replaceInStrings( QLatin1String("__files_go_here__"), FILE_PLACEHOLDER );
            if ( errors == KShell::BadQuoting )
                throw ChecksumDefinitionError( id(), i18n("Quoting error in 'verify-command' entry") );
            if ( errors == KShell::FoundMeta )
                throw ChecksumDefinitionError( id(), i18n("'verify-command' too complex (would need shell)") );
            qDebug() << "ChecksumDefinition[" << id() << ']' << l;
            if ( l.empty() )
                throw ChecksumDefinitionError( id(), i18n("'verify-command' entry is empty/missing") );
            const QFileInfo fi2( l.front() );
            if ( fi2.isAbsolute() )
                if ( !fi2.exists() )
                    throw ChecksumDefinitionError( id(), i18n("'verify-command' not found in filesystem") );
                else
                    m_verifyCommand = l.front();
            else
                m_verifyCommand = KStandardDirs::findExe( fi2.fileName() );
            if ( m_verifyCommand.isEmpty() )
                throw ChecksumDefinitionError( id(), i18n("'verify-command' empty or not found") );
            const int idx2 = l.indexOf( FILE_PLACEHOLDER );
            if ( idx2 < 0 ) {
                // none -> append
                m_verifyPrefixArguments = l.mid( 1 );
            } else {
                m_verifyPrefixArguments = l.mid( 1, idx2-1 );
                m_verifyPostfixArguments = l.mid( idx2+1 );
            }
            qDebug() << "ChecksumDefinition[" << id() << ']' << m_verifyCommand << m_verifyPrefixArguments << FILE_PLACEHOLDER << m_verifyPostfixArguments;
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
      m_patterns( patterns )
{

}

ChecksumDefinition::~ChecksumDefinition() {}

QString ChecksumDefinition::createCommand() const {
    return doGetCreateCommand();
}

QString ChecksumDefinition::verifyCommand() const {
    return doGetVerifyCommand();
}

QStringList ChecksumDefinition::createCommandArguments( const QStringList & files ) const {
    return doGetCreateArguments( files );
}

QStringList ChecksumDefinition::verifyCommandArguments( const QStringList & files ) const {
    return doGetVerifyArguments( files );
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
    const KConfigGroup group( KGlobal::config(), "ChecksumOperations" );
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
    KConfigGroup group( KGlobal::config(), "ChecksumOperations" );
    group.writeEntry( CHECKSUM_DEFINITION_ID_ENTRY, checksumDefinition->id() );
    group.sync();
}

