/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef __GNUG__
# pragma implementation "EmpathTask.h"
#endif

#include <kapp.h>

#include "EmpathTask.h"
#include "EmpathDefines.h"

EmpathTask::EmpathTask(const QString & name)
	:	QObject(),
		name_(name),
		done_(false)
{
	empathDebug("ctor");
}

EmpathTask::~EmpathTask()
{
	empathDebug("dtor");
	emit(finished());
}

	void
EmpathTask::setMax(int i)
{
	empathDebug("setMax() called");
	emit(maxChanged(i));
}

	void
EmpathTask::setPos(int i)
{
	empathDebug("setPos() called");
	emit(posChanged(i));
}

	void
EmpathTask::doneOne()
{
	empathDebug("doneOne() called");
	emit(addOne());
	kapp->processEvents();
}

	void
EmpathTask::done()
{
	empathDebug("done() called");
	done_ = true;
	emit(finished());
}

