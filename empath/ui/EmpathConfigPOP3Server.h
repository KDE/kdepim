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
# pragma interface "EmpathConfigPOP3Server.h"
#endif

#ifndef EMPATHCONFIGPOP3SERVER_H
#define EMPATHCONFIGPOP3SERVER_H

// Qt includes
#include <qdialog.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qcheckbox.h>

// Local includes
#include "EmpathDefines.h"

class RikGroupBox;
class EmpathMailboxPOP3;

/**
 * Configure name, port, password etc for pop3 server
 */
class EmpathConfigPOP3Server : public QWidget
{
	Q_OBJECT

	public:
		
		EmpathConfigPOP3Server(QWidget * parent = 0, const char * name = 0);

		~EmpathConfigPOP3Server();
		
		void setMailbox(EmpathMailboxPOP3 * mailbox);

		void fillInSavedData();
		
		void loadData();
		void saveData();
		
	protected slots:

		void	s_starPassword(bool yn);

	protected:

	private:

		EmpathMailboxPOP3	* mailbox_;

		QLabel			* l_uname_;
		QLabel			* l_pass_;
		QLabel			* l_inServer_;
		QLabel			* l_inServerPort_;
		
		QLineEdit		* le_uname_;
		QLineEdit		* le_pass_;
		QLineEdit		* le_inServer_;
		QLineEdit		* le_inServerPort_;

		QPushButton		* pb_starPassword_;
		
		QGridLayout		* topLevelLayout_;
};

#endif
