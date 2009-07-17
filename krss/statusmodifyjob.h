/*
    Copyright (C) 2009    Dmitry Ivanov <vonami@gmail.com>
    based on itemlistjob.h, Copyright (C) 2009    Frank Osterfeld <osterfeld@kde.org>

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

#ifndef KRSS_STATUSMODIFYJOB_H
#define KRSS_STATUSMODIFYJOB_H

#include "krss_export.h"
#include "item.h"

#include <KCompositeJob>
#include <KJob>

namespace boost {
template <typename T> class shared_ptr;
}

namespace KRss {

class Feed;

class KRSS_EXPORT StatusModifyJob : public KJob
{
    Q_OBJECT
public:
    explicit StatusModifyJob( QObject* parent = 0 );
    ~StatusModifyJob();

    virtual void clearFlags( const QList<KRss::Item::StatusFlag>& flags ) = 0;
    virtual void setFlags( const QList<KRss::Item::StatusFlag>& flags ) = 0;
};

class KRSS_EXPORT CompositeStatusModifyJob : public StatusModifyJob
{
    Q_OBJECT
public:
    explicit CompositeStatusModifyJob( QObject * parent = 0 );
    ~CompositeStatusModifyJob();

    void addSubJob( StatusModifyJob* job );
    void removeSubJob( StatusModifyJob* job );

    void clearFlags( const QList<KRss::Item::StatusFlag>& flags );
    void setFlags( const QList<KRss::Item::StatusFlag>& flags );

    void start();

private:
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void doStart() )
    Q_PRIVATE_SLOT( d, void jobDone( KJob* ) )
    Q_PRIVATE_SLOT( d, void slotPercent( KJob*, unsigned long ) )
    };
}

#endif // KRSS_STATUSMODIFYJOB_H
