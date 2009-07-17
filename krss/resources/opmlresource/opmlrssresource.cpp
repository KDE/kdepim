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

#include "opmlrssresource.h"
#include "opmlsettings.h"
#include "opmljobs.h"

#include <KFileDialog>
#include <KDebug>
#include <KStandardDirs>
#include <KLocale>

using namespace KRss;
using namespace KRssResource;

OpmlRssResource::OpmlRssResource( const QString& id )
    : RssResourceBase( id ), m_blockFetch( false )
{
    connect( this, SIGNAL( reloadConfiguration() ), this, SLOT( slotReloadConfiguration() ) );
    loadConfiguration();
}

void OpmlRssResource::configure( WId windowId )
{
    Q_UNUSED( windowId )
    const KUrl path = KFileDialog::getOpenUrl( KUrl(), "*.opml|" + i18n( "OPML Document (*.opml)" ),
                                              0, i18n( "Select an OPML Document" ) );

    if ( !path.url().isEmpty() )
        m_path = path;
    else
        m_path = KStandardDirs::locateLocal( "appdata", "feeds.opml" );

    Settings::setPath( m_path.url() );
    Settings::self()->writeConfig();
    synchronizeCollectionTree();
}

void OpmlRssResource::slotReloadConfiguration()
{
    loadConfiguration();
    synchronizeCollectionTree();
}

void OpmlRssResource::loadConfiguration()
{
    m_path = KUrl( Settings::path() );
    if ( m_path.isEmpty() ) {
        m_path = KStandardDirs::locateLocal( "appdata", "feeds.opml" );
        Settings::setPath( m_path.url() );
        Settings::self()->writeConfig();
    }
    m_blockFetch = Settings::blockFetch();
}

bool OpmlRssResource::flagsSynchronizable() const
{
    return false;
}

FeedsRetrieveJob* OpmlRssResource::feedsRetrieveJob() const
{
    OpmlFeedsRetrieveJob * const job = new OpmlFeedsRetrieveJob();
    job->setPath( m_path );
    return job;
}

FeedsImportJob* OpmlRssResource::feedsImportJob() const
{
    OpmlFeedsImportJob * const job = new OpmlFeedsImportJob();
    job->setPath( m_path );
    return job;
}

FeedCreateJob* OpmlRssResource::feedCreateJob() const
{
    OpmlFeedCreateJob * const job = new OpmlFeedCreateJob();
    job->setPath( m_path );
    return job;
}

FeedModifyJob* OpmlRssResource::feedModifyJob() const
{
    OpmlFeedModifyJob * const job = new OpmlFeedModifyJob();
    job->setPath( m_path );
    return job;
}

FeedDeleteJob* OpmlRssResource::feedDeleteJob() const
{
    OpmlFeedDeleteJob * const job = new OpmlFeedDeleteJob();
    job->setPath( m_path );
    return job;
}

FeedFetchJob* OpmlRssResource::feedFetchJob() const
{
    if ( m_blockFetch )
        return 0;

    OpmlFeedFetchJob * const job = new OpmlFeedFetchJob();
    job->setPath( m_path );
    return job;
}

KRssResource::ItemModifyJob* OpmlRssResource::itemModifyJob() const
{
    OpmlItemModifyJob * const job = new OpmlItemModifyJob();
    job->setPath( m_path );
    return job;
}

AKONADI_RESOURCE_MAIN( OpmlRssResource )

#include "opmlrssresource.moc"
