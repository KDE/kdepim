/*
 *  main.cpp
 *  Program:  kalarm
 *  (C) 2001, 2002 by David Jarvie  software@astrojar.org.uk
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "kalarm.h"

#include <stdlib.h>

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kdebug.h>

#include "kalarmapp.h"

#define PROGRAM_NAME "kalarm"

static KCmdLineOptions options[] =
{
	{ "b", 0L, 0L },
	{ "beep", I18N_NOOP("Beep when message is displayed"), 0L },
	{ "c", 0L, 0L },
	{ "color", 0L, 0L },
	{ "colour <colour>", I18N_NOOP("Message background color (name or hex 0xRRGGBB)"), 0L },
	{ "calendarURL <url>", I18N_NOOP("URL of calendar file"), 0L },
	{ "cancelEvent <eventID>", I18N_NOOP("Cancel message with the specified event ID"), 0L },
	{ "displayEvent <eventID>", I18N_NOOP("Display message with the specified event ID"), 0L },
	{ "f", 0L, 0L },
	{ "file <url>", I18N_NOOP("File to display"), 0L },
	{ "handleEvent <eventID>", I18N_NOOP("Display or cancel message with the specified event ID"), 0L },
	{ "i", 0L, 0L },
	{ "interval <minutes>", I18N_NOOP("Interval between display of repeated alarms"), 0L },
	{ "l", 0L, 0L },
	{ "late-cancel", I18N_NOOP("Cancel message if it cannot be displayed on time"), 0L },
	{ "L", 0L, 0L },
	{ "login", I18N_NOOP("Repeat message at every login"), 0L },
	{ "r", 0L, 0L },
	{ "repeat <count>", I18N_NOOP("Number of times to repeat alarm (after the initial occasion)"), 0L },
	{ "reset", I18N_NOOP("Reset the message scheduling daemon"), 0L },
	{ "stop", I18N_NOOP("Stop the message scheduling daemon"), 0L },
	{ "t", 0L, 0L },
	{ "time <time>", I18N_NOOP("Display message at 'time' [[[yyyy-]mm-]dd-]hh:mm"), 0L },
	{ "tray", I18N_NOOP("Display system tray icon"), 0L },
	{ "+[message]", I18N_NOOP("Message text to display"), 0L },
	{ 0L, 0L, 0L }
};


int main(int argc, char *argv[])
{
	KAboutData aboutData(PROGRAM_NAME, I18N_NOOP("KAlarm"),
		VERSION, I18N_NOOP("       " PROGRAM_NAME "\n"
		"       " PROGRAM_NAME " [-bcilLrt] -f URL\n"
		"       " PROGRAM_NAME " [-bcilLrt] message\n"
		"       " PROGRAM_NAME " --tray | --reset | --stop\n"
		"       " PROGRAM_NAME " --cancelEvent eventID [--calendarURL url]\n"
		"       " PROGRAM_NAME " --displayEvent eventID [--calendarURL url]\n"
		"       " PROGRAM_NAME " --handleEvent eventID [--calendarURL url]\n"
		"       " PROGRAM_NAME " [generic_options]\n\n"
		"KDE alarm message scheduler"),
		KAboutData::License_GPL,
		"(c) 2001, 2002, David Jarvie", 0L, "http://www.astrojar.org.uk/linux");
	aboutData.addAuthor("David Jarvie", 0L, "software@astrojar.org.uk");

	KCmdLineArgs::init(argc, argv, &aboutData);
	KCmdLineArgs::addCmdLineOptions(options);
	KUniqueApplication::addCmdLineOptions();

	if (!KAlarmApp::start())
	{
		// An instance of the application is already running
		exit(0);
	}

	// This is the child instance
	KAlarmApp* app = KAlarmApp::getInstance();
	return app->exec();
}
