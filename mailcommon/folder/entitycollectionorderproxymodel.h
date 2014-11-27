/* -*- mode: C++; c-file-style: "gnu" -*-

  Copyright (c) 2009, 2010 Montel Laurent <montel@kde.org>

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

#ifndef MAILCOMMON_ENTITYCOLLECTIONORDERPROXYMODEL_H
#define MAILCOMMON_ENTITYCOLLECTIONORDERPROXYMODEL_H

#include <EntityOrderProxyModel>

namespace MailCommon
{

class EntityCollectionOrderProxyModel : public Akonadi::EntityOrderProxyModel
{
    Q_OBJECT
public:
    explicit EntityCollectionOrderProxyModel(QObject *parent = 0);

    virtual ~EntityCollectionOrderProxyModel();

    bool lessThan(const QModelIndex &left, const QModelIndex &right) const Q_DECL_OVERRIDE;

    void setManualSortingActive(bool active);
    bool isManualSortingActive() const;

    void clearRanks();
    void setTopLevelOrder(const QStringList &list);

public slots:
    void slotSpecialCollectionsChanged();

private:
    class EntityCollectionOrderProxyModelPrivate;
    EntityCollectionOrderProxyModelPrivate *const d;
};

}

#endif

