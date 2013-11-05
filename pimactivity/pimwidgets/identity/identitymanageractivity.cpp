/*
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

// config keys:
static const char configKeyDefaultIdentity[] = "Default Identity";

#include "identitymanageractivity.h"
#include <KPIMIdentities/Identity> // for IdentityList::{export,import}Data

#include <kpimutils/email.h> // for static helper functions

#include <kemailsettings.h> // for IdentityEntry::fromControlCenter()
#include <klocale.h>
#include <klocalizedstring.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kuser.h>
#include <kconfiggroup.h>

#include <QList>
#include <QRegExp>
#include <QtDBus/QtDBus>

#include <assert.h>
#include <krandom.h>

#include "identitymanageractivityadaptor.h"

namespace PimActivity {

class IdentityManagerActivityPrivate
{
public:
    IdentityManagerActivityPrivate(ActivityManager *manager, IdentityManagerActivity *qq)
        : readOnly(false),
          activityManager(manager),
          q(qq)
    {
        config = new KConfig( QLatin1String("emailidentities") );
    }
    ~IdentityManagerActivityPrivate()
    {
        delete config;
    }

    QStringList groupList( KConfig *conf ) const
    {
        return conf->groupList().filter( QRegExp( QLatin1String("^Identity #\\d+$") ) );
    }

    void writeConfig() const
    {
        const QStringList identitiesList = groupList( config );
        QStringList::const_iterator groupEnd = identitiesList.constEnd();
        for ( QStringList::const_iterator group = identitiesList.constBegin();
              group != groupEnd; ++group ) {
            config->deleteGroup( *group );
        }
        int i = 0;
        IdentityManagerActivity::ConstIterator end = identities.constEnd();
        for ( IdentityManagerActivity::ConstIterator it = identities.constBegin();
              it != end; ++it, ++i ) {
            KConfigGroup cg( config, QString::fromLatin1( "Identity #%1" ).arg( i ) );
            ( *it ).writeConfig( cg );
            if ( ( *it ).isDefault() ) {
                // remember which one is default:
                KConfigGroup general( config, "General" );
                general.writeEntry( configKeyDefaultIdentity, ( *it ).uoid() );

                // Also write the default identity to emailsettings
                KEMailSettings es;
                es.setSetting( KEMailSettings::RealName, ( *it ).fullName() );
                es.setSetting( KEMailSettings::EmailAddress, ( *it ).primaryEmailAddress() );
                es.setSetting( KEMailSettings::Organization, ( *it ).organization() );
                es.setSetting( KEMailSettings::ReplyToAddress, ( *it ).replyToAddr() );
            }
        }
        config->sync();
    }

    void readConfig( KConfig *conf )
    {
        identities.clear();

        const QStringList identitiesList = groupList( conf );
        if ( identitiesList.isEmpty() ) {
            return; // nothing to be done...
        }

        KConfigGroup general( conf, "General" );
        uint defaultIdentity = general.readEntry( configKeyDefaultIdentity, 0 );
        bool haveDefault = false;
        QStringList::const_iterator groupEnd = identitiesList.constEnd();
        for ( QStringList::const_iterator group = identitiesList.constBegin();
              group != groupEnd; ++group ) {
            KConfigGroup configGroup( conf, *group );
            identities << KPIMIdentities::Identity();
            identities.last().readConfig( configGroup );
            if ( !haveDefault && identities.last().uoid() == defaultIdentity ) {
                haveDefault = true;
                identities.last().setIsDefault( true );
            }
        }

        if ( !haveDefault ) {
            kWarning( 5325 ) << "IdentityManagerActivity: There was no default identity."
                             << "Marking first one as default.";
            identities.first().setIsDefault( true );
        }
        qSort( identities );

        shadowIdentities = identities;
    }


    void createDefaultIdentity()
    {
        QString fullName, emailAddress;
        bool done = false;

        // Check if the application has any settings
        q->createDefaultIdentity( fullName, emailAddress );

        // If not, then use the kcontrol settings
        if ( fullName.isEmpty() && emailAddress.isEmpty() ) {
            KEMailSettings emailSettings;
            fullName = emailSettings.getSetting( KEMailSettings::RealName );
            emailAddress = emailSettings.getSetting( KEMailSettings::EmailAddress );

            if ( !fullName.isEmpty() && !emailAddress.isEmpty() ) {
                q->newFromControlCenter( i18nc( "use default address from control center",
                                             "Default" ) );
                done = true;
            } else {
                // If KEmailSettings doesn't have name and address, generate something from KUser
                KUser user;
                if ( fullName.isEmpty() ) {
                    fullName = user.property( KUser::FullName ).toString();
                }
                if ( emailAddress.isEmpty() ) {
                    emailAddress = user.loginName();
                    if ( !emailAddress.isEmpty() ) {
                        KConfigGroup general( config, "General" );
                        QString defaultdomain = general.readEntry( "Default domain" );
                        if ( !defaultdomain.isEmpty() ) {
                            emailAddress += QLatin1Char('@') + defaultdomain;
                        } else {
                            emailAddress.clear();
                        }
                    }
                }
            }
        }

        if ( !done ) {
            // Default identity name
            QString name( i18nc( "Default name for new email accounts/identities.", "Unnamed" ) );

            if ( !emailAddress.isEmpty() ) {
                // If we have an email address, create a default identity name from it
                QString idName = emailAddress;
                int pos = idName.indexOf( QLatin1Char('@') );
                if ( pos != -1 ) {
                    name = idName.mid( pos + 1, -1 );
                }

                // Make the name a bit more human friendly
                name.replace( QLatin1Char('.'), QLatin1Char(' ') );
                pos = name.indexOf( QLatin1Char(' ') );
                if ( pos != 0 ) {
                    name[pos + 1] = name[pos + 1].toUpper();
                }
                name[0] = name[0].toUpper();
            } else if ( !fullName.isEmpty() ) {
                // If we have a full name, create a default identity name from it
                name = fullName;
            }
            shadowIdentities << KPIMIdentities::Identity( name, fullName, emailAddress );
        }

        shadowIdentities.last().setIsDefault( true );
        shadowIdentities.last().setUoid( newUoid() );
        if ( readOnly ) { // commit won't do it in readonly mode
            identities = shadowIdentities;
        }
    }

    int newUoid()
    {
        int uoid;

        // determine the UOIDs of all saved identities
        QList<uint> usedUOIDs;
        QList<KPIMIdentities::Identity>::ConstIterator end( identities.constEnd() );
        for ( QList<KPIMIdentities::Identity>::ConstIterator it = identities.constBegin();
              it != end; ++it ) {
            usedUOIDs << ( *it ).uoid();
        }

        if ( q->hasPendingChanges() ) {
            // add UOIDs of all shadow identities. Yes, we will add a lot of duplicate
            // UOIDs, but avoiding duplicate UOIDs isn't worth the effort.
            QList<KPIMIdentities::Identity>::ConstIterator endShadow( shadowIdentities.constEnd() );
            for ( QList<KPIMIdentities::Identity>::ConstIterator it = shadowIdentities.constBegin();
                  it != endShadow; ++it ) {
                usedUOIDs << ( *it ).uoid();
            }
        }

        usedUOIDs << 0; // no UOID must be 0 because this value always refers to the
        // default identity

        do {
            uoid = KRandom::random();
        } while ( usedUOIDs.indexOf( uoid ) != -1 );

        return uoid;
    }

    /** The list that will be seen by everyoe */
    QList<KPIMIdentities::Identity> identities;
    /** The list that will be seen by the config dialog */
    QList<KPIMIdentities::Identity> shadowIdentities;

    KConfig *config;
    bool readOnly;
    ActivityManager *activityManager;
    IdentityManagerActivity *q;
};


