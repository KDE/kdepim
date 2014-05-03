/******************************************************************************
 *
 *  Copyright 2009 Thomas McGuire <mcguire@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *******************************************************************************/
#include "core/sortorder.h"

#include "messagelistutil.h"

#include <KLocalizedString>

#include <QMetaEnum>

using namespace MessageList::Core;

SortOrder::SortOrder()
    : mMessageSorting( SortMessagesByDateTime ),
      mMessageSortDirection( Descending ),
      mGroupSorting( NoGroupSorting ),
      mGroupSortDirection( Ascending )
{
}

QList< QPair< QString, int > > SortOrder::enumerateMessageSortingOptions( Aggregation::Threading t )
{
    QList< QPair< QString, int > > ret;
    ret.append( QPair< QString, int >( i18n( "None (Storage Order)" ), SortOrder::NoMessageSorting ) );
    ret.append( QPair< QString, int >( i18n( "By Date/Time" ), SortOrder::SortMessagesByDateTime ) );
    if ( t != Aggregation::NoThreading )
        ret.append( QPair< QString, int >( i18n( "By Date/Time of Most Recent in Subtree" ), SortOrder::SortMessagesByDateTimeOfMostRecent ) );
    ret.append( QPair< QString, int >( i18n( "By Sender" ), SortOrder::SortMessagesBySender ) );
    ret.append( QPair< QString, int >( i18n( "By Receiver" ), SortOrder::SortMessagesByReceiver ) );
    ret.append( QPair< QString, int >( i18n( "By Smart Sender/Receiver" ), SortOrder::SortMessagesBySenderOrReceiver ) );
    ret.append( QPair< QString, int >( i18n( "By Subject" ), SortOrder::SortMessagesBySubject ) );
    ret.append( QPair< QString, int >( i18n( "By Size" ), SortOrder::SortMessagesBySize ) );
    ret.append( QPair< QString, int >( i18n( "By Action Item Status" ), SortOrder::SortMessagesByActionItemStatus ) );
    ret.append( QPair< QString, int >( i18n( "By Unread Status" ), SortOrder::SortMessagesByUnreadStatus ) );
    ret.append( QPair< QString, int >( i18n( "By Important Status" ), SortOrder::SortMessagesByImportantStatus ) );
    return ret;
}

QList< QPair< QString, int > > SortOrder::enumerateMessageSortDirectionOptions( MessageSorting ms )
{
    QList< QPair< QString, int > > ret;
    if ( ms == SortOrder::NoMessageSorting )
        return ret;

    if (
            ( ms == SortOrder::SortMessagesByDateTime ) ||
            ( ms == SortOrder::SortMessagesByDateTimeOfMostRecent )
            )
    {
        ret.append( QPair< QString, int >( i18n( "Least Recent on Top" ), SortOrder::Ascending ) );
        ret.append( QPair< QString, int >( i18n( "Most Recent on Top" ), SortOrder::Descending ) );
        return ret;
    }

    ret.append( QPair< QString, int >( i18nc( "Sort order for messages", "Ascending" ), SortOrder::Ascending ) );
    ret.append( QPair< QString, int >( i18nc( "Sort order for messages", "Descending" ), SortOrder::Descending ) );
    return ret;
}


QList< QPair< QString, int > > SortOrder::enumerateGroupSortingOptions( Aggregation::Grouping g )
{
    QList< QPair< QString, int > > ret;
    if ( g == Aggregation::NoGrouping )
        return ret;
    if ( ( g == Aggregation::GroupByDate ) || ( g == Aggregation::GroupByDateRange ) )
        ret.append( QPair< QString, int >( i18n( "by Date/Time" ), SortOrder::SortGroupsByDateTime ) );
    else {
        ret.append( QPair< QString, int >( i18n( "None (Storage Order)" ), SortOrder::NoGroupSorting ) );
        ret.append( QPair< QString, int >( i18n( "by Date/Time of Most Recent Message in Group" ), SortOrder::SortGroupsByDateTimeOfMostRecent ) );
    }

    if ( g == Aggregation::GroupBySenderOrReceiver )
        ret.append( QPair< QString, int >( i18n( "by Sender/Receiver" ), SortOrder::SortGroupsBySenderOrReceiver ) );
    else if ( g == Aggregation::GroupBySender )
        ret.append( QPair< QString, int >( i18n( "by Sender" ), SortOrder::SortGroupsBySender ) );
    else if ( g == Aggregation::GroupByReceiver )
        ret.append( QPair< QString, int >( i18n( "by Receiver" ), SortOrder::SortGroupsByReceiver ) );

    return ret;
}

