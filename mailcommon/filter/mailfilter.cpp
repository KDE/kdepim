/* -*- mode: C++; c-file-style: "gnu" -*-
 * kmail: KDE mail client
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
 * Copyright (C) 2012 Andras Mantia <amantia@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

// my header
#include "mailfilter.h"

// other kmail headers
#include "filteraction.h"
#include "filteractiondict.h"
#include "filtermanager.h"
#include "filterlog.h"
#include "dialog/filteractionmissingargumentdialog.h"
using MailCommon::FilterLog;

#include "pimcommon/util/pimutil.h"

// KDEPIMLIBS headers
#include <Akonadi/AgentManager>

// other KDE headers
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <krandom.h>

#include <algorithm>
#include <boost/bind.hpp>


using namespace MailCommon;

MailFilter::MailFilter()
{
    mIdentifier = KRandom::randomString( 16 );
    bApplyOnInbound = true;
    bApplyBeforeOutbound = false;
    bApplyOnOutbound = false;
    bApplyOnExplicit = true;
    bStopProcessingHere = true;
    bConfigureShortcut = false;
    bConfigureToolbar = false;
    bAutoNaming = true;
    mApplicability = All;
    bEnabled = true;
}


MailFilter::MailFilter( const KConfigGroup & aConfig, bool interactive, bool & needUpdate )
{
    needUpdate =  readConfig( aConfig, interactive );
}


MailFilter::MailFilter( const MailFilter & aFilter )
{
    mIdentifier = aFilter.mIdentifier;
    mPattern = aFilter.mPattern;

    bApplyOnInbound = aFilter.applyOnInbound();
    bApplyBeforeOutbound = aFilter.applyBeforeOutbound();
    bApplyOnOutbound = aFilter.applyOnOutbound();
    bApplyOnExplicit = aFilter.applyOnExplicit();
    bStopProcessingHere = aFilter.stopProcessingHere();
    bConfigureShortcut = aFilter.configureShortcut();
    bConfigureToolbar = aFilter.configureToolbar();
    mToolbarName = aFilter.toolbarName();
    mApplicability = aFilter.applicability();
    bAutoNaming = aFilter.isAutoNaming();
    bEnabled = aFilter.isEnabled();
    mIcon = aFilter.icon();
    mShortcut = aFilter.shortcut();

    QListIterator<FilterAction*> it( aFilter.mActions );
    while ( it.hasNext() ) {
        FilterAction *action = it.next();
        FilterActionDesc *desc = FilterManager::filterActionDict()->value( action->name() );
        if ( desc ) {
            FilterAction *f = desc->create();
            if ( f ) {
                f->argsFromString( action->argsAsString() );
                mActions.append( f );
            }
        }
    }

    mAccounts.clear();
    QStringList::ConstIterator it2;
    QStringList::ConstIterator end2 = aFilter.mAccounts.constEnd();
    for ( it2 = aFilter.mAccounts.constBegin() ; it2 != end2 ; ++it2 )
        mAccounts.append( *it2 );
}

MailFilter::~MailFilter()
{
    qDeleteAll( mActions );
}

QString MailFilter::identifier() const
{
    return mIdentifier;
}

QString MailFilter::name() const
{
    return mPattern.name();
}

MailFilter::ReturnCode MailFilter::execActions( ItemContext &context, bool& stopIt, bool applyOnOutbound ) const
{
    ReturnCode status = NoResult;

    QList<FilterAction*>::const_iterator it( mActions.constBegin() );
    QList<FilterAction*>::const_iterator end( mActions.constEnd() );
    for ( ; it != end ; ++it ) {

        if ( FilterLog::instance()->isLogging() ) {
            const QString logText( i18n( "<b>Applying filter action:</b> %1",
                                         (*it)->displayString() ) );
            FilterLog::instance()->add( logText, FilterLog::AppliedAction );
        }

        FilterAction::ReturnCode result = (*it)->process( context, applyOnOutbound );

        switch ( result ) {
        case FilterAction::CriticalError:
            if ( FilterLog::instance()->isLogging() ) {
                const QString logText = QString::fromLatin1( "<font color=#FF0000>%1</font>" )
                        .arg( i18n( "A critical error occurred. Processing stops here." ) );
                FilterLog::instance()->add( logText, FilterLog::AppliedAction );
            }
            // in case it's a critical error: return immediately!
            return CriticalError;
        case FilterAction::ErrorButGoOn:
            if ( FilterLog::instance()->isLogging() ) {
                const QString logText = QString::fromLatin1( "<font color=#FF0000>%1</font>" )
                        .arg( i18n( "A problem was found while applying this action." ) );
                FilterLog::instance()->add( logText, FilterLog::AppliedAction );
            }
        default:
            break;
        }
    }

    if ( status == NoResult ) // No filters matched, keep copy of message
        status = GoOn;

    stopIt = stopProcessingHere();

    return status;
}

QList<FilterAction*>* MailFilter::actions()
{
    return &mActions;
}

const QList<FilterAction*>* MailFilter::actions() const
{
    return &mActions;
}

SearchPattern* MailFilter::pattern()
{
    return &mPattern;
}

const SearchPattern* MailFilter::pattern() const
{
    return &mPattern;
}

void MailFilter::setApplyOnOutbound( bool aApply )
{
    bApplyOnOutbound = aApply;
}

void MailFilter::setApplyBeforeOutbound( bool aApply )
{
    bApplyBeforeOutbound = aApply;
}

bool MailFilter::applyOnOutbound() const
{
    return bApplyOnOutbound;
}

bool MailFilter::applyBeforeOutbound() const
{
    return bApplyBeforeOutbound;
}

void MailFilter::setApplyOnInbound( bool aApply )
{
    bApplyOnInbound = aApply;
}

bool MailFilter::applyOnInbound() const
{
    return bApplyOnInbound;
}

void MailFilter::setApplyOnExplicit( bool aApply )
{
    bApplyOnExplicit = aApply;
}

bool MailFilter::applyOnExplicit() const
{
    return bApplyOnExplicit;
}

void MailFilter::setApplicability( AccountType aApply )
{
    mApplicability = aApply;
}

MailFilter::AccountType MailFilter::applicability() const
{
    return mApplicability;
}

SearchRule::RequiredPart MailFilter::requiredPart(const QString& id) const
{
    //find the required message part needed for the filter
    //this can be either only the Envelope, all Header or the CompleteMessage
    //Makes the assumption that  Envelope < Header < CompleteMessage
    int requiredPart = SearchRule::Envelope;

    if (!bEnabled || !applyOnAccount(id))
        return SearchRule::Envelope;

    if (pattern())
        requiredPart = qMax( requiredPart, (int)pattern()->requiredPart() ) ; // no pattern means always matches?

    int requiredPartByActions = SearchRule::Envelope;

    QList<FilterAction*> actionList = *actions();
    if (!actionList.isEmpty()) {
        requiredPartByActions = (*std::max_element(actionList.constBegin(), actionList.constEnd(),
                                                   boost::bind(&MailCommon::FilterAction::requiredPart, _1) <
                                                   boost::bind(&MailCommon::FilterAction::requiredPart, _2) ))->requiredPart();
    }
    requiredPart = qMax( requiredPart, requiredPartByActions);

    return static_cast<SearchRule::RequiredPart>(requiredPart);
}

bool MailFilter::folderRemoved( const Akonadi::Collection & aFolder, const Akonadi::Collection& aNewFolder )
{
    bool rem = false;

    QListIterator<FilterAction*> it( mActions );
    while ( it.hasNext() )
        if ( it.next()->folderRemoved( aFolder, aNewFolder ) )
            rem = true;

    return rem;
}

void MailFilter::setApplyOnAccount( const QString& id, bool aApply )
{
    if (aApply && !mAccounts.contains( id )) {
        mAccounts.append( id );
    } else if (!aApply && mAccounts.contains( id )) {
        mAccounts.removeAll( id );
    }
}

bool MailFilter::applyOnAccount( const QString& id ) const
{
    if ( applicability() == All )
        return true;
    if ( applicability() == ButImap ) {
        Akonadi::AgentInstance instance = Akonadi::AgentManager::self()->instance( id );
        if ( instance.isValid() ) {
            return ( instance.type().identifier() != IMAP_RESOURCE_IDENTIFIER );
        } else {
            return false;
        }
    }
    if ( applicability() == Checked )
        return mAccounts.contains( id );

    return false;
}

void MailFilter::setStopProcessingHere( bool aStop )
{
    bStopProcessingHere = aStop;
}

bool MailFilter::stopProcessingHere() const
{
    return bStopProcessingHere;
}

void MailFilter::setConfigureShortcut( bool aShort )
{
    bConfigureShortcut = aShort;
    bConfigureToolbar = (bConfigureToolbar && bConfigureShortcut);
}

bool MailFilter::configureShortcut() const
{
    return bConfigureShortcut;
}

void MailFilter::setConfigureToolbar( bool aTool )
{
    bConfigureToolbar = (aTool && bConfigureShortcut);
}

bool MailFilter::configureToolbar() const
{
    return bConfigureToolbar;
}

void MailFilter::setToolbarName( const QString &toolbarName )
{
    mToolbarName = toolbarName;
}

void MailFilter::setShortcut( const KShortcut &shortcut )
{
    mShortcut = shortcut;
}

const KShortcut& MailFilter::shortcut() const
{
    return mShortcut;
}

void MailFilter::setIcon( const QString &icon )
{
    mIcon = icon;
}

QString MailFilter::icon() const
{
    return mIcon;
}

void MailFilter::setAutoNaming( bool useAutomaticNames )
{
    bAutoNaming = useAutomaticNames;
}

bool MailFilter::isAutoNaming() const
{
    return bAutoNaming;
}

//-----------------------------------------------------------------------------
bool MailFilter::readConfig(const KConfigGroup & config, bool interactive)
{
    bool needUpdate = false;
    // MKSearchPattern::readConfig ensures
    // that the pattern is purified.
    mPattern.readConfig(config);
    mIdentifier = config.readEntry( "identifier", KRandom::randomString( 16 ) );

    const QStringList sets = config.readEntry("apply-on", QStringList() );
    if ( sets.isEmpty() && !config.hasKey("apply-on") ) {
        bApplyBeforeOutbound = false;
        bApplyOnOutbound = false;
        bApplyOnInbound = true;
        bApplyOnExplicit = true;
        mApplicability = ButImap;
    } else {
        bApplyBeforeOutbound = bool(sets.contains(QLatin1String("before-send-mail")));
        bApplyOnInbound = bool(sets.contains(QLatin1String("check-mail")));
        bApplyOnOutbound = bool(sets.contains(QLatin1String("send-mail")));
        bApplyOnExplicit = bool(sets.contains(QLatin1String("manual-filtering")));
        mApplicability = (AccountType) config.readEntry(
                    "Applicability", (int)ButImap );
    }

    bStopProcessingHere = config.readEntry( "StopProcessingHere", true );
    bConfigureShortcut = config.readEntry( "ConfigureShortcut", false );
    QString shortcut( config.readEntry( "Shortcut", QString() ) );
    if ( !shortcut.isEmpty() ) {
        KShortcut sc( shortcut );
        setShortcut( sc );
    }
    bConfigureToolbar = config.readEntry( "ConfigureToolbar", false );
    bConfigureToolbar = bConfigureToolbar && bConfigureShortcut;
    mToolbarName = config.readEntry( "ToolbarName", name() );
    mIcon = config.readEntry( "Icon", "system-run" );
    bAutoNaming = config.readEntry( "AutomaticName", false );
    bEnabled = config.readEntry( "Enabled", true );
    QString actName, argsName;

    mActions.clear();

    int numActions = config.readEntry( "actions", 0 );
    if (numActions > FILTER_MAX_ACTIONS) {
        numActions = FILTER_MAX_ACTIONS ;
        KMessageBox::information( 0, i18n("<qt>Too many filter actions in filter rule <b>%1</b>.</qt>", mPattern.name() ) );
    }

    for ( int i=0 ; i < numActions ; ++i ) {
        actName.sprintf("action-name-%d", i);
        argsName.sprintf("action-args-%d", i);
        // get the action description...
        FilterActionDesc *desc = FilterManager::filterActionDict()->value(
                    config.readEntry( actName, QString() ) );
        if ( desc ) {
            //...create an instance...
            FilterAction *fa = desc->create();
            if ( fa ) {
                //...load it with it's parameter...
                if ( interactive ) {
                    const bool ret = fa->argsFromStringInteractive( config.readEntry( argsName, QString() ), name() );
                    if ( ret )
                        needUpdate = true;
                }
                else
                    fa->argsFromString( config.readEntry( argsName, QString() ) );
                //...check if it's empty and...
                if ( !fa->isEmpty() )
                    //...append it if it's not and...
                    mActions.append( fa );
                else
                    //...delete is else.
                    delete fa;
            }
        } else
            KMessageBox::information( 0 /* app-global modal dialog box */,
                                      i18n("<qt>Unknown filter action <b>%1</b><br />in filter rule <b>%2</b>.<br />Ignoring it.</qt>",
                                           config.readEntry( actName, QString() ),
                                           mPattern.name() ) );
    }

    mAccounts = config.readEntry( "accounts-set",QStringList() );
    if ( !mAccounts.isEmpty() && interactive ) {
        if ( !FilterActionMissingAccountDialog::allAccountExist( mAccounts ) ) {
            FilterActionMissingAccountDialog *dlg = new FilterActionMissingAccountDialog(mAccounts, name());
            if ( dlg->exec() ) {
                mAccounts = dlg->selectedAccount();
                needUpdate = true;
            }
            delete dlg;
        }
    }
    return needUpdate;
}

