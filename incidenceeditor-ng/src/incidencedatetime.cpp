/*
  Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

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

#include "incidencedatetime.h"
#include "ui_dialogdesktop.h"

#include <CalendarSupport/KCalPrefs>

#include <KCalCore/ICalTimeZones>
#include <KCalUtils/IncidenceFormatter>

#include "incidenceeditor_debug.h"
#include <KSystemTimeZone>

using namespace IncidenceEditorNG;

/**
 * Returns true if the incidence's dates are equal to the default ones specified in config.
 */
static bool incidenceHasDefaultTimes(const KCalCore::Incidence::Ptr &incidence)
{
    if (!incidence || incidence->allDay()) {
        return false;
    }

    QTime defaultDuration = CalendarSupport::KCalPrefs::instance()->defaultDuration().time();
    if (!defaultDuration.isValid()) {
        return false;
    }

    QTime defaultStart = CalendarSupport::KCalPrefs::instance()->mStartTime.time();
    if (!defaultStart.isValid()) {
        return false;
    }

    if (incidence->dtStart().time() == defaultStart) {
        if (incidence->type() == KCalCore::Incidence::TypeJournal) {
            return true; // no duration to compare with
        }

        const KDateTime start = incidence->dtStart();
        const KDateTime end   = incidence->dateTime(KCalCore::Incidence::RoleEnd);
        if (!end.isValid() || !start.isValid()) {
            return false;
        }

        const int durationInSeconds = defaultDuration.hour() * 3600 + defaultDuration.minute() * 60;
        return start.secsTo(end) == durationInSeconds;
    }

    return false;
}

IncidenceDateTime::IncidenceDateTime(Ui::EventOrTodoDesktop *ui)
    : IncidenceEditor(0), mTimeZones(new KCalCore::ICalTimeZones), mUi(ui),
      mTimezoneCombosWereVisibile(false)
{
    setTimeZonesVisibility(false);
    setObjectName(QStringLiteral("IncidenceDateTime"));

    mUi->mTimeZoneLabel->setVisible(!mUi->mWholeDayCheck->isChecked());
    connect(mUi->mTimeZoneLabel, &QLabel::linkActivated, this, &IncidenceDateTime::toggleTimeZoneVisibility);
    mUi->mTimeZoneLabel->setContextMenuPolicy(Qt::NoContextMenu);

    QList<QLineEdit *> lineEdits;
    lineEdits << mUi->mStartDateEdit->lineEdit() << mUi->mEndDateEdit->lineEdit()
              << mUi->mStartTimeEdit->lineEdit() << mUi->mEndTimeEdit->lineEdit();
    foreach (QLineEdit *lineEdit, lineEdits) {
        if (lineEdit) {
            lineEdit->setClearButtonEnabled(false);
        }
    }

    connect(mUi->mFreeBusyCheck, &QCheckBox::toggled, this, &IncidenceDateTime::checkDirtyStatus);
    connect(mUi->mWholeDayCheck, &QCheckBox::toggled, this, &IncidenceDateTime::enableTimeEdits);
    connect(mUi->mWholeDayCheck, &QCheckBox::toggled, this, &IncidenceDateTime::checkDirtyStatus);

    connect(this, &IncidenceDateTime::startDateChanged, this, &IncidenceDateTime::updateStartToolTips);
    connect(this, &IncidenceDateTime::startTimeChanged, this, &IncidenceDateTime::updateStartToolTips);
    connect(this, &IncidenceDateTime::endDateChanged, this, &IncidenceDateTime::updateEndToolTips);
    connect(this, &IncidenceDateTime::endTimeChanged, this, &IncidenceDateTime::updateEndToolTips);
    connect(mUi->mWholeDayCheck, &QCheckBox::toggled, this, &IncidenceDateTime::updateStartToolTips);
    connect(mUi->mWholeDayCheck, &QCheckBox::toggled, this, &IncidenceDateTime::updateEndToolTips);
    connect(mUi->mStartCheck, &QCheckBox::toggled, this, &IncidenceDateTime::updateStartToolTips);
    connect(mUi->mEndCheck, &QCheckBox::toggled, this, &IncidenceDateTime::updateEndToolTips);
}

IncidenceDateTime::~IncidenceDateTime()
{
    delete mTimeZones;
}

bool IncidenceDateTime::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FocusIn) {
        if (obj == mUi->mStartDateEdit) {
            qCDebug(INCIDENCEEDITOR_LOG) << "emiting startDateTime: " << mUi->mStartDateEdit;
            Q_EMIT startDateFocus(obj);
        } else if (obj == mUi->mEndDateEdit) {
            qCDebug(INCIDENCEEDITOR_LOG) << "emiting endDateTime: " << mUi->mEndDateEdit;
            Q_EMIT endDateFocus(obj);
        } else if (obj == mUi->mStartTimeEdit) {
            qCDebug(INCIDENCEEDITOR_LOG) << "emiting startTimeTime: " << mUi->mStartTimeEdit;
            Q_EMIT startTimeFocus(obj);
        } else if (obj == mUi->mEndTimeEdit) {
            qCDebug(INCIDENCEEDITOR_LOG) << "emiting endTimeTime: " << mUi->mEndTimeEdit;
            Q_EMIT endTimeFocus(obj);
        }

        return true;
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}

