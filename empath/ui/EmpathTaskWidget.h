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

#ifndef EMPATH_TASK_WIDGET_H
#define EMPATH_TASK_WIDGET_H

// Qt includes
#include <qwidget.h>
#include <qpushbutton.h>
#include <qlabel.h>

// Local includes
#include "EmpathTask.h" 

class EmpathTaskWidget : public QWidget
{
	Q_OBJECT
		
	public:
		
		static void addTask(EmpathTask *);
	
		virtual	~EmpathTaskWidget();
		void	resizeEvent(QResizeEvent *);
		
	private:
		
		EmpathTaskWidget();
		
		static EmpathTaskWidget * taskWidget_;
		
		void _addTask(EmpathTask *);
};

#endif