void MailFilter::generateSieveScript(QStringList &requires, QString &code)
{
    mPattern.generateSieveScript(requires, code);

    QList<FilterAction*>::const_iterator it;
    QList<FilterAction*>::const_iterator end( mActions.constEnd() );

    code += QLatin1String(")\n{\n");
    bool firstAction = true;
    for ( it = mActions.constBegin() ; it != end ; ++it) {
        //Add endline here.
        if (firstAction) {
            firstAction = false;
        } else {
            code += QLatin1Char('\n');
        }
        code += QLatin1String("    ") + (*it)->sieveCode();
        Q_FOREACH(const QString &str, (*it)->sieveRequires()) {
            if (!requires.contains(str)) {
                requires.append(str);
            }
        }
    }
    code += QLatin1String("\n}\n");
}

void MailFilter::writeConfig(KConfigGroup & config, bool exportFilter) const
{
    mPattern.writeConfig(config);
    config.writeEntry( "identifier", mIdentifier );

    QStringList sets;
    if ( bApplyOnInbound )
        sets.append( QLatin1String("check-mail") );
    if ( bApplyBeforeOutbound )
        sets.append( QLatin1String("before-send-mail") );
    if ( bApplyOnOutbound )
        sets.append( QLatin1String("send-mail") );
    if ( bApplyOnExplicit )
        sets.append( QLatin1String("manual-filtering") );
    config.writeEntry( "apply-on", sets );

    config.writeEntry( "StopProcessingHere", bStopProcessingHere );
    config.writeEntry( "ConfigureShortcut", bConfigureShortcut );
    if ( !mShortcut.isEmpty() )
        config.writeEntry( "Shortcut", mShortcut.toString() );
    config.writeEntry( "ConfigureToolbar", bConfigureToolbar );
    config.writeEntry( "ToolbarName", mToolbarName );
    if ( !mIcon.isEmpty() )
        config.writeEntry( "Icon", mIcon );
    config.writeEntry( "AutomaticName", bAutoNaming );
    config.writeEntry( "Applicability", (int)mApplicability );
    config.writeEntry( "Enabled", bEnabled );
    QString key;
    int i;

    QList<FilterAction*>::const_iterator it;
    QList<FilterAction*>::const_iterator end( mActions.constEnd() );

    for ( i=0, it = mActions.constBegin() ; it != end ; ++it, ++i ) {
        config.writeEntry( key.sprintf("action-name-%d", i),
                           (*it)->name() );
        config.writeEntry( key.sprintf("action-args-%d", i),
                           exportFilter ? ( *it )->argsAsStringReal() :  (*it)->argsAsString() );
    }
    config.writeEntry( "actions", i );
    if ( !mAccounts.isEmpty() )
        config.writeEntry( "accounts-set", mAccounts );
}

