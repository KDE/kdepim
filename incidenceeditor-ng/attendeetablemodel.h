
/*
  Copyright (C) 2014 Sandro Knauß <knauss@kolabsys.com>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
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

    AttendeeTableModel(const KCalCore::Attendee::List &resources, QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole);

    bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex());

    bool insertAttendee(int position, const KCalCore::Attendee::Ptr &attendee);

    void setAttendees(const KCalCore::Attendee::List resources);
    KCalCore::Attendee::List attendees() const;

    void setKeepEmpty(bool keepEmpty);
    bool keepEmpty();
private:
    void addEmptyAttendee(bool layoutChange);

    KCalCore::Attendee::List attendeeList;
    bool mKeepEmpty;
};

class ResourceFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    ResourceFilterProxyModel(QObject *parent = 0);
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
};

class AttendeeFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    AttendeeFilterProxyModel(QObject *parent = 0);
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
};


}

#endif