void IncidenceDateTime::load(const KCalCore::Incidence::Ptr &incidence)
{
    if (mLoadedIncidence && *mLoadedIncidence == *incidence) {
        return;
    }

    const bool isTemplate             = incidence->customProperty("kdepim", "isTemplate") == QLatin1String("true");
    const bool templateOverridesTimes = incidenceHasDefaultTimes(mLoadedIncidence);

    mLoadedIncidence = incidence;
    mLoadingIncidence = true;

    // We can only handle events or todos.
    if (KCalCore::Todo::Ptr todo = IncidenceDateTime::incidence<KCalCore::Todo>()) {
        load(todo, isTemplate, templateOverridesTimes);
    } else if (KCalCore::Event::Ptr event = IncidenceDateTime::incidence<KCalCore::Event>()) {
        load(event, isTemplate, templateOverridesTimes);
    } else if (KCalCore::Journal::Ptr journal = IncidenceDateTime::incidence<KCalCore::Journal>()) {
        load(journal, isTemplate, templateOverridesTimes);
    } else {
        qCDebug(INCIDENCEEDITOR_LOG) << "Not an Incidence.";
    }

    // Set the initial times before calling enableTimeEdits, as enableTimeEdits
    // assumes that the initial times are initialized.
    mInitialStartDT = currentStartDateTime();
    mInitialEndDT = currentEndDateTime();

    enableTimeEdits();

    if (mUi->mTimeZoneComboStart->currentIndex() == 0) {   // Floating
        mInitialStartDT.setTimeSpec(mInitialStartDT.toLocalZone().timeSpec());
    }

    if (mUi->mTimeZoneComboEnd->currentIndex() == 0) {   // Floating
        mInitialEndDT.setTimeSpec(mInitialEndDT.toLocalZone().timeSpec());
    }

    mWasDirty = false;
    mLoadingIncidence = false;
}

void IncidenceDateTime::save(const KCalCore::Incidence::Ptr &incidence)
{
    if (KCalCore::Todo::Ptr todo =
                IncidenceDateTime::incidence<KCalCore::Todo>(incidence)) {
        save(todo);
    } else if (KCalCore::Event::Ptr event =
                   IncidenceDateTime::incidence<KCalCore::Event>(incidence)) {
        save(event);
    } else if (KCalCore::Journal::Ptr journal =
                   IncidenceDateTime::incidence<KCalCore::Journal>(incidence)) {
        save(journal);
    } else {
        Q_ASSERT_X(false, "IncidenceDateTimeEditor::save",
                   "Only implemented for todos, events and journals");
    }
}

bool IncidenceDateTime::isDirty() const
{
    if (KCalCore::Todo::Ptr todo = IncidenceDateTime::incidence<KCalCore::Todo>()) {
        return isDirty(todo);
    } else if (KCalCore::Event::Ptr event = IncidenceDateTime::incidence<KCalCore::Event>()) {
        return isDirty(event);
    } else if (KCalCore::Journal::Ptr journal = IncidenceDateTime::incidence<KCalCore::Journal>()) {
        return isDirty(journal);
    } else {
        Q_ASSERT_X(false, "IncidenceDateTimeEditor::isDirty",
                   "Only implemented for todos and events");
        return false;
    }
}

void IncidenceDateTime::setActiveDate(const QDate &activeDate)
{
    mActiveDate = activeDate;
}

QDate IncidenceDateTime::startDate() const
{
    return currentStartDateTime().date();
}

QDate IncidenceDateTime::endDate() const
{
    return currentEndDateTime().date();
}

QTime IncidenceDateTime::startTime() const
{
    return currentStartDateTime().time();
}

QTime IncidenceDateTime::endTime() const
{
    return currentEndDateTime().time();
}

/// private slots for General

void IncidenceDateTime::setTimeZonesVisibility(bool visible)
{
    static const QString tz(i18nc("@action show or hide the time zone widgets", "Time zones"));
    QString placeholder(QStringLiteral("<a href=\"hide\"><font color='blue'>&lt;&lt; %1</font></a>"));
    if (visible) {
        placeholder = placeholder.arg(tz);
    } else {
        placeholder = QStringLiteral("<a href=\"show\"><font color='blue'>%1 &gt;&gt;</font></a>");
        placeholder = placeholder.arg(tz);
    }
    mUi->mTimeZoneLabel->setText(placeholder);

    mUi->mTimeZoneComboStart->setVisible(visible);
    mUi->mTimeZoneComboEnd->setVisible(visible && type() != KCalCore::Incidence::TypeJournal);
}

void IncidenceDateTime::toggleTimeZoneVisibility()
{
    setTimeZonesVisibility(!mUi->mTimeZoneComboStart->isVisible());
}

void IncidenceDateTime::updateStartTime(const QTime &newTime)
{
    if (!newTime.isValid()) {
        return;
    }

    KDateTime endDateTime = currentEndDateTime();
    const int secsep = mCurrentStartDateTime.secsTo(endDateTime);
    mCurrentStartDateTime.setTime(newTime);
    if (mUi->mEndCheck->isChecked()) {
        // Only update the end time when it is actually enabled, adjust end time so
        // that the event/todo has the same duration as before.
        endDateTime = mCurrentStartDateTime.addSecs(secsep);
        mUi->mEndTimeEdit->setTime(endDateTime.time());
        mUi->mEndDateEdit->setDate(endDateTime.date());
    }

    Q_EMIT startTimeChanged(mCurrentStartDateTime.time());
    checkDirtyStatus();
}

