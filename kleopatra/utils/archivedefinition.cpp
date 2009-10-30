/* -*- mode: c++; c-basic-offset:4 -*-
    utils/archivedefinition.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2009 Klarälvdalens Datakonsult AB

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

#include <utils/exception.h>
#include <utils/input.h>
#include <utils/path-helper.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KGlobal>
#include <KConfig>

#include <QProcess>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QDebug>

#include <boost/shared_ptr.hpp>

using namespace Kleo;
using namespace boost;

static const QLatin1String ID_ENTRY( "id" );
static const QLatin1String NAME_ENTRY( "name" );
static const QLatin1String COMMAND_ENTRY( "pack-command" );
static const QLatin1String EXTENSIONS_ENTRY( "extensions" );
static const QLatin1String FILE_PLACEHOLDER( "%f" );

namespace {

    class ArchiveDefinitionError : Kleo::Exception {
        const QString m_id;
    public:
        ArchiveDefinitionError( const QString & id, const QString & message )
            : Kleo::Exception( GPG_ERR_INV_PARAMETER, i18n("Error in archive definition %1: %2").arg( id, message ) ),
              m_id( id )
        {

        }
        ~ArchiveDefinitionError() throw() {}

        const QString & archiveDefinitionId() const { return m_id; }
    };

    class KConfigBasedArchiveDefinition : public ArchiveDefinition {
    public:
        explicit KConfigBasedArchiveDefinition( const KConfigGroup & group )
            : ArchiveDefinition( group.readEntryUntranslated( ID_ENTRY ),
                                 group.readEntry( NAME_ENTRY ),
                                 group.readEntry( EXTENSIONS_ENTRY, QStringList() ) )
        {
            if ( extensions().empty() )
                throw ArchiveDefinitionError( id(), i18n("'extensions' entry is empty/missing") );
            const QStringList l = group.readEntry( COMMAND_ENTRY ).split( QLatin1Char(' '), QString::SkipEmptyParts );
            qDebug() << "ArchiveDefinition[" << id() << ']' << l;
            if ( l.empty() )
                throw ArchiveDefinitionError( id(), i18n("'command' entry is empty/missing") );
            m_command = l.front();
            const int idx = l.indexOf( FILE_PLACEHOLDER );
            if ( idx < 0 ) {
                // none -> append
                m_prefixArguments = l.mid( 1 );
            } else {
                m_prefixArguments = l.mid( 1, idx-1 );
                m_postfixArguments = l.mid( idx+1 );
            }
            qDebug() << "ArchiveDefinition[" << id() << ']' << m_command << m_prefixArguments << FILE_PLACEHOLDER << m_postfixArguments;
        }

    private:
        /* reimp */ QString doGetCommand() const { return m_command; }
        /* reimp */ QStringList doGetArguments( const QStringList & files ) const {
            return m_prefixArguments + files + m_postfixArguments;
        }

    private:
        QString m_command;
        QStringList m_prefixArguments, m_postfixArguments;
    };

}

ArchiveDefinition::ArchiveDefinition( const QString & id, const QString & label, const QStringList & extensions )
    : m_id( id ), m_label( label ), m_extensions( extensions )
{

}

ArchiveDefinition::~ArchiveDefinition() {}

shared_ptr<Input> ArchiveDefinition::createInput( const QStringList & files ) const {
    const QString base = heuristicBaseDirectory( files );
    qDebug() << "heuristicBaseDirectory(" << files << ") ->" << base;
    const QStringList relative = makeRelativeTo( base, files );
    qDebug() << "relative" << relative;
    return Input::createFromProcessStdOut( doGetCommand(),
                                           doGetArguments( relative ),
                                           QDir( base ) );
}

// static
std::vector< shared_ptr<ArchiveDefinition> > ArchiveDefinition::getArchiveDefinitions() {
    std::vector< shared_ptr<ArchiveDefinition> > result;
    if ( const KSharedConfigPtr config = KGlobal::config() ) {
        const QStringList groups = config->groupList().filter( QRegExp(QLatin1String("^Archive Definition #")) );
        result.reserve( groups.size() );
        Q_FOREACH( const QString & group, groups )
            try {
                const shared_ptr<ArchiveDefinition> ad( new KConfigBasedArchiveDefinition( KConfigGroup( config, group ) ) );
                result.push_back( ad );
            } catch ( const std::exception & e ) {
                qDebug() << e.what();
            }
    }
    return result;
}
