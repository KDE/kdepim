/*
    Copyright (C) 2009    Dmitry Ivanov <vonami@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "rssdata.h"
#include "factories.h"

#include <krss/rssitem.h>
#include <Akonadi/LinkJob>
#include <Akonadi/Collection>
#include <akonadi/filter/datatype.h>
#include <akonadi/filter/functiondescriptor.h>
#include <KDebug>

using namespace KRss;
using Akonadi::Filter::FunctionDescriptor;
using Akonadi::Filter::DataMemberDescriptor;

RssData::RssData( const Akonadi::Item& item )
    : m_item( item )
{
}

bool RssData::executeCommand( const Akonadi::Filter::CommandDescriptor* command,
                              const QList<QVariant>& parameters )
{
    switch( command->id() ) {
        case RssIdentifiers::LinkCommand: {
            Q_ASSERT( parameters.count() == 1 );
            bool ok;
            const qlonglong id = parameters.first().toLongLong( &ok );
            if ( !ok ) {
                kWarning() << "Linking the item" << m_item.id()
                           << "failed: the target collection id is not valid";
                return false;
            }

            Akonadi::LinkJob* const job = new Akonadi::LinkJob( Akonadi::Collection( id ),
                                                                QList<Akonadi::Item>() << m_item );
            if ( !job->exec() ) {
                kWarning() << "Linking the item" << m_item.id() << "failed:" << job->errorString();
                return false;
            }

            return true;
        }
        default:
            // not my command: fall through
            Q_ASSERT_X( false, __FUNCTION__, "Unhandled command" );
    }

    Q_ASSERT_X( false, __FUNCTION__, "This point should be never reached" );
    return false;
}

QVariant RssData::getPropertyValue( const FunctionDescriptor* function, const DataMemberDescriptor* dataMember )
{
    if( !function ) {
        kDebug() << "*** ASKING FOR VALUE OF HEADER ***";
        return getDataMemberValue( dataMember ); // value of ("header" in sieve)
    }

    Q_ASSERT( function->acceptableInputDataTypeMask() & dataMember->dataType() );

    kDebug() << "data member id:" << dataMember->id();
    kDebug() << "function id:" << function->id();
    switch( function->id() ) {
      case Akonadi::Filter::FunctionValueOf:
          Q_ASSERT( dataMember->dataType() == Akonadi::Filter::DataTypeString );
          kDebug() << "Returning:" << getDataMemberValue( dataMember );
          return getDataMemberValue( dataMember );
      case Akonadi::Filter::FunctionSizeOf:
          Q_ASSERT( dataMember->id() == RssIdentifiers::Title ||
                    dataMember->id() == RssIdentifiers::Description ||
                    dataMember->id() == RssIdentifiers::Content );
          return getDataMemberValue( dataMember ).toString().length();
      case Akonadi::Filter::FunctionCountOf:
          Q_ASSERT( false );
      case Akonadi::Filter::FunctionExists:
          Q_ASSERT( dataMember->id() == RssIdentifiers::Title ||
                    dataMember->id() == RssIdentifiers::Description ||
                    dataMember->id() == RssIdentifiers::Content );
          return !getDataMemberValue( dataMember ).toString().isEmpty();
      case Akonadi::Filter::FunctionDateIn:
      default:
          Q_ASSERT( false );
    }

  return QVariant();
}

QVariant RssData::getDataMemberValue( const DataMemberDescriptor* dataMember )
{
    if ( !m_item.hasPayload<RssItem>() ) {
        return QVariant();
    }

    kDebug() << "data member id:" << dataMember->id();
    const RssItem rssItem = m_item.payload<RssItem>();
    if ( !rssItem.headersLoaded() || !rssItem.contentLoaded() ) {
        kDebug() << "Some parts of the item not loaded";
        return QVariant();
    }

    switch( dataMember->id() ) {
        case RssIdentifiers::Status:
            return 0;
        case RssIdentifiers::Title:
            return rssItem.title();
        case RssIdentifiers::Description:
            return rssItem.description();
        case RssIdentifiers::Content:
            return rssItem.content();
        default:
            return QVariant();
    }

    return QVariant();
}

