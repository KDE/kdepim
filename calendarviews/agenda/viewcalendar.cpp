#include "viewcalendar.h"
#include "agendaview.h"
#include "helper.h"

#include <calendarsupport/utils.h>

using namespace EventViews;

ViewCalendar::~ViewCalendar()
{
}

MultiViewCalendar::~MultiViewCalendar()
{
}

KCalCore::Calendar::Ptr MultiViewCalendar::getCalendar() const
{
    return KCalCore::Calendar::Ptr();
}

KCalCore::Incidence::List MultiViewCalendar::incidences() const
{
    KCalCore::Incidence::List list;
    foreach(const ViewCalendar::Ptr &cal, mSubCalendars) {
        if (cal->getCalendar()) {
            list += cal->getCalendar()->incidences();
        }
    }
    return list;
}

int MultiViewCalendar::calendars() const
{
    return mSubCalendars.size();
}


ViewCalendar::Ptr MultiViewCalendar::findCalendar(const KCalCore::Incidence::Ptr &incidence) const
{
    foreach(const ViewCalendar::Ptr &cal, mSubCalendars) {
        if (cal->isValid(incidence)) {
            return cal;
        }
    }
    return ViewCalendar::Ptr();
}

void MultiViewCalendar::addCalendar(const ViewCalendar::Ptr &calendar)
{
    if (!mSubCalendars.contains(calendar)) {
        mSubCalendars.append(calendar);
    }
}

void MultiViewCalendar::setETMCalendar(const Akonadi::ETMCalendar::Ptr &calendar)
{
    if (!mETMCalendar) {
        mETMCalendar = AkonadiViewCalendar::Ptr(new AkonadiViewCalendar);
        mETMCalendar->mAgendaView = mAgendaView;
    }
    mETMCalendar->mCalendar = calendar;
    addCalendar(mETMCalendar);
}

QString MultiViewCalendar::displayName(const KCalCore::Incidence::Ptr &incidence) const
{
    ViewCalendar::Ptr cal = findCalendar(incidence);
    if (cal) {
        return cal->displayName(incidence);
    }
    return QString();
}

QString MultiViewCalendar::iconForIncidence(const KCalCore::Incidence::Ptr &incidence) const
{
    ViewCalendar::Ptr cal = findCalendar(incidence);
    if (cal) {
        return cal->iconForIncidence(incidence);
    }
    return QString();
}

bool MultiViewCalendar::isValid(const KCalCore::Incidence::Ptr &incidence) const
{
    ViewCalendar::Ptr cal = findCalendar(incidence);
    return cal;
}

QColor MultiViewCalendar::resourceColor(const KCalCore::Incidence::Ptr &incidence) const
{
    ViewCalendar::Ptr cal = findCalendar(incidence);
    if (cal) {
        return cal->resourceColor(incidence);
    }
    return QColor();
}

Akonadi::Item MultiViewCalendar::item(const KCalCore::Incidence::Ptr &incidence) const
{
    if (mETMCalendar->isValid(incidence)) {
        return mETMCalendar->item(incidence);
    }

    return Akonadi::Item();
}

AkonadiViewCalendar::~AkonadiViewCalendar()
{
}

bool AkonadiViewCalendar::isValid(const KCalCore::Incidence::Ptr &incidence) const
{
    if (!mCalendar) {
        return false;
    }

    if (item(incidence).isValid()) {
        return true;
    }
    return false;
}

Akonadi::Item AkonadiViewCalendar::item(const KCalCore::Incidence::Ptr &incidence) const
{
    if (!mCalendar) {
        return Akonadi::Item();
    }
    bool ok = false;
    Akonadi::Item::Id id = incidence->customProperty("VOLATILE", "AKONADI-ID").toLongLong(&ok);

    if (id == -1 || !ok) {
        id = mCalendar->item(incidence).id();

        if (id == -1) {
            // Ok, we really don't know the ID, give up.
            kDebug() << "Item is invalid. uid = "
                       << incidence->instanceIdentifier();
            return Akonadi::Item();
        }
        return mCalendar->item(incidence->instanceIdentifier());
    }
    return mCalendar->item(id);
}

QString AkonadiViewCalendar::displayName(const KCalCore::Incidence::Ptr &incidence) const
{
    return CalendarSupport::displayName( mCalendar.data(), item(incidence).parentCollection() );
}

QColor AkonadiViewCalendar::resourceColor(const KCalCore::Incidence::Ptr &incidence) const
{
    return EventViews::resourceColor( item(incidence), mAgendaView->preferences() );
}

QString AkonadiViewCalendar::iconForIncidence(const KCalCore::Incidence::Ptr &incidence) const
{
    return mAgendaView->iconForItem(item(incidence));
}

KDateTime::Spec AkonadiViewCalendar::timeSpec() const
{
    return mCalendar->timeSpec();
}

KCalCore::Calendar::Ptr AkonadiViewCalendar::getCalendar() const
{
    return mCalendar;
}
