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

#include "invalidfilterlistmodel.h"

using namespace MailCommon;

InvalidFilterListModel::InvalidFilterListModel(QObject *parent) :
    QAbstractListModel(parent),
    mInvalidFilterItems()
{
}

InvalidFilterListModel::~InvalidFilterListModel()
{
}

bool InvalidFilterListModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (row > rowCount()) {
        return false;
    }

    if (count <= 0) {
        count = 1;
    }

    beginInsertRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        MailCommon::InvalidFilterInfo info;
        mInvalidFilterItems.insert(row, info);
    }
    endInsertRows();

    return true;
}

bool InvalidFilterListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    const int row = index.row();
    if (row >= rowCount()) {
        return false;
    }

    switch (role) {
    case Qt::DisplayRole:
        mInvalidFilterItems[row].setName(value.toString());
        break;
    case InformationRole:
        mInvalidFilterItems[row].setInformation(value.toString());
        break;
    default:
        return false;
    }

    Q_EMIT dataChanged(index, index);
    return true;
}

QVariant InvalidFilterListModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    if (row < rowCount()) {
        switch (role) {
        case Qt::DisplayRole:
            return mInvalidFilterItems[row].name();
        case InformationRole:
            return mInvalidFilterItems[row].information();
        default:
            break;
        }
    }

    return QVariant();
}

int InvalidFilterListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mInvalidFilterItems.count();
}