static QString newDBusObjectName()
{
    static int s_count = 0;
    QString name = QLatin1String( "/KPIMIDENTITIES_IdentityManagerActivity" );
    if ( s_count++ ) {
        name += QLatin1Char('_');
        name += QString::number( s_count );
    }
    return name;
}

IdentityManagerActivity::IdentityManagerActivity(ActivityManager *manager, bool readonly, QObject *parent,
                                                 const char *name )
    : QObject( parent ),
      d(new IdentityManagerActivityPrivate(manager, this))
{
    setObjectName( QLatin1String(name) );
    KGlobal::locale()->insertCatalog( QLatin1String("libkpimidentities") );
    new IdentityManagerActivityAdaptor( this );
    QDBusConnection dbus = QDBusConnection::sessionBus();
    const QString dbusPath = newDBusObjectName();
    setProperty( "uniqueDBusPath", dbusPath );
    const QString dbusInterface = QLatin1String("org.kde.pim.IdentityManagerActivity");
    dbus.registerObject( dbusPath, this );
    dbus.connect( QString(), QString(), dbusInterface, QLatin1String("identitiesChanged"), this,
                  SLOT(slotIdentitiesChanged(QString)) );

    d->readOnly = readonly;

    d->readConfig( d->config );
    if ( d->identities.isEmpty() ) {
        kDebug( 5325 ) << "emailidentities is empty -> convert from kmailrc";
        // No emailidentities file, or an empty one due to broken conversion
        // (kconf_update bug in kdelibs <= 3.2.2)
        // => convert it, i.e. read settings from kmailrc
        KConfig kmailConf( QLatin1String("kmailrc") );
        d->readConfig( &kmailConf );
    }
    // we need at least a default identity:
    if ( d->identities.isEmpty() ) {
        kDebug( 5325 ) << "IdentityManagerActivity: No identity found. Creating default.";
        d->createDefaultIdentity();
        commit();
    }
    // Migration: people without settings in kemailsettings should get some
    if ( KEMailSettings().getSetting( KEMailSettings::EmailAddress ).isEmpty() ) {
        d->writeConfig();
    }
}

