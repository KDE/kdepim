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

#ifndef EMPATHMESSAGEDOWNLOADSTATUS_H
#define EMPATHMESSAGEDOWNLOADSTATUS_H

// Qt includes
#include <qwidget.h>
#include <qlabel.h>
#include <qlayout.h>

// KDE includes
#include <kprogress.h>

// Local includes
#include "EmpathDefines.h"

class EmpathMessageDownloadStatus : public QWidget
{
	Q_OBJECT
	
	public:

		EmpathMessageDownloadStatus(
				QWidget * parent = 0,
				const char * name = 0);

		~EmpathMessageDownloadStatus();

	public slots:

		void s_setNumberOfMessages(Q_UINT32);
		void s_setInitialMailboxBytes(Q_UINT32);
		void s_setMailboxBytes(Q_UINT32);
		void s_nextMessage(Q_UINT32);

	private:

		Q_UINT32	numMsgs_;
		Q_UINT32	totalMailboxBytes_;
		Q_UINT32	initialTotalMailboxBytes_;
		Q_UINT32	msgNo_;

		QLabel			* l_messageNumber_;
		QLabel			* l_totalKbLeft_;
		KProgress		* pr_allMessages_;
		QHBoxLayout		* mainLayout_;

//Message 1/10 [..   ] Total 54Kb left Message progress [...  ] 43 bytes left
};

#endif

