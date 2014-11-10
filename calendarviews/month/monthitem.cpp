/*
  Copyright (c) 2008 Bruno Virlet <bruno.virlet@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "monthitem.h"
#include "helper.h"
#include "monthgraphicsitems.h"
#include "monthscene.h"
#include "monthview.h"
#include "prefs.h"
#include "prefs_base.h" // Ugly, but needed for the Enums

#include <Akonadi/Calendar/IncidenceChanger>
#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>

#include <KCalUtils/IncidenceFormatter>
#include <KCalUtils/RecurrenceActions>

#include <KMessageBox>
#include <QDebug>

using namespace EventViews;
using namespace KCalCore;

MonthItem::MonthItem(MonthScene *monthScene)
    : mMonthScene(monthScene),
      mSelected(false),
      mMoving(false),
      mResizing(false)
{
}

MonthItem::~MonthItem()
{
    deleteAll();
}

void MonthItem::deleteAll()
{
    qDeleteAll(mMonthGraphicsItemList);
    mMonthGraphicsItemList.clear();
}

QWidget *MonthItem::parentWidget() const
{
    return mMonthScene ? mMonthScene->monthView() : 0;
}

void MonthItem::updateMonthGraphicsItems()
{
    // Remove all items
    qDeleteAll(mMonthGraphicsItemList);
    mMonthGraphicsItemList.clear();

    const QDate monthStartDate = startDate();
    const QDate monthEndDate = endDate();

    // For each row of the month view, create an item to build the whole
    // MonthItem's MonthGraphicsItems.
    for (QDate d = mMonthScene->mMonthView->actualStartDateTime().date();
            d < mMonthScene->mMonthView->actualEndDateTime().date(); d = d.addDays(7)) {
        QDate end = d.addDays(6);

        int span;
        QDate start;
        if (monthStartDate <= d && monthEndDate >= end) {   // MonthItem takes the whole line
            span = 6;
            start = d;
        } else if (monthStartDate >= d && monthEndDate <= end) {   // starts and ends on this line
            start = monthStartDate;
            span = daySpan();
        } else if (d <= monthEndDate && monthEndDate <= end) {   // MonthItem ends on this line
            span = mMonthScene->getLeftSpan(monthEndDate);
            start = d;
        } else if (d <= monthStartDate && monthStartDate <= end) {   // MonthItem begins on this line
            span = mMonthScene->getRightSpan(monthStartDate);
            start = monthStartDate;
        } else { // MonthItem is not on the line
            continue;
        }

        // A new item needs to be created
        MonthGraphicsItem *newItem = new MonthGraphicsItem(this);
        mMonthGraphicsItemList << newItem;
        newItem->setStartDate(start);
        newItem->setDaySpan(span);
    }

    if (isMoving() || isResizing()) {
        setZValue(100);
    } else {
        setZValue(0);
    }
}

void MonthItem::beginResize()
{
    mOverrideDaySpan = daySpan();
    mOverrideStartDate = startDate();
    mResizing = true;
    setZValue(100);
}

void MonthItem::endResize()
{
    setZValue(0);
    mResizing = false; // startDate() and daySpan() return real values again

    if (mOverrideStartDate != startDate() || mOverrideDaySpan != daySpan()) {
        finalizeResize(mOverrideStartDate, mOverrideStartDate.addDays(mOverrideDaySpan));
    }
}

void MonthItem::beginMove()
{
    mOverrideDaySpan = daySpan();
    mOverrideStartDate = startDate();
    mMoving = true;
    setZValue(100);
}

void MonthItem::endMove()
{
    setZValue(0);
    mMoving = false; // startDate() and daySpan() return real values again

    if (mOverrideStartDate != startDate()) {
        finalizeMove(mOverrideStartDate);
    }
}

bool MonthItem::resizeBy(int offsetToPreviousDate)
{
    bool ret = false;
    if (mMonthScene->resizeType() == MonthScene::ResizeLeft) {
        if (mOverrideDaySpan - offsetToPreviousDate >= 0) {
            mOverrideStartDate = mOverrideStartDate.addDays(offsetToPreviousDate);
            mOverrideDaySpan = mOverrideDaySpan - offsetToPreviousDate;
            ret = true;
        }
    } else if (mMonthScene->resizeType() == MonthScene::ResizeRight) {
        if (mOverrideDaySpan + offsetToPreviousDate >= 0) {
            mOverrideDaySpan = mOverrideDaySpan + offsetToPreviousDate;
            ret = true;
        }
    }

    if (ret) {
        updateMonthGraphicsItems();
    }
    return ret;
}

void MonthItem::moveBy(int offsetToPreviousDate)
{
    mOverrideStartDate = mOverrideStartDate.addDays(offsetToPreviousDate);
    updateMonthGraphicsItems();
}

void MonthItem::updateGeometry()
{
    foreach (MonthGraphicsItem *item, mMonthGraphicsItemList) {
        item->updateGeometry();
    }
}

void MonthItem::setZValue(qreal z)
{
    foreach (MonthGraphicsItem *item, mMonthGraphicsItemList) {
        item->setZValue(z);
    }
}

QDate MonthItem::startDate() const
{
    if (isMoving() || isResizing()) {
        return mOverrideStartDate;
    }

    return realStartDate();
}

QDate MonthItem::endDate() const
{
    if (isMoving() || isResizing()) {
        return mOverrideStartDate.addDays(mOverrideDaySpan);
    }

    return realEndDate();
}

int MonthItem::daySpan() const
{
    if (isMoving() || isResizing()) {
        return mOverrideDaySpan;
    }

    QDateTime start(startDate());
    QDateTime end(endDate());

    if (start.isValid() && end.isValid()) {
        return start.daysTo(end);
    }

    return 0;
}

bool MonthItem::greaterThan(const MonthItem *e1, const MonthItem *e2)
{
    const QDate leftStartDate = e1->startDate();
    const QDate rightStartDate = e2->startDate();

    if (!leftStartDate.isValid() || !rightStartDate.isValid()) {
        return false;
    }

    if (leftStartDate == rightStartDate) {
        const int leftDaySpan = e1->daySpan();
        const int rightDaySpan = e2->daySpan();
        if (leftDaySpan == rightDaySpan) {
            if (e1->allDay() && !e2->allDay()) {
                return true;
            }
            if (!e1->allDay() && e2->allDay()) {
                return false;
            }
            return e1->greaterThanFallback(e2);
        } else {
            return leftDaySpan >  rightDaySpan;
        }
    }

    return leftStartDate < rightStartDate;
}

bool MonthItem::greaterThanFallback(const MonthItem *other) const
{
    const HolidayMonthItem *h = qobject_cast<const HolidayMonthItem *>(other);

    // If "other" is a holiday, display it first.
    return !h;
}

void MonthItem::updatePosition()
{
    if (!startDate().isValid() || !endDate().isValid()) {
        return;
    }

    int firstFreeSpace = 0;
    for (QDate d = startDate(); d <= endDate(); d = d.addDays(1)) {
        MonthCell *cell = mMonthScene->mMonthCellMap.value(d);
        if (!cell) {
            continue; // cell can be null if the item begins outside the month
        }
        int firstFreeSpaceTmp = cell->firstFreeSpace();
        if (firstFreeSpaceTmp > firstFreeSpace) {
            firstFreeSpace = firstFreeSpaceTmp;
        }
    }

    for (QDate d = startDate(); d <= endDate(); d = d.addDays(1)) {
        MonthCell *cell = mMonthScene->mMonthCellMap.value(d);
        if (!cell) {
            continue;
        }
        cell->addMonthItem(this, firstFreeSpace);
    }

    mPosition = firstFreeSpace;
}

QList<MonthGraphicsItem *> EventViews::MonthItem::monthGraphicsItems() const
{
    return mMonthGraphicsItemList;
}

//-----------------------------------------------------------------
// INCIDENCEMONTHITEM
IncidenceMonthItem::IncidenceMonthItem(MonthScene *monthScene,
                                       const Akonadi::ETMCalendar::Ptr &calendar,
                                       const Akonadi::Item &aitem,
                                       const KCalCore::Incidence::Ptr &incidence,
                                       const QDate &recurStartDate)
    : MonthItem(monthScene), mCalendar(calendar),
      mIncidence(incidence),
      mAkonadiItemId(aitem.id())
{
    mIsEvent = CalendarSupport::hasEvent(aitem);
    mIsJournal = CalendarSupport::hasJournal(aitem);
    mIsTodo = CalendarSupport::hasTodo(aitem);

    KCalCore::Incidence::Ptr inc = mIncidence;
    if (inc->customProperty("KABC", "BIRTHDAY") == QLatin1String("YES") ||
            inc->customProperty("KABC", "ANNIVERSARY") == QLatin1String("YES")) {
        const int years = EventViews::yearDiff(inc->dtStart().date(), recurStartDate);
        if (years > 0) {
            inc = KCalCore::Incidence::Ptr(inc->clone());
            inc->setReadOnly(false);
            inc->setDescription(i18np("%2 1 year", "%2 %1 years", years, i18n("Age:")));
            inc->setReadOnly(true);
            mIncidence = inc;
        }
    }

    connect(monthScene, SIGNAL(incidenceSelected(Akonadi::Item,QDate)),
            this, SLOT(updateSelection(Akonadi::Item,QDate)));

    // first set to 0, because it's used in startDate()
    mRecurDayOffset = 0;
    if ((mIncidence->recurs() || mIncidence->recurrenceId().isValid()) &&
            startDate().isValid() && recurStartDate.isValid()) {
        mRecurDayOffset = startDate().daysTo(recurStartDate);
    }
}

IncidenceMonthItem::~IncidenceMonthItem()
{
}

bool IncidenceMonthItem::greaterThanFallback(const MonthItem *other) const
{

    const IncidenceMonthItem *o = qobject_cast<const IncidenceMonthItem *>(other);
    if (!o) {
        return MonthItem::greaterThanFallback(other);
    }

    if (allDay() != o->allDay()) {
        return allDay();
    }
    const KCalCore::Incidence::Ptr otherIncidence = o->mIncidence;

    if (mIncidence->dtStart().time() != otherIncidence->dtStart().time()) {
        return mIncidence->dtStart().time() < otherIncidence->dtStart().time();
    }

    // as a last resort, compare uids
    return mIncidence->uid() < otherIncidence->uid();
}

QDate IncidenceMonthItem::realStartDate() const
{
    if (!mIncidence) {
        return QDate();
    }

    const KDateTime dt = mIncidence->dateTime(Incidence::RoleDisplayStart);
    const QDate start = dt.isDateOnly() ?
                        dt.date() :
                        dt.toTimeSpec(CalendarSupport::KCalPrefs::instance()->timeSpec()).date();

    return start.addDays(mRecurDayOffset);
}
QDate IncidenceMonthItem::realEndDate() const
{
    if (!mIncidence) {
        return QDate();
    }

    const KDateTime dt = mIncidence->dateTime(KCalCore::Incidence::RoleDisplayEnd);
    const QDate end = dt.isDateOnly() ?
                      dt.date() :
                      dt.toTimeSpec(CalendarSupport::KCalPrefs::instance()->timeSpec()).date();

    return end.addDays(mRecurDayOffset);
}
bool IncidenceMonthItem::allDay() const
{
    return mIncidence->allDay();
}

bool IncidenceMonthItem::isMoveable() const
{
    return monthScene()->mMonthView->calendar()->hasRight(akonadiItem(),
            Akonadi::Collection::CanChangeItem);
}
bool IncidenceMonthItem::isResizable() const
{
    return mIsEvent && monthScene()->mMonthView->calendar()->hasRight(akonadiItem(),
            Akonadi::Collection::CanChangeItem);
}

void IncidenceMonthItem::finalizeMove(const QDate &newStartDate)
{
    Q_ASSERT(isMoveable());

    if (startDate().isValid() && newStartDate.isValid()) {
        updateDates(startDate().daysTo(newStartDate),
                    startDate().daysTo(newStartDate));
    }
}
void IncidenceMonthItem::finalizeResize(const QDate &newStartDate,
                                        const QDate &newEndDate)
{
    Q_ASSERT(isResizable());

    if (startDate().isValid() && endDate().isValid() &&
            newStartDate.isValid() && newEndDate.isValid()) {
        updateDates(startDate().daysTo(newStartDate),
                    endDate().daysTo(newEndDate));
    }
}

void IncidenceMonthItem::updateDates(int startOffset, int endOffset)
{
    Akonadi::IncidenceChanger *changer = monthScene()->incidenceChanger();
    if (!changer || (startOffset == 0 && endOffset == 0)) {
        qDebug() << changer << startOffset << endOffset;
        return;
    }

    Akonadi::Item item = akonadiItem();
    item.setPayload(mIncidence);
    if (mIncidence->recurs()) {
        const int res = monthScene()->mMonthView->showMoveRecurDialog(mIncidence, startDate());
        switch (res) {
        case KCalUtils::RecurrenceActions::AllOccurrences: {
            // All occurrences
            KCalCore::Incidence::Ptr oldIncidence(mIncidence->clone());
            setNewDates(mIncidence, startOffset, endOffset);
            changer->modifyIncidence(item, oldIncidence);
            break;
        }
        case KCalUtils::RecurrenceActions::SelectedOccurrence: // Just this occurrence
        case KCalUtils::RecurrenceActions::FutureOccurrences: { // All future occurrences
            const bool thisAndFuture = (res == KCalUtils::RecurrenceActions::FutureOccurrences);
            KDateTime occurrenceDate(mIncidence->dtStart());
            occurrenceDate.setDate(startDate());
            KCalCore::Incidence::Ptr newIncidence(KCalCore::Calendar::createException(
                    mIncidence, occurrenceDate, thisAndFuture));
            if (newIncidence) {
                changer->startAtomicOperation(i18n("Move occurrence(s)"));
                setNewDates(newIncidence, startOffset, endOffset);
                changer->createIncidence(newIncidence, item.parentCollection(), parentWidget());
                changer->endAtomicOperation();
            } else {
                KMessageBox::sorry(
                    parentWidget(),
                    i18n("Unable to add the exception item to the calendar. "
                         "No change will be done."),
                    i18n("Error Occurred"));
            }
            break;
        }
        }
    } else { // Doesn't recur
        KCalCore::Incidence::Ptr oldIncidence(mIncidence->clone());
        setNewDates(mIncidence, startOffset, endOffset);
        changer->modifyIncidence(item, oldIncidence);
    }
}

void IncidenceMonthItem::updateSelection(const Akonadi::Item &incidence, const QDate &date)
{
    Q_UNUSED(date);
    setSelected(incidence == akonadiItem());
}

QString IncidenceMonthItem::text(bool end) const
{
    QString ret = mIncidence->summary();
    if (!allDay() && !mIsJournal && monthScene()->monthView()->preferences()->showTimeInMonthView()) {
        // Prepend the time str to the text
        QString timeStr;
        if (mIsTodo) {
            KCalCore::Todo::Ptr todo = mIncidence.staticCast<Todo>();
            timeStr = KCalUtils::IncidenceFormatter::timeToString(
                          todo->dtDue(), true, CalendarSupport::KCalPrefs::instance()->timeSpec());
        } else {
            if (!end) {
                timeStr = KCalUtils::IncidenceFormatter::timeToString(
                              mIncidence->dtStart(), true, CalendarSupport::KCalPrefs::instance()->timeSpec());
            } else {
                KCalCore::Event::Ptr event = mIncidence.staticCast<Event>();
                timeStr = KCalUtils::IncidenceFormatter::timeToString(
                              event->dtEnd(), true, CalendarSupport::KCalPrefs::instance()->timeSpec());
            }
        }
        if (!timeStr.isEmpty()) {
            if (!end) {
                ret = timeStr + QLatin1Char(' ') + ret;
            } else {
                ret = ret + QLatin1Char(' ') + timeStr;
            }
        }
    }

    return ret;
}

QString IncidenceMonthItem::toolTipText(const QDate &date) const
{
    return KCalUtils::IncidenceFormatter::toolTipStr(
               CalendarSupport::displayName(mCalendar.data(), akonadiItem().parentCollection()),
               mIncidence,
               date, true, CalendarSupport::KCalPrefs::instance()->timeSpec());
}

QList<QPixmap> IncidenceMonthItem::icons() const
{
    QList<QPixmap> ret;

    if (!mIncidence) {
        return ret;
    }

    bool specialEvent = false;
    Akonadi::Item item = akonadiItem();

    const QSet<EventView::ItemIcon> icons =
        monthScene()->monthView()->preferences()->monthViewIcons();

    QString customIconName;
    if (icons.contains(EventViews::EventView::CalendarCustomIcon)) {
        const QString iconName = monthScene()->monthView()->iconForItem(item);
        if (!iconName.isEmpty() && iconName != QLatin1String("view-calendar") && iconName != QLatin1String("office-calendar")) {
            customIconName = iconName;
            ret << QPixmap(cachedSmallIcon(iconName));
        }
    }

    if (mIsEvent) {
        if (mIncidence->customProperty("KABC", "ANNIVERSARY") == QLatin1String("YES")) {
            specialEvent = true;
            ret << monthScene()->anniversaryPixmap();
        } else if (mIncidence->customProperty("KABC", "BIRTHDAY") == QLatin1String("YES")) {
            specialEvent = true;
            // Disabling birthday icon because it's the birthday agent's icon
            // and we allow to display the agent's icon now.
            //ret << monthScene()->birthdayPixmap();
        }

        // smartins: Disabling the event Pixmap because:
        // 1. Save precious space so we can read the event's title better.
        // 2. We don't need a pixmap to tell us an item is an event we
        //    only need one to tell us it's not, as month view was designed for events.
        // 3. If only to-dos and journals have a pixmap they will be distinguished
        //    from event's much easier.

        // ret << monthScene()->eventPixmap();

    } else if ((mIsTodo || mIsJournal) && icons.contains(mIsTodo ?
               EventView::TaskIcon :
               EventView::JournalIcon)) {
        KDateTime occurrenceDateTime = mIncidence->dateTime(Incidence::RoleRecurrenceStart);
        occurrenceDateTime.setDate(realStartDate());

        const QString incidenceIconName = mIncidence->iconName(occurrenceDateTime);
        if (customIconName != incidenceIconName) {
            ret << QPixmap(cachedSmallIcon(incidenceIconName));
        }
    }

    if (icons.contains(EventView::ReadOnlyIcon) &&
            !monthScene()->mMonthView->calendar()->hasRight(item, Akonadi::Collection::CanChangeItem) &&
            !specialEvent) {
        ret << monthScene()->readonlyPixmap();
    }

    /* sorry, this looks too cluttered. disable until we can
       make something prettier; no idea at this time -- allen */
    if (icons.contains(EventView::ReminderIcon) &&
            mIncidence->hasEnabledAlarms() && !specialEvent) {
        ret << monthScene()->alarmPixmap();
    }
    if (icons.contains(EventView::RecurringIcon) &&
            mIncidence->recurs() && !specialEvent) {
        ret << monthScene()->recurPixmap();
    }
    //TODO: check what to do with Reply

    return ret;
}

