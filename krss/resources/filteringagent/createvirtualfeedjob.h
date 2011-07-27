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

#ifndef RSSFILTERINGAGENT_CREATEVIRTUALFEEDJOB_H
#define RSSFILTERINGAGENT_CREATEVIRTUALFEEDJOB_H

#include "krss/virtualfeedcollection.h"

#include <KJob>

class CreateVirtualFeedJob : public KJob
{
    Q_OBJECT
public:
    explicit CreateVirtualFeedJob( const QString& name, const QString& agentId, QObject* parent = 0 );

    enum Error {
        CouldNotCreateVirtualFeed = KJob::UserDefinedError,
        CouldNotRetrieveVirtualFeed,
        CouldNotModifyVirtualFeed = CouldNotCreateVirtualFeed + 100
    };

    void start();
    QString errorString() const;

private Q_SLOTS:
    void doStart();
    void slotVirtualFeedCreated( KJob* job );
    void slotVirtualFeedRetrieved( KJob* job );
    void slotVirtualFeedModified( KJob *job );

private:
    const QString m_name;
    const QString m_agentId;
    Akonadi::Collection m_virtualFeed;
};

#endif // RSSFILTERINGAGENT_CREATEVIRTUALFEEDJOB_H