void IncidenceDateTime::updateStartDate(const QDate &newDate)
{
    if (!newDate.isValid()) {
        return;
    }

    const bool dateChanged = mCurrentStartDateTime.date().day() != newDate.day() ||
                             mCurrentStartDateTime.date().month() != newDate.month();

    KDateTime endDateTime = currentEndDateTime();
    int daysep = mCurrentStartDateTime.daysTo(endDateTime);
    mCurrentStartDateTime.setDate(newDate);
    if (mUi->mEndCheck->isChecked()) {
        // Only update the end time when it is actually enabled, adjust end time so
        // that the event/todo has the same duration as before.
        endDateTime.setDate(mCurrentStartDateTime.date().addDays(daysep));
        mUi->mEndDateEdit->setDate(endDateTime.date());
    }

    checkDirtyStatus();

    if (dateChanged) {
        Q_EMIT startDateChanged(mCurrentStartDateTime.date());
    }
}

void IncidenceDateTime::updateStartSpec()
{
    const QDate prevDate = mCurrentStartDateTime.date();

    if (mUi->mEndCheck->isChecked() &&
            currentEndDateTime().timeSpec() == mCurrentStartDateTime.timeSpec()) {
        mUi->mTimeZoneComboEnd->selectTimeSpec(mUi->mTimeZoneComboStart->selectedTimeSpec());
    }

    mCurrentStartDateTime.setTimeSpec(mUi->mTimeZoneComboStart->selectedTimeSpec());

    const bool dateChanged = mCurrentStartDateTime.date().day() != prevDate.day() ||
                             mCurrentStartDateTime.date().month() != prevDate.month();

    if (dateChanged) {
        Q_EMIT startDateChanged(mCurrentStartDateTime.date());
    }

    if (type() == KCalCore::Incidence::TypeJournal) {
        checkDirtyStatus();
    }
}

/// private slots for Todo

void IncidenceDateTime::enableStartEdit(bool enable)
{
    mUi->mStartDateEdit->setEnabled(enable);

    if (mUi->mEndCheck->isChecked() || mUi->mStartCheck->isChecked()) {
        mUi->mWholeDayCheck->setEnabled(true);
        setTimeZoneLabelEnabled(!mUi->mWholeDayCheck->isChecked());
    } else {
        mUi->mWholeDayCheck->setEnabled(false);
        mUi->mWholeDayCheck->setChecked(false);
        setTimeZoneLabelEnabled(false);
    }

    if (enable) {
        mUi->mStartTimeEdit->setEnabled(!mUi->mWholeDayCheck->isChecked());
        mUi->mTimeZoneComboStart->setEnabled(!mUi->mWholeDayCheck->isChecked());
    } else {
        mUi->mStartTimeEdit->setEnabled(false);
        mUi->mTimeZoneComboStart->setEnabled(false);
    }

    mUi->mTimeZoneComboStart->setFloating(!mUi->mTimeZoneComboStart->isEnabled());
    checkDirtyStatus();
}

void IncidenceDateTime::enableEndEdit(bool enable)
{
    mUi->mEndDateEdit->setEnabled(enable);

    if (mUi->mEndCheck->isChecked() || mUi->mStartCheck->isChecked()) {
        mUi->mWholeDayCheck->setEnabled(true);
        setTimeZoneLabelEnabled(!mUi->mWholeDayCheck->isChecked());
    } else {
        mUi->mWholeDayCheck->setEnabled(false);
        mUi->mWholeDayCheck->setChecked(false);
        setTimeZoneLabelEnabled(false);
    }

    if (enable) {
        mUi->mEndTimeEdit->setEnabled(!mUi->mWholeDayCheck->isChecked());
        mUi->mTimeZoneComboEnd->setEnabled(!mUi->mWholeDayCheck->isChecked());
    } else {
        mUi->mEndTimeEdit->setEnabled(false);
        mUi->mTimeZoneComboEnd->setEnabled(false);
    }

    mUi->mTimeZoneComboEnd->setFloating(!mUi->mTimeZoneComboEnd->isEnabled());
    checkDirtyStatus();
}

bool IncidenceDateTime::timeZonesAreLocal(const KDateTime &start, const KDateTime &end)
{
    // Returns false if the incidence start or end timezone is not the local zone.

    if ((start.isValid() && !start.timeSpec().isLocalZone()) ||
            (end.isValid() && !end.timeSpec().isLocalZone())) {
        return false;
    } else {
        return true;
    }
}

