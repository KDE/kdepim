
#include <iostream>

// Qt includes
#include <qstrlist.h>
#include <qlabel.h>

// KDE includes
#include <kconfig.h>
#include <klocale.h>
#include <kapp.h>

// Local includes
#include "EmpathHeaderViewWidget.h"
#include "EmpathUIUtils.h"
#include "EmpathConfig.h"
#include "EmpathDefines.h"
#include <RMM_Header.h>

EmpathHeaderViewWidget::EmpathHeaderViewWidget(
		QWidget * parent, const char * name)
	:	QWidget(parent, name),
		layout_(0)
{
	empathDebug("ctor");
}

EmpathHeaderViewWidget::~EmpathHeaderViewWidget()
{
	empathDebug("dtor");
}

	void
EmpathHeaderViewWidget::useEnvelope(REnvelope & e)
{
	if (layout_ != 0) {
		
		QLayoutIterator lit(layout_->iterator());
		QLayoutItem * i;
		while (i = lit.current())
			delete i->widget();
		
		delete layout_;
		layout_ = 0;
	}

	layout_ = new QGridLayout(this, 0, 3);
	CHECK_PTR(layout_);
	
	layout_->setColStretch(0, 0);
	layout_->setColStretch(1, 10);
	layout_->setColStretch(2, 0);
	
	KConfig * c(kapp->getConfig());
	c->setGroup(EmpathConfig::GROUP_DISPLAY);
	
	// FIXME Must be QStringList when available.
	QStrList l;
	c->readListEntry(EmpathConfig::KEY_SHOW_HEADERS, l);
	
	QStrListIterator it(l);
	
	for (; it.current(); ++it) {
		
		RHeader * h(e.get(it.current()));
		
		if (h == 0)
			continue;
		
		layout_->expand(layout_->numRows() + 1, 3);
		cerr << "Expanded layout to " << QString().setNum(layout_->numCols())
			<< ", " << QString().setNum(layout_->numRows()) << endl;
		
		QLabel * label = new QLabel(QString(h->headerName()) + ":", this);
		label->show();
		
		QLabel * urlLabel =
			new QLabel(QString(h->headerBody()->asString()), this);
		urlLabel->show();
		
		cerr << "Adding widget at row " << QString().setNum(layout_->numRows()) << endl;
		layout_->addWidget(label, layout_->numRows() - 1, 0);
		cerr << "Adding widget at row " << QString().setNum(layout_->numRows()) << endl;
		layout_->addWidget(urlLabel, layout_->numRows() - 1, 1);
	}
	
	QPixmap clipIcon(empathIcon("clip.png"));
	
	clipButton_ =
		new KToolBarButton(clipIcon, 0,
		this, "clipButton_", 26, i18n("Message Structure"));
	
	clipButton_->setFixedSize(clipIcon.width() + 2, clipIcon.height() + 2);
	
	clipButton_->show();
	
	layout_->addMultiCellWidget(clipButton_, 0, layout_->numRows() - 1, 2, 2);
	
	QObject::connect(clipButton_, SIGNAL(clicked()),
		parent(), SLOT(s_clipClicked()));
		
	layout_->activate();
}
