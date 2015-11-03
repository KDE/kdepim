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

#ifndef INVALIDFILTERLISTMODEL_H
#define INVALIDFILTERLISTMODEL_H

#include <QAbstractListModel>
#include <QList>
#include "invalidfilterinfo.h"
namespace MailCommon
{
class InvalidFilterListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role {
        InformationRole = Qt::UserRole + 1
    };

    explicit InvalidFilterListModel(QObject *parent = Q_NULLPTR);
    virtual ~InvalidFilterListModel();

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

private:
    QList<MailCommon::InvalidFilterInfo> mInvalidFilterItems;
};
}

#endif // INVALIDFILTERLISTMODEL_H
