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

#ifndef EMPATHMAINWIDGET_H
#define EMPATHMAINWIDGET_H

// Qt includes
#include <qwidget.h>

// KDE includes
#include <kapp.h>
#include <kiconloader.h>
#include <knewpanner.h>

// Local includes
#include "EmpathDefines.h"

class EmpathFolderWidget;
class EmpathMessageViewWidget;
class EmpathMessageListWidget;
class EmpathStatusWidget;
class RMessage;

class EmpathMainWidget : public QWidget
{
	Q_OBJECT

	public:
		
		EmpathMainWidget(QWidget * parent = 0, const char * name = 0);
		~EmpathMainWidget();
		EmpathMessageListWidget * messageListWidget();
		EmpathMessageViewWidget * messageViewWidget();
		
	protected slots:
		
		void s_displayMessage(RMessage *);
		void s_folderWidgetSizeChange(int, int, int);
		void resizeEvent(QResizeEvent * e);

	private:

		KNewPanner				* vSplit;
		KNewPanner				* hSplit;
		
		EmpathFolderWidget		* folderWidget_;
		EmpathMessageListWidget	* messageListWidget_;
		EmpathMessageViewWidget	* messageViewWidget_;

		Q_UINT32			horizPannerAbsSeparator;
};

#endif
