
#ifndef EMPATH_LEFT_SIDE_WIDGET_H
#define EMPATH_LEFT_SIDE_WIDGET_H

#include <qwidget.h>
#include <qlayout.h>

class EmpathFolderWidget;
class EmpathTaskWidget;
class EmpathMessageListWidget;

class EmpathLeftSideWidget : public QWidget
{
	Q_OBJECT
		
	public:
		
		EmpathLeftSideWidget(
			EmpathMessageListWidget *, QWidget * parent, const char * name);
		virtual ~EmpathLeftSideWidget();
		
	private:
		
		EmpathFolderWidget	* folderWidget_;
		EmpathTaskWidget	* taskWidget_;
		QGridLayout			* layout_;
};

#endif
