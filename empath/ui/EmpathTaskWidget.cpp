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
# pragma implementation "EmpathTaskWidget.h"
#endif

// Qt includes
#include <qpixmap.h>

// KDE includes
#include <kapp.h>
#include <klocale.h>
#include <kpixmap.h>

// Local includes
#include "Empath.h"
#include "EmpathUIUtils.h"
#include "EmpathDefines.h"
#include "EmpathTaskWidget.h"

EmpathTaskWidget::EmpathTaskWidget(QWidget * parent, const char * name)
	:	QWidget(parent, name)
{
	empathDebug("ctor");

	KPixmap px;
	px.resize(width(), 20);
	px.gradientFill(
		kapp->palette().color(QPalette::Normal, QColorGroup::Background),
		kapp->palette().color(QPalette::Normal, QColorGroup::Base));
	
	l = new QLabel(this);
	CHECK_PTR(l);
	l->resize(width(), 20);
	l->setPixmap(px);

	itemList_.setAutoDelete(true);
	QObject::connect(
		empath, SIGNAL(newTask(EmpathTask *)),
		this,	SLOT(s_addTask(EmpathTask *)));
}

EmpathTaskWidget::~EmpathTaskWidget()
{
	empathDebug("dtor");
}

	void
EmpathTaskWidget::resizeEvent(QResizeEvent *)
{
	QListIterator<EmpathTaskItem> it(itemList_);
	
	int i = 0;
	
	for (; it.current(); ++it) {
		
		it.current()->move(
			0, it.current()->minimumSizeHint().height() * i++ + 20);
		it.current()->setFixedWidth(width());
	}
	
	KPixmap px;
	px.resize(width(), 20);
	px.gradientFill(
		kapp->palette().color(QPalette::Normal, QColorGroup::Background),
		kapp->palette().color(QPalette::Normal, QColorGroup::Base));
	l->setPixmap(px);
	l->resize(width(), 20);
}

	void
EmpathTaskWidget::s_addTask(EmpathTask * t)
{
	if (t->isDone()) return;
	empathDebug("s_addTask(" + t->name() + ") called");
	EmpathTaskItem * newItem = new EmpathTaskItem(t->name(), this, "taskItem");
	CHECK_PTR(newItem);
	
	QObject::connect(
		t,			SIGNAL(finished()),
		newItem,	SLOT(s_done()));
	
	QObject::connect(
		newItem,	SIGNAL(done(EmpathTaskItem *)),
		this,		SLOT(s_done(EmpathTaskItem *)));
	
	QObject::connect(
		t,			SIGNAL(maxChanged(int)),
		newItem,	SLOT(s_setMax(int)));
			
	QObject::connect(
		t,			SIGNAL(posChanged(int)),
		newItem,	SLOT(s_setPos(int)));
	
	QObject::connect(
		t,			SIGNAL(addOne()),
		newItem,	SLOT(s_inc()));
	
	itemList_.append(newItem);
	
	itemHeight_ = (newItem->minimumSizeHint().height());
	newItem->move(0, ((itemList_.count() - 1) * itemHeight_) + 20);
	newItem->setFixedWidth(width());
	newItem->s_setMax(t->max());
	newItem->s_setPos(t->pos());
	newItem->show();
	
	setFixedHeight(itemList_.count() * itemHeight_ + 20);
	resize(width(), itemList_.count() * itemHeight_ + 20);
	show();
	kapp->processEvents();
}

	void
EmpathTaskWidget::s_done(EmpathTaskItem * item)
{
	empathDebug("s_done() called");
	QListIterator<EmpathTaskItem> it(itemList_);
	itemList_.remove(item);
	
	int i = 0;
	
	for (; it.current(); ++it) {
		
		it.current()->move(
			0, it.current()->minimumSizeHint().height() * i++ + 20);
	}
	
	setFixedHeight(itemHeight_ * itemList_.count());
}

EmpathTaskItem::EmpathTaskItem(const QString & title,
		QWidget * parent, const char * name)
	:
		QWidget(parent, name),
		title_(title),
		pos_(0),
		max_(100)
{
	empathDebug("ctor");
	layout_ = new QGridLayout(this, 1, 2, 2, 10);
	CHECK_PTR(layout_);
	label_ = new QLabel(title_, this, "l_taskTitle");
	CHECK_PTR(label_);
	progressMeter_ = new QProgressBar(this, "taskProgress");
	CHECK_PTR(progressMeter_);
	
	setFixedHeight(progressMeter_->sizeHint().height());
	layout_->addWidget(label_,			0, 0);
	layout_->addWidget(progressMeter_,	0, 1);
	layout_->activate();
	show();
}

EmpathTaskItem::~EmpathTaskItem()
{
	empathDebug("dtor");
}

	void
EmpathTaskItem::s_done()
{
	empathDebug("s_done() called");
	emit(done(this));
	kapp->processEvents();
}

	void
EmpathTaskItem::s_inc()
{
	empathDebug("s_inc() called");
	progressMeter_->setProgress(++pos_);
	kapp->processEvents();
}

	void
EmpathTaskItem::s_setMax(int max)
{
	empathDebug("s_setMax(" + QString().setNum(max) + ") called");
	progressMeter_->setTotalSteps(max);
	max_ = max;
}

	void
EmpathTaskItem::s_setPos(int pos)
{
	empathDebug("s_setPos() called");
	progressMeter_->setProgress(pos_);
	kapp->processEvents();
}

	QSize
EmpathTaskItem::minimumSizeHint() const
{
	QSize s(0, progressMeter_->sizeHint().height());
	return s;
}

