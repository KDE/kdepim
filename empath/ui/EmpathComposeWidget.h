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

#ifndef EMPATHCOMPOSEWIDGET_H
#define EMPATHCOMPOSEWIDGET_H

// Qt includes
#include <qwidget.h>
#include <qsplitter.h>
#include <qlistview.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcombo.h>
#include <qdict.h>
#include <qdatetime.h>
#include <qfileinfo.h>
#include <qmultilinedit.h>

// Local includes
#include "EmpathEnum.h"
#include "EmpathDefines.h"
#include "EmpathURL.h"

class EmpathHeaderEditWidget;
class EmpathSubjectSpecWidget;
class RMessage;
class KProcess;

/**
 * The container for the various widgets used when composing.
 */
class EmpathComposeWidget : public QWidget
{
	Q_OBJECT

	public:
		
		/**
		 * Standard ctor
		 */
		EmpathComposeWidget(QWidget * parent = 0, const char * name = 0);

		EmpathComposeWidget(
			ComposeType t, const EmpathURL &,
			QWidget * parent = 0, const char * name = 0);
	
		EmpathComposeWidget(
			const QString &,
			QWidget * parent = 0, const char * name = 0);
		
		/**
		 * dtor
		 */
		~EmpathComposeWidget();

		/**
		 * The message we're editing.
		 */
		RMessage message();

		/**
		 * Test if there are any attachments for this message.
		 */
		bool messageHasAttachments();
		
		void init() { _init(); }
		
		void bugReport();
		
	protected slots:
		
		void	s_editorDone(bool ok, QCString text);
		
	private:

		void	_init();
		void	_reply(bool toAll = false);
		void	_forward();
		void	spawnExternalEditor(const QCString & text);

		QMultiLineEdit				* editorWidget_;
		EmpathHeaderEditWidget		* headerEditWidget_;
		EmpathSubjectSpecWidget		* subjectSpecWidget_;
		QListView					* lv_attachments_;
		QComboBox					* cmb_priority_;
		QLabel						* l_priority_;
		QGridLayout					* layout_;
		QGridLayout					* midLayout_;
		QSplitter					* vSplit;
		QSplitter					* hSplit;

		Q_UINT32				horizPannerAbsSeparator;
		ComposeType				composeType_;
		EmpathURL				url_;
		QString					recipient_;
};

#endif
