#ifndef OVERVIEWWIDGET_H
#define OVERVIEWWIDGET_H

#include <qvariant.h>
#include <qscrollview.h>
#include <qwidget.h>

class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QFrame;
class QLabel;
class newProgress;

namespace KitchenSync{

  class OverviewWidget : public QWidget
    { 
      Q_OBJECT
	
	public:
      OverviewWidget( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
      ~OverviewWidget();
      
    private:
      QFrame* Line;
      /**
       * The logo should be 120 x 120 pixel
       *
       */
      QLabel* deviceLogo;
      QLabel* deviceName;
      QLabel* nameField;
      QLabel* progressWindow;
      QWidget* progressWindowPart;
      QScrollView *sv;
      
    private:
      void showProgressPart();
      
    public:
      void setDeviceName(QString);
      void setNameField(QString);
      void setLogo(QPixmap);
      
    };
  
  class NewProgress : public QWidget {
    Q_OBJECT
    
      public:
    NewProgress( QPixmap &icon, 
		 QString text, 
		 bool progress,
		 QWidget* parent = 0,
		 const char* name = 0,
		 WFlags fl = 0) ; 
    
    QLabel* progressItemPix;
    QLabel* progressLabel;
};
  
};

#endif 
