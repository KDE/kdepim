/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>
  based on code from kdepim-runtime/tray/global.cpp

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.
  
  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.
  
  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "akonadidatabase.h"

#include <akonadi/private/xdgbasedirs_p.h>

#include <QSettings>

AkonadiDataBase::AkonadiDataBase()
{
    init();
}

AkonadiDataBase::~AkonadiDataBase()
{

}

QStringList AkonadiDataBase::options() const
{
    return m_dboptions;
}

QString AkonadiDataBase::driver() const
{
    return m_dbdriver;
}

QString AkonadiDataBase::name() const
{
    return m_dbname;
}

void AkonadiDataBase::init()
{
    const QString serverConfigFile = Akonadi::XdgBaseDirs::akonadiServerConfigFile( Akonadi::XdgBaseDirs::ReadWrite );
    QSettings settings( serverConfigFile, QSettings::IniFormat );

    m_dbdriver = settings.value( QLatin1String("General/Driver"), QLatin1String("QMYSQL") ).toString();
    settings.beginGroup( m_dbdriver );

    if ( m_dbdriver == QLatin1String("QPSQL") ) {
        m_dbname = settings.value( QLatin1String("Name"), QLatin1String("akonadi") ).toString();
        m_dboptions.append( QLatin1String("--host=") + settings.value( QLatin1String("Host"), QString() ).toString() );
        // If the server is started by the user, we don't need to know the username/password.
        bool startServer = settings.value( QLatin1String("StartServer"), QLatin1String("true") ).toBool();
        if ( !startServer ) {
            // TODO: postgres will always ask for the user password ! implement .pgpass
            m_dboptions.append( QLatin1String("--username=") + settings.value( QLatin1String("User"), QString() ).toString() );
        }
        settings.endGroup();
    } else if ( m_dbdriver == QLatin1String("QMYSQL") ) {
        m_dbname = settings.value( QLatin1String("Name"), QLatin1String("akonadi") ).toString();
        // If the server is started by the user, we don't need to know the username/password.
        bool startServer = settings.value( QLatin1String("StartServer"), QString() ).toBool();
        if ( !startServer ) {
            m_dboptions.append( QLatin1String("--host=") + settings.value( QLatin1String("Host"), QString() ).toString() );
            m_dboptions.append( QLatin1String("--user=") + settings.value( QLatin1String("User"), QString() ).toString() );
            m_dboptions.append( QLatin1String("--password=") + settings.value( QLatin1String("Password"), QString() ).toString() );
        } else {
            const QString options = settings.value( QLatin1String("Options"), QString() ).toString();
            const QStringList list = options.split( QLatin1Char('=') );
            m_dboptions.append( QLatin1String("--socket=") + list.at( 1 ) );
        }

        settings.endGroup();
    }
}
