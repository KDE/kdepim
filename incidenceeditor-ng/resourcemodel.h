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

#ifndef RESOURCEMODEL_H
#define RESOURCEMODEL_H

#include "resourceitem.h"

#include <libkdepim/ldap/ldapclientsearch.h>

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QSet>

namespace IncidenceEditorNG
{

class ResourceModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    /* Copied from http://qt-project.org/doc/qt-4.8/itemviews-editabletreemodel.html:
     * QT 4.8: Editable Tree Model Example
     */
    ResourceModel(const QStringList &headers,
                  QObject *parent = 0);
    ~ResourceModel();

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;

    bool removeRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex());

    ResourceItem *getItem(const QModelIndex &index) const;

private:
    ResourceItem *rootItem;


public:

    /* Start search on LDAP Server with the given string.
     * If the model is not ready to search, the string is cached and is executed afterwards.
     */
    void startSearch(const QString&);
private:

    /* Start search with cached string (stored in searchString)
     *
     */
    void startSearch();

    /* Search for collections of resources
     *
     */
    KLDAP::LdapClientSearch ldapSearchCollections;

    /* Search for matching resources
     *
     */
    KLDAP::LdapClientSearch ldapSearch;

    /* Map from dn of resource -> collectionItem
     * A Resource can be part of different collection, so a QMuliMap is needed
     *
     */
    QMultiMap<QString, ResourceItem*> ldapCollectionsMap;

    /* A Set of all collection ResourceItems
     *
     */
    QSet <ResourceItem*> ldapCollections;

    /* Cached searchString (setted by startSearch(QString))
     *
     */
    QString searchString;

    /* Is the search of collections ended
     *
     */
    bool foundCollection;

    /* List of all attributes in LDAP an the headers of the model
     *
     */
    QStringList headers;


private slots:
    /* Slot for founded collections
     *
     */
    void slotLDAPCollectionData(const QList<KLDAP::LdapResultObject> &);

    /* Slot for matching resources
     *
     */
    void slotLDAPSearchData(const QList<KLDAP::LdapResultObject> &);
};

}
#endif // RESOURCEMODEL_H