IdentityManagerActivity::~IdentityManagerActivity()
{
    kWarning( hasPendingChanges(), 5325 )
            << "IdentityManagerActivity: There were uncommitted changes!";
    delete d;
}

PimActivity::ActivityManager *IdentityManagerActivity::activityManager() const
{
    return d->activityManager;
}


QString IdentityManagerActivity::makeUnique( const QString &name ) const
{
    int suffix = 1;
    QString result = name;
    while ( identities().contains( result ) ) {
        result = i18nc( "%1: name; %2: number appended to it to make it unique "
                        "among a list of names", "%1 #%2",
                        name, suffix );
        suffix++;
    }
    return result;
}

bool IdentityManagerActivity::isUnique( const QString &name ) const
{
    return !identities().contains( name );
}

void IdentityManagerActivity::commit()
{
    // early out:
    if ( !hasPendingChanges() || d->readOnly ) {
        return;
    }

    QList<uint> seenUOIDs;
    QList<KPIMIdentities::Identity>::ConstIterator end = d->identities.constEnd();
    for ( QList<KPIMIdentities::Identity>::ConstIterator it = d->identities.constBegin();
          it != end; ++it ) {
        seenUOIDs << ( *it ).uoid();
    }

    QList<uint> changedUOIDs;
    // find added and changed identities:
    for ( QList<KPIMIdentities::Identity>::ConstIterator it = d->shadowIdentities.constBegin();
          it != d->shadowIdentities.constEnd(); ++it ) {
        int index = seenUOIDs.indexOf( ( *it ).uoid() );
        if ( index != -1 ) {
            uint uoid = seenUOIDs.at( index );
            const KPIMIdentities::Identity &orig = identityForUoid( uoid );  // look up in identities
            if ( *it != orig ) {
                // changed identity
                kDebug( 5325 ) << "emitting changed() for identity" << uoid;
                emit changed( *it );
                changedUOIDs << uoid;
            }
            seenUOIDs.removeAll( uoid );
        } else {
            // new identity
            kDebug( 5325 ) << "emitting added() for identity" << ( *it ).uoid();
            emit added( *it );
        }
    }

    // what's left are deleted identities:
    for ( QList<uint>::ConstIterator it = seenUOIDs.constBegin();
          it != seenUOIDs.constEnd(); ++it ) {
        kDebug( 5325 ) << "emitting deleted() for identity" << ( *it );
        emit deleted( *it );
    }

    d->identities = d->shadowIdentities;
    d->writeConfig();

    // now that identities has all the new info, we can emit the added/changed
    // signals that ship a uoid. This is because the slots might use
    // identityForUoid(uoid)...
    QList<uint>::ConstIterator changedEnd( changedUOIDs.constEnd() );
    for ( QList<uint>::ConstIterator it = changedUOIDs.constBegin();
          it != changedEnd; ++it ) {
        emit changed( *it );
    }

    emit changed(); // normal signal

    // DBus signal for other IdentityManagerActivity instances
    const QString ourIdentifier = QString::fromLatin1( "%1/%2" ).
            arg( QDBusConnection::sessionBus().baseService() ).
            arg( property( "uniqueDBusPath" ).toString() );
    emit identitiesChanged( ourIdentifier );
}