void IncidenceDateTime::enableTimeEdits()
{
    // NOTE: assumes that the initial times are initialized.
    const bool wholeDayChecked = mUi->mWholeDayCheck->isChecked();

    setTimeZoneLabelEnabled(!wholeDayChecked);

    if (mUi->mStartCheck->isChecked()) {
        mUi->mStartTimeEdit->setEnabled(!wholeDayChecked);
        mUi->mTimeZoneComboStart->setEnabled(!wholeDayChecked);
        mUi->mTimeZoneComboStart->setFloating(wholeDayChecked, mInitialStartDT.timeSpec());
    }
    if (mUi->mEndCheck->isChecked()) {
        mUi->mEndTimeEdit->setEnabled(!wholeDayChecked);
        mUi->mTimeZoneComboEnd->setEnabled(!wholeDayChecked);
        mUi->mTimeZoneComboEnd->setFloating(wholeDayChecked, mInitialEndDT.timeSpec());
    }

    /**
       When editing a whole-day event, unchecking mWholeDayCheck shouldn't set both
       times to 00:00. DTSTART must always be smaller than DTEND
     */
    if (sender() == mUi->mWholeDayCheck && !wholeDayChecked &&  // Somebody unchecked it, the incidence will now have time.
            mUi->mStartCheck->isChecked() && mUi->mEndCheck->isChecked() && // The incidence has both start and end/due dates
            currentStartDateTime() == currentEndDateTime()) {  // DTSTART == DTEND. This is illegal, lets correct it.
        // Not sure about the best time here... doesn't really matter, when someone unchecks mWholeDayCheck, she will
        // always want to set a time.
        mUi->mStartTimeEdit->setTime(QTime(0, 0));
        mUi->mEndTimeEdit->setTime(QTime(1, 0));
    }

    const bool currentlyVisible = mUi->mTimeZoneLabel->text().contains(QStringLiteral("&lt;&lt;"));
    setTimeZonesVisibility(!wholeDayChecked && mTimezoneCombosWereVisibile);
    mTimezoneCombosWereVisibile = currentlyVisible;
    if (!wholeDayChecked && !timeZonesAreLocal(currentStartDateTime(), currentEndDateTime())) {
        setTimeZonesVisibility(true);
        mTimezoneCombosWereVisibile = true;
    }
}

bool IncidenceDateTime::isDirty(const KCalCore::Todo::Ptr &todo) const
{
    Q_ASSERT(todo);

    const bool hasDateTimes = mUi->mStartCheck->isChecked() ||
                              mUi->mEndCheck->isChecked();

    // First check the start time/date of the todo
    if (todo->hasStartDate() != mUi->mStartCheck->isChecked()) {
        return true;
    }

    if ((hasDateTimes && todo->allDay()) != mUi->mWholeDayCheck->isChecked()) {
        return true;
    }

    if (todo->hasDueDate() != mUi->mEndCheck->isChecked()) {
        return true;
    }

    if (mUi->mStartCheck->isChecked()) {
        // Use mActiveStartTime. This is the KDateTime::Spec selected on load coming from
        // the combobox. We use this one as it can slightly differ (e.g. missing
        // country code in the incidence time spec) from the incidence.
        if (currentStartDateTime() != mInitialStartDT) {
            return true;
        }
    }

    if (mUi->mEndCheck->isChecked() && currentEndDateTime() != mInitialEndDT) {
        return true;
    }

    return false;
}

/// Event specific methods

bool IncidenceDateTime::isDirty(const KCalCore::Event::Ptr &event) const
{
    if (event->allDay() != mUi->mWholeDayCheck->isChecked()) {
        return true;
    }

    if (mUi->mFreeBusyCheck->isChecked() &&
            event->transparency() != KCalCore::Event::Opaque) {
        return true;
    }

    if (!mUi->mFreeBusyCheck->isChecked() &&
            event->transparency() != KCalCore::Event::Transparent) {
        return true;
    }

    if (event->allDay()) {
        if (mUi->mStartDateEdit->date() != mInitialStartDT.date() ||
                mUi->mEndDateEdit->date() != mInitialEndDT.date()) {
            return true;
        }
    } else {
        if (currentStartDateTime() != mInitialStartDT ||
                currentEndDateTime() != mInitialEndDT ||
                currentStartDateTime().timeSpec() != mInitialStartDT.timeSpec() ||
                currentEndDateTime().timeSpec() != mInitialEndDT.timeSpec()) {
            return true;
        }
    }

    return false;
}

bool IncidenceDateTime::isDirty(const KCalCore::Journal::Ptr &journal) const
{
    if (journal->allDay() != mUi->mWholeDayCheck->isChecked()) {
        return true;
    }

    if (journal->allDay()) {
        if (mUi->mStartDateEdit->date() != mInitialStartDT.date()) {
            return true;
        }
    } else {
        if (currentStartDateTime() != mInitialStartDT) {
            return true;
        }
    }

    return false;
}

/// Private methods

KDateTime IncidenceDateTime::currentStartDateTime() const
{
    return KDateTime(
               mUi->mStartDateEdit->date(),
               mUi->mStartTimeEdit->time(),
               mUi->mTimeZoneComboStart->selectedTimeSpec());
}

KDateTime IncidenceDateTime::currentEndDateTime() const
{
    return KDateTime(
               mUi->mEndDateEdit->date(),
               mUi->mEndTimeEdit->time(),
               mUi->mTimeZoneComboEnd->selectedTimeSpec());
}

