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

// KDE includes
#include <klocale.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathConfigPOP3Logging.h"
#include "EmpathMailboxPOP3.h"

EmpathConfigPOP3Logging::EmpathConfigPOP3Logging(QWidget * parent, const char * name)
	: QWidget(parent, name)
{
	empathDebug("ctor");

	mailbox_	= 0;

	QLineEdit	tempLineEdit((QWidget *)0);
	Q_UINT32 h	= tempLineEdit.sizeHint().height();

	cb_logConversation_		= new QCheckBox(
			i18n("Log conversations with the server"), this, "cb_logConversation");
	CHECK_PTR(cb_logConversation_);
	
	l_logFile_				= new QLabel(
			i18n("Log file"), this, "l_logFile");
	CHECK_PTR(l_logFile_);
	
	l_logFile_->setMaximumHeight(h);
	l_logFile_->setMinimumHeight(h);
	
	le_logFile_				= new QLineEdit(this, "le_logFile");
	CHECK_PTR(le_logFile_);
	
	le_logFile_->setMaximumHeight(h);
	le_logFile_->setMinimumHeight(h);
	
	pb_browseLogFile_		= new QPushButton("...", this, "pb_browseLogFile");
	CHECK_PTR(pb_browseLogFile_);
	
	pb_browseLogFile_->setMaximumHeight(h);
	pb_browseLogFile_->setMinimumHeight(h);
	
	cb_appendToLog_			= new QCheckBox(
			i18n("Append to log file (rather than overwrite)"),
			this, "cb_appendToLog_");

	CHECK_PTR(cb_appendToLog_);
	
	cb_appendToLog_->setMaximumHeight(h);
	cb_appendToLog_->setMinimumHeight(h);
	
	pb_viewCurrentLog_		= new QPushButton(
			i18n("View log file"), this, "pb_viewCurrentLog");
	CHECK_PTR(pb_viewCurrentLog_);
	
	pb_viewCurrentLog_->setMaximumHeight(h);
	pb_viewCurrentLog_->setMinimumHeight(h);
	
	l_maxLogFileSize_		= new QLabel(
			i18n("Maximum size of log file"), this, "l_maxLogFileSize");
	CHECK_PTR(l_maxLogFileSize_);
	
	l_maxLogFileSize_->setMaximumHeight(h);
	l_maxLogFileSize_->setMinimumHeight(h);
	
	sb_maxLogFileSize_		=
		new QSpinBox(0, 10000, 10, this, "sb_maxLogFileSize");
	
	CHECK_PTR(sb_maxLogFileSize_);
	
	sb_maxLogFileSize_->setMaximumHeight(h);
	sb_maxLogFileSize_->setMinimumHeight(h);
	
	l_logFileKb_			= new QLabel("Kb", this, "l_logFileKb");
	CHECK_PTR(l_logFileKb_);
	
	l_logFileKb_->setMaximumHeight(h);
	l_logFileKb_->setMinimumHeight(h);
	
	// Layout
	
	topLevelLayout_		= new QGridLayout(this,	4, 4, 10, 10);
	CHECK_PTR(topLevelLayout_);
	
	topLevelLayout_->setColStretch(0, 3);
	topLevelLayout_->setColStretch(1, 7);
	topLevelLayout_->setColStretch(2, 3);
	topLevelLayout_->setColStretch(3, 1);

	topLevelLayout_->addMultiCellWidget(cb_logConversation_,	0, 0, 0, 1);
	topLevelLayout_->addMultiCellWidget(pb_viewCurrentLog_,		0, 0, 2, 3);
	
	topLevelLayout_->addWidget(l_logFile_, 						1, 0);
	topLevelLayout_->addMultiCellWidget(le_logFile_, 			1, 1, 1, 2);
	topLevelLayout_->addWidget(pb_browseLogFile_, 				1, 3);
	
	topLevelLayout_->addMultiCellWidget(cb_appendToLog_, 		2, 2, 0, 3);
	
	topLevelLayout_->addMultiCellWidget(l_maxLogFileSize_, 		3, 3, 0, 1);
	topLevelLayout_->addWidget(sb_maxLogFileSize_, 			3, 2);
	topLevelLayout_->addWidget(l_logFileKb_, 					3, 3);


	topLevelLayout_->activate();
}

	void
EmpathConfigPOP3Logging::setMailbox(EmpathMailboxPOP3 * mailbox)
{
	empathDebug("Set mailbox " + mailbox->name());
	mailbox_ = mailbox;
	loadData();
}

	void
EmpathConfigPOP3Logging::saveData()
{
	empathDebug("saveData() called");
	mailbox_->setLoggingPolicy(cb_logConversation_->isChecked());
	mailbox_->setLogFilePath(le_logFile_->text());
	mailbox_->setLogFileDisposalPolicy(cb_appendToLog_->isChecked());
	mailbox_->setMaxLogFileSize(QString(sb_maxLogFileSize_->text()).toInt());
}

	void
EmpathConfigPOP3Logging::loadData()
{
	if (mailbox_ == 0) return;
	
	cb_logConversation_->setChecked(mailbox_->loggingPolicy());
	le_logFile_->setText(mailbox_->logFilePath());
	cb_appendToLog_->setChecked(mailbox_->logFileDisposalPolicy());
	sb_maxLogFileSize_->setValue(mailbox_->maxLogFileSize());
}

EmpathConfigPOP3Logging::~EmpathConfigPOP3Logging()
{
	empathDebug("dtor");
}

	void
EmpathConfigPOP3Logging::s_viewLog()
{
}

	void
EmpathConfigPOP3Logging::s_chooseLogFile()
{
}

