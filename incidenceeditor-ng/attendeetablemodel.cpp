#include "attendeetablemodel.h"

#include <KCalCore/Attendee>
#include <KPIMUtils/Email>

#include <KDebug>

using namespace IncidenceEditorNG;

AttendeeTableModel::AttendeeTableModel(const KCalCore::Attendee::List &attendees, QObject *parent)
    : QAbstractTableModel(parent)
    , attendeeList(attendees)
{
    insertRows(0,1);
}

int AttendeeTableModel::rowCount(const QModelIndex &/*parent*/) const
{
    return attendeeList.count();
}

int AttendeeTableModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 6;
}

Qt::ItemFlags AttendeeTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::ItemIsEnabled;
    }
    if (index.column() == Available) {          //Available is read only
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

    if (index.row() >= attendeeList.size()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case Role:
            return attendeeList[index.row()]->role();
        case Name:
            return attendeeList[index.row()]->fullName();
        case Available:
            return 0;//attendeeList.at(index.row()).available;
        case Status:
            return attendeeList[index.row()]->status();
        case CuType:
            return attendeeList[index.row()]->cuType();
        case Response:
            return attendeeList[index.row()]->RSVP();
        }

    }
    return QVariant();
}

bool AttendeeTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    QString email, name;
    if (index.isValid() && role == Qt::EditRole) {
        switch (index.column()) {
        case Role:
            attendeeList[index.row()]->setRole(static_cast<KCalCore::Attendee::Role>(value.toInt()));
            break;
        case Name:
            KPIMUtils::extractEmailAddressAndName(value.toString(), email, name);
            attendeeList[index.row()]->setName(name);
            attendeeList[index.row()]->setEmail(email);
            break;
        case Available:
            //attendeeList[index.row()].available = value.toBool();
            break;
        case Status:
            attendeeList[index.row()]->setStatus(static_cast<KCalCore::Attendee::PartStat>(value.toInt()));
            break;
        case CuType:
            attendeeList[index.row()]->setCuType(static_cast<KCalCore::Attendee::CuType>(value.toInt()));
            break;
        case Response:
            attendeeList[index.row()]->setRSVP(value.toBool());
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
        case Role:
            return QString("role");
        case Name:
            return QString("name");
        case Available:
            return QString("available");
        case Status:
            return QString("status");
        case CuType:
            return QString("cuType");
        case Response:
            return QString("response");
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