QList< QPair< QString, int > > SortOrder::enumerateGroupSortDirectionOptions( Aggregation::Grouping g,
                                                                              GroupSorting gs )
{
    QList< QPair< QString, int > > ret;
    if ( g == Aggregation::NoGrouping || gs == SortOrder::NoGroupSorting)
        return ret;

    if ( gs == SortOrder::SortGroupsByDateTimeOfMostRecent )
    {
        ret.append( QPair< QString, int >( i18n( "Least Recent on Top" ), SortOrder::Ascending ) );
        ret.append( QPair< QString, int >( i18n( "Most Recent on Top" ), SortOrder::Descending ) );
        return ret;
    }
    ret.append( QPair< QString, int >( i18nc( "Sort order for mail groups", "Ascending" ), SortOrder::Ascending ) );
    ret.append( QPair< QString, int >( i18nc( "Sort order for mail groups", "Descending" ), SortOrder::Descending ) );
    return ret;
}

typedef QPair< QString, int > Pair;
typedef QList< Pair > OptionList;
static bool optionListHasOption( const OptionList &optionList, int optionValue,
                                 int defaultOptionValue )
{
    foreach( const Pair &pair, optionList ) {
        if ( pair.second == optionValue ) {
            return true;
        }
    }
    if ( optionValue != defaultOptionValue )
        return false;
    else return true;
}

bool SortOrder::validForAggregation( const Aggregation *aggregation ) const
{
    OptionList messageSortings = enumerateMessageSortingOptions( aggregation->threading() );
    OptionList messageSortDirections = enumerateMessageSortDirectionOptions( mMessageSorting );
    OptionList groupSortings = enumerateGroupSortingOptions( aggregation->grouping() );
    OptionList groupSortDirections = enumerateGroupSortDirectionOptions( aggregation->grouping(),
                                                                         mGroupSorting );
    SortOrder defaultSortOrder = defaultForAggregation( aggregation, SortOrder() );
    bool messageSortingOk = optionListHasOption( messageSortings,
                                                 mMessageSorting, defaultSortOrder.messageSorting() );
    bool messageSortDirectionOk = optionListHasOption( messageSortDirections, mMessageSortDirection,
                                                       defaultSortOrder.messageSortDirection() );

    bool groupSortingOk = optionListHasOption( groupSortings, mGroupSorting,
                                               defaultSortOrder.groupSorting() );
    bool groupSortDirectionOk = optionListHasOption( groupSortDirections, mGroupSortDirection,
                                                     defaultSortOrder.groupSortDirection() );
    return messageSortingOk && messageSortDirectionOk &&
            groupSortingOk && groupSortDirectionOk;
}

SortOrder SortOrder::defaultForAggregation( const Aggregation *aggregation,
                                            const SortOrder &oldSortOrder )
{
    SortOrder newSortOrder;

    //
    // First check if we can adopt the message sorting and the message sort direction from
    // the old sort order. This is mostly true, except, for example, when the old message sorting
    // was "by most recent in subtree", and the aggregation doesn't use threading.
    //
    OptionList messageSortings = enumerateMessageSortingOptions( aggregation->threading() );
    bool messageSortingOk = optionListHasOption( messageSortings,
                                                 oldSortOrder.messageSorting(),
                                                 SortOrder().messageSorting() );
    bool messageSortDirectionOk = false;
    if ( messageSortingOk ) {
        OptionList messageSortDirections = enumerateMessageSortDirectionOptions(
                    oldSortOrder.messageSorting() );
        messageSortDirectionOk = optionListHasOption( messageSortDirections, oldSortOrder.messageSortDirection(),
                                                      SortOrder().messageSortDirection() );
    }

    //
    // Ok, if we can partly adopt the old sort order, set the values now.
    //
    if ( messageSortingOk )
        newSortOrder.setMessageSorting( oldSortOrder.messageSorting() );
    else
        newSortOrder.setMessageSorting( SortMessagesByDateTime );
    if ( messageSortDirectionOk )
        newSortOrder.setMessageSortDirection( oldSortOrder.messageSortDirection() );
    else
        newSortOrder.setMessageSortDirection( Descending );

    //
    // Now set the group sorting and group sort direction, depending on the aggregation.
    //
    Aggregation::Grouping grouping = aggregation->grouping();
    if ( grouping == Aggregation::GroupByDate ||
         grouping == Aggregation::GroupByDateRange ) {
        newSortOrder.setGroupSortDirection( Descending );
        newSortOrder.setGroupSorting( SortGroupsByDateTime );

    }
    else if ( grouping == Aggregation::GroupByReceiver || grouping == Aggregation::GroupBySender ||
              grouping == Aggregation::GroupBySenderOrReceiver ) {
        newSortOrder.setGroupSortDirection( Descending );
        switch ( grouping ) {
        case Aggregation::GroupByReceiver:
            newSortOrder.setGroupSorting( SortGroupsByReceiver ); break;
        case Aggregation::GroupBySender:
            newSortOrder.setGroupSorting( SortGroupsBySender ); break;
        case Aggregation::GroupBySenderOrReceiver:
            newSortOrder.setGroupSorting( SortGroupsBySenderOrReceiver ); break;
        default: break;
        }
    }

    return newSortOrder;
}

