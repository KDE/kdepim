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

// Qt includes
#include <qstrlist.h>

// KDE includes
#include <klocale.h>

// Local includes
#include <RMM_Message.h>
#include "EmpathHeaderViewWidget.h"
#include "EmpathMessageViewWidget.h"
#include "EmpathComposeWindow.h"
#include "EmpathUtilities.h"
#include "EmpathUIUtils.h"
#include "Empath.h"
	
EmpathMessageViewWidget::EmpathMessageViewWidget(
		const EmpathURL & url,
		QWidget *parent,
		const char *name)
	:	QWidget(parent, name),
		url_(url)
{
	empathDebug("ctor");
	
	structureWidget_ =
		new EmpathMessageStructureWidget(0, "structureWidget");
	CHECK_PTR(structureWidget_);
	
	connect(
		structureWidget_,	SIGNAL(partChanged(RBodyPart *)),
		this,				SLOT(s_partChanged(RBodyPart *)));


	mainLayout_ = new QGridLayout(this, 3, 2, 0, 0);
	CHECK_PTR(mainLayout_);
	
	mainLayout_->setColStretch(0, 1);
	mainLayout_->setColStretch(1, 0);
	mainLayout_->setRowStretch(0, 1);
	mainLayout_->setRowStretch(1, 10);
	mainLayout_->setRowStretch(2, 0);
	
	empathDebug("Creating HTML widget");
	
	messageWidget_ = new EmpathMessageHTMLWidget(this, "messageWidget");
	CHECK_PTR(messageWidget_);
	
	empathDebug("Creating scrollbars");
	verticalScrollBar_ = new QScrollBar(QScrollBar::Vertical, this, "hScBar");
	CHECK_PTR(verticalScrollBar_);

	scrollbarSize_ = verticalScrollBar_->sizeHint().width();
	
	verticalScrollBar_->setFixedWidth(scrollbarSize_);
	
	horizontalScrollBar_ =
		new QScrollBar(QScrollBar::Horizontal, this, "hScBar");
	CHECK_PTR(horizontalScrollBar_);
	
	horizontalScrollBar_->setFixedHeight(scrollbarSize_);
	
	headerViewWidget_ =
		new EmpathHeaderViewWidget(this, "headerViewWidget");
	
	QObject::connect(
		headerViewWidget_,	SIGNAL(clipClicked()),
		this,				SLOT(s_clipClicked()));
	

	empathDebug("Adding widgets to layout");
	mainLayout_->addMultiCellWidget(headerViewWidget_,	0, 0, 0, 1);
	mainLayout_->addWidget(messageWidget_,				1, 0);
	mainLayout_->addWidget(verticalScrollBar_,			1, 1);
	mainLayout_->addWidget(horizontalScrollBar_,		2, 0);
	
	QObject::connect(messageWidget_, SIGNAL(scrollVert(int)),
			SLOT(s_vScrollbarSetValue(int)));
	
	QObject::connect(messageWidget_, SIGNAL(scrollHorz(int)),
			SLOT(s_hScrollbarSetValue(int)));
	
	QObject::connect(verticalScrollBar_, SIGNAL(valueChanged(int)),
			messageWidget_, SLOT(slotScrollVert(int)));
	
	QObject::connect(horizontalScrollBar_, SIGNAL(valueChanged(int)),
			messageWidget_, SLOT(slotScrollHorz(int)));
	
	QObject::connect(messageWidget_, SIGNAL(documentChanged()), SLOT(s_docChanged()));

	QObject::connect(messageWidget_, SIGNAL(URLSelected(const char *, int)),
			SLOT(s_URLSelected(const char *, int)));
	
	s_docChanged();
	mainLayout_->activate();
	show();	
}

	void
