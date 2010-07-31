/*
 *  dateedit.cpp  -  date entry widget
 *  Program:  kalarm
 *  Copyright © 2002-2007 by David Jarvie <software@astrojar.org.uk>
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

#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "dateedit.moc"


DateEdit::DateEdit(TQWidget* parent, const char* name)
	: KDateEdit(parent, name)
{
	connect(this, TQT_SIGNAL(dateEntered(const TQDate&)), TQT_SLOT(newDateEntered(const TQDate&)));
}

void DateEdit::setMinDate(const TQDate& d, const TQString& errorDate)
{
	mMinDate = d;
	if (mMinDate.isValid()  &&  date().isValid()  &&  date() < mMinDate)
		setDate(mMinDate);
	mMinDateErrString = errorDate;
}

void DateEdit::setMaxDate(const TQDate& d, const TQString& errorDate)
{
	mMaxDate = d;
	if (mMaxDate.isValid()  &&  date().isValid()  &&  date() > mMaxDate)
		setDate(mMaxDate);
	mMaxDateErrString = errorDate;
}

void DateEdit::setInvalid()
{
	setDate(TQDate());
}

// Check a new date against any minimum or maximum date.
void DateEdit::newDateEntered(const TQDate& newDate)
{
	if (newDate.isValid())
	{
		if (mMinDate.isValid()  &&  newDate < mMinDate)
		{
			pastLimitMessage(mMinDate, mMinDateErrString,
					 i18n("Date cannot be earlier than %1"));
			setDate(mMinDate);
		}
		else if (mMaxDate.isValid()  &&  newDate > mMaxDate)
		{
			pastLimitMessage(mMaxDate, mMaxDateErrString,
					 i18n("Date cannot be later than %1"));
			setDate(mMaxDate);
		}
	}
}

void DateEdit::pastLimitMessage(const TQDate& limit, const TQString& error, const TQString& defaultError)
{
	TQString errString = error;
	if (errString.isNull())
	{
		if (limit == TQDate::currentDate())
			errString = i18n("today");
		else
			errString = KGlobal::locale()->formatDate(limit, true);
		errString = defaultError.arg(errString);
	}
	KMessageBox::sorry(this, errString);
}

void DateEdit::mousePressEvent(TQMouseEvent *e)
{
	if (isReadOnly())
	{
		// Swallow up the event if it's the left button
		if (e->button() == Qt::LeftButton)
			return;
	}
	KDateEdit::mousePressEvent(e);
}

void DateEdit::mouseReleaseEvent(TQMouseEvent* e)
{
	if (!isReadOnly())
		KDateEdit::mouseReleaseEvent(e);
}

void DateEdit::mouseMoveEvent(TQMouseEvent* e)
{
	if (!isReadOnly())
		KDateEdit::mouseMoveEvent(e);
}

void DateEdit::keyPressEvent(TQKeyEvent* e)
{
	if (!isReadOnly())
		KDateEdit::keyPressEvent(e);
}

void DateEdit::keyReleaseEvent(TQKeyEvent* e)
{
	if (!isReadOnly())
		KDateEdit::keyReleaseEvent(e);
}
