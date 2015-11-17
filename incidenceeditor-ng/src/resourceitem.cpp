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
#include "resourceitem.h"

#include <kldap/ldapserver.h>

using namespace IncidenceEditorNG;

ResourceItem::ResourceItem(const KLDAP::LdapDN &dn, const QStringList &attrs, const KLDAP::LdapClient &ldapClient, const ResourceItem::Ptr &parent)
    : parentItem(parent)
    , dn(dn)
    , mAttrs(attrs)
    , mLdapClient(0, this)
{
    if (!dn.isEmpty()) {
        KLDAP::LdapServer server = ldapClient.server();

        server.setScope(KLDAP::LdapUrl::Base);
        server.setBaseDn(dn);
        mLdapClient.setServer(server);

        connect(&mLdapClient, &KLDAP::LdapClient::result, this, &ResourceItem::slotLDAPResult);

        mAttrs << QStringLiteral("uniqueMember");
        mLdapClient.setAttributes(attrs);
    } else {
        itemData.reserve(mAttrs.count());
        foreach (const QString &header, mAttrs) {
            itemData << header;
        }
    }
}

ResourceItem::~ResourceItem()
{
}

ResourceItem::Ptr ResourceItem::child(int number)
{
    return childItems.value(number);
}

int ResourceItem::childCount() const
{
    return childItems.count();
}

int ResourceItem::childNumber() const
{
    if (parentItem) {
        int i = 0;
        foreach (const ResourceItem::Ptr &child, parentItem->childItems) {
            if (child == this) {
                return i;
            }
            i++;
        }
    }

    return 0;
}

int ResourceItem::columnCount() const
{
    return itemData.count();
}

QVariant ResourceItem::data(int column) const
{
    return itemData.value(column);
}

QVariant ResourceItem::data(const QString &column) const
{
    if (!mLdapObject.attributes()[column].isEmpty()) {
        return QString::fromUtf8(mLdapObject.attributes()[column][0]);
    }
    return QVariant();
}

bool ResourceItem::insertChild(int position, const ResourceItem::Ptr &item)
{
    if (position < 0 || position > childItems.size()) {
        return false;
    }

    childItems.insert(position, item);

    return true;
}

ResourceItem::Ptr ResourceItem::parent()
{
    return parentItem;
}

bool ResourceItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > childItems.size()) {
        return false;
    }

    for (int row = 0; row < count; ++row) {
        childItems.removeAt(position);
    }

    return true;
}

const QStringList &ResourceItem::attributes() const
{
    return mAttrs;
}

const KLDAP::LdapObject &ResourceItem::ldapObject() const
{
    return mLdapObject;
}

void ResourceItem::startSearch()
{
    mLdapClient.startQuery(QStringLiteral("objectclass=*"));
}

void ResourceItem::setLdapObject(const KLDAP::LdapObject &obj)
{
    slotLDAPResult(mLdapClient, obj);
}

const KLDAP::LdapClient &ResourceItem::ldapClient() const
{
    return mLdapClient;
}

void ResourceItem::slotLDAPResult(const KLDAP::LdapClient &/*client*/,
                                  const KLDAP::LdapObject &obj)
{
    mLdapObject = obj;
    foreach (const QString &header, mAttrs) {
        if (!obj.attributes()[header].isEmpty()) {
            itemData << QString::fromUtf8(obj.attributes()[header][0]);
        } else {
            itemData << "";
        }
    }
    Q_EMIT searchFinished();
}
