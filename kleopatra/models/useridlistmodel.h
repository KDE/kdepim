/* -*- mode: c++; c-basic-offset:4 -*-
    models/userIDlistmodel.h

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
#ifndef __KLEOPATRA_MODELS_USERIDLISTMODEL_H__
#define __KLEOPATRA_MODELS_USERIDLISTMODEL_H__

#include <QAbstractItemModel>

#include <gpgme++/key.h> // since Signature is nested in UserID...

class UIDModelItem;

namespace Kleo
{

class UserIDListModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit UserIDListModel(QObject *parent = Q_NULLPTR);
    ~UserIDListModel();

    GpgME::Key key() const;

public:
    QVector<GpgME::UserID> userIDs(const QModelIndexList &indexs) const;
    QVector<GpgME::UserID::Signature> signatures(const QModelIndexList &indexs) const;

public Q_SLOTS:
    void setKey(const GpgME::Key &key);

public:
    int columnCount(const QModelIndex &pindex = QModelIndex()) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &pindex = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation o, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    QModelIndex index(int row, int col, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;

private:
    GpgME::Key mKey;
    UIDModelItem *mRootItem;
};

}

#endif /* __KLEOPATRA_MODELS_USERIDLISTMODEL_H__ */
