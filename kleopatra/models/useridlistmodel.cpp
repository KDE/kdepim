/* -*- mode: c++; c-basic-offset:4 -*-
    models/useridlistmodel.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB
                  2016 Andre Heinecke <aheinecke@gnupg.org>

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config-kleopatra.h>

#include "useridlistmodel.h"

#include <utils/formatting.h>

#include <gpgme++/key.h>

#include <KLocalizedString>

#include <QVariant>
#include <QDate>

using namespace GpgME;
using namespace Kleo;

class UIDModelItem
{
    // A uid model item can either be a UserID::Signature or a UserID.
    // you can find out which it is if the uid or the signature return
    // null values. (Not null but isNull)
    //
public:
    explicit UIDModelItem(const UserID::Signature &sig, UIDModelItem *parentItem)
    {
        mItemData  << QString::fromUtf8(sig.signerKeyID())
                   << Formatting::prettyName(sig)
                   << Formatting::prettyEMail(sig)
                   << Formatting::creationDateString(sig)
                   << Formatting::expirationDateString(sig)
                   << Formatting::validityShort(sig);
        mSig = sig;
        mParentItem = parentItem;
    }

    explicit UIDModelItem(const UserID &uid, UIDModelItem *parentItem)
    {
        mItemData << Formatting::prettyUserID(uid);
        mUid = uid;
        mParentItem = parentItem;
    }

    // The root item
    explicit UIDModelItem() : mParentItem(0)
    {
        mItemData << i18n("ID")
                  << i18n("Name")
                  << i18n("EMail")
                  << i18n("Valid From")
                  << i18n("Valid Until")
                  << i18n("Status");
    }

    ~UIDModelItem()
    {
        qDeleteAll(mChildItems);
    }

    void appendChild(UIDModelItem *child)
    {
        mChildItems << child;
    }

    UIDModelItem *child(int row) const
    {
        return mChildItems.value(row);
    }

    const UIDModelItem *constChild(int row) const
    {
        return mChildItems.value(row);
    }

    int childCount() const
    {
        return mChildItems.count();
    }

    int columnCount() const
    {
        if (childCount()) {
            // We take the value from the first child
            // as we are likely a UID and our children
            // are UID Signatures.
            return constChild(0)->columnCount();
        }
        return mItemData.count();
    }

    QVariant data(int column) const
    {
        return mItemData.value(column);
    }

    int row() const
    {
        if (mParentItem) {
            return mParentItem->mChildItems.indexOf(const_cast<UIDModelItem*>(this));
        }
        return 0;
    }

    UIDModelItem *parentItem() const
    {
        return mParentItem;
    }

    UserID::Signature signature() const
    {
        return mSig;
    }

    UserID uid() const
    {
        return mUid;
    }

private:
    QList<UIDModelItem*> mChildItems;
    QList<QVariant> mItemData;
    UIDModelItem *mParentItem;
    UserID::Signature mSig;
    UserID mUid;
};

UserIDListModel::UserIDListModel(QObject *p)
    : QAbstractItemModel(p), mRootItem(0)
{
}

UserIDListModel::~UserIDListModel()
{
    delete mRootItem;
}

Key UserIDListModel::key() const
{
    return mKey;
}

void UserIDListModel::setKey(const Key &key)
{
    beginResetModel();
    delete mRootItem;
    mKey = key;

    mRootItem = new UIDModelItem();
    for (int i = 0, ids = key.numUserIDs(); i < ids; ++i) {
        UserID uid = key.userID(i);
        UIDModelItem *uidItem = new UIDModelItem(uid, mRootItem);
        mRootItem->appendChild(uidItem);
        for (int j = 0, sigs = uid.numSignatures(); j < sigs; ++j) {
            UserID::Signature sig = uid.signature(j);
            UIDModelItem *sigItem = new UIDModelItem(sig, uidItem);
            uidItem->appendChild(sigItem);
        }
    }

    endResetModel();
}

int UserIDListModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return static_cast<UIDModelItem*>(parent.internalPointer())->columnCount();
    }

    if (!mRootItem) {
        return 0;
    }

    return mRootItem->columnCount();
}

int UserIDListModel::rowCount(const QModelIndex &parent) const
{
    UIDModelItem *parentItem;
    if (parent.column() > 0 || !mRootItem) {
        return 0;
    }

    if (!parent.isValid()) {
        parentItem = mRootItem;
    } else {
        parentItem = static_cast<UIDModelItem*>(parent.internalPointer());
    }

    return parentItem->childCount();
}

QModelIndex UserIDListModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    UIDModelItem *parentItem;

    if (!parent.isValid()) {
        parentItem = mRootItem;
    } else {
        parentItem = static_cast<UIDModelItem*>(parent.internalPointer());
    }

    UIDModelItem *childItem = parentItem->child(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    } else {
        return QModelIndex();
    }
}

QModelIndex UserIDListModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }
    UIDModelItem *childItem = static_cast<UIDModelItem*>(index.internalPointer());
    UIDModelItem *parentItem = childItem->parentItem();

    if (parentItem == mRootItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

QVariant UserIDListModel::headerData(int section, Qt::Orientation o, int role) const
{
    if (o == Qt::Horizontal && mRootItem) {
        if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::ToolTipRole) {
            return mRootItem->data(section);
        }
    }
    return QVariant();
}

QVariant UserIDListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::ToolTipRole) {
        return QVariant();
    }

    UIDModelItem *item = static_cast<UIDModelItem*>(index.internalPointer());

    return item->data(index.column());
}

QVector<UserID> UserIDListModel::userIDs (const QModelIndexList &indexs) const {
    QVector<GpgME::UserID> ret;
    Q_FOREACH (const QModelIndex &idx, indexs) {
        if (!idx.isValid()) {
            continue;
        }
        UIDModelItem *item = static_cast<UIDModelItem*>(idx.internalPointer());
        if (!item->uid().isNull()) {
            ret << item->uid();
        }
    }
    return ret;
}

QVector<UserID::Signature> UserIDListModel::signatures (const QModelIndexList &indexs) const {
    QVector<GpgME::UserID::Signature> ret;
    Q_FOREACH (const QModelIndex &idx, indexs) {
        if (!idx.isValid()) {
            continue;
        }
        UIDModelItem *item = static_cast<UIDModelItem*>(idx.internalPointer());
        if (!item->signature().isNull()) {
            ret << item->signature();
        }
    }
    return ret;
}
