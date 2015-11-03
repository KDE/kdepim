/****************************************************************************
 ** Copyright (C) 2001-2006 Klarälvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Gantt library.
 **
 ** This file may be distributed and/or modified under the terms of the
 ** GNU General Public License version 2 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.GPL included in the
 ** packaging of this file.
 **
 ** Licensees holding valid commercial KD Gantt licenses may use this file in
 ** accordance with the KD Gantt Commercial License Agreement provided with
 ** the Software.
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** See http://www.kdab.net/kdgantt for
 **   information about KD Gantt Commercial License Agreements.
 **
 ** Contact info@kdab.net if any conditions of this
 ** licensing are not clear to you.
 **
 **********************************************************************/
#ifndef KDGANTTFORWARDINGPROXYMODEL_H
#define KDGANTTFORWARDINGPROXYMODEL_H

#include <QAbstractProxyModel>

#include "kdganttglobal.h"

namespace KDGantt
{
class KDGANTT_EXPORT ForwardingProxyModel : public QAbstractProxyModel
{
    Q_OBJECT
    Q_DISABLE_COPY(ForwardingProxyModel)
public:
    explicit ForwardingProxyModel(QObject *parent = Q_NULLPTR);
    virtual ~ForwardingProxyModel();

    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const Q_DECL_OVERRIDE;
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const Q_DECL_OVERRIDE;

    void setSourceModel(QAbstractItemModel *model) Q_DECL_OVERRIDE;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &idx) const Q_DECL_OVERRIDE;

    int rowCount(const QModelIndex &idx = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &idx = QModelIndex()) const Q_DECL_OVERRIDE;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;

protected Q_SLOTS:
    virtual void sourceModelAboutToBeReset();
    virtual void sourceModelReset();
    virtual void sourceLayoutAboutToBeChanged();
    virtual void sourceLayoutChanged();
    virtual void sourceDataChanged(const QModelIndex &from, const QModelIndex &to);
    virtual void sourceColumnsAboutToBeInserted(const QModelIndex &idx, int start, int end);
    virtual void sourceColumnsInserted(const QModelIndex &idx, int start, int end);
    virtual void sourceColumnsAboutToBeRemoved(const QModelIndex &idx, int start, int end);
    virtual void sourceColumnsRemoved(const QModelIndex &idx, int start, int end);
    virtual void sourceRowsAboutToBeInserted(const QModelIndex &idx, int start, int end);
    virtual void sourceRowsInserted(const QModelIndex &idx, int start, int end);
    virtual void sourceRowsAboutToBeRemoved(const QModelIndex &, int start, int end);
    virtual void sourceRowsRemoved(const QModelIndex &, int start, int end);
};
}

#endif /* KDGANTTFORWARDINGPROXYMODEL_H */