QColor IncidenceMonthItem::catColor() const
{
    Q_ASSERT(mIncidence);
    const QStringList categories = mIncidence->categories();
    QString cat;
    if (!categories.isEmpty()) {
        cat = categories.first();
    }

    return cat.isEmpty() ? CalendarSupport::KCalPrefs::instance()->unsetCategoryColor() :
           CalendarSupport::KCalPrefs::instance()->categoryColor(cat);
}

QColor IncidenceMonthItem::bgColor() const
{
    QColor bgColor = QColor(); // Default invalid color;

    PrefsPtr prefs = monthScene()->monthView()->preferences();
    if (mIsTodo && !prefs->todosUseCategoryColors()) {
        Todo::Ptr todo = CalendarSupport::todo(akonadiItem());
        Q_ASSERT(todo);
        if (todo) {
            const QDate dtRecurrence = // this is dtDue if there's no dtRecurrence
                todo->dtRecurrence().toTimeSpec(CalendarSupport::KCalPrefs::instance()->timeSpec()).date();
            const QDate today =
                KDateTime::currentDateTime(CalendarSupport::KCalPrefs::instance()->timeSpec()).date();
            if (todo->isOverdue() && today > startDate() && startDate() >= dtRecurrence) {
                bgColor = prefs->todoOverdueColor();
            } else if (today == startDate() && !todo->isCompleted() && startDate() >= dtRecurrence) {
                bgColor = prefs->todoDueTodayColor();
            }
        }
    }

    if (!bgColor.isValid()) {
        if (prefs->monthViewColors() == PrefsBase::MonthItemResourceOnly ||
                prefs->monthViewColors() == PrefsBase::MonthItemResourceInsideCategoryOutside) {

            const QString id = QString::number(akonadiItem().storageCollectionId());
            if (id.isEmpty()) {
                // item got removed from calendar, give up.
                return QColor();
            }

            bgColor = monthScene()->monthView()->preferences()->resourceColor(id);
        } else {
            bgColor = catColor();
        }
    }

    if (!bgColor.isValid()) {
        bgColor = Qt::white;
    }

    return bgColor;
}

