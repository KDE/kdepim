#ifndef OVERVIEWPROGRESSENTRY_H
#define OVERVIEWPROGRESSENTRY_H

#include <qlabel.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qwidget.h>

namespace KSync {

namespace OverView {

class OverViewProgressEntry : public QWidget
{
  Q_OBJECT

  public:
    OverViewProgressEntry( QWidget* parent, const char* name );
    ~OverViewProgressEntry();
	
    void setText( QString );
    void setProgress( int );
    void setPixmap( QPixmap );
    QString name();
	
  private:
    QString m_name;
    QLabel* m_textLabel;
    QLabel* m_progressField;
    QLabel* m_pixmapLabel;
};

}

}

#endif