void MailFilter::purify()
{
    mPattern.purify();

    QListIterator<FilterAction*> it( mActions );
    it.toBack();
    while ( it.hasPrevious() ) {
        FilterAction *action = it.previous();
        if ( action->isEmpty() )
            mActions.removeAll ( action );
    }

    if ( !Akonadi::AgentManager::self()->instances().isEmpty() ) { // safety test to ensure that Akonadi system is ready

        // Remove invalid accounts from mAccounts - just to be tidy
        QStringList::Iterator it2 = mAccounts.begin();
        while ( it2 != mAccounts.end() ) {
            if ( !Akonadi::AgentManager::self()->instance( *it2 ).isValid() )
                it2 = mAccounts.erase( it2 );
            else
                ++it2;
        }
    }
}

bool MailFilter::isEmpty() const
{
    return ( mPattern.isEmpty() && mActions.isEmpty() ) ||
            ( ( applicability() == Checked ) && mAccounts.isEmpty() );
}

QString MailFilter::toolbarName() const
{
    if ( mToolbarName.isEmpty() )
        return name();
    else
        return mToolbarName;
}

#ifndef NDEBUG
const QString MailFilter::asString() const
{
    QString result;

    result += "Filter name: " + name() + " (" + mIdentifier  + ")\n";
    result += mPattern.asString() + '\n';

    result += QString("Filter is %1\n").arg(bEnabled ? QLatin1String("enabled") : QLatin1String("disabled"));

    QList<FilterAction*>::const_iterator it( mActions.constBegin() );
    QList<FilterAction*>::const_iterator end( mActions.constEnd() );
    for ( ; it != end ; ++it ) {
        result += "    action: ";
        result += (*it)->label();
        result += ' ';
        result += (*it)->argsAsString();
        result += '\n';
    }
    result += "This filter belongs to the following sets:";
    if ( bApplyOnInbound )
        result += " Inbound";
    if ( bApplyBeforeOutbound )
        result += " before-Outbound";
    if ( bApplyOnOutbound )
        result += " Outbound";
    if ( bApplyOnExplicit )
        result += " Explicit";
    result += '\n';
    if ( bApplyOnInbound && mApplicability == All ) {
        result += "This filter applies to all accounts.\n";
    } else if ( bApplyOnInbound && mApplicability == ButImap ) {
        result += "This filter applies to all but IMAP accounts.\n";
    } else if ( bApplyOnInbound ) {
        QStringList::ConstIterator it2;
        result += "This filter applies to the following accounts:";
        if ( mAccounts.isEmpty() )
            result += " None";
        else {
            for ( it2 = mAccounts.begin() ; it2 != mAccounts.end() ; ++it2 ) {
                if ( Akonadi::AgentManager::self()->instance( *it2 ).isValid() ) {
                    result += ' ' + Akonadi::AgentManager::self()->instance( *it2 ).name();
                }
            }
        }
        result += '\n';
    }
    if ( bStopProcessingHere )
        result += "If it matches, processing stops at this filter.\n";

    return result;
}
#endif

