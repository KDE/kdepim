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

#ifndef RESOURCEITEM_H
#define RESOURCEITEM_H

#include <QList>
#include <QMetaType>
#include <QSharedPointer>
#include <QObject>
#include <QStringList>
#include <QVariant>
#include <QVector>

#include <ldap/ldapclient.h>
#include <kldap/ldapobject.h>

namespace IncidenceEditorNG
{

class ResourceItem : public QObject
{
    Q_OBJECT
public:
    /* Copied from http://qt-project.org/doc/qt-4.8/itemviews-editabletreemodel.html:
     * QT 4.8: Editable Tree Model Example
     */

    /**
      A shared pointer to an ResourceItem object.
    */
    typedef QSharedPointer<ResourceItem> Ptr;

    ResourceItem(const KLDAP::LdapDN &dn, QStringList attrs, const KLDAP::LdapClient &ldapClient, ResourceItem::Ptr parent = ResourceItem::Ptr());
    ~ResourceItem();

    ResourceItem::Ptr child(int number);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    QVariant data(const QString &column) const;
    bool insertChild(int position, ResourceItem::Ptr item);
    ResourceItem::Ptr parent();
    bool removeChildren(int position, int count);
    int childNumber() const;

private:
    QList<ResourceItem::Ptr> childItems;
    QVector<QVariant> itemData;
    ResourceItem::Ptr parentItem;

public:
    /* Returns the attributes of the requested ldapObject.
     *
     */
    const QStringList &attributes() const;

    /* Returns the ldapObject, that is used as data source.
     *
     */
    const KLDAP::LdapObject& ldapObject() const;

    /* Set the ldapObject, either directy via this function
     * or use startSearch to request the ldapServer for the ldapObject
     * with the dn specified via the constructor.
     *
     */
    void setLdapObject(const KLDAP::LdapObject&);

    /* The used ldapClient.
     *
     */
    const KLDAP::LdapClient& ldapClient() const;

    /* Start querying the ldapServer for a object that name is dn
     *
     */
    void startSearch();

private:
    /* data source
     *
     */
    KLDAP::LdapObject mLdapObject;

    /* dn of the ldapObject
     *
     */
    const KLDAP::LdapDN dn;

    /* Attributes of the ldapObject to request and the header of the Item
     *
     */
    QStringList attrs;

    /* ldapClient to request
     *
     */
    KLDAP::LdapClient mLdapClient;


private slots:
    /* Answer of the LdapServer for the given dn
     *
     */
    void slotLDAPResult(const KLDAP::LdapClient&, const KLDAP::LdapObject&);

};
}

//@cond PRIVATE
  Q_DECLARE_TYPEINFO(IncidenceEditorNG::ResourceItem::Ptr, Q_MOVABLE_TYPE);
  Q_DECLARE_METATYPE(IncidenceEditorNG::ResourceItem::Ptr)
//@endcond
#endif // RESOURCEITEM_H
