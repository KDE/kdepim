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

// System includes
#include <signal.h>
#include <iostream.h>

// Qt includes
#include <qmessagebox.h>
#include <qobject.h>

// KDE includes
#include <kapp.h>
#include <kconfig.h>

// Local includes
#include "lib/Empath.h"
#include "lib/EmpathEnum.h"
#include "ui/EmpathUI.h"

class RMessage;

void myMessageHandler(QtMsgType type, const char * msg);

	int
EmpathMain(int argc, char * argv[])
{
	KApplication app(argc, argv, "empath");
	
	app.getConfig()->setDollarExpansion(false);
	
	Empath		e;
	EmpathUI	ui;
	
	QObject::connect(
		&e, SIGNAL(newComposer(ComposeType, RMessage *)),
		&ui, SLOT(s_newComposer(ComposeType, RMessage *)));

	cerr << "Entering event loop";
	return app.exec();
}

main(int argc, char * argv[])
{
//	qInstallMsgHandler(myMessageHandler);
	return EmpathMain(argc, argv);
}

	void
myMessageHandler(QtMsgType type, const char * msg)
{
	switch (type) {
		case QtDebugMsg:
			cerr << "Qt Debug: " << msg << endl;
			break;
		case QtWarningMsg:
			cerr << "Qt Warning: " << msg << endl;
			break;
		case QtFatalMsg:
			cerr << "Qt FATAL ERROR: " << msg << endl;
			QMessageBox::critical(0, "Empath",
				QString("Fatal error in Qt toolkit:\n" + QString(msg) +
					"\nPlease notify the program maintainer at\n" +
					"rik@rikkus.demon.co.uk\n"));
			abort();
			break;
		default:
			cerr << "Unknown Qt Error: " << msg << endl;
			break;	
	}	
}

/*
void handleSignal(int sig)
{
	if (sig == SIGINT) {
		cerr << "Got SIGINT - dying" << endl;
		if (Empath::EMPATH != 0) delete Empath::EMPATH;
		exit(0);
	}

	cerr << "Caught signal " << sig << " - exiting" << endl;
	
	exit(1);
}
*/
