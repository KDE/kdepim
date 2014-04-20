#include "attendeetablemodel.h"

#include <KCalCore/Attendee>
#include <KPIMUtils/Email>

#include <KDebug>

using namespace IncidenceEditorNG;

AttendeeTableModel::AttendeeTableModel(const KCalCore::Attendee::List &attendees, QObject *parent)
    : QAbstractTableModel(parent)
    , attendeeList(attendees)
{

}

int AttendeeTableModel::rowCount(const QModelIndex &/*parent*/) const
{
    return attendeeList.count();
}

int AttendeeTableModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 5;
}

Qt::ItemFlags AttendeeTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::ItemIsEnabled;
    }

    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

QVariant AttendeeTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= attendeeList.size()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case 0:
            return attendeeList[index.row()]->role();
        case 1:
            return attendeeList[index.row()]->fullName();
        case 2:
            return 0;//attendeeList.at(index.row()).available;
        case 3:
            return attendeeList[index.row()]->status();
        case 4:
            return attendeeList[index.row()]->cuType();
        }

    }
    return QVariant();
}

bool AttendeeTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    QString email, name;
    if (index.isValid() && role == Qt::EditRole) {
        switch (index.column()) {
        case 0:
            attendeeList[index.row()]->setRole(static_cast<KCalCore::Attendee::Role>(value.toInt()));
            break;
        case 1:
            KPIMUtils::extractEmailAddressAndName(value.toString(), email, name);
            attendeeList[index.row()]->setName(name);
            attendeeList[index.row()]->setEmail(email);
            break;
        case 2:
            //attendeeList[index.row()].available = value.toBool();
            break;
        case 3:
            attendeeList[index.row()]->setStatus(static_cast<KCalCore::Attendee::PartStat>(value.toInt()));
            break;
        case 4:
            attendeeList[index.row()]->setCuType(static_cast<KCalCore::Attendee::CuType>(value.toInt()));
            break;
        default:
            return false;
        }
        emit dataChanged(index, index);
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
        case 0:
            return QString("role");
        case 1:
            return QString("name");
        case 2:
            return QString("available");
        case 3:
            return QString("status");
        case 4:
            return QString("cuType");
        }
    }

    return QVariant();
}

bool AttendeeTableModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    beginInsertRows(QModelIndex(), position, position + rows);

    for (int row = 0; row < rows; ++row) {
        KCalCore::Attendee::Ptr attendee(new KCalCore::Attendee("", ""));
        attendeeList.insert(position, attendee);
    }

    endInsertRows();
    return true;
}

bool AttendeeTableModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    beginRemoveRows(QModelIndex(), position, position + rows);

    for (int row = 0; row < rows; ++row) {
        attendeeList.remove(position);
    }

    endRemoveRows();
    return true;
}

bool AttendeeTableModel::insertAttendee(int position, const KCalCore::Attendee::Ptr& attendee)
{
    beginInsertRows(QModelIndex(), position, position);

    attendeeList.insert(position, attendee);

    endInsertRows();

    return true;
}

void AttendeeTableModel::setAttendees(const KCalCore::Attendee::List attendees)
{
    emit layoutAboutToBeChanged();

    attendeeList = attendees;

    emit layoutChanged();
}


KCalCore::Attendee::List AttendeeTableModel::attendees() const
{
    return attendeeList;
}

ResourceFilterProxyModel::ResourceFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool ResourceFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex cuTypeIndex = sourceModel()->index(sourceRow, 4, sourceParent);
    KCalCore::Attendee::CuType cuType = static_cast<KCalCore::Attendee::CuType>(sourceModel()->data(cuTypeIndex).toUInt());

    return (cuType == KCalCore::Attendee::Resource || cuType == KCalCore::Attendee::Room);
}

AttendeeFilterProxyModel::AttendeeFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool AttendeeFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex cuTypeIndex = sourceModel()->index(sourceRow, 4, sourceParent);
    KCalCore::Attendee::CuType cuType = static_cast<KCalCore::Attendee::CuType>(sourceModel()->data(cuTypeIndex).toUInt());

    return !(cuType == KCalCore::Attendee::Resource || cuType == KCalCore::Attendee::Room);
}