void IncidenceDateTime::load(const KCalCore::Event::Ptr &event, bool isTemplate, bool templateOverridesTimes)
{
    // First en/disable the necessary ui bits and pieces
    mUi->mStartCheck->setVisible(false);
    mUi->mStartCheck->setChecked(true);   // Set to checked so we can reuse enableTimeEdits.
    mUi->mEndCheck->setVisible(false);
    mUi->mEndCheck->setChecked(true);   // Set to checked so we can reuse enableTimeEdits.

    // Start time
    connect(mUi->mStartTimeEdit, &KTimeComboBox::timeChanged, this, &IncidenceDateTime::updateStartTime);  // when editing with mouse, or up/down arrows
    connect(mUi->mStartTimeEdit, &KTimeComboBox::timeEdited, this, &IncidenceDateTime::updateStartTime);  // When editing with any key except up/down
    connect(mUi->mStartDateEdit, &KDateComboBox::dateChanged, this, &IncidenceDateTime::updateStartDate);
    connect(mUi->mTimeZoneComboStart, static_cast<void (IncidenceEditorNG::KTimeZoneComboBox::*)(int)>(&IncidenceEditorNG::KTimeZoneComboBox::currentIndexChanged), this, &IncidenceDateTime::updateStartSpec);
    // End time
    connect(mUi->mEndTimeEdit, &KTimeComboBox::timeChanged, this, &IncidenceDateTime::checkDirtyStatus);
    connect(mUi->mEndTimeEdit, &KTimeComboBox::timeEdited, this, &IncidenceDateTime::checkDirtyStatus);
    connect(mUi->mEndDateEdit, &KDateComboBox::dateChanged, this, &IncidenceDateTime::checkDirtyStatus);
    connect(mUi->mEndTimeEdit, &KTimeComboBox::timeChanged, this, &IncidenceDateTime::endTimeChanged);
    connect(mUi->mEndTimeEdit, &KTimeComboBox::timeEdited, this, &IncidenceDateTime::endTimeChanged);
    connect(mUi->mEndDateEdit, &KDateComboBox::dateChanged, this, &IncidenceDateTime::endDateChanged);
    connect(mUi->mTimeZoneComboEnd, static_cast<void (IncidenceEditorNG::KTimeZoneComboBox::*)(int)>(&IncidenceEditorNG::KTimeZoneComboBox::currentIndexChanged), this, &IncidenceDateTime::checkDirtyStatus);

    mUi->mWholeDayCheck->setChecked(event->allDay());
    enableTimeEdits();

    if (isTemplate) {
        if (templateOverridesTimes) {
            // We only use the template times if the user didn't override them.
            setTimes(event->dtStart(), event->dtEnd());
        }
    } else {
        KDateTime startDT = event->dtStart();
        KDateTime endDT = event->dtEnd();
        setDateTimes(startDT, endDT);
    }

    switch (event->transparency()) {
    case KCalCore::Event::Transparent:
        mUi->mFreeBusyCheck->setChecked(false);
        break;
    case KCalCore::Event::Opaque:
        mUi->mFreeBusyCheck->setChecked(true);
        break;
    }
}

void IncidenceDateTime::load(const KCalCore::Journal::Ptr &journal, bool isTemplate, bool templateOverridesTimes)
{
    // First en/disable the necessary ui bits and pieces
    mUi->mStartCheck->setVisible(false);
    mUi->mStartCheck->setChecked(true);   // Set to checked so we can reuse enableTimeEdits.
    mUi->mEndCheck->setVisible(false);
    mUi->mEndCheck->setChecked(true);   // Set to checked so we can reuse enableTimeEdits.
    mUi->mEndDateEdit->setVisible(false);
    mUi->mEndTimeEdit->setVisible(false);
    mUi->mTimeZoneComboEnd->setVisible(false);
    mUi->mEndLabel->setVisible(false);
    mUi->mFreeBusyCheck->setVisible(false);

    // Start time
    connect(mUi->mStartTimeEdit, &KTimeComboBox::timeChanged, this, &IncidenceDateTime::updateStartTime);
    connect(mUi->mStartDateEdit, &KDateComboBox::dateChanged, this, &IncidenceDateTime::updateStartDate);
    connect(mUi->mTimeZoneComboStart, static_cast<void (IncidenceEditorNG::KTimeZoneComboBox::*)(int)>(&IncidenceEditorNG::KTimeZoneComboBox::currentIndexChanged), this, &IncidenceDateTime::updateStartSpec);

    mUi->mWholeDayCheck->setChecked(journal->allDay());
    enableTimeEdits();

    if (isTemplate) {
        if (templateOverridesTimes) {
            // We only use the template times if the user didn't override them.
            setTimes(journal->dtStart(), KDateTime());
        }
    } else {
        KDateTime startDT = journal->dtStart();

        // Convert UTC to local timezone, if needed (i.e. for kolab #204059)
        if (startDT.isUtc()) {
            startDT = startDT.toLocalZone();
        }
        setDateTimes(startDT, KDateTime());
    }
}

