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
# pragma interface "EmpathConfigPOP3Logging.h"
#endif

#ifndef EMPATHCONFIGPOP3LOGGING_H
#define EMPATHCONFIGPOP3LOGGING_H

// Qt includes
#include <qwidget.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qspinbox.h>

// Local includes
#include "EmpathDefines.h"

class EmpathMailboxPOP3;

/**
 * Configure what happens during a pop3 transaction wrt logging.
 * Perhaps we'll be using kio to do this kind of stuff soon.
 */
class EmpathConfigPOP3Logging : public QWidget
{
	Q_OBJECT

	public:
		
		EmpathConfigPOP3Logging(QWidget * parent = 0, const char * name = 0);
		
		~EmpathConfigPOP3Logging();
		
		void setMailbox(EmpathMailboxPOP3 * mailbox);
		
		void loadData();
		void saveData();
		
	protected slots:

		void	s_viewLog();
		void	s_chooseLogFile();

	private:

		EmpathMailboxPOP3	* mailbox_;

		QCheckBox		* cb_logConversation_;
		QCheckBox		* cb_appendToLog_;
		QLabel			* l_logFile_;
		QLineEdit		* le_logFile_;
		QPushButton		* pb_browseLogFile_;
		QPushButton		* pb_viewCurrentLog_;
		QLabel			* l_maxLogFileSize_;
		QSpinBox		* sb_maxLogFileSize_;
		QLabel			* l_logFileKb_;
		
		QGridLayout		* topLevelLayout_;
};
#endif
