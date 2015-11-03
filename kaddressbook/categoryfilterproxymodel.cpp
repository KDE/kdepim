/*
    Copyright (c) 2014 Jonathan Marten <jjm@keelhaul.me.uk>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "categoryfilterproxymodel.h"

#include "kaddressbook_debug.h"
#include <KLocalizedString>

#include <AkonadiCore/entitytreemodel.h>
#include <AkonadiCore/item.h>

#include <kcontacts/addressee.h>
#include <kcontacts/contactgroup.h>

#include "categoryselectwidget.h"

using namespace Akonadi;

class CategoryFilterProxyModelPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(CategoryFilterProxyModel)

public:
    CategoryFilterProxyModelPrivate(CategoryFilterProxyModel *parent);

    QList<Tag::Id> filterIdList;
    bool filterEnabled;

private:
    CategoryFilterProxyModel *q_ptr;
};

CategoryFilterProxyModelPrivate::CategoryFilterProxyModelPrivate(CategoryFilterProxyModel *parent)
    : QObject(),
      filterEnabled(false),
      q_ptr(parent)
{
}

CategoryFilterProxyModel::CategoryFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent),
      d_ptr(new CategoryFilterProxyModelPrivate(this))
{
    setDynamicSortFilter(true);
}

CategoryFilterProxyModel::~CategoryFilterProxyModel()
{
    delete d_ptr;
}

bool CategoryFilterProxyModel::filterAcceptsRow(int row, const QModelIndex &parent) const
{
    Q_D(const CategoryFilterProxyModel);

    const QModelIndex index = sourceModel()->index(row, 0, parent);
    const Akonadi::Item item = index.data(EntityTreeModel::ItemRole).value<Akonadi::Item>();

    if (!d->filterEnabled) {
        return true;    // filter not enabled
    }
    if (d->filterIdList.isEmpty()) {
        return false;    // nothing accepted
    }
    // all accepted
    if (d->filterIdList.at(0) == CategorySelectWidget::FilterAll) {
        return true;
    }

    //qCDebug(KADDRESSBOOK_LOG) << "for row" << row << "item" << item.url() << "filter" << d->filterIdList;
    if (item.hasPayload<KContacts::Addressee>()) {
        const KContacts::Addressee contact = item.payload<KContacts::Addressee>();

        const QStringList categories = contact.categories();
        //qCDebug(KADDRESSBOOK_LOG) << "is contact" << contact.assembledName() << "cats" << categories;

        int validCategories = 0;
        int count = categories.count();
        for (int i = 0; i < count; ++i) {
            const QString cat = categories.at(i);
            if (cat.startsWith(QStringLiteral("akonadi:"))) {
                const int idx = cat.indexOf(QStringLiteral("?tag="));
                if (idx >= 0) {
                    ++validCategories;
                    Tag::Id id = cat.midRef(idx + 5).toInt();
                    if (d->filterIdList.contains(id)) {
                        //qCDebug(KADDRESSBOOK_LOG) << "matches category" << cat;
                        return true;            // a category matches filter
                    }
                }
            }
        }

        if (validCategories > 0) {
            //qCDebug(KADDRESSBOOK_LOG) << "valid item but no match";
            return false;               // categorised but no match
        } else {
            //qCDebug(KADDRESSBOOK_LOG) << "item with no categories";
            return d->filterIdList.contains(CategorySelectWidget::FilterUntagged);
        }
    } else if (item.hasPayload<KContacts::ContactGroup>()) { // a contact group item
        return d->filterIdList.contains(CategorySelectWidget::FilterGroups);
    }

    return true;                    // not a recognised item
}

void CategoryFilterProxyModel::setFilterCategories(const QList<Akonadi::Tag::Id> &idList)
{
    Q_D(CategoryFilterProxyModel);

    if (idList != d->filterIdList) {
        //qCDebug(KADDRESSBOOK_LOG) << idList;
        d->filterIdList = idList;
        invalidateFilter();
    }
}

void CategoryFilterProxyModel::setFilterEnabled(bool enable)
{
    Q_D(CategoryFilterProxyModel);

    if (enable != d->filterEnabled) {
        //qCDebug(KADDRESSBOOK_LOG) << enable;
        d->filterEnabled = enable;
        invalidateFilter();
    }
}

#include "categoryfilterproxymodel.moc"
