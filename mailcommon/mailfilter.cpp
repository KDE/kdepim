/* -*- mode: C++; c-file-style: "gnu" -*-
 * kmail: KDE mail client
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
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
#include "mailutil.h"
#include "filterlog.h"
#include "mailkernel.h"
using MailCommon::FilterLog;

// KDEPIMLIBS headers
#include <Akonadi/AgentManager>

// other KDE headers
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kconfiggroup.h>

// other headers
#include <assert.h>

using namespace MailCommon;

MailFilter::MailFilter( bool popFilter )
  : bPopFilter(popFilter)
{
  if ( bPopFilter )
    mAction = Down;
  else {
    bApplyOnInbound = true;
    bApplyBeforeOutbound = false;
    bApplyOnOutbound = false;
    bApplyOnExplicit = true;
    bStopProcessingHere = true;
    bConfigureShortcut = false;
    bConfigureToolbar = false;
    bAutoNaming = true;
    mApplicability = All;
  }
}


MailFilter::MailFilter( KConfigGroup & aConfig, bool popFilter )
  : bPopFilter(popFilter)
{
  readConfig( aConfig );
}


MailFilter::MailFilter( const MailFilter & aFilter )
{
  bPopFilter = aFilter.isPopFilter();

  mPattern = aFilter.mPattern;

  if ( bPopFilter ){
    mAction = aFilter.mAction;
  } else {
    bApplyOnInbound = aFilter.applyOnInbound();
    bApplyBeforeOutbound = aFilter.applyBeforeOutbound();
    bApplyOnOutbound = aFilter.applyOnOutbound();
    bApplyOnExplicit = aFilter.applyOnExplicit();
    bStopProcessingHere = aFilter.stopProcessingHere();
    bConfigureShortcut = aFilter.configureShortcut();
    bConfigureToolbar = aFilter.configureToolbar();
    mToolbarName = aFilter.toolbarName();
    mApplicability = aFilter.applicability();
    mIcon = aFilter.icon();
    mShortcut = aFilter.shortcut();

    QListIterator<FilterAction*> it( aFilter.mActions );
    while ( it.hasNext() ) {
      FilterAction *action = it.next();
      FilterActionDesc *desc = FilterIf->filterActionDict()->value( action->name() );
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
    for ( it2 = aFilter.mAccounts.begin() ; it2 != aFilter.mAccounts.end() ; ++it2 )
      mAccounts.append( *it2 );
  }
}

MailFilter::~MailFilter()
{
  if ( !bPopFilter ) {
    qDeleteAll( mActions );
  }
}

// only for !bPopFilter
MailFilter::ReturnCode MailFilter::execActions( const Akonadi::Item &item, bool& stopIt ) const
{
  ReturnCode status = NoResult;

  QList<FilterAction*>::const_iterator it( mActions.begin() );
  for ( ; it != mActions.end() ; ++it ) {

    if ( FilterLog::instance()->isLogging() ) {
      const QString logText( i18n( "<b>Applying filter action:</b> %1",
                         (*it)->displayString() ) );
      FilterLog::instance()->add( logText, FilterLog::appliedAction );
    }

    FilterAction::ReturnCode result = (*it)->process( item );

    switch ( result ) {
    case FilterAction::CriticalError:
      if ( FilterLog::instance()->isLogging() ) {
        const QString logText = QString( "<font color=#FF0000>%1</font>" )
          .arg( i18n( "A critical error occurred. Processing stops here." ) );
        FilterLog::instance()->add( logText, FilterLog::appliedAction );
      }
      // in case it's a critical error: return immediately!
      return CriticalError;
    case FilterAction::ErrorButGoOn:
      if ( FilterLog::instance()->isLogging() ) {
        const QString logText = QString( "<font color=#FF0000>%1</font>" )
          .arg( i18n( "A problem was found while applying this action." ) );
        FilterLog::instance()->add( logText, FilterLog::appliedAction );
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

bool MailFilter::requiresBody()
{
  if (pattern() && pattern()->requiresBody())
    return true; // no pattern means always matches?
  QListIterator<FilterAction*> it( *actions() );
  while ( it.hasNext() )
    if ( it.next()->requiresBody() )
      return true;
  return false;
}

/** No descriptions */
// only for bPopFilter
void MailFilter::setAction(const PopFilterAction aAction)
{
  mAction = aAction;
}

// only for bPopFilter
PopFilterAction MailFilter::action()
{
  return mAction;
}

// only for !bPopFilter
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


