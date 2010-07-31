/*
 *  latecancel.cpp  -  widget to specify cancellation if late
 *  Program:  kalarm
 *  Copyright (C) 2004, 2005 by David Jarvie <software@astrojar.org.uk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kalarm.h"

#include <tqwidgetstack.h>
#include <tqlayout.h>
#include <tqwhatsthis.h>
#include <klocale.h>
#include <kdialog.h>

#include "checkbox.h"
#include "latecancel.moc"


// Collect these widget labels together to ensure consistent wording and
// translations across different modules.
TQString LateCancelSelector::i18n_CancelIfLate()       { return i18n("Cancel if late"); }
TQString LateCancelSelector::i18n_n_CancelIfLate()     { return i18n("Ca&ncel if late"); }
TQString LateCancelSelector::i18n_AutoCloseWin()       { return i18n("Auto-close window after this time"); }
TQString LateCancelSelector::i18n_AutoCloseWinLC()     { return i18n("Auto-close window after late-cancelation time"); }
TQString LateCancelSelector::i18n_i_AutoCloseWinLC()   { return i18n("Auto-close w&indow after late-cancelation time"); }


LateCancelSelector::LateCancelSelector(bool allowHourMinute, TQWidget* parent, const char* name)
	: TQFrame(parent, name),
	  mDateOnly(false),
	  mReadOnly(false),
	  mAutoCloseShown(false)
{
	TQString whatsThis = i18n("If checked, the alarm will be canceled if it cannot be triggered within the "
	                         "specified period after its scheduled time. Possible reasons for not triggering "
	                         "include your being logged off, X not running, or the alarm daemon not running.\n\n"
	                         "If unchecked, the alarm will be triggered at the first opportunity after "
	                         "its scheduled time, regardless of how late it is.");

	setFrameStyle(TQFrame::NoFrame);
	mLayout = new TQVBoxLayout(this, 0, KDialog::spacingHint());

	mStack = new TQWidgetStack(this);
	mCheckboxFrame = new TQFrame(mStack);
	mCheckboxFrame->setFrameStyle(TQFrame::NoFrame);
	mStack->addWidget(mCheckboxFrame, 1);
	TQBoxLayout* layout = new TQVBoxLayout(mCheckboxFrame, 0, 0);
	mCheckbox = new CheckBox(i18n_n_CancelIfLate(), mCheckboxFrame);
	mCheckbox->setFixedSize(mCheckbox->sizeHint());
	connect(mCheckbox, TQT_SIGNAL(toggled(bool)), TQT_SLOT(slotToggled(bool)));
	TQWhatsThis::add(mCheckbox, whatsThis);
	layout->addWidget(mCheckbox, 0, Qt::AlignAuto);

	mTimeSelectorFrame = new TQFrame(mStack);
	mTimeSelectorFrame->setFrameStyle(TQFrame::NoFrame);
	mStack->addWidget(mTimeSelectorFrame, 2);
	layout = new TQVBoxLayout(mTimeSelectorFrame, 0, 0);
	mTimeSelector = new TimeSelector(i18n("Cancel if late by 10 minutes", "Ca&ncel if late by"), TQString::null,
	                                 whatsThis, i18n("Enter how late will cause the alarm to be canceled"),
	                                 allowHourMinute, mTimeSelectorFrame);
	connect(mTimeSelector, TQT_SIGNAL(toggled(bool)), TQT_SLOT(slotToggled(bool)));
	layout->addWidget(mTimeSelector);
	mLayout->addWidget(mStack);

	layout = new TQHBoxLayout(mLayout, KDialog::spacingHint());
	layout->addSpacing(3*KDialog::spacingHint());
	mAutoClose = new CheckBox(i18n_AutoCloseWin(), this);
	mAutoClose->setFixedSize(mAutoClose->sizeHint());
	TQWhatsThis::add(mAutoClose, i18n("Automatically close the alarm window after the expiry of the late-cancelation period"));
	layout->addWidget(mAutoClose);
	layout->addStretch();

	mAutoClose->hide();
	mAutoClose->setEnabled(false);
}

/******************************************************************************
*  Set the read-only status.
*/
void LateCancelSelector::setReadOnly(bool ro)
{
	if ((int)ro != (int)mReadOnly)
	{
		mReadOnly = ro;
		mCheckbox->setReadOnly(mReadOnly);
		mTimeSelector->setReadOnly(mReadOnly);
		mAutoClose->setReadOnly(mReadOnly);
	}
}

int LateCancelSelector::minutes() const
{
	return mTimeSelector->minutes();
}

void LateCancelSelector::setMinutes(int minutes, bool dateOnly, TimePeriod::Units defaultUnits)
{
	slotToggled(minutes);
	mTimeSelector->setMinutes(minutes, dateOnly, defaultUnits);
}

void LateCancelSelector::setDateOnly(bool dateOnly)
{
	if (dateOnly != mDateOnly)
	{
		mDateOnly = dateOnly;
		if (mTimeSelector->isChecked())      // don't change when it's not visible
			mTimeSelector->setDateOnly(dateOnly);
	}
}

void LateCancelSelector::showAutoClose(bool show)
{
	if (show)
		mAutoClose->show();
	else
		mAutoClose->hide();
	mAutoCloseShown = show;
	mLayout->activate();
}

bool LateCancelSelector::isAutoClose() const
{
	return mAutoCloseShown  &&  mAutoClose->isEnabled()  &&  mAutoClose->isChecked();
}

void LateCancelSelector::setAutoClose(bool autoClose)
{
	mAutoClose->setChecked(autoClose);
}

/******************************************************************************
*  Called when either of the checkboxes is toggled.
*/
void LateCancelSelector::slotToggled(bool on)
{
	mCheckbox->setChecked(on);
	mTimeSelector->setChecked(on);
	if (on)
	{
		mTimeSelector->setDateOnly(mDateOnly);
		mStack->raiseWidget(mTimeSelectorFrame);
	}
	else
		mStack->raiseWidget(mCheckboxFrame);
	mAutoClose->setEnabled(on);
}
