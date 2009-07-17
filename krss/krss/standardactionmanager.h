/*
    Copyright (C) 2008    Dmitry Ivanov <vonami@gmail.com>

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

#ifndef KRSS_STANDARDACTIONMANAGER_H
#define KRSS_STANDARDACTIONMANAGER_H

#include "krss_export.h"

#include <QtCore/QObject>

class KAction;
class KActionCollection;
class KJob;
class QItemSelectionModel;
class QWidget;

namespace boost {
template <typename T> class shared_ptr;
}

namespace KRss {

class TagProvider;
class StandardActionManagerPrivate;

class KRSS_EXPORT StandardActionManager : public QObject
{
    Q_OBJECT

public:

    enum Type {
        CreateNetFeed,
        FetchFeed,
        AbortFetch,
        FeedProperties,
        DeleteFeed,
        CreateTag,
        ModifyTag,
        DeleteTag,
        ManageSubscriptions,
        MarkItemNew,
        MarkItemRead,
        MarkItemUnread,
        MarkItemImportant,
        DeleteItem,
        LastType
    };

    explicit StandardActionManager( KActionCollection *actionCollection, QWidget *parent = 0 );
    ~StandardActionManager();

    void setFeedSelectionModel( QItemSelectionModel *selectionModel );
    void setItemSelectionModel( QItemSelectionModel *selectionModel );
    void setSubscriptionLabel( const QString &subscriptionLabel );
    void setTagProvider( const boost::shared_ptr<const TagProvider>& tagProvider );
    KAction* action( Type type );
    void createAllActions();

private:

    //helper methods
    void enableAction( Type type, bool enable );

private Q_SLOTS:

    void slotCreateNetFeed();
    void slotFetchFeed();
    void slotAbortFetch();
    void slotFeedProperties();
    void slotDeleteFeed();
    void slotCreateTag();
    void slotModifyTag();
    void slotDeleteTag();
    void slotManageSubscriptions();
    void slotMarkItemNew();
    void slotMarkItemRead();
    void slotMarkItemUnread();
    void slotMarkItemImportant( bool checked );
    void slotDeleteItem();
    void updateActions();

    void slotNetFeedCreated( KJob* job );
    void slotFeedModified( KJob* job );
    void slotFeedDeleted( KJob* job );
    void slotItemModified( KJob* job );
    void slotItemDeleted( KJob* job );
    void slotTagCreated( KJob *job );
    void slotTagModified( KJob *job );
    void slotTagDeleted( KJob *job );

private:

    Q_DISABLE_COPY( StandardActionManager )
    StandardActionManagerPrivate * const d;
};

} // namespace KRss

#endif /* KRSS_STANDARDACTIONMANAGER_H */