QColor IncidenceMonthItem::frameColor() const
{
    QColor frameColor;

    PrefsPtr prefs = monthScene()->monthView()->preferences();
    if (prefs->monthViewColors() == PrefsBase::MonthItemResourceOnly ||
            prefs->monthViewColors() == PrefsBase::MonthItemCategoryInsideResourceOutside ||
            (mIncidence->categories().isEmpty() && prefs->monthViewColors() ==
             PrefsBase::MonthItemResourceInsideCategoryOutside)) {
        Q_ASSERT(mIncidence);
        const QString id = QString::number(akonadiItem().storageCollectionId());
        if (!id.isEmpty()) {
            frameColor = prefs->resourceColor(id);
        }
    } else {
        frameColor = catColor();
    }

    return EventView::itemFrameColor(frameColor, selected());
}

Akonadi::Item IncidenceMonthItem::akonadiItem() const
{
    if (mIncidence) {
        return monthScene()->mMonthView->calendar()->item(mIncidence);
    } else {
        return Akonadi::Item();
    }
}

KCalCore::Incidence::Ptr IncidenceMonthItem::incidence() const
{
    return mIncidence;
}

Akonadi::Item::Id IncidenceMonthItem::akonadiItemId() const
{
    return mAkonadiItemId;
}

void IncidenceMonthItem::setNewDates(const KCalCore::Incidence::Ptr &incidence,
                                     int startOffset, int endOffset)
{
    if (mIsTodo) {

        // For to-dos endOffset is ignored because it will always be == to startOffset because we only
        // support moving to-dos, not resizing them. There are no multi-day to-dos.
        // Lets just call it offset to reduce confusion.
        const int offset = startOffset;

        KCalCore::Todo::Ptr todo = incidence.staticCast<Todo>();
        KDateTime due   = todo->dtDue();
        KDateTime start = todo->dtStart();
        if (due.isValid()) {   // Due has priority over start.
            // We will only move the due date, unlike events where we move both.
            due = due.addDays(offset);
            todo->setDtDue(due);

            if (start.isValid() && start > due) {
                // Start can't be bigger than due.
                todo->setDtStart(due);
            }
        } else if (start.isValid()) {
            // So we're displaying a to-do that doesn't have due date, only start...
            start = start.addDays(offset);
            todo->setDtStart(start);
        } else {
            // This never happens
            qWarning() << "Move what? uid:" << todo->uid() << "; summary=" << todo->summary();
        }
    } else {
        incidence->setDtStart(incidence->dtStart().addDays(startOffset));
        if (mIsEvent) {
            KCalCore::Event::Ptr event = incidence.staticCast<Event>();
            event->setDtEnd(event->dtEnd().addDays(endOffset));
        }
    }
}