void IncidenceDateTime::load(const KCalCore::Todo::Ptr &todo, bool isTemplate, bool templateOverridesTimes)
{
    // First en/disable the necessary ui bits and pieces
    mUi->mStartCheck->setVisible(true);
    mUi->mStartCheck->setChecked(todo->hasStartDate());
    mUi->mStartDateEdit->setEnabled(todo->hasStartDate());
    mUi->mStartTimeEdit->setEnabled(todo->hasStartDate());
    mUi->mTimeZoneComboStart->setEnabled(todo->hasStartDate());

    mUi->mEndLabel->setText(i18nc("@label The due date/time of a to-do", "Due:"));
    mUi->mEndCheck->setVisible(true);
    mUi->mEndCheck->setChecked(todo->hasDueDate());
    mUi->mEndDateEdit->setEnabled(todo->hasDueDate());
    mUi->mEndTimeEdit->setEnabled(todo->hasDueDate());
    mUi->mTimeZoneComboEnd->setEnabled(todo->hasDueDate());

    // These fields where not enabled in the old code either:
    mUi->mFreeBusyCheck->setVisible(false);

    const bool hasDateTimes = mUi->mEndCheck->isChecked() || mUi->mStartCheck->isChecked();
    mUi->mWholeDayCheck->setChecked(hasDateTimes && todo->allDay());
    mUi->mWholeDayCheck->setEnabled(hasDateTimes);

    // Connect to the right logic
    connect(mUi->mStartCheck, &QCheckBox::toggled, this, &IncidenceDateTime::enableStartEdit);
    connect(mUi->mStartCheck, &QCheckBox::toggled, this, &IncidenceDateTime::startDateTimeToggled);
    connect(mUi->mStartDateEdit, &KDateComboBox::dateChanged, this, &IncidenceDateTime::checkDirtyStatus);
    connect(mUi->mStartTimeEdit, &KTimeComboBox::timeChanged, this, &IncidenceDateTime::updateStartTime);
    connect(mUi->mTimeZoneComboStart, static_cast<void (IncidenceEditorNG::KTimeZoneComboBox::*)(int)>(&IncidenceEditorNG::KTimeZoneComboBox::currentIndexChanged), this, &IncidenceDateTime::checkDirtyStatus);

    connect(mUi->mEndCheck, &QCheckBox::toggled, this, &IncidenceDateTime::enableEndEdit);
    connect(mUi->mEndCheck, &QCheckBox::toggled, this, &IncidenceDateTime::endDateTimeToggled);
    connect(mUi->mEndDateEdit, &KDateComboBox::dateChanged, this, &IncidenceDateTime::checkDirtyStatus);
    connect(mUi->mEndTimeEdit, &KTimeComboBox::timeChanged, this, &IncidenceDateTime::checkDirtyStatus);
    connect(mUi->mEndDateEdit, &KDateComboBox::dateChanged, this, &IncidenceDateTime::endDateChanged);
    connect(mUi->mEndTimeEdit, &KTimeComboBox::timeChanged, this, &IncidenceDateTime::endTimeChanged);
    connect(mUi->mTimeZoneComboEnd, static_cast<void (IncidenceEditorNG::KTimeZoneComboBox::*)(int)>(&IncidenceEditorNG::KTimeZoneComboBox::currentIndexChanged), this, &IncidenceDateTime::checkDirtyStatus);

    const KDateTime rightNow = KDateTime(QDate::currentDate(), QTime::currentTime()).toLocalZone();

    if (isTemplate) {
        if (templateOverridesTimes) {
            // We only use the template times if the user didn't override them.
            setTimes(todo->dtStart(), todo->dateTime(KCalCore::Incidence::RoleEnd));
        }
    } else {
        const KDateTime endDT   = todo->hasDueDate() ? todo->dtDue(true/** first */) : rightNow;
        const KDateTime startDT = todo->hasStartDate() ? todo->dtStart(true/** first */) : rightNow;
        setDateTimes(startDT, endDT);
    }
}

void IncidenceDateTime::save(const KCalCore::Event::Ptr &event)
{
    if (mUi->mWholeDayCheck->isChecked()) {   // All day event
        event->setAllDay(true);

        // TODO: need to change this.
        KDateTime eventDTStart = currentStartDateTime();
        eventDTStart.setDateOnly(true);
        event->setDtStart(eventDTStart);

        KDateTime eventDTEnd = currentEndDateTime();
        eventDTEnd.setDateOnly(true);

        event->setDtEnd(eventDTEnd);
    } else { // Timed Event
        event->setAllDay(false);

        // set date/time end
        event->setDtStart(currentStartDateTime());
        event->setDtEnd(currentEndDateTime());
    }

    // Free == Event::Transparent
    // Busy == Event::Opaque
    event->setTransparency(mUi->mFreeBusyCheck->isChecked() ?
                           KCalCore::Event::Opaque :
                           KCalCore::Event::Transparent);
}

void IncidenceDateTime::save(const KCalCore::Todo::Ptr &todo)
{
    if (mUi->mStartCheck->isChecked()) {
        todo->setDtStart(currentStartDateTime());
        // Set allday must be executed after setDtStart
        todo->setAllDay(mUi->mWholeDayCheck->isChecked());
        if (currentStartDateTime() != mInitialStartDT) {
            // We don't offer any way to edit the current completed occurrence.
            // So, if the start date changes, reset the dtRecurrence
            todo->setDtRecurrence(currentStartDateTime());
        }
    } else {
        todo->setDtStart(KDateTime());
    }

    if (mUi->mEndCheck->isChecked()) {
        todo->setDtDue(currentEndDateTime(), true/** first */);
        // Set allday must be executed after setDtDue
        todo->setAllDay(mUi->mWholeDayCheck->isChecked());
    } else {
        todo->setDtDue(KDateTime());
    }
}

void IncidenceDateTime::save(const KCalCore::Journal::Ptr &journal)
{
    journal->setAllDay(mUi->mWholeDayCheck->isChecked());

    if (mUi->mWholeDayCheck->isChecked()) {   // All day journal
        KDateTime journalDTStart = currentStartDateTime();
        journalDTStart.setDateOnly(true);
        journal->setDtStart(journalDTStart);
    } else { // Timed Journal
        // set date/time end
        journal->setDtStart(currentStartDateTime());
    }
}

