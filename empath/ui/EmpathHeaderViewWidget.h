#ifndef EMPATH_HEADER_VIEW_WIDGET_H
#define EMPATH_HEADER_VIEW_WIDGET_H

// Qt includes
#include <qwidget.h>
#include <qstrlist.h>
#include <qpixmap.h>

// Local includes
#include <RMM_Envelope.h>

class EmpathHeaderViewWidget : public QWidget
{
	Q_OBJECT
		
	public:
		
		EmpathHeaderViewWidget(QWidget * parent, const char * name);
		virtual ~EmpathHeaderViewWidget();
	
		void useEnvelope(REnvelope &);
		
	signals:
	
		void clipClicked();
		
	protected:
		
		void paintEvent(QPaintEvent *);
		void resizeEvent(QResizeEvent *);
		void mouseMoveEvent(QMouseEvent *);
		void leaveEvent(QEvent * e) { mouseMoveEvent(0); }
		void mousePressEvent(QMouseEvent * e);

	private:
		
		bool			resized_;
		QPixmap			underClip_;
		QPixmap			buf_;
		QStrList		headerList_;
		QPixmap			clipIcon_;
		QPixmap			clipGlow_;
		bool			glowing_;
};

#endif

