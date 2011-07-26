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

#include "rssfilteringagent.h"
#include "rssdata.h"
#include "filterlisteditor.h"

#include <krss/resourcemanager.h>
#include <krss/feedcollection.h>

#include <Akonadi/ItemFetchJob>
#include <akonadi/itemfetchscope.h>
#include <akonadi/attributefactory.h>
#include <akonadi/filter/agent.h>
#include <akonadi/filter/io/sieveencoder.h>
#include <akonadi/filter/io/sievedecoder.h>

#include <KDebug>
#include <KLocale>
#include <KStandardDirs>

#include <QFile>

using namespace KRss;

RssFilteringAgent::RssFilteringAgent( const QString& id )
    : PreprocessorBase( id ), m_componentFactory( new RssComponentFactory ),
      m_editorFactory( new RssEditorFactory )
{
    // register some stuff
    Akonadi::Filter::Agent::registerMetaTypes();
    ResourceManager::self()->registerAttributes();

    // load the filters
    KConfig conf;
    KConfigGroup group = conf.group( QLatin1String( "Filters" ) );
    const QStringList filterIds = group.readEntry( QLatin1String( "FilterIds" ), QStringList() );
    Q_FOREACH( const QString& filterId, filterIds ) {
        kDebug() << "Loading filter id:" << filterId;
        FilterData filter;
        filter.setId( filterId );
        group = conf.group( filterId );
        filter.setName( group.readEntry( QLatin1String( "Name" ) ) );
        filter.setSourceFeeds( group.readEntry( QLatin1String( "SourceFeeds" ), QList<KRss::Feed::Id>() ) );

        const QString path = KStandardDirs::locateLocal( "appdata", filterId );
        QFile file( path );
        if ( !file.open( QFile::ReadOnly ) ) {
            kWarning() << "Failed to open the file" << path << "in order to read the source for filter"
                       << filterId;
            return;
        }

        const QByteArray source = file.readAll();
        if ( source.isEmpty() ) {
            kWarning() << "The source file for filter" << filterId << "is empty";
            file.close();
            return;
        }

        file.close();

        Akonadi::Filter::IO::SieveDecoder decoder( m_componentFactory );
        filter.setProgram( decoder.run( source ) );

        kDebug() << "Filter name:" << filter.name();
        kDebug() << "Filter source feeds:" << filter.sourceFeeds();
        kDebug() << "Filter program:" << source;

        // fill in the mappings
        m_filters.insert( filterId, filter );
        const QList<KRss::Feed::Id> sourceFeeds = filter.sourceFeeds();
        Q_FOREACH( const KRss::Feed::Id& feedId, sourceFeeds ) {
            m_programs[ feedId ].append( filter.program() );
        }
    }
}

Akonadi::PreprocessorBase::ProcessingResult RssFilteringAgent::processItem( const Akonadi::Item &item )
{
    kDebug() << "Item id:" << item.id() << ", collection id:" << item.parentCollection().id() << ", mimeType:" << item.mimeType();

    const KRss::Feed::Id feedId = KRss::FeedCollection::feedIdFromAkonadi( item.parentCollection().id() );
    if ( !m_programs.contains( feedId ) ) {
        kDebug() << "I don't care about this collection";
        return Akonadi::PreprocessorBase::ProcessingFailed;
    }

    // TODO: remove once base class allows to set a fetch scope
    Akonadi::ItemFetchJob* const ijob = new Akonadi::ItemFetchJob( item );
    ijob->fetchScope().fetchAllAttributes();
    ijob->fetchScope().fetchFullPayload();
    if ( !ijob->exec() ) {
        kWarning() << "Failed to fetch the full item";
        return Akonadi::PreprocessorBase::ProcessingFailed;
    }

    RssData rssData( ijob->items().at( 0 ) );
    Akonadi::Filter::Program* program = m_programs.value( feedId ).at( 0 );

    // TODO: support more programs
    switch( program->execute( &rssData ) ) {
        case Akonadi::Filter::Program::SuccessAndContinue:
            return Akonadi::PreprocessorBase::ProcessingCompleted; // continue processing
        case Akonadi::Filter::Program::SuccessAndStop:
            return Akonadi::PreprocessorBase::ProcessingCompleted; // stop processing
        case Akonadi::Filter::Program::Failure:
            // filter failed, but continue processing
            return Akonadi::PreprocessorBase::ProcessingFailed;
        default:
            Q_ASSERT_X( false, __FUNCTION__, "You forgot to handle a Program::execute() result, "
                                             "or it returned something unexpected anyway" );
    }

    return Akonadi::PreprocessorBase::ProcessingCompleted;
}

void RssFilteringAgent::configure( WId windowId )
{
    Q_UNUSED( windowId )

    FilterListEditor* const editor = new FilterListEditor();
    editor->setComponentFactory( m_componentFactory );
    editor->setEditorFactory( m_editorFactory );
    editor->setFilters( m_filters );

    if ( editor->exec() ) {
        m_filters = editor->filters();
        m_programs.clear();

        // replace the filters in memory
        Q_FOREACH( const FilterData& filter, m_filters ) {
            const QList<KRss::Feed::Id> sourceFeeds = filter.sourceFeeds();
            Q_FOREACH( const KRss::Feed::Id& feedId, sourceFeeds ) {
                m_programs[ feedId ].append( filter.program() );
            }
        }

        // save the filters to disk
        KConfig conf;
        KConfigGroup group = conf.group( QLatin1String( "Filters" ) );

        // TODO: remove the source files as well
        const QStringList oldFilterIds = group.readEntry( QLatin1String( "FilterIds" ), QStringList() );
        Q_FOREACH( const QString& oldFilterId, oldFilterIds ) {
            conf.deleteGroup( oldFilterId );
        }

        group.writeEntry( QLatin1String( "FilterIds" ), m_filters.keys() );

        Q_FOREACH( const FilterData& filter, m_filters ) {
            group = conf.group( filter.id() );
            group.writeEntry( QLatin1String( "Name" ), filter.name() );
            group.writeEntry( QLatin1String( "SourceFeeds" ), filter.sourceFeeds() );

            const QString path = KStandardDirs::locateLocal( "appdata", filter.id() );
            QFile file( path );
            if ( !file.open( QFile::WriteOnly | QFile::Truncate ) ) {
                kWarning() << "Failed to open the file" << path << "in order to save the source for filter"
                           << filter.id();
                return;
            }

            Akonadi::Filter::IO::SieveEncoder encoder;
            const QByteArray source = encoder.run( filter.program() );
            if ( file.write( source ) != source.length() ) {
                kWarning() << "Failed to write the source for filter with id" << filter.id()
                           << "to file" << path << ":" << file.errorString();
                file.close();
                return;
            }

            file.close();
        }
    }

    delete editor;
}

AKONADI_PREPROCESSOR_MAIN( RssFilteringAgent )
