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

#include "rssbackendjobs.h"

using namespace KRssResource;

FeedsRetrieveJob::FeedsRetrieveJob( QObject *parent )
    : KJob( parent )
{
}

FeedsImportJob::FeedsImportJob( QObject *parent )
    : KJob( parent )
{
}

FeedCreateJob::FeedCreateJob( QObject *parent )
    : KJob( parent )
{
}

FeedModifyJob::FeedModifyJob( QObject *parent )
    : KJob( parent )
{
}

FeedDeleteJob::FeedDeleteJob( QObject *parent )
    : KJob( parent )
{
}

FeedFetchJob::FeedFetchJob( QObject *parent )
    : KJob( parent )
{
}

ItemModifyJob::ItemModifyJob( QObject *parent )
    : KJob( parent )
{
}

#include "rssbackendjobs.moc"