//-----------------------------------------------------------------------------
void MailFilter::readConfig(KConfigGroup & config)
{
  // MKSearchPattern::readConfig ensures
  // that the pattern is purified.
  mPattern.readConfig(config);

  if (bPopFilter) {
    // get the action description...
    QString action = config.readEntry( "action" );
    if ( action == "down" )
      mAction = Down;
    else if ( action == "later" )
      mAction = Later;
    else if ( action == "delete" )
      mAction = Delete;
    else
      mAction = NoAction;
  }
  else {
    QStringList sets = config.readEntry("apply-on", QStringList() );
    if ( sets.isEmpty() && !config.hasKey("apply-on") ) {
      bApplyBeforeOutbound = false;
      bApplyOnOutbound = false;
      bApplyOnInbound = true;
      bApplyOnExplicit = true;
      mApplicability = ButImap;
    } else {
      bApplyBeforeOutbound = bool(sets.contains("before-send-mail"));
      bApplyOnInbound = bool(sets.contains("check-mail"));
      bApplyOnOutbound = bool(sets.contains("send-mail"));
      bApplyOnExplicit = bool(sets.contains("manual-filtering"));
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

    QString actName, argsName;

    mActions.clear();

    int numActions = config.readEntry( "actions", 0 );
    if (numActions > FILTER_MAX_ACTIONS) {
      numActions = FILTER_MAX_ACTIONS ;
      KMessageBox::information( 0, i18n("<qt>Too many filter actions in filter rule <b>%1</b>.</qt>", mPattern.name() ) );
    }

    for ( int i=0 ; i < numActions ; i++ ) {
      actName.sprintf("action-name-%d", i);
      argsName.sprintf("action-args-%d", i);
      // get the action description...
      FilterActionDesc *desc = FilterIf->filterActionDict()->value(
            config.readEntry( actName, QString() ) );
      if ( desc ) {
        //...create an instance...
        FilterAction *fa = desc->create();
        if ( fa ) {
          //...load it with it's parameter...
          fa->argsFromString( config.readEntry( argsName, QString() ) );
          //...check if it's emoty and...
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
  }
}


void MailFilter::writeConfig(KConfigGroup & config) const
{
  mPattern.writeConfig(config);

  if (bPopFilter) {
    switch ( mAction ) {
    case Down:
      config.writeEntry( "action", "down" );
      break;
    case Later:
      config.writeEntry( "action", "later" );
      break;
    case Delete:
      config.writeEntry( "action", "delete" );
      break;
    default:
      config.writeEntry( "action", "" );
    }
  } else {
    QStringList sets;
    if ( bApplyOnInbound )
      sets.append( "check-mail" );
    if ( bApplyBeforeOutbound )
      sets.append( "before-send-mail" );
    if ( bApplyOnOutbound )
      sets.append( "send-mail" );
    if ( bApplyOnExplicit )
      sets.append( "manual-filtering" );
    config.writeEntry( "apply-on", sets );

    config.writeEntry( "StopProcessingHere", bStopProcessingHere );
    config.writeEntry( "ConfigureShortcut", bConfigureShortcut );
    if ( !mShortcut.isEmpty() )
      config.writeEntry( "Shortcut", mShortcut.toString() );
    config.writeEntry( "ConfigureToolbar", bConfigureToolbar );
    config.writeEntry( "ToolbarName", mToolbarName );
    config.writeEntry( "Icon", mIcon );
    config.writeEntry( "AutomaticName", bAutoNaming );
    config.writeEntry( "Applicability", (int)mApplicability );

    QString key;
    int i;

    QList<FilterAction*>::const_iterator it;
    for ( i=0, it = mActions.begin() ; it != mActions.end() ; ++it, ++i ) {
      config.writeEntry( key.sprintf("action-name-%d", i),
                          (*it)->name() );
      config.writeEntry( key.sprintf("action-args-%d", i),
                          (*it)->argsAsString() );
    }
    config.writeEntry( "actions", i );
    config.writeEntry( "accounts-set", mAccounts );
  }
}

void MailFilter::purify()
{
  mPattern.purify();

  if (!bPopFilter) {
    QListIterator<FilterAction*> it( mActions );
    it.toBack();
    while ( it.hasPrevious() ) {
      FilterAction *action = it.previous();
      if ( action->isEmpty() )
        mActions.removeAll ( action );
    }
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
  if (bPopFilter)
    return mPattern.isEmpty();
  else
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

  result += "Filter name: " + name() + '\n';
  result += mPattern.asString() + '\n';

  if (bPopFilter){
    result += "    action: ";
    result += mAction;
    result += '\n';
  }
  else {
    QList<FilterAction*>::const_iterator it( mActions.begin() );
    for ( ; it != mActions.end() ; ++it ) {
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
  }
  return result;
}
#endif
