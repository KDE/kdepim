/* -*- mode: C++; c-file-style: "gnu" -*-

  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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

#include "filteractionstatus.h"
#include <KDE/Akonadi/KMime/MessageStatus>
#include <KDE/KLocale>
using namespace MailCommon;

Akonadi::MessageStatus MailCommon::FilterActionStatus::stati[] =
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

int MailCommon::FilterActionStatus::StatiCount = sizeof( MailCommon::FilterActionStatus::stati ) / sizeof( Akonadi::MessageStatus );

FilterActionStatus::FilterActionStatus(const QString &name, const QString &label, QObject *parent )
    : FilterActionWithStringList( name, label, parent )
{
    // if you change this list, also update
    // FilterActionSetStatus::stati above
    mParameterList.append(QString() );
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

SearchRule::RequiredPart FilterActionStatus::requiredPart() const
{
    return SearchRule::Envelope;
}

bool FilterActionStatus::isEmpty() const
{
    return false;
}

QString FilterActionStatus::realStatusString( const QString &statusStr )
{
    QString result( statusStr );

    if ( result.size() == 2 )
        result.remove( QLatin1Char( 'U' ) );

    return result;
}


void FilterActionStatus::argsFromString( const QString &argsStr )
{
    if ( argsStr.length() == 1 ) {
        Akonadi::MessageStatus status;

        for ( int i = 0 ; i < FilterActionStatus::StatiCount; ++i ) {
            status = stati[i];
            if ( realStatusString( status.statusStr() ) == QLatin1String(argsStr.toLatin1()) ) {
                mParameter = mParameterList.at( i + 1 );
                return;
            }
        }
    }

    mParameter = mParameterList.at( 0 );
}

QString FilterActionStatus::argsAsString() const
{
    const int index = mParameterList.indexOf( mParameter );
    if ( index < 1 )
        return QString();

    return realStatusString( FilterActionStatus::stati[index - 1].statusStr() );
}

QString FilterActionStatus::displayString() const
{
    return label() + QLatin1String( " \"" ) + mParameter + QLatin1String( "\"" );
}

