/* KPilot
**
** Copyright (C) 2006 by Bertjan Broeksema <b.broeksema@gmail.com>
**
** This file is the implementation of the CleanUpState.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <options.h>

#include <kio/netaccess.h>
#include <qfile.h>

#include "pilotDatabase.h"

#include "vcal-conduitbase.h"
#include "vcalconduitSettings.h"
#include "cleanupstate.h"


CleanUpState::CleanUpState()
{
	fState = eCleanUp;
}

CleanUpState::~CleanUpState()
{
}

void CleanUpState::startSync( ConduitAction *ca )
{
	FUNCTIONSETUP;

	VCalConduitBase *vccb = dynamic_cast<VCalConduitBase*>(ca);
	if( !vccb )
	{
		return;
	}

	DEBUGKPILOT << fname << ": Starting CleanUpState." << endl;

	vccb->addLogMessage( i18n( "Cleaning up ..." ) );
	vccb->postSync();

	if ( vccb->database() )
	{
		vccb->database()->resetSyncFlags();
		vccb->database()->cleanup();
	}
	if ( vccb->localDatabase() )
	{
		vccb->localDatabase()->resetSyncFlags();
		vccb->localDatabase()->cleanup();
	}

	KCal::Calendar *fCalendar = vccb->calendar();
	QString fCalendarFile = vccb->calendarFile();

	if ( fCalendar )
	{
		KURL kurl( vccb->config()->calendarFile() );
		switch( vccb->config()->calendarType() )
		{
		case VCalConduitSettings::eCalendarLocal:
			dynamic_cast<KCal::CalendarLocal*>(fCalendar)->save( fCalendarFile );
			if(!kurl.isLocalFile())
			{
				if( !KIO::NetAccess::upload( fCalendarFile
					, vccb->config()->calendarFile(), 0L) )
				{
					vccb->addLogError( i18n( "An error occurred while uploading"
						" \"%1\". You can try to upload "
						"the temporary local file \"%2\" manually.")
						.arg(vccb->config()->calendarFile()).arg(fCalendarFile));
				}
				else {
					KIO::NetAccess::removeTempFile( fCalendarFile );
				}
				QFile backup( fCalendarFile + CSL1( "~" ) );
				backup.remove();
			}
			break;
		case VCalConduitSettings::eCalendarResource:
			fCalendar->save();
			break;
		default:
			break;
		}
		fCalendar->close();
	}

	vccb->setHasNextRecord( false );
}

void CleanUpState::handleRecord( ConduitAction * )
{
	FUNCTIONSETUP;
}

void CleanUpState::finishSync( ConduitAction *ca )
{
	FUNCTIONSETUP;

	VCalConduitBase *vccb = dynamic_cast<VCalConduitBase*>(ca);
	if( !vccb )
	{
		return;
	}

	DEBUGKPILOT << fname << ": Finished CleanUpState." << endl;
	vccb->setState( 0L );
}