void IdentityManagerActivity::rollback()
{
    d->shadowIdentities = d->identities;
}

bool IdentityManagerActivity::hasPendingChanges() const
{
    return d->identities != d->shadowIdentities;
}

QStringList IdentityManagerActivity::identities() const
{
    QStringList result;
    ConstIterator end = d->identities.constEnd();
    for ( ConstIterator it = d->identities.constBegin();
          it != end; ++it ) {
        result << ( *it ).identityName();
    }
    return result;
}

QStringList IdentityManagerActivity::shadowIdentities() const
{
    QStringList result;
    ConstIterator end = d->shadowIdentities.constEnd();
    for ( ConstIterator it = d->shadowIdentities.constBegin();
          it != end; ++it ) {
        result << ( *it ).identityName();
    }
    return result;
}

void IdentityManagerActivity::sort()
{
    qSort( d->shadowIdentities );
}

QList<KPIMIdentities::Identity>::ConstIterator IdentityManagerActivity::begin() const
{
    return d->identities.constBegin();
}

QList<KPIMIdentities::Identity>::ConstIterator IdentityManagerActivity::end() const
{
    return d->identities.constEnd();
}

IdentityManagerActivity::Iterator IdentityManagerActivity::modifyBegin()
{
    return d->shadowIdentities.begin();
}

IdentityManagerActivity::Iterator IdentityManagerActivity::modifyEnd()
{
    return d->shadowIdentities.end();
}

const KPIMIdentities::Identity &IdentityManagerActivity::identityForUoid( uint uoid ) const
{
    for ( ConstIterator it = begin(); it != end(); ++it ) {
        if ( ( *it ).uoid() == uoid ) {
            return ( *it );
        }
    }
    return KPIMIdentities::Identity::null();
}

const KPIMIdentities::Identity &IdentityManagerActivity::identityForUoidOrDefault( uint uoid ) const
{
    const KPIMIdentities::Identity &ident = identityForUoid( uoid );
    if ( ident.isNull() ) {
        return defaultIdentity();
    } else {
        return ident;
    }
}

const KPIMIdentities::Identity &IdentityManagerActivity::identityForAddress( const QString &addresses ) const
{
    const QStringList addressList = KPIMUtils::splitAddressList( addresses );
    foreach ( const QString &fullAddress, addressList ) {
        const QString addrSpec = KPIMUtils::extractEmailAddress( fullAddress ).toLower();
        for ( ConstIterator it = begin(); it != end(); ++it ) {
            const KPIMIdentities::Identity &identity = *it;
            if ( identity.matchesEmailAddress( addrSpec ) ) {
                return identity;
            }
        }
    }
    return KPIMIdentities::Identity::null();
}

bool IdentityManagerActivity::thatIsMe( const QString &addressList ) const
{
    return !identityForAddress( addressList ).isNull();
}

KPIMIdentities::Identity &IdentityManagerActivity::modifyIdentityForName( const QString &name )
{
    for ( Iterator it = modifyBegin(); it != modifyEnd(); ++it ) {
        if ( ( *it ).identityName() == name ) {
            return ( *it );
        }
    }

    kWarning( 5325 ) << "IdentityManagerActivity::modifyIdentityForName() used as"
                     << "newFromScratch() replacement!"
                     << endl << "  name == \"" << name << "\"";
    return newFromScratch( name );
}

KPIMIdentities::Identity &IdentityManagerActivity::modifyIdentityForUoid( uint uoid )
{
    for ( Iterator it = modifyBegin(); it != modifyEnd(); ++it ) {
        if ( ( *it ).uoid() == uoid ) {
            return ( *it );
        }
    }

    kWarning( 5325 ) << "IdentityManagerActivity::identityForUoid() used as"
                     << "newFromScratch() replacement!"
                     << endl << "  uoid == \"" << uoid << "\"";
    return newFromScratch( i18n( "Unnamed" ) );
}

