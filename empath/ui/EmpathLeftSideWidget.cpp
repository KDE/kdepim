#include "EmpathLeftSideWidget.h"
#include "EmpathFolderWidget.h"
#include "EmpathTaskWidget.h"
#include "EmpathMessageListWidget.h"

EmpathLeftSideWidget::EmpathLeftSideWidget(
		EmpathMessageListWidget * messageListWidget,
		QWidget * parent, const char * name)
	:	QWidget(parent, name)
{
	empathDebug("ctor");
	
	layout_ = new QGridLayout(this, 2, 1, 0, 0);
	CHECK_PTR(layout_);
	
	folderWidget_ = new EmpathFolderWidget(this, "folderWidget", true);
	CHECK_PTR(folderWidget_);
	
	QObject::connect(
			folderWidget_,		SIGNAL(showFolder(const EmpathURL &)),
			messageListWidget,	SLOT(s_showFolder(const EmpathURL &)));
	
	QObject::connect(
			messageListWidget,	SIGNAL(showing()),
			folderWidget_,		SLOT(s_showing()));
	
	taskWidget_ = new EmpathTaskWidget(this, "taskWidget");
	CHECK_PTR(taskWidget_);
	
	layout_->addWidget(folderWidget_,	0, 0);
	layout_->addWidget(taskWidget_,		1, 0);
	
	layout_->activate();
}

EmpathLeftSideWidget::~EmpathLeftSideWidget()
{
	empathDebug("dtor");
}

