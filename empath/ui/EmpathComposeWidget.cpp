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
#include <unistd.h> // Linux man pages lie - mkstemp is in stdlib.h
#include <stdlib.h>
#include <sys/stat.h>

// KDE includes
#include <kprocess.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapp.h>

// Local includes
#include "EmpathComposeWidget.h"
#include "EmpathHeaderEditWidget.h"
#include "EmpathAttachmentListWidget.h"
#include "EmpathSubjectSpecWidget.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "EmpathMailSender.h"

EmpathComposeWidget::EmpathComposeWidget(
	ComposeType t,
	RMessage * message,
	QWidget * parent,
	const char * name)
	: QWidget(parent, name)
{
	empathDebug("ctor");

	headerEditWidget_		=
		new EmpathHeaderEditWidget(this, "headerEditWidget");
	CHECK_PTR(headerEditWidget_);

	subjectSpecWidget_		=
		new EmpathSubjectSpecWidget(this, "subjectSpecWidget");
	CHECK_PTR(subjectSpecWidget_);

	l_priority_			=
		new QLabel(i18n("Priority:"), this, "l_priority_");
	CHECK_PTR(l_priority_);

	l_priority_->setFixedWidth(l_priority_->sizeHint().width());

	cmb_priority_		=
		new QComboBox(this, "cmb_priority_");
	CHECK_PTR(cmb_priority_);

	cmb_priority_->insertItem("Highest");
	cmb_priority_->insertItem("High");
	cmb_priority_->insertItem("Normal");
	cmb_priority_->insertItem("Low");
	cmb_priority_->insertItem("Lowest");

	cmb_priority_->setFixedWidth(cmb_priority_->sizeHint().width());
	cmb_priority_->setCurrentItem(2);

	editorWidget_			=
		new QMultiLineEdit(this, "editorWidget");
	CHECK_PTR(editorWidget_);

	layout_	= new QGridLayout(this, 3, 1, 2, 0, "layout_");
	CHECK_PTR(layout_);

	layout_->setColStretch(0, 7);

	layout_->setRowStretch(0, 0);
	layout_->setRowStretch(1, 0);
	layout_->setRowStretch(2, 10);


	midLayout_ = new QGridLayout(1, 3, 10);
	CHECK_PTR(midLayout_);
	layout_->addLayout(midLayout_, 1, 0);

	midLayout_->setColStretch(0, 1);
	midLayout_->addWidget(subjectSpecWidget_,	0, 0);
	midLayout_->addWidget(l_priority_,			0, 1);
	midLayout_->addWidget(cmb_priority_,		0, 2);
	midLayout_->activate();

	empathDebug("Adding header edit");
	layout_->addWidget(headerEditWidget_,	0, 0);
	empathDebug("Adding editor");
	layout_->addWidget(editorWidget_,		2, 0);

	layout_->activate();

	headerEditWidget_->setFocus();

	switch (t) {

	// If we're just composing a new message, we drop out here.
		case ComposeNormal:
			break;

		case ComposeReply:
			{
				KConfig * config = kapp->getConfig();
				config->setGroup(GROUP_COMPOSE);
				if (!config->readBoolEntry(KEY_USE_EXTERNAL_EDITOR)) {
					//	editorWidget_->setText(message->firstPlainBody());
					return;
				}
			}
			break;

		case ComposeReplyAll:
			break;

		case ComposeForward:
			{
				QCString s = message->envelope().to().asString();
				headerEditWidget_->setToText(s);
				if (!s.isEmpty())
					subjectSpecWidget_->setFocus();
			}
			break;

		default:
			break;
	}
}

EmpathComposeWidget::~EmpathComposeWidget()
{
	empathDebug("dtor");
}

	QCString
EmpathComposeWidget::messageAsString()
{
	QCString msgData;

	msgData =	headerEditWidget_->headersAsText();
	msgData +=	"Subject: " + subjectSpecWidget_->getSubject();

	// Header / body separator
	msgData +=	'\n';
	msgData +=	editorWidget_->text();

	return msgData;
}

	bool
EmpathComposeWidget::messageHasAttachments()
{
	empathDebug("messageHasAttachments() called");
//	return attachmentListWidget_->hasAttachments();
	return false;
}

	QList<EmpathAttachmentSpec>
EmpathComposeWidget::messageAttachments()
{
	empathDebug("messageAttachments() called");
//	return attachmentListWidget_->attachmentList();
}

	void
