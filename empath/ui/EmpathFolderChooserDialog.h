
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

#ifndef EMPATHFOLDERCHOOSERDIALOG_H
#define EMPATHFOLDERCHOOSERDIALOG_H

// Qt includes
#include <qwidget.h>
#include <qdialog.h>
#include <qlayout.h>
#include <qpushbutton.h>

// KDE includes
#include <kbuttonbox.h>

class EmpathFolder;
class EmpathFolderWidget;

class EmpathFolderChooserDialog : public QDialog
{
	Q_OBJECT

	public:

		EmpathFolderChooserDialog(QWidget * parent = 0, const char * name = 0);

		~EmpathFolderChooserDialog();

		EmpathFolder * selectedFolder() const;

	protected slots:

		void s_OK();
		void s_cancel();
		void s_help();

	private:

		QGridLayout		* mainLayout_;

		KButtonBox		* buttonBox_;

		QPushButton		* pb_OK_;
		QPushButton		* pb_cancel_;
		QPushButton		* pb_help_;

		EmpathFolderWidget		* folderWidget_;
};

#endif