const KPIMIdentities::Identity &IdentityManagerActivity::defaultIdentity() const
{
    for ( ConstIterator it = begin(); it != end(); ++it ) {
        if ( ( *it ).isDefault() ) {
            return ( *it );
        }
    }

    if ( d->identities.isEmpty() ) {
        kFatal( 5325 ) << "IdentityManagerActivity: No default identity found!";
    } else {
        kWarning( 5325 ) << "IdentityManagerActivity: No default identity found!";
    }
    return *begin();
}

bool IdentityManagerActivity::setAsDefault( uint uoid )
{
    // First, check if the identity actually exists:
    bool found = false;
    for ( ConstIterator it = d->shadowIdentities.constBegin();
          it != d->shadowIdentities.constEnd(); ++it ) {
        if ( ( *it ).uoid() == uoid ) {
            found = true;
            break;
        }
    }

    if ( !found ) {
        return false;
    }

    // Then, change the default as requested:
    for ( Iterator it = modifyBegin(); it != modifyEnd(); ++it ) {
        ( *it ).setIsDefault( ( *it ).uoid() == uoid );
    }

    // and re-sort:
    sort();
    return true;
}

bool IdentityManagerActivity::removeIdentity( const QString &name )
{
    if ( d->shadowIdentities.size() <= 1 ) {
        return false;
    }

    for ( Iterator it = modifyBegin(); it != modifyEnd(); ++it ) {
        if ( ( *it ).identityName() == name ) {
            bool removedWasDefault = ( *it ).isDefault();
            d->shadowIdentities.erase( it );
            if ( removedWasDefault && !d->shadowIdentities.isEmpty() ) {
                d->shadowIdentities.first().setIsDefault( true );
            }
            return true;
        }
    }
    return false;
}

bool IdentityManagerActivity::removeIdentityForced( const QString &name )
{
    for ( Iterator it = modifyBegin(); it != modifyEnd(); ++it ) {
        if ( ( *it ).identityName() == name ) {
            bool removedWasDefault = ( *it ).isDefault();
            d->shadowIdentities.erase( it );
            if ( removedWasDefault && !d->shadowIdentities.isEmpty() ) {
                d->shadowIdentities.first().setIsDefault( true );
            }
            return true;
        }
    }
    return false;
}

KPIMIdentities::Identity &IdentityManagerActivity::newFromScratch( const QString &name )
{
    return newFromExisting( KPIMIdentities::Identity( name ) );
}

KPIMIdentities::Identity &IdentityManagerActivity::newFromControlCenter( const QString &name )
{
    KEMailSettings es;
    es.setProfile( es.defaultProfileName() );

    return
            newFromExisting( KPIMIdentities::Identity( name,
                                       es.getSetting( KEMailSettings::RealName ),
                                       es.getSetting( KEMailSettings::EmailAddress ),
                                       es.getSetting( KEMailSettings::Organization ),
                                       es.getSetting( KEMailSettings::ReplyToAddress ) ) );
}

KPIMIdentities::Identity &IdentityManagerActivity::newFromExisting( const KPIMIdentities::Identity &other, const QString &name )
{
    d->shadowIdentities << other;
    KPIMIdentities::Identity &result = d->shadowIdentities.last();
    result.setIsDefault( false );  // we don't want two default identities!
    result.setUoid( d->newUoid() );  // we don't want two identies w/ same UOID
    if ( !name.isNull() ) {
        result.setIdentityName( name );
    }
    return result;
}


QStringList IdentityManagerActivity::allEmails() const
{
    QStringList lst;
    for ( ConstIterator it = begin(); it != end(); ++it ) {
        lst << ( *it ).primaryEmailAddress();
        if ( !( *it ).emailAliases().isEmpty() ) {
            lst << ( *it ).emailAliases();
        }
    }
    return lst;
}

void IdentityManagerActivity::slotRollback()
{
    rollback();
}

void IdentityManagerActivity::slotIdentitiesChanged( const QString &id )
{
    kDebug( 5325 ) << " KPIMIdentities::IdentityManagerActivity::slotIdentitiesChanged :" << id;
    const QString ourIdentifier = QString::fromLatin1( "%1/%2" ).
            arg( QDBusConnection::sessionBus().baseService() ).
            arg( property( "uniqueDBusPath" ).toString() );
    if ( id != ourIdentifier ) {
        d->config->reparseConfiguration();
        Q_ASSERT( !hasPendingChanges() );
        d->readConfig( d->config );
        emit changed();
    }
}

}

