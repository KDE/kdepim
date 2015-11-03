/*
  Copyright (c) 2014 Christian Mollekopf <mollekopf@kolabsys.com>

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

#include "tagselectioncombo.h"

#include <AkonadiCore/Monitor>
#include <AkonadiCore/TagModel>

#include <KCheckableProxyModel>
#include <qitemselectionmodel.h>

using namespace KPIM;

class MatchingCheckableProxyModel : public KCheckableProxyModel
{
public:
    MatchingCheckableProxyModel(QObject *parent = Q_NULLPTR): KCheckableProxyModel(parent) {}
    QModelIndexList match(const QModelIndex &start, int role, const QVariant &value, int hits = 1, Qt::MatchFlags flags = Qt::MatchExactly) const Q_DECL_OVERRIDE
    {
        if (role == Qt::CheckStateRole) {
            return selectionModel()->selectedRows();
        }
        return KCheckableProxyModel::match(start, role, value, hits, flags);
    }
};

TagSelectionCombo::TagSelectionCombo(QWidget *parent)
    :   KPIM::KCheckComboBox(parent)
{
    Akonadi::Monitor *monitor = new Akonadi::Monitor(this);
    monitor->setTypeMonitored(Akonadi::Monitor::Tags);

    Akonadi::TagModel *model = new Akonadi::TagModel(monitor, this);

    QItemSelectionModel *selectionModel = new QItemSelectionModel(model, this);
    KCheckableProxyModel *checkableProxy = new MatchingCheckableProxyModel(this);
    checkableProxy->setSourceModel(model);
    checkableProxy->setSelectionModel(selectionModel);

    setModel(checkableProxy);

    //We need to reconnect from the constructor of KCheckComboBox to the new model
    connect(checkableProxy, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(updateCheckedItems(QModelIndex,QModelIndex)));
}

TagCombo::TagCombo(QWidget *parent)
    :   KComboBox(parent)
{
    Akonadi::Monitor *monitor = new Akonadi::Monitor(this);
    monitor->setTypeMonitored(Akonadi::Monitor::Tags);
    Akonadi::TagModel *model = new Akonadi::TagModel(monitor, this);
    setModel(model);
}
