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
# pragma interface "EmpathComposeWidget.h"
#endif

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
#include "Empath.h"
#include "EmpathDefines.h"
#include "EmpathURL.h"
#include "EmpathHeaderSpecWidget.h"
#include "RMM_Message.h"

class EmpathAttachmentListWidget;

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
			Empath::ComposeType t, const EmpathURL &,
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
		
		bool haveTo();
		bool haveSubject();
		
	protected slots:
		
		void	s_editorDone(bool ok, QCString text);
	
		void	s_cut();
		void	s_copy();
		void	s_paste();
		void	s_selectAll();
		
		void	s_addAttachment();
		void	s_editAttachment();
		void	s_removeAttachment();
		
	private:

		void	_init();
		void	_reply(bool toAll = false);
		void	_forward();
		void	_spawnExternalEditor(const QCString & text);
		void	_addExtraHeaders();
		void	_addHeader(const QString &, const QString & = QString::null);
		void	_set(const QString &, const QString &);
		QString	_get(const QString &);
		
		EmpathAttachmentListWidget * attachmentWidget_;

		QMultiLineEdit	* editorWidget_;
		QListView		* lv_attachments_;
		QComboBox		* cmb_priority_;
		QLabel			* l_priority_;
		QGridLayout		* layout_;
		QGridLayout		* extraLayout_;
		QGridLayout		* headerLayout_;
		QLabel			* l_subject_;
		QLineEdit		* le_subject_;
		
		QList<EmpathHeaderSpecWidget> headerSpecList_;

		Empath::ComposeType	composeType_;

		EmpathURL	url_;
		QString		recipient_;
		
		int			maxSizeColOne_;
};

#endif
