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

// KDE includes
#include <klocale.h>

// Local includes
#include "Empath.h"
#include "RikGroupBox.h"
#include "EmpathDefines.h"
#include "EmpathFilterManagerDialog.h"
#include "EmpathFilterEditDialog.h"
#include "EmpathFilterList.h"
#include "EmpathFilter.h"

EmpathFilterManagerDialog::EmpathFilterManagerDialog(
		QWidget * parent, const char * name)
	:	QWidget(parent, name)
{
	empathDebug("ctor");

	mainLayout_ = new QGridLayout(this, 1, 1, 10, 10);
	CHECK_PTR(mainLayout_);
	
	QPushButton	tempButton((QWidget *)0);
	Q_UINT32 h	= tempButton.sizeHint().height();

	rgb_filters_ = new RikGroupBox(QString::null, 8, this, "rgb_filters");
	CHECK_PTR(rgb_filters_);
	
	w_filters_ = new QWidget(rgb_filters_, "w_filters");
	CHECK_PTR(w_filters_);

	rgb_filters_->setWidget(w_filters_);
	
	l_about_ =
		new QLabel(i18n("The following filters will be applied, in order, to new mail."),
				w_filters_, "l_filtersFolder");
	CHECK_PTR(l_about_);

	l_about_->setFixedHeight(h);
	
	lv_filters_ = new QListView(w_filters_, "lv_matches");
	CHECK_PTR(lv_filters_);

	lv_filters_->addColumn(i18n("Filter descriptions"));
	lv_filters_->setFrameStyle(QFrame::Box | QFrame::Sunken);

	filtersButtonBox_ = new KButtonBox(w_filters_, KButtonBox::VERTICAL);
	CHECK_PTR(filtersButtonBox_);

	pb_addFilter_ = filtersButtonBox_->addButton(i18n("Add filter"));
	CHECK_PTR(pb_addFilter_);

	pb_editFilter_ = filtersButtonBox_->addButton(i18n("Edit filter"));
	CHECK_PTR(pb_editFilter_);
	
	pb_removeFilter_ = filtersButtonBox_->addButton(i18n("Delete filter"));
	CHECK_PTR(pb_removeFilter_);
	
	filtersButtonBox_->addStretch();
	
	pb_moveUp_ = filtersButtonBox_->addButton(i18n("Move up"));
	CHECK_PTR(pb_moveUp_);
	
	pb_moveDown_ = filtersButtonBox_->addButton(i18n("Move down"));
	CHECK_PTR(pb_moveDown_);
	
	filtersButtonBox_->addStretch();

	QObject::connect(pb_addFilter_, SIGNAL(clicked()),
			this, SLOT(s_addFilter()));
	
	QObject::connect(pb_editFilter_, SIGNAL(clicked()),
			this, SLOT(s_editFilter()));
	
	QObject::connect(pb_removeFilter_, SIGNAL(clicked()),
			this, SLOT(s_removeFilter()));
	
	QObject::connect(pb_moveUp_, SIGNAL(clicked()),
			this, SLOT(s_moveUp()));
	
	QObject::connect(pb_moveDown_, SIGNAL(clicked()),
			this, SLOT(s_moveDown()));
	
	filtersButtonBox_->layout();
	filtersButtonBox_->setFixedWidth(filtersButtonBox_->sizeHint().width());
	filtersButtonBox_->setMinimumHeight(filtersButtonBox_->sizeHint().height());

	rgb_filters_->setMinimumHeight(h * 4);  
	rgb_filters_->setMinimumWidth(filtersButtonBox_->sizeHint().width() + 40);

	filtersLayout_ = new QGridLayout(w_filters_, 2, 2, 10, 10);
	CHECK_PTR(filtersLayout_);

	filtersLayout_->addMultiCellWidget(l_about_,	0, 0, 0, 1);
	filtersLayout_->addWidget(lv_filters_,			1, 0);
	filtersLayout_->addWidget(filtersButtonBox_,	1, 1);
	
	filtersLayout_->activate();
	
	mainLayout_->addWidget(rgb_filters_, 0, 0);
	mainLayout_->activate();
	update();
}

EmpathFilterManagerDialog::~EmpathFilterManagerDialog()
{
	empathDebug("dtor");
}
		
	void
EmpathFilterManagerDialog::s_addFilter()
{
	empathDebug("s_addFilter() called");
	
	EmpathFilter * newFilter = new EmpathFilter;
	
	EmpathFilterEditDialog filterEditDialog(newFilter,
			this, "filterEditDialog");
	
	if (filterEditDialog.exec() == Cancel) {
		empathDebug("Deleting unwanted filter");
		delete newFilter;
		return;
	}
	
	// 0 is the first- filter becomes the last when it's added.
	newFilter->setId(empath->filterList().count());
	empath->filterList().append(newFilter);
	update();
}

	void
EmpathFilterManagerDialog::s_editFilter()
{
	empathDebug("s_editFilter() called");
	
	EmpathFilter * editedFilter =
		((EmpathFilterListItem *)lv_filters_->currentItem())->filter();
	
	EmpathFilterEditDialog filterEditDialog(editedFilter,
			this, "filterEditDialog");

	if (filterEditDialog.exec() == Cancel) return;
	
	update();
}

	void
EmpathFilterManagerDialog::s_removeFilter()
{
	empathDebug("s_removeFilter() called");
}

	void
EmpathFilterManagerDialog::s_moveUp()
{
	empathDebug("s_moveUp() called");
}

	void
EmpathFilterManagerDialog::s_moveDown()
{
	empathDebug("s_moveDown() called");
}

	void
EmpathFilterManagerDialog::update()
{
	lv_filters_->setUpdatesEnabled(false);

	lv_filters_->clear();

	EmpathFilterListIterator it(empath->filterList());

	for (; it.current(); ++it)
		new EmpathFilterListItem(lv_filters_, it.current());
	
	lv_filters_->setUpdatesEnabled(true);
	lv_filters_->triggerUpdate();
}

	void
EmpathFilterManagerDialog::saveData()
{
	empathDebug("saveData() called");
	empath->filterList().save();
}

