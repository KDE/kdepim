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

#include "newsgatorrssresource.h"
#include "settingsdialog.h"
#include "newsgatorsettings.h"
#include "newsgatorjobs.h"

#include <KDebug>

using namespace KRss;
using namespace KRssResource;

NewsgatorRssResource::NewsgatorRssResource( const QString& id )
    : RssResourceBase( id )
{
    connect( this, SIGNAL( reloadConfiguration() ), this, SLOT( slotReloadConfiguration() ) );
    loadConfiguration();
}

void NewsgatorRssResource::configure( WId windowId )
{
    Q_UNUSED( windowId );

    SettingsDialog * const dialog = new SettingsDialog();
    if ( dialog->exec() ) {
        kDebug() << "Location:" << dialog->location().name << ", id:" << dialog->location().id;
        m_userName = dialog->userName();
        m_password = dialog->password();
        m_location = dialog->location();
        Settings::setUserName( dialog->userName() );
        Settings::setLocationName( dialog->location().name );
        Settings::setLocationId( dialog->location().id );
        Settings::self()->writeConfig();
        synchronizeCollectionTree();
    }
    delete dialog;
}

void NewsgatorRssResource::slotReloadConfiguration()
{
    loadConfiguration();
    synchronizeCollectionTree();
}

void NewsgatorRssResource::loadConfiguration()
{
    m_userName = Settings::userName();
    m_location.name = Settings::locationName();
    m_location.id = Settings::locationId();
    kDebug() << "Loaded location:" << m_location.name << ", id:" << m_location.id;
}

bool NewsgatorRssResource::flagsSynchronizable() const
{
    return true;
}

FeedsRetrieveJob* NewsgatorRssResource::feedsRetrieveJob() const
{
    NewsgatorFeedsRetrieveJob *job = new NewsgatorFeedsRetrieveJob();
    job->setUserName( m_userName );
    job->setPassword( m_password );
    job->setLocation( m_location );
    return job;
}

FeedsImportJob* NewsgatorRssResource::feedsImportJob() const
{
    return 0;
}

FeedCreateJob* NewsgatorRssResource::feedCreateJob() const
{
    NewsgatorFeedCreateJob *job = new NewsgatorFeedCreateJob();
    job->setUserName( m_userName );
    job->setPassword( m_password );
    job->setLocation( m_location );
    return job;
}

FeedModifyJob* NewsgatorRssResource::feedModifyJob() const
{
    NewsgatorFeedModifyJob *job = new NewsgatorFeedModifyJob();
    job->setUserName( m_userName );
    job->setPassword( m_password );
    job->setLocation( m_location );
    return job;
}

FeedDeleteJob* NewsgatorRssResource::feedDeleteJob() const
{
    NewsgatorFeedDeleteJob *job = new NewsgatorFeedDeleteJob();
    job->setUserName( m_userName );
    job->setPassword( m_password );
    job->setLocation( m_location );
    return job;
}

FeedFetchJob* NewsgatorRssResource::feedFetchJob() const
{
    NewsgatorFeedFetchJob *job = new NewsgatorFeedFetchJob();
    job->setUserName( m_userName );
    job->setPassword( m_password );
    job->setLocation( m_location );
    return job;
}

KRssResource::ItemModifyJob* NewsgatorRssResource::itemModifyJob() const
{
    NewsgatorItemModifyJob *job = new NewsgatorItemModifyJob();
    job->setUserName( m_userName );
    job->setPassword( m_password );
    job->setLocation( m_location );
    return job;
}

AKONADI_RESOURCE_MAIN( NewsgatorRssResource )

#include "newsgatorrssresource.moc"
