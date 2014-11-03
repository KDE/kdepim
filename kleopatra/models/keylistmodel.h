/* -*- mode: c++; c-basic-offset:4 -*-
    models/keylistmodel.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

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
#ifndef __KLEOPATRA_MODELS_KEYLISTMODEL_H__
#define __KLEOPATRA_MODELS_KEYLISTMODEL_H__

#include <QAbstractItemModel>

#include <models/keylistmodelinterface.h>

#include <utils/pimpl_ptr.h>

#include <vector>

namespace GpgME
{
class Key;
}

namespace Kleo
{

class AbstractKeyListModel : public QAbstractItemModel, public KeyListModelInterface
{
    Q_OBJECT
public:
    explicit AbstractKeyListModel(QObject *parent = 0);
    ~AbstractKeyListModel();

    static AbstractKeyListModel *createFlatKeyListModel(QObject *parent = 0);
    static AbstractKeyListModel *createHierarchicalKeyListModel(QObject *parent = 0);

    /* reimp */ GpgME::Key key(const QModelIndex &idx) const;
    /* reimp */ std::vector<GpgME::Key> keys(const QList<QModelIndex> &indexes) const;

    using QAbstractItemModel::index;
    /* reimp */ QModelIndex index(const GpgME::Key &key) const
    {
        return index(key, 0);
    }
    QModelIndex index(const GpgME::Key &key, int col) const;
    /* reimp */ QList<QModelIndex> indexes(const std::vector<GpgME::Key> &keys) const;

Q_SIGNALS:
    void rowAboutToBeMoved(const QModelIndex &old_parent, int old_row);
    void rowMoved(const QModelIndex &new_parent, int new_row);

public Q_SLOTS:
    void setKeys(const std::vector<GpgME::Key> &keys);
    QModelIndex addKey(const GpgME::Key &key);
    QList<QModelIndex> addKeys(const std::vector<GpgME::Key> &keys);
    void removeKey(const GpgME::Key &key);
    void clear();

public:
    /* reimp */ int columnCount(const QModelIndex &pidx) const;
    /* reimp */ QVariant headerData(int section, Qt::Orientation o, int role = Qt::DisplayRole) const;
    /* reimp */ QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    /**
     * defines which information is displayed in tooltips
     * see Kleo::Formatting::ToolTipOption
     */
    int toolTipOptions() const;

    void setToolTipOptions(int opts);

private:
    virtual GpgME::Key doMapToKey(const QModelIndex &index) const = 0;
    virtual QModelIndex doMapFromKey(const GpgME::Key &key, int column) const = 0;
    virtual QList<QModelIndex> doAddKeys(const std::vector<GpgME::Key> &keys) = 0;
    virtual void doRemoveKey(const GpgME::Key &key) = 0;
    virtual void doClear() = 0;

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
};

}

#endif /* __KLEOPATRA_MODELS_KEYLISTMODEL_H__ */