QDataStream& MailCommon::operator<<( QDataStream &stream, const MailCommon::MailFilter &filter )
{
    stream << filter.mIdentifier;
    stream << filter.mPattern.serialize();

    stream << filter.mActions.count();
    QListIterator<FilterAction*> it( filter.mActions );
    while ( it.hasNext() ) {
        const FilterAction *action = it.next();
        stream << action->name();
        stream << action->argsAsString();
    }

    stream << filter.mAccounts;
    stream << filter.mIcon;
    stream << filter.mToolbarName;
    stream << filter.mShortcut.primary() << filter.mShortcut.alternate();
    stream << filter.bApplyOnInbound;
    stream << filter.bApplyBeforeOutbound;
    stream << filter.bApplyOnOutbound;
    stream << filter.bApplyOnExplicit;
    stream << filter.bStopProcessingHere;
    stream << filter.bConfigureShortcut;
    stream << filter.bConfigureToolbar;
    stream << filter.bAutoNaming;
    stream << filter.mApplicability;
    stream << filter.bEnabled;

    return stream;
}

QDataStream& MailCommon::operator>>( QDataStream &stream, MailCommon::MailFilter &filter )
{
    QByteArray pattern;
    int numberOfActions;
    QKeySequence primary, alternate;
    bool bApplyOnInbound;
    bool bApplyBeforeOutbound;
    bool bApplyOnOutbound;
    bool bApplyOnExplicit;
    bool bStopProcessingHere;
    bool bConfigureShortcut;
    bool bConfigureToolbar;
    bool bAutoNaming;
    int applicability;
    bool bEnabled;

    stream >> filter.mIdentifier;
    stream >> pattern;

    stream >> numberOfActions;
    qDeleteAll(filter.mActions);
    filter.mActions.clear();

    for (int i = 0; i < numberOfActions; ++i) {
        QString actionName;
        QString actionArguments;

        stream >> actionName;
        stream >> actionArguments;

        FilterActionDesc *description = FilterManager::filterActionDict()->value( actionName );
        if ( description ) {
            FilterAction *filterAction = description->create();
            if ( filterAction ) {
                filterAction->argsFromString( actionArguments );
                filter.mActions.append( filterAction );
            }
        }
    }

    stream >> filter.mAccounts;
    stream >> filter.mIcon;
    stream >> filter.mToolbarName;
    stream >> primary >> alternate;
    stream >> bApplyOnInbound;
    stream >> bApplyBeforeOutbound;
    stream >> bApplyOnOutbound;
    stream >> bApplyOnExplicit;
    stream >> bStopProcessingHere;
    stream >> bConfigureShortcut;
    stream >> bConfigureToolbar;
    stream >> bAutoNaming;
    stream >> applicability;
    stream >> bEnabled;

    filter.mPattern.deserialize(pattern);
    filter.mShortcut = KShortcut( primary, alternate );
    filter.bApplyOnInbound = bApplyOnInbound;
    filter.bApplyBeforeOutbound = bApplyBeforeOutbound;
    filter.bApplyOnOutbound = bApplyOnOutbound;
    filter.bApplyOnExplicit = bApplyOnExplicit;
    filter.bStopProcessingHere = bStopProcessingHere;
    filter.bConfigureShortcut = bConfigureShortcut;
    filter.bConfigureToolbar = bConfigureToolbar;
    filter.bAutoNaming = bAutoNaming;
    filter.bEnabled = bEnabled;
    filter.mApplicability = static_cast<MailCommon::MailFilter::AccountType>( applicability );

    return stream;
}


bool MailFilter::isEnabled() const
{
    return bEnabled;
}

void MailFilter::setEnabled( bool enabled )
{
    bEnabled = enabled;
}
