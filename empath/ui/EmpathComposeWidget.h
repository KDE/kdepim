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
#include "EmpathDefines.h"
#include "EmpathURL.h"

class EmpathHeaderEditWidget;
class EmpathAttachmentListWidget;
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
		EmpathComposeWidget(
			ComposeType t, const EmpathURL &,
			QWidget * parent = 0, const char * name = 0);
		
		/**
		 * dtor
		 */
		~EmpathComposeWidget();

		/**
		 * Creates a stringified version of the message we're editing - that
		 * is the body only. The attachments are got with messageAttachments()
		 */
		QCString messageAsString();

		/**
		 * Test if there are any attachments for this message.
		 */
		bool messageHasAttachments();
		
		/**
		 * Get the attachment list. Check if there are attachments to be
		 * clipped on the end first by using messageHasAttachments()
		 */
		QList<EmpathAttachmentSpec> messageAttachments();
		
	protected slots:
		
	private:

		void spawnExternalEditor(const QCString & text);
		QSplitter *vSplit, *hSplit;

		QMultiLineEdit					* editorWidget_;
		EmpathHeaderEditWidget			* headerEditWidget_;
//		EmpathAttachmentListWidget		* attachmentListWidget_;
		EmpathSubjectSpecWidget			* subjectSpecWidget_;
		QListView						* lv_attachments_;
		QComboBox						* cmb_priority_;
		QLabel							* l_priority_;
		QGridLayout						* layout_;
		QGridLayout						* midLayout_;

		Q_UINT32 horizPannerAbsSeparator;
		QDict<KProcess>			processTable_;
		QDict<char>				externEditModified_;

};

#endif
