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
#include <sys/stat.h>
#include <unistd.h>

// Qt includes
#include <qfile.h>
#include <qfileinfo.h>

// KDE includes
#include <kapp.h>
#include <kglobal.h>
#include <kconfig.h>

// Local includes
#include "EmpathEditorProcess.h"

EmpathEditorProcess::EmpathEditorProcess(const QCString & text)
	:	QObject(),
		text_(text)
{
	empathDebug("ctor");

	QObject::connect(&p, SIGNAL(receivedStdout(KProcess *, char *, int)),
			this, SLOT(s_debugExternalEditorOutput(KProcess *, char *, int)));

	QObject::connect(&p, SIGNAL(receivedStderr(KProcess *, char *, int)),
			this, SLOT(s_debugExternalEditorOutput(KProcess *, char *, int)));

	QObject::connect(&p, SIGNAL(processExited(KProcess *)),
		this, SLOT(s_composeFinished(KProcess *)));
}

EmpathEditorProcess::~EmpathEditorProcess()
{
	empathDebug("dtor");
	p.kill(SIGKILL);
}

	void
EmpathEditorProcess::go()
{	
	empathDebug("run() called");

	QCString tempComposeFilename = "/tmp/empathCompose_XXXXXX";
	char * tn = new char[strlen(tempComposeFilename)];
	strcpy(tn, tempComposeFilename);

	empathDebug("tempName = \"" + QCString(tn) + "\"");
	empathDebug("running mkstemp");

	int fd = mkstemp(tn);
	QCString tempName(tn);
	delete [] tn;

	empathDebug("Opening file " + QString(tempName));
	QFile f;

	if (!f.open(IO_WriteOnly, fd)) {
		empathDebug("Couldn't open the temporary file " + tempName);
		return;
	}
	
	fileName = QString(tempName);

	f.writeBlock(text_.data(), text_.length());
	f.flush();

	// f doesn't own the file so I must ::close().
	if (::close(fd) != 0) {
		empathDebug("Couldn't successfully close the file.");
		return;
	}

	// Hold mtime for the file.
	// FIXME: Will this work over NFS ?
	struct stat statbuf;
	fstat(fd, &statbuf);

	KConfig * config = KGlobal::config();
	config->setGroup(EmpathConfig::GROUP_COMPOSE);
	QString externalEditor =
		config->readEntry(EmpathConfig::KEY_EXTERNAL_EDITOR);

	p	<< externalEditor
		<< tempName;

	if (!p.start(KProcess::NotifyOnExit, KProcess::All)) {
		empathDebug("Couldn't start process");
		return;
	}
	
	kapp->processEvents();

	myModTime_.setTime_t(statbuf.st_mtime);
}

	void
EmpathEditorProcess::s_composeFinished(KProcess * p)
{
	empathDebug("s_composeFinished called (process exited)");
	// Find the process' filename in the process table.
	// Once we have the filename, we can re-read the text from that file and use
	// that text to send the new message. We must check if the file has been
	// modified too. If not, we'll just remove it.

	// Find out when the file was last modified.

	QFileInfo finfo(fileName);

	QDateTime modTime = finfo.lastModified();

	empathDebug("modification time on file == " + modTime.toString());
	empathDebug("modification time I held  == " + myModTime_.toString());

	if (myModTime_ == modTime) {

		// File was NOT modified.
		empathDebug("The temporary file was NOT modified");
		emit(done(false, ""));
		return;
	}

	// File was modified.
	empathDebug("The temporary file WAS modified");

	// Create a new message and send it via the mail sender.

	QFile f(fileName);

	if (!f.open(IO_ReadOnly)) {

		empathDebug("Couldn't reopen the temporary file");
		emit(done(false, ""));
		return;
	}
	
	char buf[1024];

	while (!f.atEnd()) {
		f.readBlock(buf, 1024);
		text_ += buf;
	}
	
	emit(done(true, text_));
}

	void
EmpathEditorProcess::s_debugExternalEditorOutput(
	KProcess * p, char * buffer, int buflen)
{
	empathDebug("Received: " + QString(buffer));
}

