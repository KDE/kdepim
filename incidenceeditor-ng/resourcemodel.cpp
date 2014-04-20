/*
 * Copyright 2014  Sandro Knauß <knauss@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "resourcemodel.h"


using namespace IncidenceEditorNG;

ResourceModel::ResourceModel(const QStringList &headers,
                             QObject *parent)
    : QAbstractItemModel(parent)
    , foundCollection(false)
{

    this->headers = headers;
    rootItem = new ResourceItem(KLDAP::LdapDN(), headers, KLDAP::LdapClient(0), 0);

    ldapSearchCollections.setFilter(QString::fromLatin1("&(objectClass=kolabGroupOfUniqueNames)(mail=*)"
                                    "(|(cn=%1)(givenName=%1)(sn=%1))"));
    ldapSearch.setFilter(QString::fromLatin1("&(objectClass=kolabSharedFolder)(kolabFolderType=event)(mail=*)"
                         "(|(cn=%1)(givenName=%1)(sn=%1))"));

    QStringList attrs = ldapSearchCollections.attributes();
    attrs << QLatin1String("uniqueMember");
    ldapSearchCollections.setAttributes(attrs);

    connect(&ldapSearchCollections, SIGNAL(searchData(const QList<KLDAP::LdapResultObject> &)),
            SLOT(slotLDAPCollectionData(const QList<KLDAP::LdapResultObject> &)));
    connect(&ldapSearch, SIGNAL(searchData(const QList<KLDAP::LdapResultObject> &)),
            SLOT(slotLDAPSearchData(const QList<KLDAP::LdapResultObject> &)));

    ldapSearchCollections.startSearch("*");
}

ResourceModel::~ResourceModel()
{
    delete rootItem;
}

int ResourceModel::columnCount(const QModelIndex & /* parent */) const
{
    return 1;
}

QVariant ResourceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role != Qt::DisplayRole && role != Qt::EditRole) {
        return QVariant();
    }

    ResourceItem *item = getItem(index);

    return item->data(index.column());
}

Qt::ItemFlags ResourceModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return 0;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

ResourceItem *ResourceModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        ResourceItem *item = static_cast<ResourceItem*>(index.internalPointer());
        if (item) {
            return item;
        }
    }
    return rootItem;
}

QVariant ResourceModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return rootItem->data(section);
    }

    return QVariant();
}

QModelIndex ResourceModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0) {
        return QModelIndex();
    }
    ResourceItem *parentItem = getItem(parent);

    ResourceItem *childItem = parentItem->child(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    } else {
        return QModelIndex();
    }
}


QModelIndex ResourceModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }
    ResourceItem *childItem = getItem(index);
    ResourceItem *parentItem = childItem->parent();

    if (parentItem == rootItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->childNumber(), 0, parentItem);
}


bool ResourceModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    ResourceItem *parentItem = getItem(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

int ResourceModel::rowCount(const QModelIndex &parent) const
{
    ResourceItem *parentItem = getItem(parent);

    return parentItem->childCount();
}

void ResourceModel::startSearch(const QString &query)
{
    searchString = query;

    if (foundCollection) {
        startSearch();
    }
}

void ResourceModel::startSearch()
{
    emit layoutAboutToBeChanged();
    // Delete all resources -> only collection elements are shown
    for (int i = 0; i < rootItem->childCount(); i++) {
        if (ldapCollections.contains(rootItem->child(i))) {
            rootItem->child(i)->removeChildren(0, rootItem->child(i)->childCount());
        } else {
            rootItem->removeChildren(i, 1);
        }
    }
    emit layoutChanged();

    if (searchString.count() > 0) {
        ldapSearch.startSearch("*" + searchString + "*");
    } else {
        ldapSearch.startSearch("*");
    }
}

void ResourceModel::slotLDAPCollectionData(const QList<KLDAP::LdapResultObject> &results)
{
    foundCollection = true;
    ldapCollectionsMap.clear();
    ldapCollections.clear();

    emit layoutAboutToBeChanged();

    foreach(const KLDAP::LdapResultObject & result, results) {
        ResourceItem *item = new ResourceItem(result.object.dn(), headers, *result.client, rootItem);
        item->setLdapObject(result.object);
        rootItem->insertChild(rootItem->childCount(), item);
        ldapCollections.insert(item);

        // Resources in a collection add this link into ldapCollectionsMap
        foreach(const QByteArray & member, result.object.attributes()["uniqueMember"]) {
            ldapCollectionsMap.insert(QString::fromLatin1(member), item);
        }
    }

    emit layoutChanged();

    startSearch();
}

void ResourceModel::slotLDAPSearchData(const QList<KLDAP::LdapResultObject> &results)
{
    emit layoutAboutToBeChanged();

    foreach(const KLDAP::LdapResultObject & result, results) {
        //Add the found items to all collections, where it is member
        QList<ResourceItem*> parents = ldapCollectionsMap.values(result.object.dn().toString());
        if (parents.count() == 0) {
            parents << rootItem;
        }

        foreach(ResourceItem * parent, parents) {
            ResourceItem *item = new ResourceItem(result.object.dn(), headers, *result.client, parent);
            item->setLdapObject(result.object);
            parent->insertChild(parent->childCount(), item);
        }
    }

    emit layoutChanged();
}
