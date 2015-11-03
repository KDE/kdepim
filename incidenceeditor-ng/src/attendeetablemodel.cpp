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

#include "attendeetablemodel.h"

#include <klocalizedstring.h>

#include <KCalCore/Attendee>
#include <KEmailAddress>

using namespace IncidenceEditorNG;

AttendeeTableModel::AttendeeTableModel(const KCalCore::Attendee::List &attendees, QObject *parent)
    : QAbstractTableModel(parent)
    , mAttendeeList(attendees)
    , mKeepEmpty(false)
    , mRemoveEmptyLines(false)
{

}

int AttendeeTableModel::rowCount(const QModelIndex &/*parent*/) const
{
    return mAttendeeList.count();
}

int AttendeeTableModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 8;
}

Qt::ItemFlags AttendeeTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::ItemIsEnabled;
    }
    if (index.column() == Available || index.column() == Name || index.column() == Email) {          //Available is read only
        return QAbstractTableModel::flags(index);
    } else {
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    }
}

QVariant AttendeeTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= mAttendeeList.size()) {
        return QVariant();
    }

    KCalCore::Attendee::Ptr attendee = mAttendeeList[index.row()];
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case Role:
            return attendee->role();
        case FullName:
            return attendee->fullName();
        case Available: {
            AvailableStatus available = mAttendeeAvailable[attendee];
            if (role == Qt::DisplayRole) {
                switch (available) {
                case Free:
                    return i18n("Free");
                case Busy:
                    return i18n("Busy");
                case Accepted:
                    return i18n("Accepted");
                case Unkown:
                    return i18n("Unknown");
                default:
                    return i18n("Unknown");
                }
            } else {
                return available;
            }
        }
        case Status:
            return attendee->status();
        case CuType:
            return attendee->cuType();
        case Response:
            return attendee->RSVP();
        case Name:
            return attendee->name();
        case Email:
            return attendee->email();
        }

    }
    if (role == AttendeeRole) {
        return QVariant::fromValue(attendee);
    }
    return QVariant();
}

bool AttendeeTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    QString email, name;
    if (index.isValid() && role == Qt::EditRole) {
        KCalCore::Attendee::Ptr attendee = mAttendeeList[index.row()];
        switch (index.column()) {
        case Role:
            attendee->setRole(static_cast<KCalCore::Attendee::Role>(value.toInt()));
            break;
        case FullName:
            if (mRemoveEmptyLines && value.toString().trimmed().isEmpty()) {
                // Do not remove last empty line if mKeepEmpty==true (only works if initaly there is only one empty line)
                if (!mKeepEmpty || !(attendee->name().isEmpty() && attendee->email().isEmpty())) {
                    removeRows(index.row(), 1);
                    return true;
                }
            }
            KEmailAddress::extractEmailAddressAndName(value.toString(), email, name);
            attendee->setName(name);
            attendee->setEmail(email);

            addEmptyAttendee();
            break;
        case Available:
            mAttendeeAvailable[attendee] = static_cast<AvailableStatus>(value.toInt());
            break;
        case Status:
            attendee->setStatus(static_cast<KCalCore::Attendee::PartStat>(value.toInt()));
            break;
        case CuType:
            attendee->setCuType(static_cast<KCalCore::Attendee::CuType>(value.toInt()));
            break;
        case Response:
            attendee->setRSVP(value.toBool());
            break;
        default:
            return false;
        }
        Q_EMIT dataChanged(index, index);
        return true;
    }
    return false;
}

QVariant AttendeeTableModel::headerData(int section, Qt::Orientation orientation,
                                        int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case Role:
            return i18nc("vCard attendee role", "Role");
        case FullName:
            return i18nc("Attendees  (name+emailaddress)",  "Name");
        case Available:
            return i18nc("Is attendee available for incidence", "Available");
        case Status:
            return i18nc("Status of attendee in an incidence (accepted, declined, delegated, ...)", "Status");
        case CuType:
            return i18nc("Type of calendar user (vCard attribute)", "User Type");
        case Response:
            return i18nc("Has attendee to respond to the invitation", "Response");
        case Name:
            return i18nc("Attendee name", "Name");
        case Email:
            return i18nc("Attendee email",  "Email");
        }
    }

    return QVariant();
}

bool AttendeeTableModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    beginInsertRows(parent, position, position + rows - 1);

    for (int row = 0; row < rows; ++row) {
        KCalCore::Attendee::Ptr attendee(new KCalCore::Attendee(QLatin1String(""), QLatin1String("")));
        mAttendeeList.insert(position, attendee);
    }

    endInsertRows();
    return true;
}

bool AttendeeTableModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    beginRemoveRows(parent, position, position + rows - 1);

    for (int row = 0; row < rows; ++row) {
        mAttendeeAvailable.remove(mAttendeeList.at(position));
        mAttendeeList.remove(position);
    }

    endRemoveRows();
    return true;
}

bool AttendeeTableModel::insertAttendee(int position, const KCalCore::Attendee::Ptr &attendee)
{
    beginInsertRows(QModelIndex(), position, position);

    mAttendeeList.insert(position, attendee);

    endInsertRows();

    addEmptyAttendee();

    return true;
}

void AttendeeTableModel::setAttendees(const KCalCore::Attendee::List &attendees)
{
    Q_EMIT layoutAboutToBeChanged();

    mAttendeeList = attendees;
    mAttendeeAvailable = QMap<KCalCore::Attendee::Ptr, AvailableStatus>();

    addEmptyAttendee();

    Q_EMIT layoutChanged();
}

KCalCore::Attendee::List AttendeeTableModel::attendees() const
{
    return mAttendeeList;
}

void AttendeeTableModel::addEmptyAttendee()
{
    if (mKeepEmpty) {
        bool create = true;
        foreach (const KCalCore::Attendee::Ptr &attendee, mAttendeeList) {
            if (attendee->fullName().isEmpty()) {
                create = false;
                break;
            }
        }

        if (create) {
            insertRows(rowCount(), 1);
        }
    }
}

bool AttendeeTableModel::keepEmpty() const
{
    return mKeepEmpty;
}

void AttendeeTableModel::setKeepEmpty(bool keepEmpty)
{
    if (keepEmpty != mKeepEmpty) {
        mKeepEmpty = keepEmpty;
        addEmptyAttendee();
    }
}

bool AttendeeTableModel::removeEmptyLines() const
{
    return mRemoveEmptyLines;
}

void AttendeeTableModel::setRemoveEmptyLines(bool removeEmptyLines)
{
    mRemoveEmptyLines = removeEmptyLines;
}

ResourceFilterProxyModel::ResourceFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool ResourceFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex cuTypeIndex = sourceModel()->index(sourceRow, AttendeeTableModel::CuType, sourceParent);
    KCalCore::Attendee::CuType cuType = static_cast<KCalCore::Attendee::CuType>(sourceModel()->data(cuTypeIndex).toUInt());

    return (cuType == KCalCore::Attendee::Resource || cuType == KCalCore::Attendee::Room);
}

AttendeeFilterProxyModel::AttendeeFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool AttendeeFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex cuTypeIndex = sourceModel()->index(sourceRow, AttendeeTableModel::CuType, sourceParent);
    KCalCore::Attendee::CuType cuType = static_cast<KCalCore::Attendee::CuType>(sourceModel()->data(cuTypeIndex).toUInt());

    return !(cuType == KCalCore::Attendee::Resource || cuType == KCalCore::Attendee::Room);
}
