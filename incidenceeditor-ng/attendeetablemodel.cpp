#include "attendeetablemodel.h"

#include <klocalizedstring.h>

#include <KCalCore/Attendee>
#include <KPIMUtils/Email>

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

    KCalCore::Attendee::Ptr attendee = attendeeList[index.row()];
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case Role:
            return attendee->role();
        case FullName:
            return attendee->fullName();
        case Available: {
            AvailableStatus available = attendeeAvailable[attendee];
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
    if (role ==  AttendeeRole) {
        return QVariant::fromValue(attendee);
    }
    return QVariant();
}

bool AttendeeTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    QString email, name;
    if (index.isValid() && role == Qt::EditRole) {
        KCalCore::Attendee::Ptr attendee = attendeeList[index.row()];
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
            KPIMUtils::extractEmailAddressAndName(value.toString(), email, name);
            attendee->setName(name);
            attendee->setEmail(email);

            addEmptyAttendee();
            break;
        case Available:
            attendeeAvailable[attendee] = static_cast<AvailableStatus>(value.toInt());
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
            return i18nc("vcard attendee role", "Role");
        case FullName:
            return i18nc("attendees  (name+emailaddress)",  "Fullname");
        case Available:
            return i18nc("is attendee available for incidence", "Available");
        case Status:
          return i18nc("Status of attendee in an incidence (accepted, declined, delegated, ...)", "Status");
        case CuType:
            return i18nc("Type of resource (vCard attribute)", "cuType");
        case Response:
            return i18nc("has attendee to respond to the invitation", "Response");
        case Name:
            return i18nc("attendee name", "name");
        case Email:
            return i18nc("attendee email",  "email");
        }
    }

    return QVariant();
}

bool AttendeeTableModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    beginInsertRows(parent, position, position + rows-1);

    for (int row = 0; row < rows; ++row) {
        KCalCore::Attendee::Ptr attendee(new KCalCore::Attendee("", ""));
        attendeeList.insert(position, attendee);
    }

    endInsertRows();
    return true;
}

bool AttendeeTableModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    beginRemoveRows(parent, position, position + rows-1);

    for (int row = 0; row < rows; ++row) {
        attendeeAvailable.remove(attendeeList.at(position));
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

    addEmptyAttendee();

    return true;
}

void AttendeeTableModel::setAttendees(const KCalCore::Attendee::List attendees)
{
    emit layoutAboutToBeChanged();

    attendeeList = attendees;
    attendeeAvailable = QMap<KCalCore::Attendee::Ptr, AvailableStatus>();

    addEmptyAttendee();

    emit layoutChanged();
}


KCalCore::Attendee::List AttendeeTableModel::attendees() const
{
    return attendeeList;
}

void AttendeeTableModel::addEmptyAttendee()
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
            insertRows(rowCount(),1);
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
        addEmptyAttendee();
    }
}

bool AttendeeTableModel::removeEmptyLines()
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