EmpathComposeWidget::spawnExternalEditor(const QCString & text)
{
	empathDebug("spawnExternalEditor() called");

	char * tn = new char[strlen(TEMP_COMPOSE_FILENAME)];
	strcpy(tn, TEMP_COMPOSE_FILENAME);

	empathDebug("tempName = \"" + QCString(tn) + "\"");
	empathDebug("running mkstemp");

	int fd = mkstemp(tn);
	QCString tempName(tn);
	delete [] tn;

	empathDebug("Opening file " + QString(tempName));
	QFile f;

	if (!f.open(IO_WriteOnly, fd)) {
		empathDebug("Couldn't open a temporary file for the external editor");
		return;
	}

	empathDebug("Opened temporary file \"" + QString(tempName) + "\" OK");
	f.writeBlock(text.data(), text.length());
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

	KConfig * config = kapp->getConfig();
	config->setGroup(GROUP_COMPOSE);
	QString externalEditor = config->readEntry(KEY_EXTERNAL_EDITOR);

	KProcess * p = new KProcess;
	empathDebug("p << " + externalEditor);
	*p << externalEditor;
	empathDebug("p << " + QString(tempName));
	*p << tempName;

	QObject::connect(p, SIGNAL(receivedStdout(KProcess *, char *, int)),
			this, SLOT(s_debugExternalEditorOutput(KProcess *, char *, int)));

	QObject::connect(p, SIGNAL(receivedStderr(KProcess *, char *, int)),
			this, SLOT(s_debugExternalEditorOutput(KProcess *, char *, int)));

	QObject::connect(p, SIGNAL(processExited(KProcess *)),
		this, SLOT(s_composeFinished(KProcess *)));

	empathDebug("Starting external editor process");
	if (!p->start(KProcess::NotifyOnExit, KProcess::All)) {
		empathDebug("Couldn't start external editor process");
		return;
	}
	qApp->processEvents();

	empathDebug("processTable_.insert(" + QString(tempName) + ",p)");
	processTable_.insert(QString(tempName), p);

	QCString ss;
	ss.setNum(statbuf.st_mtime);

	char * sc = new char[ss.length()];
	strcpy(sc, ss.data());

	empathDebug("externEditModified_.insert(" + QString(tempName) +
		", \"" + QString(sc) + "\")");

	externEditModified_.insert(QString(tempName), sc);
	empathDebug("Arse !");
	delete [] sc;
}

	void
EmpathComposeWidget::s_composeFinished(KProcess * p)
{
	empathDebug("s_composeFinished called (process exited)");
	// Find the process' filename in the process table.
	// Once we have the filename, we can re-read the text from that file and use
	// that text to send the new message. We must check if the file has been
	// modified too. If not, we'll just remove it.

	// First find the process' filename.

	QCString fileName;

	QDictIterator<KProcess> it(processTable_);

	for (; it.current(); ++it) {

		if (it.current() == p) {
			fileName = it.currentKey();
			break;
		}
	}

	if (fileName.isEmpty()) {
		empathDebug("Couldn't find the filename that the process was using.");
		return;
	}

	empathDebug("The process was using filename \"" + fileName + "\"");

	// Ok we have the filename.
	// Now find out when the file was last modified.

	QFileInfo finfo(fileName);

	QDateTime modTime = finfo.lastModified();

	QDateTime myModTime;

	// Now we know when the temporary file was modified, we compare that timestamp
	// to the one we held. We held it in a dict based on the filename.

	QDictIterator<char> modit(externEditModified_);

	bool found = false;
	for (; modit.current(); ++modit) {

		empathDebug("Checking timestamp for filename \"" +
			QString(modit.currentKey()) + "\"");

		if (strcmp(modit.currentKey(), fileName) == 0) {

			// Found the timestamp.

			empathDebug("Found timestamp. It's \"" +
				QString(modit.current()) + "\"");

			myModTime.setTime_t(QCString(modit.current()).toULong());
			found = true;
			break;
		}
	}

	if (!found) {

		empathDebug("Couldn't find the timestamp for the temporary file");
		return;
	}

	empathDebug("modification time on file == " + modTime.toString());
	empathDebug("modification time I held  == " + myModTime.toString());

	if (myModTime == modTime) {

		// File was NOT modified.
		empathDebug("The temporary file was NOT modified by the external editor");
		return;
	}

	// File was modified.
	empathDebug("The temporary file WAS modified by the external editor");

	// Create a new message and send it via the mail sender.

	QFile f(fileName);

	if (!f.open(IO_ReadOnly)) {

		empathDebug("Couldn't reopen the temporary file");
		return;
	}

	QCString msgBuf;

	char buf[1024];

	while (!f.atEnd()) {
		f.readBlock(buf, 1024);
		msgBuf += buf;
	}

	// Create the message from the file's contents.
	RMessage m(msgBuf);

	// Send the message
	empathDebug("Sending the externally edited message");
	empath->mailSender().sendOne(m);
}

	void
EmpathComposeWidget::s_debugExternalEditorOutput(
	KProcess * p, char * buffer, int buflen)
{
	empathDebug("Received from process: " + QString(buffer));
}

