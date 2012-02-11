/*
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

#include "filteractionsetstatus.h"

#include <KDE/Akonadi/KMime/MessageStatus>
#include <KDE/KLocale>

#include <QtGui/QTextDocument>

using namespace MailCommon;

static const Akonadi::MessageStatus stati[] =
{
  Akonadi::MessageStatus::statusImportant(),
  Akonadi::MessageStatus::statusRead(),
  Akonadi::MessageStatus::statusUnread(),
  Akonadi::MessageStatus::statusReplied(),
  Akonadi::MessageStatus::statusForwarded(),
  Akonadi::MessageStatus::statusWatched(),
  Akonadi::MessageStatus::statusIgnored(),
  Akonadi::MessageStatus::statusSpam(),
  Akonadi::MessageStatus::statusHam(),
  Akonadi::MessageStatus::statusToAct()
};

static const int StatiCount = sizeof( stati ) / sizeof( Akonadi::MessageStatus );

FilterAction* FilterActionSetStatus::newAction()
{
  return new FilterActionSetStatus;
}

FilterActionSetStatus::FilterActionSetStatus( QObject *parent )
  : FilterActionWithStringList( "set status", i18n( "Mark As" ), parent )
{
  // if you change this list, also update
  // FilterActionSetStatus::stati above
  mParameterList.append( "" );
  mParameterList.append( i18nc( "msg status", "Important" ) );
  mParameterList.append( i18nc( "msg status", "Read" ) );
  mParameterList.append( i18nc( "msg status", "Unread" ) );
  mParameterList.append( i18nc( "msg status", "Replied" ) );
  mParameterList.append( i18nc( "msg status", "Forwarded" ) );
  mParameterList.append( i18nc( "msg status", "Watched" ) );
  mParameterList.append( i18nc( "msg status", "Ignored" ) );
  mParameterList.append( i18nc( "msg status", "Spam" ) );
  mParameterList.append( i18nc( "msg status", "Ham" ) );
  mParameterList.append( i18nc( "msg status", "Action Item" ) );

  mParameter = mParameterList.at( 0 );
}

bool FilterActionSetStatus::isEmpty() const
{
  return false;
}

FilterAction::ReturnCode FilterActionSetStatus::process( ItemContext &context ) const
{
  const int index = mParameterList.indexOf( mParameter );
  if ( index < 1 )
    return ErrorButGoOn;

  Akonadi::MessageStatus status;
  status.setStatusFromFlags( context.item().flags() );

  const Akonadi::MessageStatus newStatus = stati[ index - 1 ];
  if ( newStatus == Akonadi::MessageStatus::statusUnread() )
    status.setRead( false );
  else
    status.set( newStatus );

  context.item().setFlags( status.statusFlags() );
  context.setNeedsFlagStore();

  return GoOn;
}

bool FilterActionSetStatus::requiresBody() const
{
  return false;
}

static QString realStatusString( const QString &statusStr )
{
  QString result( statusStr );

  if ( result.size() == 2 )
    result.remove( QLatin1Char( 'U' ) );

  return result;
}


void FilterActionSetStatus::argsFromString( const QString &argsStr )
{
  if ( argsStr.length() == 1 ) {
    Akonadi::MessageStatus status;

    for ( int i = 0 ; i < StatiCount ; ++i ) {
      status = stati[i];
      if ( realStatusString( status.statusStr() ) == argsStr.toLatin1() ) {
        mParameter = mParameterList.at( i + 1 );
        return;
      }
    }
  }

  mParameter = mParameterList.at( 0 );
}

QString FilterActionSetStatus::argsAsString() const
{
  const int index = mParameterList.indexOf( mParameter );
  if ( index < 1 )
    return QString();

  return realStatusString( stati[index - 1].statusStr() );
}

QString FilterActionSetStatus::displayString() const
{
  return label() + QLatin1String( " \"" ) + Qt::escape( argsAsString() ) + QLatin1String( "\"" );
}