void IncidenceDateTime::setDateTimes(const KDateTime &start, const KDateTime &end)
{
    const KDateTime::Spec startSpec = start.timeSpec();
    const KDateTime::Spec endSpec = end.timeSpec();

    // Combo boxes only have system time zones
    if (startSpec.type() == KDateTime::TimeZone) {
        const KTimeZone systemTz = KSystemTimeZones::zone(startSpec.timeZone().name());
        if (!systemTz.isValid()) {
            const KCalCore::ICalTimeZone icalTz(startSpec.timeZone());
            mTimeZones->add(icalTz);
        }
    }

    if (endSpec.type() == KDateTime::TimeZone) {
        const KTimeZone systemTz = KSystemTimeZones::zone(endSpec.timeZone().name());
        if (!systemTz.isValid()) {
            const KCalCore::ICalTimeZone icalTz(endSpec.timeZone());
            mTimeZones->add(icalTz);
        }
    }

    mUi->mTimeZoneComboStart->setAdditionalTimeZones(mTimeZones);
    mUi->mTimeZoneComboEnd->setAdditionalTimeZones(mTimeZones);

    if (start.isValid()) {
        mUi->mStartDateEdit->setDate(start.date());
        mUi->mStartTimeEdit->setTime(start.time());
        mUi->mTimeZoneComboStart->selectTimeSpec(start.timeSpec());
    } else {
        KDateTime dt(QDate::currentDate(), QTime::currentTime());
        mUi->mStartDateEdit->setDate(dt.date());
        mUi->mStartTimeEdit->setTime(dt.time());
        mUi->mTimeZoneComboStart->selectTimeSpec(dt.timeSpec());
    }

    if (end.isValid()) {
        mUi->mEndDateEdit->setDate(end.date());
        mUi->mEndTimeEdit->setTime(end.time());
        mUi->mTimeZoneComboEnd->selectTimeSpec(end.timeSpec());
    } else {
        KDateTime dt(QDate::currentDate(), QTime::currentTime().addSecs(60 * 60));
        mUi->mEndDateEdit->setDate(dt.date());
        mUi->mEndTimeEdit->setTime(dt.time());
        mUi->mTimeZoneComboEnd->selectTimeSpec(dt.timeSpec());
    }

    mCurrentStartDateTime = currentStartDateTime();
    Q_EMIT startDateChanged(start.date());
    Q_EMIT startTimeChanged(start.time());
    Q_EMIT endDateChanged(end.date());
    Q_EMIT endTimeChanged(end.time());

    updateStartToolTips();
    updateEndToolTips();
}

void IncidenceDateTime::updateStartToolTips()
{
    if (mUi->mStartCheck->isChecked()) {
        QString datetimeStr =
            KCalUtils::IncidenceFormatter::dateTimeToString(
                currentStartDateTime(),
                mUi->mWholeDayCheck->isChecked(),
                false,
                KSystemTimeZones::local());
        mUi->mStartDateEdit->setToolTip(i18n("Starts: %1", datetimeStr));
        mUi->mStartTimeEdit->setToolTip(i18n("Starts: %1", datetimeStr));
    } else {
        mUi->mStartDateEdit->setToolTip(i18n("Starting Date"));
        mUi->mStartTimeEdit->setToolTip(i18n("Starting Time"));
    }
}

void IncidenceDateTime::updateEndToolTips()
{
    if (mUi->mStartCheck->isChecked()) {
        QString datetimeStr =
            KCalUtils::IncidenceFormatter::dateTimeToString(
                currentEndDateTime(),
                mUi->mWholeDayCheck->isChecked(),
                false,
                KSystemTimeZones::local());
        if (mLoadedIncidence->type() == KCalCore::Incidence::TypeTodo) {
            mUi->mEndDateEdit->setToolTip(i18n("Due on: %1", datetimeStr));
            mUi->mEndTimeEdit->setToolTip(i18n("Due on: %1", datetimeStr));
        } else {
            mUi->mEndDateEdit->setToolTip(i18n("Ends: %1", datetimeStr));
            mUi->mEndTimeEdit->setToolTip(i18n("Ends: %1", datetimeStr));
        }
    } else {
        if (mLoadedIncidence->type() == KCalCore::Incidence::TypeTodo) {
            mUi->mEndDateEdit->setToolTip(i18n("Due Date"));
            mUi->mEndTimeEdit->setToolTip(i18n("Due Time"));
        } else {
            mUi->mEndDateEdit->setToolTip(i18n("Ending Date"));
            mUi->mEndTimeEdit->setToolTip(i18n("Ending Time"));
        }
    }
}

void IncidenceDateTime::setTimes(const KDateTime &start, const KDateTime &end)
{
    // like setDateTimes(), but it set only the start/end time, not the date
    // it is used while applying a template to an event.
    mUi->mStartTimeEdit->blockSignals(true);
    mUi->mStartTimeEdit->setTime(start.time());
    mUi->mStartTimeEdit->blockSignals(false);

    mUi->mEndTimeEdit->setTime(end.time());

    mUi->mTimeZoneComboStart->selectTimeSpec(start.timeSpec());
    mUi->mTimeZoneComboEnd->selectTimeSpec(end.timeSpec());

//   emitDateTimeStr();
}

void IncidenceDateTime::setStartDate(const QDate &newDate)
{
    mUi->mStartDateEdit->setDate(newDate);
    updateStartDate(newDate);
}

void IncidenceDateTime::setStartTime(const QTime &newTime)
{
    mUi->mStartTimeEdit->setTime(newTime);
    updateStartTime(newTime);
}

bool IncidenceDateTime::startDateTimeEnabled() const
{
    return mUi->mStartCheck->isChecked();
}

bool IncidenceDateTime::endDateTimeEnabled() const
{
    return mUi->mEndCheck->isChecked();
}

