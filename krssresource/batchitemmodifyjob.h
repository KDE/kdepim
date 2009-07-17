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

#ifndef KRSSRESOURCE_BATCHITEMMODIFY_H
#define KRSSRESOURCE_BATCHITEMMODIFY_H

#include <KJob>
#include <boost/function.hpp>

namespace Akonadi {
class Collection;
class Item;
}

namespace KRssResource
{

class BatchItemModifyJob : public KJob
{
    Q_OBJECT
public:
    explicit BatchItemModifyJob( QObject *parent = 0 );
    ~BatchItemModifyJob();

    void setFeed( const Akonadi::Collection& collection );
    void setModifier( const boost::function1<bool, Akonadi::Item&>& modifier );

    void start();
    QString errorString() const;

    enum Error {
        CouldNotRetrieveItems = KJob::UserDefinedError,
        CouldNotModifyItem,
        UserDefinedError
    };

private:
    class Private;
    Private * const d;
    Q_DISABLE_COPY( BatchItemModifyJob )
    Q_PRIVATE_SLOT( d, void doStart() )
    Q_PRIVATE_SLOT( d, void slotItemsRetrieved( KJob* ) )
    Q_PRIVATE_SLOT( d, void slotItemModified( KJob* ) )
};

} // namespace KRssResource

#endif // KRSSRESOURCE_BATCHITEMMODIFY_H
