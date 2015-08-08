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
#ifndef KDGANTTSUMMARYHANDLINGPROXYMODEL_H
#define KDGANTTSUMMARYHANDLINGPROXYMODEL_H

#include "kdganttforwardingproxymodel.h"

namespace KDGantt
{
class KDGANTT_EXPORT SummaryHandlingProxyModel : public ForwardingProxyModel
{
    Q_OBJECT
    KDGANTT_DECLARE_PRIVATE_BASE_POLYMORPHIC(SummaryHandlingProxyModel)
public:
    explicit SummaryHandlingProxyModel(QObject *parent = Q_NULLPTR);
    virtual ~SummaryHandlingProxyModel();

    void setSourceModel(QAbstractItemModel *model) Q_DECL_OVERRIDE;

    QVariant data(const QModelIndex &proxyIndex, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;

    Qt::ItemFlags flags(const QModelIndex &idx) const Q_DECL_OVERRIDE;

protected:
    void sourceModelReset() Q_DECL_OVERRIDE;
    void sourceLayoutChanged() Q_DECL_OVERRIDE;
    void sourceDataChanged(const QModelIndex &from, const QModelIndex &to) Q_DECL_OVERRIDE;
    void sourceColumnsAboutToBeInserted(const QModelIndex &idx, int start, int end) Q_DECL_OVERRIDE;
    void sourceColumnsAboutToBeRemoved(const QModelIndex &idx, int start, int end) Q_DECL_OVERRIDE;
    void sourceRowsAboutToBeInserted(const QModelIndex &idx, int start, int end) Q_DECL_OVERRIDE;
    void sourceRowsAboutToBeRemoved(const QModelIndex &, int start, int end) Q_DECL_OVERRIDE;
};
}

#endif /* KDGANTTSUMMARYHANDLINGPROXYMODEL_H */

