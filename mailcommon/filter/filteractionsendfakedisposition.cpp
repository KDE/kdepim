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

#include "filteractionsendfakedisposition.h"

#include <messagecore/misc/mdnstateattribute.h>

#include <KDE/KLocale>

#include <QTextDocument>

using namespace MailCommon;

// if you change this list, also update
// the count in argsFromString
static const KMime::MDN::DispositionType mdns[] =
{
  KMime::MDN::Displayed,
  KMime::MDN::Deleted,
  KMime::MDN::Dispatched,
  KMime::MDN::Processed,
  KMime::MDN::Denied,
  KMime::MDN::Failed,
};
static const int numMDNs = sizeof( mdns ) / sizeof( *mdns );


FilterActionSendFakeDisposition::FilterActionSendFakeDisposition( QObject *parent )
  : FilterActionWithStringList( QLatin1String("fake mdn"), i18n( "Send Fake MDN" ), parent )
{
  // if you change this list, also update
  // mdns above
    mParameterList.append( QString() );
  mParameterList.append( i18nc( "MDN type", "Ignore" ) );
  mParameterList.append( i18nc( "MDN type", "Displayed" ) );
  mParameterList.append( i18nc( "MDN type", "Deleted" ) );
  mParameterList.append( i18nc( "MDN type", "Dispatched" ) );
  mParameterList.append( i18nc( "MDN type", "Processed" ) );
  mParameterList.append( i18nc( "MDN type", "Denied" ) );
  mParameterList.append( i18nc( "MDN type", "Failed" ) );

  mParameter = mParameterList.at( 0 );
}

FilterAction* FilterActionSendFakeDisposition::newAction()
{
  return new FilterActionSendFakeDisposition;
}

bool FilterActionSendFakeDisposition::isEmpty() const
{
  return false;
}

FilterAction::ReturnCode FilterActionSendFakeDisposition::process( ItemContext &context ) const
{
  const int index = mParameterList.indexOf( mParameter );
  if ( index < 1 )
    return ErrorButGoOn;

  if ( index == 1 ) { // ignore
    if ( context.item().hasAttribute<MessageCore::MDNStateAttribute>() ) {
      context.item().attribute<MessageCore::MDNStateAttribute>()->setMDNState( MessageCore::MDNStateAttribute::MDNIgnore );
      context.setNeedsFlagStore();
    }
  } else // send
    sendMDN( context.item(), mdns[ index - 2 ] ); // skip first two entries: "" and "ignore"

  return GoOn;
}

SearchRule::RequiredPart FilterActionSendFakeDisposition::requiredPart() const
{
  return SearchRule::CompleteMessage;
}


void FilterActionSendFakeDisposition::argsFromString( const QString &argsStr )
{
  if ( argsStr.length() == 1 ) {
    if ( argsStr[ 0 ] == 'I' ) { // ignore
      mParameter = mParameterList.at( 1 );
      return;
    }

    for ( int i = 0 ; i < numMDNs ; ++i ) {
      if ( char( mdns[ i ] ) == argsStr[ 0 ] ) { // send
        mParameter = mParameterList.at( i + 2 );
        return;
      }
    }
  }

  mParameter = mParameterList.at( 0 );
}

QString FilterActionSendFakeDisposition::argsAsString() const
{
  const int index = mParameterList.indexOf( mParameter );
  if ( index < 1 )
    return QString();

  return QString( QChar( index < 2 ? 'I' : char( mdns[ index - 2 ] ) ) );
}

QString FilterActionSendFakeDisposition::displayString() const
{
  return label() + QLatin1String( " \"" ) + mParameter + QLatin1String( "\"" );
}


#include "filteractionsendfakedisposition.moc"