bool IncidenceDateTime::isValid() const
{
    if (startDateTimeEnabled() && !currentStartDateTime().isValid()) {
        //TODO: Add strings
        qCWarning(INCIDENCEEDITOR_LOG) << "Start date is invalid";
        return false;
    }

    if (endDateTimeEnabled() && !currentEndDateTime().isValid()) {
        //TODO: Add strings
        qCWarning(INCIDENCEEDITOR_LOG) << "End date is invalid";
        return false;
    }

    if (startDateTimeEnabled() && endDateTimeEnabled() &&
            currentStartDateTime() > currentEndDateTime()) {
        if (mLoadedIncidence->type() == KCalCore::Incidence::TypeEvent) {
            mLastErrorString = i18nc("@info",
                                     "The event ends before it starts.\n"
                                     "Please correct dates and times.");

        } else if (mLoadedIncidence->type() == KCalCore::Incidence::TypeTodo) {
            mLastErrorString = i18nc("@info",
                                     "The to-do is due before it starts.\n"
                                     "Please correct dates and times.");

        } else if (mLoadedIncidence->type() == KCalCore::Incidence::TypeJournal) {
            return true;
        }

        qCDebug(INCIDENCEEDITOR_LOG) << mLastErrorString;
        return false;
    } else {
        mLastErrorString.clear();
        return true;
    }
}

static QString timespecToString(const KDateTime::Spec &spec)
{
    QString str = QLatin1String("type=") + QString::number(spec.type()) + QLatin1String("; timezone=") + spec.timeZone().name();
    return str;
}

void IncidenceDateTime::printDebugInfo() const
{
    qCDebug(INCIDENCEEDITOR_LOG) << "startDateTimeEnabled()          : " << startDateTimeEnabled();
    qCDebug(INCIDENCEEDITOR_LOG) << "endDateTimeEnabled()            : " << endDateTimeEnabled();
    qCDebug(INCIDENCEEDITOR_LOG) << "currentStartDateTime().isValid(): " << currentStartDateTime().isValid();
    qCDebug(INCIDENCEEDITOR_LOG) << "currentEndDateTime().isValid()  : " << currentEndDateTime().isValid();
    qCDebug(INCIDENCEEDITOR_LOG) << "currentStartDateTime()          : " << currentStartDateTime().toString();
    qCDebug(INCIDENCEEDITOR_LOG) << "currentEndDateTime()            : " << currentEndDateTime().toString();
    qCDebug(INCIDENCEEDITOR_LOG) << "Incidence type                  : " << mLoadedIncidence->type();
    qCDebug(INCIDENCEEDITOR_LOG) << "allday                          : " << mLoadedIncidence->allDay();
    qCDebug(INCIDENCEEDITOR_LOG) << "mInitialStartDT                 : " << mInitialStartDT.toString();
    qCDebug(INCIDENCEEDITOR_LOG) << "mInitialEndDT                   : " << mInitialEndDT.toString();

    qCDebug(INCIDENCEEDITOR_LOG) << "currentStartDateTime().timeSpec(): " << timespecToString(currentStartDateTime().timeSpec());
    qCDebug(INCIDENCEEDITOR_LOG) << "currentEndDateTime().timeSpec()  : " << timespecToString(currentStartDateTime().timeSpec());
    qCDebug(INCIDENCEEDITOR_LOG) << "mInitialStartDT.timeSpec()       : " << timespecToString(mInitialStartDT.timeSpec());
    qCDebug(INCIDENCEEDITOR_LOG) << "mInitialEndDT.timeSpec()         : " << timespecToString(mInitialEndDT.timeSpec());

    qCDebug(INCIDENCEEDITOR_LOG) << "dirty test1: " << (mLoadedIncidence->allDay() != mUi->mWholeDayCheck->isChecked());
    if (mLoadedIncidence->type() == KCalCore::Incidence::TypeEvent) {
        KCalCore::Event::Ptr event = mLoadedIncidence.staticCast<KCalCore::Event>();
        qCDebug(INCIDENCEEDITOR_LOG) << "dirty test2: " << (mUi->mFreeBusyCheck->isChecked() && event->transparency() != KCalCore::Event::Opaque);
        qCDebug(INCIDENCEEDITOR_LOG) << "dirty test3: " << (!mUi->mFreeBusyCheck->isChecked() && event->transparency() != KCalCore::Event::Transparent);
    }

    if (mLoadedIncidence->allDay()) {
        qCDebug(INCIDENCEEDITOR_LOG) << "dirty test4: " << (mUi->mStartDateEdit->date() != mInitialStartDT.date() || mUi->mEndDateEdit->date() != mInitialEndDT.date());
    } else {
        qCDebug(INCIDENCEEDITOR_LOG) << "dirty test4.1: " << (currentStartDateTime() != mInitialStartDT);
        qCDebug(INCIDENCEEDITOR_LOG) << "dirty test4.2: " << (currentEndDateTime() != mInitialEndDT);
        qCDebug(INCIDENCEEDITOR_LOG) << "dirty test4.3: " << (currentStartDateTime().timeSpec() != mInitialStartDT.timeSpec());
        qCDebug(INCIDENCEEDITOR_LOG) << "dirty test4.4: " << (currentEndDateTime().timeSpec() != mInitialEndDT.timeSpec());
    }
}

void IncidenceDateTime::setTimeZoneLabelEnabled(bool enable)
{
    mUi->mTimeZoneLabel->setVisible(enable);
}

