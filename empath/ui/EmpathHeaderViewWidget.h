#ifndef EMPATH_HEADER_VIEW_WIDGET_H
#define EMPATH_HEADER_VIEW_WIDGET_H

// Qt includes
#include <qwidget.h>
#include <qlayout.h>

// KDE includes
#include <ktoolbar.h>

// Local includes
#include <RMM_Envelope.h>

class EmpathHeaderViewWidget : public QWidget
{
	Q_OBJECT
		
	public:
		
		EmpathHeaderViewWidget(QWidget * parent, const char * name);
		virtual ~EmpathHeaderViewWidget();
	
		void useEnvelope(REnvelope &);
		
	private:
		
		QGridLayout		* layout_;
		KToolBarButton	* clipButton_;
};

#endif

