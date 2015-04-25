/*
 *  Copyright (C) 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 *  This library is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  This library is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 *  License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to the
 *  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *  02110-1301, USA.
 */

#ifndef INCIDENCEEDITOR_INCIDENCERESOURCETABLEMODEL_H
#define INCIDENCEEDITOR_INCIDENCERESOURCETABLEMODEL_H

#include <KCalCore/Attendee>

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QModelIndex>
#include <QVector>

namespace IncidenceEditorNG
{

class AttendeeTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Roles {
        AttendeeRole = Qt::UserRole
    };

    enum Columns {
        CuType,
        Role,
        FullName,
        Name,
        Email,
        Available,
        Status,
        Response
    };

    enum AvailableStatus {
        Unkown,
        Free,
        Accepted,
        Busy,
        Tentative
    };

    explicit AttendeeTableModel(const KCalCore::Attendee::List &resources, QObject *parent = Q_NULLPTR);

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) Q_DECL_OVERRIDE;

    bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex()) Q_DECL_OVERRIDE;
    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex()) Q_DECL_OVERRIDE;

    bool insertAttendee(int position, const KCalCore::Attendee::Ptr &attendee);

    void setAttendees(const KCalCore::Attendee::List &resources);
    KCalCore::Attendee::List attendees() const;

    void setKeepEmpty(bool keepEmpty);
    bool keepEmpty() const;

    void setRemoveEmptyLines(bool removeEmptyLines);
    bool removeEmptyLines() const;
private:
    void addEmptyAttendee();

    KCalCore::Attendee::List mAttendeeList;
    QMap<KCalCore::Attendee::Ptr, AvailableStatus> mAttendeeAvailable;
    bool mKeepEmpty;
    bool mRemoveEmptyLines;
};

class ResourceFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit ResourceFilterProxyModel(QObject *parent = Q_NULLPTR);
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const Q_DECL_OVERRIDE;
};

class AttendeeFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit AttendeeFilterProxyModel(QObject *parent = Q_NULLPTR);
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const Q_DECL_OVERRIDE;
};

}

#endif
