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

#ifndef EMPATHMESSAGEHTMLVIEW_H
#define EMPATHMESSAGEHTMLVIEW_H

// Qt includes
#include <qpopupmenu.h>
#include <qregexp.h>

// KDE includes
#include <ktmainwindow.h>
#include <kstdaccel.h>
#include <khtml.h>

// Local includes
#include <RMM_Message.h>
#include "EmpathDefines.h"
#include "EmpathURL.h"

class EmpathMessageHTMLWidget : public KHTMLWidget
{
	Q_OBJECT

	public:
		
		EmpathMessageHTMLWidget(
				QWidget		* _parent	= 0,
				const char	* _name		= 0);
		
		~EmpathMessageHTMLWidget();
		
		/**
		 * Do the parsing and print to the widget
		 */
		void go();
		void toHTML(QCString &);
		void use(REnvelope &, RBodyPart &);
		void use(RBodyPart &);
//		void s_imageRequest(KHTMLWidget * view, const char *url);

	protected slots:


	private:
		
		REnvelope envelope_;
		RBodyPart bodyPart_;
		
		bool	loadTemplate(const QString & filename);

		void	replaceHeaderTagsByData(
					const QCString & messageHeaders, QCString & htmlTemplate);
		void	replaceBodyTagsByData(
					const QCString & messageBody, QCString & htmlTemplate);
		
		void	markupBackgroundColour(QCString & s);
		void	markupTextColour(QCString & s);
		void	markupHeaderBodies(const QCString & body, QCString & html);
		void	markupHeaderNames(QCString & html);
		
		QCString htmlTemplate;
		
		bool useEnvelope_;
};

#endif
