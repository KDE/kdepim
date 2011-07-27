/*
    Copyright (C) 2009    Frank Osterfeld <osterfeld@kde.org>

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

#ifndef FEEDITEMMODEL_EXAMPLE_H
#define FEEDITEMMODEL_EXAMPLE_H

#include <krss/feedlistmodel.h>
#include <krss/feeditemmodel.h>
#include <krss/tagprovider.h>

#include <QTreeView>

class QItemSelectionModel;

class KJob;

class MainWidget : public QWidget {
    Q_OBJECT
public:
    explicit MainWidget( QWidget* parent=0 );

private Q_SLOTS:
    void feedListRetrieved( KJob* );

    void tagProviderRetrieved( KJob* );

private:
    void init();
private:
    boost::shared_ptr<KRss::TagProvider> m_tagProvider;
    boost::shared_ptr<KRss::FeedList> m_feedList;
    //KRss::FeedListModel* m_feedModel;
    KRss::FeedItemModel* m_itemModel;
    QTreeView* m_feedView;
    QTreeView* m_itemView;
    QItemSelectionModel* m_selection;
};

#endif // FEEDITEMMODEL_EXAMPLE_H
