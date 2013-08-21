/* -*- mode: c++; c-basic-offset:4 -*-
    configreader.cpp

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

#include "configreader.h"
#include "configuration.h"
#include "exception.h"

#include <gpgme++/global.h>
#include <gpgme++/engineinfo.h>

#include <KDebug>
#include <KLocale>
#include <KStandardDirs>

#include <QBuffer>
#include <QByteArray>
#include <QMap>
#include <QProcess>
#include <QStringList>
#include <QFile>

#include <cassert>
#include <memory>

namespace {

struct GpgConfResult {
    QByteArray stdOut;
    QByteArray stdErr;
};

static const int GPGCONF_FLAG_GROUP = 1;
static const int GPGCONF_FLAG_OPTIONAL = 2;
static const int GPGCONF_FLAG_LIST = 4;
static const int GPGCONF_FLAG_RUNTIME = 8;
static const int GPGCONF_FLAG_DEFAULT = 16; // fixed default value available
static const int GPGCONF_FLAG_DEFAULT_DESC = 32; // runtime default value available
static const int GPGCONF_FLAG_NOARG_DESC = 64; // option with optional arg; special meaning if no arg set
static const int GPGCONF_FLAG_NO_CHANGE = 128; // readonly
// Change size of mFlags bitfield if adding new values here


// gpgconf arg type number -> CryptoConfigEntry arg type enum mapping
static ConfigEntry::ArgType knownArgType( int argType, bool& ok ) {
    ok = true;
    switch( argType )
    {
    case 0: // none
        return ConfigEntry::None;
    case 1: // string
        return ConfigEntry::String;
    case 2: // int32
        return ConfigEntry::Int;
    case 3: // uint32
        return ConfigEntry::UInt;
    case 32: // pathname
        return ConfigEntry::Path;
    case 33: // ldap server
        return ConfigEntry::LdapUrl;
    default:
        ok = false;
        return ConfigEntry::None;
    }
}

}

class ConfigReader::Private
{
public:
    GpgConfResult runGpgConf( const QStringList& args ) const;
    GpgConfResult runGpgConf( const QString& arg ) const;

    QMap<QString,QString> readComponentInfo() const;
    void readEntriesForComponent( ConfigComponent* component ) const;
    ConfigEntry* createEntryFromParsedLine( const QStringList& lst ) const;
    void readConfConf( Config* cfg ) const;
};

ConfigReader::ConfigReader() : d( new Private )
{
}

ConfigReader::~ConfigReader()
{
    delete d;
}


Config* ConfigReader::readConfig() const
{
    std::auto_ptr<Config> cfg( new Config );
    const QMap<QString,QString> componentInfo = d->readComponentInfo();

    Q_FOREACH ( const QString& i, componentInfo )
    {
        std::auto_ptr<ConfigComponent> component( new ConfigComponent( i ) );
        component->setDescription( componentInfo[i] );
        d->readEntriesForComponent( component.get() );
        cfg->addComponent( component.release() );
    }
    d->readConfConf( cfg.get() );
    return cfg.release();
}

ConfigEntry* ConfigReader::Private::createEntryFromParsedLine( const QStringList& parsedLine ) const
{
    // Format: NAME:FLAGS:LEVEL:DESCRIPTION:TYPE:ALT-TYPE:ARGNAME:DEFAULT:ARGDEF:VALUE
    assert( parsedLine.count() >= 10 ); // called checked for it already
    QStringList::const_iterator it = parsedLine.begin();
    const QString name = *it++;
    std::auto_ptr<ConfigEntry> entry( new ConfigEntry( name ) );
    const int flags = (*it++).toInt();
    const int level = (*it++).toInt();
    Q_UNUSED( level );
    entry->setDescription( *it++ );
    bool ok;
    // we keep the real (int) arg type, since it influences the parsing (e.g. for ldap urls)
    uint realArgType = (*it++).toInt();
    ConfigEntry::ArgType argType = ::knownArgType( realArgType, ok );
    if ( !ok && !(*it).isEmpty() ) {
    // use ALT-TYPE
        realArgType = (*it).toInt();
        argType = ::knownArgType( realArgType, ok );
    }
    entry->setArgType( argType, flags & GPGCONF_FLAG_LIST ? ConfigEntry::List : ConfigEntry::NoList );
    if ( !ok )
        kWarning() << "Unsupported datatype:" << parsedLine[4] <<" :" << *it <<" for" << parsedLine[0];
    entry->unsetDirty();
    return entry.release();
}

void ConfigReader::Private::readEntriesForComponent( ConfigComponent* component ) const
{
    assert( component );
    QStringList args;
    args << QLatin1String("--list-options") << component->name();
    GpgConfResult res = runGpgConf( args );

    std::auto_ptr<ConfigGroup> currentGroup;

    QBuffer buf( &res.stdOut );
    buf.open( QIODevice::ReadOnly );
    while( buf.canReadLine() ) {
        QString line = QString::fromUtf8( buf.readLine() );
        if ( line.endsWith( QLatin1Char('\n') ) )
            line.chop( 1 );
        if ( line.endsWith( QLatin1Char('\r') ) )
            line.chop( 1 );
        //kDebug(5150) <<"GOT LINE:" << line;
        // Format: NAME:FLAGS:LEVEL:DESCRIPTION:TYPE:ALT-TYPE:ARGNAME:DEFAULT:ARGDEF:VALUE
        const QStringList lst = line.split( QLatin1Char(':') );
        if ( lst.count() >= 10 ) {
            const int flags = lst[1].toInt();
            const int level = lst[2].toInt();
            if ( level > 2 ) // invisible or internal -> skip it;
                continue;
            if ( flags & GPGCONF_FLAG_GROUP ) {
                if ( currentGroup.get() && !currentGroup->isEmpty() ) // only add non-empty groups
                    component->addGroup( currentGroup.release() );
                else {
                    currentGroup.reset();
                }
            //else
            //  kDebug(5150) <<"Discarding empty group" << mCurrentGroupName;
                currentGroup.reset( new ConfigGroup( lst[0] ) );
                currentGroup->setDescription( lst[3] );
                //currentGroup->setLevel( level );
            } else {
                // normal entry
                if ( !currentGroup.get() ) {  // first toplevel entry -> create toplevel group
                    currentGroup.reset( new ConfigGroup( QLatin1String("<nogroup>") ) );
                }
                currentGroup->addEntry( createEntryFromParsedLine( lst ) );
            }
        } else {
            // This happens on lines like
            // dirmngr[31465]: error opening `/home/dfaure/.gnupg/dirmngr_ldapservers.conf': No such file or directory
            // so let's not bother the user with it.
            //kWarning() <<"Parse error on gpgconf --list-options output:" << line;
        }
    }
    if ( currentGroup.get() && !currentGroup->isEmpty() )
        component->addGroup( currentGroup.release() );
}

void ConfigReader::Private::readConfConf( Config* cfg ) const
{
    GpgConfResult res = runGpgConf( QLatin1String("--list-config") );
    QBuffer buf( &(res.stdOut) );
    buf.open( QIODevice::ReadOnly | QIODevice::Text );
    while ( buf.canReadLine() )
    {
        QString line = QLatin1String(buf.readLine());
        if ( line.endsWith( QLatin1Char('\n') ) )
            line.chop( 1 );
        if ( line.endsWith( QLatin1Char('\r') ) )
            line.chop( 1 );
        const QStringList lst = line.split( QLatin1Char(':') );
        if ( lst.isEmpty() || lst[0] != QLatin1String("r") ) // only parse 'r'-type value entries
            continue;

        if ( lst.count() < 8 )
        {
            throw MalformedGpgConfOutputException( i18n( "Parse error on gpgconf --list-config output: %1", line ) );
        }
        ConfigComponent* const component = cfg->component( lst[3] );
        if ( !component )
        {
            throw MalformedGpgConfOutputException( i18n( "gpgconf --list-config: Unknown component: %1", lst[3] ) );
        }
        ConfigEntry* const entry = component->entry( lst[4] );
        if ( !entry )
        {
            throw MalformedGpgConfOutputException( i18n( "gpgconf --list-config: Unknown entry: %1:%2", lst[3], lst[4] ) );
        }
        const QString flag = lst[5];
        const QString value = lst[6];
        if ( !value.isEmpty() && !value.startsWith( QLatin1Char('\"') ) )
        {
            throw MalformedGpgConfOutputException( i18n( "gpgconf --list-config: Invalid entry: value must start with '\"': %1", lst[6] ) );
        }
        if ( !lst[6].isEmpty() )
            entry->setValueFromRawString( lst[6].mid( 1 ) );

        if ( flag == QLatin1String( "no-change" ) )
            entry->setMutability( ConfigEntry::NoChange );
        else if ( flag == QLatin1String( "change" ) )
            entry->setMutability( ConfigEntry::Change );
        else if ( flag == QLatin1String( "default" ) )
            entry->setUseBuiltInDefault( true );
    }
    buf.close();
}

QMap<QString, QString> ConfigReader::Private::readComponentInfo() const
{
    GpgConfResult res = runGpgConf( QLatin1String("--list-components") );
    QBuffer buf( &(res.stdOut) );
    buf.open( QIODevice::ReadOnly );
    QMap<QString, QString> components;
    while( buf.canReadLine() ) {
        QString line = QString::fromUtf8( buf.readLine() );
        if ( line.endsWith( QLatin1Char('\n') ) )
            line.chop( 1 );
        if ( line.endsWith( QLatin1Char('\r') ) )
            line.chop( 1 );
        //kDebug(5150) <<"GOT LINE:" << line;
        // Format: NAME:DESCRIPTION
        const QStringList lst = line.split( QLatin1Char(':') );
        if ( lst.count() >= 2 ) {
            components[lst[0]] = lst[1];
        } else {
            throw MalformedGpgConfOutputException( i18n( "Parse error on gpgconf --list-components. output: %1", line ) );
        }
    }
    return components;
}

GpgConfResult ConfigReader::Private::runGpgConf( const QString& arg ) const
{
    return runGpgConf( QStringList( arg ) );
}

static QString gpgConfPath() {
    const GpgME::EngineInfo ei = GpgME::engineInfo( GpgME::GpgConfEngine );
    return ei.fileName() ? QFile::decodeName( ei.fileName() ) : KStandardDirs::findExe( QLatin1String("gpgconf") ) ;
}

GpgConfResult ConfigReader::Private::runGpgConf( const QStringList& args ) const
{
    QProcess process;
    process.start( gpgConfPath(), args );

    process.waitForStarted();
    process.waitForFinished();

    if ( process.exitStatus() != QProcess::NormalExit ) {
        switch ( process.error() )
        {
        case QProcess::FailedToStart:
            throw GpgConfRunException( -2, i18n( "gpgconf not found or cannot be started" ) );
        case QProcess::Crashed:
            throw GpgConfRunException( -1, i18n( "gpgconf terminated unexpectedly" ) );
        case QProcess::Timedout:
            throw GpgConfRunException( -3, i18n( "timeout while executing gpgconf" ) );
        case QProcess::WriteError:
            throw GpgConfRunException( -4, i18n( "error while writing to gpgconf" ) );
        case QProcess::ReadError:
            throw GpgConfRunException( -5, i18n( "error while reading from gpgconf" ) );
        case QProcess::UnknownError:
        default:
            throw GpgConfRunException( -6, i18n( "Unknown error while executing gpgconf" ) );
        }
    }

    GpgConfResult res;
    res.stdOut = process.readAllStandardOutput();
    res.stdErr = process.readAllStandardError();
    return res;
}

