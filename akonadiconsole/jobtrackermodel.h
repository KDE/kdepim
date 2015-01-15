/*
 This file is part of Akonadi.

 Copyright (c) 2009 Till Adam <adam@kde.org>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 USA.
 */

#ifndef AKONADICONSOLE_JOBTRACKERMODEL_H
#define AKONADICONSOLE_JOBTRACKERMODEL_H

#include <QtCore/QAbstractItemModel>

class JobTrackerModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    JobTrackerModel(const char *name, QObject *parent);
    virtual ~JobTrackerModel();

    /* QAIM API */
    QModelIndex index(int, int, const QModelIndex &) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    bool isEnabled() const;
public Q_SLOTS:
    void resetTracker();
    void jobsAdded(const QList< QPair<int, int> > &);
    void jobsUpdated(const QList< QPair<int, int> > &);
    void setEnabled(bool);

private:
    class Private;
    Private *const d;

};

#endif /* JOBTRACKERMODEL_H_ */
