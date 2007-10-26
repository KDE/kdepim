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

#include <KLocale>

#include <QBuffer>
#include <QByteArray>
#include <QDebug>
#include <QMap>
#include <QProcess>
#include <QStringList>

#include <cassert>

namespace {
 
struct GpgConfResult {
    GpgConfResult() : rc( 0 ) {}
    QByteArray stdOut;
    QByteArray stdErr;
    int rc;
    QString errorReason;
    bool error() const { return rc != 0; }
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
    ::GpgConfResult runGpgConf( const QStringList& args ) const;
    ::GpgConfResult runGpgConf( const QString& arg ) const;

    QMap<QString,QString> readComponentInfo() const;
    void readEntriesForComponent( ConfigComponent* component ) const;
    ConfigEntry* createEntryFromParsedLine( const QStringList& lst ) const;
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
    Config* cfg = new Config;
    const QMap<QString,QString> componentInfo = d->readComponentInfo();

    Q_FOREACH ( const QString i, componentInfo.keys() )
    {
        ConfigComponent* component = new ConfigComponent( i );
        component->setDescription( componentInfo[i] );
        cfg->addComponent( component );
        d->readEntriesForComponent( component );
    }
    return cfg;
}

ConfigEntry* ConfigReader::Private::createEntryFromParsedLine( const QStringList& parsedLine ) const
{
    // Format: NAME:FLAGS:LEVEL:DESCRIPTION:TYPE:ALT-TYPE:ARGNAME:DEFAULT:ARGDEF:VALUE
    assert( parsedLine.count() >= 10 ); // called checked for it already
    QStringList::const_iterator it = parsedLine.begin();
    const QString name = *it++;
    ConfigEntry* entry = new ConfigEntry( name );
    const int flags = (*it++).toInt();
    const int level = (*it++).toInt();
    entry->setDescription( *it++ );
    entry->setReadOnly( ( flags & GPGCONF_FLAG_NO_CHANGE ) != 0 );
    bool ok;
    // we keep the real (int) arg type, since it influences the parsing (e.g. for ldap urls)
    uint realArgType = (*it++).toInt();
    uint argType = ::knownArgType( realArgType, ok );
    if ( !ok && !(*it).isEmpty() ) {
    // use ALT-TYPE
        realArgType = (*it).toInt();
        argType = ::knownArgType( realArgType, ok );
    }
    if ( !ok )
        qWarning() <<"Unsupported datatype:" << parsedLine[4] <<" :" << *it <<" for" << parsedLine[0];
    ++it; // done with alt-type
    ++it; // skip argname (not useful in GUIs)
#if 0
    QString value;
    if ( mFlags & GPGCONF_FLAG_DEFAULT ) {
        value = *it; // get default value
        mDefaultValue = stringToValue( value, true );
    }
#endif
    ++it; // done with DEFAULT
    ++it; // ### skip ARGDEF for now. It's only for options with an "optional arg"
    //kDebug(5150) <<"Entry" << parsedLine[0] <<" val=" << *it;
#if 0    
    if ( !(*it).isEmpty() ) 
        entry->setValue( stringToValue( *it, true ) );
#endif
    entry->unsetDirty();
    return entry;
}

void ConfigReader::Private::readEntriesForComponent( ConfigComponent* component ) const
{
    assert( component );
    QStringList args;
    args << "--list-options" << component->name();
    ::GpgConfResult res = runGpgConf( args );

    ConfigGroup* currentGroup = 0;

    QBuffer buf( &res.stdOut );
    buf.open( QIODevice::ReadOnly );
    while( buf.canReadLine() ) {
        QString line = QString::fromUtf8( buf.readLine() );
        if ( line.endsWith( '\n' ) )
            line.chop( 1 );
        if ( line.endsWith( '\r' ) )
            line.chop( 1 );
        //kDebug(5150) <<"GOT LINE:" << line;
        // Format: NAME:FLAGS:LEVEL:DESCRIPTION:TYPE:ALT-TYPE:ARGNAME:DEFAULT:ARGDEF:VALUE
        const QStringList lst = line.split( ':' );
        if ( lst.count() >= 10 ) {
            const int flags = lst[1].toInt();
            const int level = lst[2].toInt();
            if ( level > 2 ) // invisible or internal -> skip it;
                continue;
            if ( flags & GPGCONF_FLAG_GROUP ) {
                if ( currentGroup && !currentGroup->entryList().isEmpty() ) // only add non-empty groups
                    component->addGroup( currentGroup );
                else {
                    delete currentGroup;
                    currentGroup = 0;
                }
            //else
            //  kDebug(5150) <<"Discarding empty group" << mCurrentGroupName;
                currentGroup = new ConfigGroup( lst[0] );
                currentGroup->setDescription( lst[3] );
                //currentGroup->setLevel( level );
            } else {
                // normal entry
                if ( !currentGroup ) {  // first toplevel entry -> create toplevel group
                    currentGroup = new ConfigGroup( "<nogroup>" );
                }
                ConfigEntry* const entry = createEntryFromParsedLine( lst );
                currentGroup->addEntry( entry );
            }
        } else {
            // This happens on lines like
            // dirmngr[31465]: error opening `/home/dfaure/.gnupg/dirmngr_ldapservers.conf': No such file or directory
            // so let's not bother the user with it.
            //kWarning(5150) <<"Parse error on gpgconf --list-options output:" << line;
        }
    }
}

QMap<QString, QString> ConfigReader::Private::readComponentInfo() const
{
    ::GpgConfResult res = runGpgConf( "--list-components" );
    QBuffer buf( &(res.stdOut) );
    buf.open( QIODevice::ReadOnly );
    QMap<QString, QString> components;
    while( buf.canReadLine() ) {
        QString line = QString::fromUtf8( buf.readLine() );
        if ( line.endsWith( '\n' ) )
            line.chop( 1 );
        if ( line.endsWith( '\r' ) )
            line.chop( 1 );
        //kDebug(5150) <<"GOT LINE:" << line;
        // Format: NAME:DESCRIPTION
        const QStringList lst = line.split( ':' );
        if ( lst.count() >= 2 ) {
            components[lst[0]] = lst[1];
        } else {
            qWarning() <<"Parse error on gpgconf --list-components output:" << line;
        }
    }
    return components;
}

::GpgConfResult ConfigReader::Private::runGpgConf( const QString& arg ) const
{
    return runGpgConf( QStringList( arg ) );
}

::GpgConfResult ConfigReader::Private::runGpgConf( const QStringList& args ) const
{
    GpgConfResult res;
    QProcess process;

    process.start( "gpgconf", args );

    if ( !process.waitForStarted() || !process.waitForFinished() ) {
        res.errorReason = i18n( "program not found or cannot be started" );
        res.rc = -2;
    }
    else if ( process.exitStatus() == QProcess::NormalExit ) {
        res.rc = process.exitCode();
        res.stdOut = process.readAllStandardOutput();
        res.stdErr = process.readAllStandardError();
    }
    else {
        res.rc = -1;
        res.errorReason = i18n( "program terminated unexpectedly" );
    }
    return res;
}

