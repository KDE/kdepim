#ifndef OVERVIEWWIDGET_H
#define OVERVIEWWIDGET_H

#include <qvariant.h>
#include <qscrollview.h>
#include <qwidget.h>
#include <qptrlist.h>

#include <manipulatorpart.h>

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
      OverviewWidget( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
      ~OverviewWidget();
      
      
    public:
      void setDeviceName(QString);
      void setNameField(QString);
      void setLogo(QPixmap);
      /**
       * Prints the list of possible syncable features
       */
      void showList(QPtrList<ManipulatorPart>);
    };
  
  class NewProgress : public QWidget {
    Q_OBJECT

     private:
    QLabel* progressItemPix;
    QLabel* progressLabel;
    QLabel* statusLabel;

      public:
    NewProgress( QPixmap &icon, 
		 QString text, 
		 QWidget* parent = 0,
		 const char* name = 0,
		 WFlags fl = 0) ; 
    ~NewProgress();
    
  public: 
    void setProgressItemPix(QPixmap);
    void setProgressLabel(QString);
    void setStatusLabel(QPixmap);
    
};
  
};

#endif 