EmpathMessageViewWidget::go()
{
	empathDebug("go() called");

	QCString s;
	RMessage * m(empath->message(url_));
	
	if (m == 0) {
		empathDebug("Can't load message from \"" + url_.asString() + "\"");
		return;
	}
	
	RBodyPart message(m->decode());
	structureWidget_->setMessage(message);
	
	// Ok I'm going to try and get the viewable body parts now.
	// To start with, I'll see if there's only one part. If so, I'll show it,
	// if possible. Otherwise, I'll have to have a harder look and see what
	// we're supposed to be showing. If we're looking at a
	// multipart/alternative, I'll pick the 'best' of the possibilities.
	
	headerViewWidget_->useEnvelope(message.envelope());
	
	empathDebug("envelope:");
	empathDebug(message.envelope().asString());
	
	if (message.body().count() == 0)
		s = message.data();
	
	else if (message.body().count() == 1)
		s = message.body().at(0)->decode().data();
	
	else {
		
		// Multipart message
		
		empathDebug("===================== MULTIPART ====================");
		
		QListIterator<RBodyPart> it(message.body());
		
		int i = 0;
		for (; it.current(); ++it) {
		
			++i;
		
			empathDebug(
				" ===================== PART # "
				+ QString().setNum(i) +
				" =====================");

			if (it.current()->envelope().has(RMM::HeaderContentType)) {
				
				empathDebug("Ok this part has a Content-Type");                            
					
				RContentType t = it.current()->envelope().contentType();
				
				empathDebug("   Type of this part is \"" + t.type() + "\"");
				empathDebug("SubType of this part is \"" + t.subType() + "\"");

				if (!stricmp(t.type(),
							RMM::mimeTypeEnum2Str(RMM::MimeTypeText))) {
					
					if (!stricmp(t.subType(),
							RMM::mimeSubTypeEnum2Str(RMM::MimeSubTypeHTML))) {

						empathDebug("Using this part as body");

						RBodyPart p(*it.current());
					
						s = p.decode().data();
						messageWidget_->show(s, true);
						return;
	
					} else if (!stricmp(t.subType(),
							RMM::mimeSubTypeEnum2Str(RMM::MimeSubTypePlain))) {
					
						empathDebug("Using this part as body");

						RBodyPart p(*it.current());
					
						s = p.decode().data();
						messageWidget_->show(s);
						return;
					}
					
				} else {

					empathDebug("Haven't decided what to do with this part yet");
				}
			}
			empathDebug("...");
		}
		
		empathDebug("=================== END MULTIPART =====================");
	}

	messageWidget_->show(s);
}

	void
EmpathMessageViewWidget::s_print()
{
	empathDebug("print() called");
	messageWidget_->print();
}

EmpathMessageViewWidget::~EmpathMessageViewWidget()
{
	empathDebug("dtor");
	delete structureWidget_;
}

	void
EmpathMessageViewWidget::resizeEvent(QResizeEvent * e)
{
	resize(e->size());
	s_docChanged();
}

	void
EmpathMessageViewWidget::s_docChanged()
{
	// Hide scrollbars if they're not necessary

	if (messageWidget_->docHeight() > messageWidget_->height() ||
		verticalScrollBar_->value() != 0) {
		verticalScrollBar_->show();
		verticalScrollBar_->setFixedWidth(scrollbarSize_);
	} else {
		verticalScrollBar_->hide();
		verticalScrollBar_->setFixedWidth(0);
	}
	
	if (messageWidget_->docWidth() > messageWidget_->width() ||
		horizontalScrollBar_->value() != 0) {
		horizontalScrollBar_->show();
		horizontalScrollBar_->setFixedHeight(scrollbarSize_);
	} else {
		horizontalScrollBar_->hide();
		horizontalScrollBar_->setFixedHeight(0);
	}
	
	verticalScrollBar_->setRange(0,
		messageWidget_->docHeight() - messageWidget_->height());
	verticalScrollBar_->setSteps(12, messageWidget_->height() - 24);

	
	horizontalScrollBar_->setRange(0,
		messageWidget_->docWidth() - messageWidget_->width());
	horizontalScrollBar_->setSteps(12, messageWidget_->width() - 24);
}

	void
EmpathMessageViewWidget::s_setMessage(const EmpathURL & url)
{
	empathDebug("setMessage() called with \"" + url.asString() + "\"");
	url_ = url;
}

	void
EmpathMessageViewWidget::s_vScrollbarSetValue(int val)
{
	empathDebug("vScrollbarSetValue(" + QString().setNum(val) + ") called");
	verticalScrollBar_->setValue(val);
}

	void
EmpathMessageViewWidget::s_hScrollbarSetValue(int val)
{
	empathDebug("hScrollbarSetValue(" + QString().setNum(val) + ") called");
	horizontalScrollBar_->setValue(val);
}

	void
EmpathMessageViewWidget::s_URLSelected(const char * url, int button)
{
	empathDebug("URL \" " + QString(url) + "\" clicked with button " +
			QString().setNum(button));

	QString fixedURL(url);

	fixedURL = fixedURL.stripWhiteSpace();

	if (fixedURL.left(7) == "mailto:") {
		
		fixedURL = fixedURL.right(fixedURL.length() - 7);

		if (button == 1) {
			empath->s_compose();
		}
	
	} else {

		// It's an URL we don't handle. Pass to a KFM.

//		KFM *kfm = new KFM();
//		kfm->openURL(fixedURL);
//		delete kfm;
	}
}

	void
EmpathMessageViewWidget::s_clipClicked()
{
	structureWidget_->show();
}

	void
EmpathMessageViewWidget::s_partChanged(RBodyPart * part)
{
	empathDebug("s_partChanged() called");
	RBodyPart p(*part);
	messageWidget_->show(p.data());
}