//-----------------------------------------------------------------
// HOLIDAYMONTHITEM
HolidayMonthItem::HolidayMonthItem(MonthScene *monthScene, const QDate &date,
                                   const QString &name)
    : MonthItem(monthScene), mDate(date), mName(name)
{
}

HolidayMonthItem::~HolidayMonthItem()
{
}

bool HolidayMonthItem::greaterThanFallback(const MonthItem *other) const
{
    const HolidayMonthItem *o = qobject_cast<const HolidayMonthItem *>(other);
    if (o) {
        return MonthItem::greaterThanFallback(other);
    }

    // always put holidays on top
    return false;
}

void HolidayMonthItem::finalizeMove(const QDate &newStartDate)
{
    Q_UNUSED(newStartDate);
    Q_ASSERT(false);
}
void HolidayMonthItem::finalizeResize(const QDate &newStartDate,
                                      const QDate &newEndDate)
{
    Q_UNUSED(newStartDate);
    Q_UNUSED(newEndDate);
    Q_ASSERT(false);
}

QList<QPixmap> HolidayMonthItem::icons() const
{
    QList<QPixmap> ret;
    ret << monthScene()->holidayPixmap();

    return ret;
}

QColor HolidayMonthItem::bgColor() const
{
    // FIXME: Currently, only this value is settable in the options.
    // There is a monthHolidaysBackgroundColor() option too. Maybe it would be
    // wise to merge those two.
    return monthScene()->monthView()->preferences()->agendaHolidaysBackgroundColor();
}

QColor HolidayMonthItem::frameColor() const
{
    return Qt::black;
}