bool SortOrder::readConfigHelper( KConfigGroup &conf, const QString &id )
{
    if ( !conf.hasKey( id + MessageList::Util::messageSortingConfigName() ) )
        return false;
    mMessageSorting = messageSortingForName(
                conf.readEntry( id + MessageList::Util::messageSortingConfigName() ) );
    mMessageSortDirection = sortDirectionForName(
                conf.readEntry( id + MessageList::Util::messageSortDirectionConfigName() ) );
    mGroupSorting = groupSortingForName(
                conf.readEntry( id + MessageList::Util::groupSortingConfigName() ) );
    mGroupSortDirection = sortDirectionForName(
                conf.readEntry( id + MessageList::Util::groupSortDirectionConfigName() ) );
    return true;
}

void SortOrder::readConfig( KConfigGroup &conf, const QString &storageId,
                            bool *storageUsesPrivateSortOrder )
{
    SortOrder privateSortOrder, globalSortOrder;
    globalSortOrder.readConfigHelper( conf, QLatin1String( "GlobalSortOrder" ) );
    *storageUsesPrivateSortOrder = privateSortOrder.readConfigHelper( conf, storageId );
    if ( *storageUsesPrivateSortOrder )
        *this = privateSortOrder;
    else
        *this = globalSortOrder;

}

void SortOrder::writeConfig( KConfigGroup &conf, const QString &storageId,
                             bool storageUsesPrivateSortOrder ) const
{
    QString id = storageId;
    if ( !storageUsesPrivateSortOrder ) {
        id = QLatin1String( "GlobalSortOrder" );
        conf.deleteEntry( storageId + MessageList::Util::messageSortingConfigName() );
        conf.deleteEntry( storageId + MessageList::Util::messageSortDirectionConfigName() );
        conf.deleteEntry( storageId + MessageList::Util::groupSortingConfigName() );
        conf.deleteEntry( storageId + MessageList::Util::groupSortDirectionConfigName() );
    }

    conf.writeEntry( id + MessageList::Util::messageSortingConfigName(),
                     nameForMessageSorting( mMessageSorting ) );
    conf.writeEntry( id + MessageList::Util::messageSortDirectionConfigName(),
                     nameForSortDirection( mMessageSortDirection ) );
    conf.writeEntry( id + MessageList::Util::groupSortingConfigName(),
                     nameForGroupSorting( mGroupSorting ) );
    conf.writeEntry( id + MessageList::Util::groupSortDirectionConfigName(),
                     nameForSortDirection( mGroupSortDirection ) );

}

bool SortOrder::isValidMessageSorting( SortOrder::MessageSorting ms )
{
    switch( ms )
    {
    case SortOrder::NoMessageSorting:
    case SortOrder::SortMessagesByDateTime:
    case SortOrder::SortMessagesByDateTimeOfMostRecent:
    case SortOrder::SortMessagesBySenderOrReceiver:
    case SortOrder::SortMessagesBySender:
    case SortOrder::SortMessagesByReceiver:
    case SortOrder::SortMessagesBySubject:
    case SortOrder::SortMessagesBySize:
    case SortOrder::SortMessagesByActionItemStatus:
    case SortOrder::SortMessagesByUnreadStatus:
    case SortOrder::SortMessagesByImportantStatus:
        // ok
        break;
    default:
        // b0rken
        return false;
        break;
    }

    return true;
}

const QString SortOrder::nameForSortDirection( SortDirection sortDirection )
{
    int index = staticMetaObject.indexOfEnumerator( "SortDirection" );
    return QLatin1String( staticMetaObject.enumerator( index ).valueToKey( sortDirection ) );
}

const QString SortOrder::nameForMessageSorting( MessageSorting messageSorting )
{
    int index = staticMetaObject.indexOfEnumerator( "MessageSorting" );
    return QLatin1String( staticMetaObject.enumerator( index ).valueToKey( messageSorting ) );
}

const QString SortOrder::nameForGroupSorting( GroupSorting groupSorting )
{
    int index = staticMetaObject.indexOfEnumerator( "GroupSorting" );
    return QLatin1String( staticMetaObject.enumerator( index ).valueToKey( groupSorting  ) );
}

SortOrder::SortDirection SortOrder::sortDirectionForName( const QString& name )
{
    int index = staticMetaObject.indexOfEnumerator( "SortDirection" );
    return static_cast<SortDirection>( staticMetaObject.enumerator( index ).keyToValue(
                                           name.toLatin1().constData() ) );
}

SortOrder::MessageSorting SortOrder::messageSortingForName( const QString& name )
{
    int index = staticMetaObject.indexOfEnumerator( "MessageSorting" );
    return static_cast<MessageSorting>( staticMetaObject.enumerator( index ).keyToValue(
                                            name.toLatin1().constData() ) );
}

SortOrder::GroupSorting SortOrder::groupSortingForName( const QString& name )
{
    int index = staticMetaObject.indexOfEnumerator( "GroupSorting" );
    return static_cast<GroupSorting>( staticMetaObject.enumerator( index ).keyToValue(
                                          name.toLatin1().constData() ) );
}

#include "moc_sortorder.cpp"
