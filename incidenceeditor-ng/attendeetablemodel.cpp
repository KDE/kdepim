#include "attendeetablemodel.h"

#include <KCalCore/Attendee>
#include <KPIMUtils/Email>

#include <KDebug>

using namespace IncidenceEditorNG;

AttendeeTableModel::AttendeeTableModel(const KCalCore::Attendee::List &attendees, QObject *parent)
    : QAbstractTableModel(parent)
    , attendeeList(attendees)
    , mKeepEmpty(false)
{

}

int AttendeeTableModel::rowCount(const QModelIndex &/*parent*/) const
{
    return attendeeList.count();
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

    if (index.row() >= attendeeList.size()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case Role:
            return attendeeList[index.row()]->role();
        case FullName:
            return attendeeList[index.row()]->fullName();
        case Available:
            return 0;//attendeeList.at(index.row()).available;
        case Status:
            return attendeeList[index.row()]->status();
        case CuType:
            return attendeeList[index.row()]->cuType();
        case Response:
            return attendeeList[index.row()]->RSVP();
        case Name:
            return attendeeList[index.row()]->name();
        case Email:
            return attendeeList[index.row()]->email();
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
        case FullName:
            KPIMUtils::extractEmailAddressAndName(value.toString(), email, name);
            attendeeList[index.row()]->setName(name);
            attendeeList[index.row()]->setEmail(email);

            addEmptyAttendee(true);
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
        case FullName:
            return QString("fullname");
        case Available:
            return QString("available");
        case Status:
            return QString("status");
        case CuType:
            return QString("cuType");
        case Response:
            return QString("response");
        case Name:
            return QString("name");
        case Email:
            return QString("email");
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

    addEmptyAttendee(false);

    emit layoutChanged();
}


KCalCore::Attendee::List AttendeeTableModel::attendees() const
{
    return attendeeList;
}

void AttendeeTableModel::addEmptyAttendee(bool layoutChange)
{
    if (mKeepEmpty) {
        bool create=true;
        foreach(KCalCore::Attendee::Ptr attendee, attendeeList) {
            if (attendee->fullName().isEmpty()) {
                create=false;
                break;
            }
        }

        if (create) {
            if (layoutChange) {
                emit layoutAboutToBeChanged();
            }

            insertRows(rowCount(),1);

            if (layoutChange) {
                emit layoutChanged();
            }
        }
    }
}


bool AttendeeTableModel::keepEmpty()
{
    return mKeepEmpty;
}

void AttendeeTableModel::setKeepEmpty(bool keepEmpty)
{
    if (keepEmpty != mKeepEmpty) {
        mKeepEmpty = keepEmpty;
        addEmptyAttendee(true);
    }
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
