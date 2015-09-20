/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "invalidfilterlistview.h"
#include "invalidfilterlistitemdelegate.h"
#include "invalidfilterlistmodel.h"
using namespace MailCommon;

InvalidFilterListView::InvalidFilterListView(QWidget *parent)
    : QListView(parent)
{
    InvalidFilterListItemDelegate *invalidFilterDelegate = new InvalidFilterListItemDelegate(this, this);

    InvalidFilterListModel *invalidFilterListModel  = new InvalidFilterListModel(this);
    connect(invalidFilterDelegate, &InvalidFilterListItemDelegate::showDetails, this, &InvalidFilterListView::showDetails);
    //connect(this, SIGNAL(pressed(QModelIndex)), SIGNAL(hideInformationWidget()));
    setModel(invalidFilterListModel);
    setItemDelegate(invalidFilterDelegate);
}

InvalidFilterListView::~InvalidFilterListView()
{

}

void InvalidFilterListView::setInvalidFilters(const QVector<MailCommon::InvalidFilterInfo> &lst)
{
    Q_FOREACH (const MailCommon::InvalidFilterInfo &info, lst) {
        model()->insertRow(0);
        const QModelIndex index = model()->index(0, 0);
        model()->setData(index, info.name(), Qt::DisplayRole);
        model()->setData(index, info.information(), InvalidFilterListModel::InformationRole);
    }
    model()->sort(Qt::DisplayRole);
}
