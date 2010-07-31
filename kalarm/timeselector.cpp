/*
 *  timeselector.cpp  -  widget to optionally set a time period
 *  Program:  kalarm
 *  Copyright (C) 2004 by David Jarvie <software@astrojar.org.uk>
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

#include <tqlayout.h>
#include <tqlabel.h>
#include <tqhbox.h>
#include <tqwhatsthis.h>

#include <klocale.h>
#include <kdialog.h>
#include <kdebug.h>

#include "checkbox.h"
#include "timeselector.moc"


TimeSelector::TimeSelector(const TQString& selectText, const TQString& postfix, const TQString& selectWhatsThis,
                           const TQString& valueWhatsThis, bool allowHourMinute, TQWidget* parent, const char* name)
	: TQFrame(parent, name),
	  mLabel(0),
	  mReadOnly(false)
{
	setFrameStyle(TQFrame::NoFrame);
	TQVBoxLayout* topLayout = new TQVBoxLayout(this, 0, KDialog::spacingHint());
	TQHBoxLayout* layout = new TQHBoxLayout(topLayout, KDialog::spacingHint());
	mSelect = new CheckBox(selectText, this);
	mSelect->setFixedSize(mSelect->sizeHint());
	connect(mSelect, TQT_SIGNAL(toggled(bool)), TQT_SLOT(selectToggled(bool)));
	TQWhatsThis::add(mSelect, selectWhatsThis);
	layout->addWidget(mSelect);

	TQHBox* box = new TQHBox(this);    // to group widgets for TQWhatsThis text
	box->setSpacing(KDialog::spacingHint());
	layout->addWidget(box);
	mPeriod = new TimePeriod(allowHourMinute, box);
	mPeriod->setFixedSize(mPeriod->sizeHint());
	mPeriod->setSelectOnStep(false);
	connect(mPeriod, TQT_SIGNAL(valueChanged(int)), TQT_SLOT(periodChanged(int)));
	mSelect->setFocusWidget(mPeriod);
	mPeriod->setEnabled(false);

	if (!postfix.isEmpty())
	{
		mLabel = new TQLabel(postfix, box);
		TQWhatsThis::add(box, valueWhatsThis);
		mLabel->setEnabled(false);
	}
}

/******************************************************************************
*  Set the read-only status.
*/
void TimeSelector::setReadOnly(bool ro)
{
	if ((int)ro != (int)mReadOnly)
	{
		mReadOnly = ro;
		mSelect->setReadOnly(mReadOnly);
		mPeriod->setReadOnly(mReadOnly);
	}
}

bool TimeSelector::isChecked() const
{
	return mSelect->isChecked();
}

void TimeSelector::setChecked(bool on)
{
	if (on != mSelect->isChecked())
	{
		mSelect->setChecked(on);
		emit valueChanged(minutes());
	}
}

void TimeSelector::setMaximum(int hourmin, int days)
{
	mPeriod->setMaximum(hourmin, days);
}

void TimeSelector::setDateOnly(bool dateOnly)
{
	mPeriod->setDateOnly(dateOnly);
}

/******************************************************************************
 * Get the specified number of minutes.
 * Reply = 0 if unselected.
 */
int TimeSelector::minutes() const
{
	return mSelect->isChecked() ? mPeriod->minutes() : 0;
}

/******************************************************************************
*  Initialise the controls with a specified time period.
*  If minutes = 0, it will be deselected.
*  The time unit combo-box is initialised to 'defaultUnits', but if 'dateOnly'
*  is true, it will never be initialised to hours/minutes.
*/
void TimeSelector::setMinutes(int minutes, bool dateOnly, TimePeriod::Units defaultUnits)
{
	mSelect->setChecked(minutes);
	mPeriod->setEnabled(minutes);
	if (mLabel)
		mLabel->setEnabled(minutes);
	mPeriod->setMinutes(minutes, dateOnly, defaultUnits);
}

/******************************************************************************
*  Set the input focus on the count field.
*/
void TimeSelector::setFocusOnCount()
{
	mPeriod->setFocusOnCount();
}

/******************************************************************************
*  Called when the TimeSelector checkbox is toggled.
*/
void TimeSelector::selectToggled(bool on)
{
	mPeriod->setEnabled(on);
	if (mLabel)
		mLabel->setEnabled(on);
	if (on)
		mPeriod->setFocus();
	emit toggled(on);
	emit valueChanged(minutes());
}

/******************************************************************************
*  Called when the period value changes.
*/
void TimeSelector::periodChanged(int minutes)
{
	if (mSelect->isChecked())
		emit valueChanged(minutes);
}
