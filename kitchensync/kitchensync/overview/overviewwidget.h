#ifndef OVERVIEWWIDGET_H
#define OVERVIEWWIDGET_H

#include <qvariant.h>
#include <qscrollview.h>
#include <qwidget.h>
#include <qptrlist.h>

#include <konnector.h>

#include <manipulatorpart.h>

class QVBoxLayout;
class QVBox;
class QHBoxLayout;
class QGridLayout;
class QFrame;
class QLabel;
class newProgress;

namespace KitchenSync{
    class ManipulatorPart;
    class NewProgress;
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
      NewProgress* current;
      QPtrList<NewProgress> m_progress;
      QVBox *m_svView;
       QVBoxLayout *box;

    private:
      void showProgressPart();

    public:
      OverviewWidget( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
      ~OverviewWidget();


    public:
      /**
       * Set the name of the device to sync with.
       * Its the bigger Name on top
       * @param the Name
       */
      void setDeviceName(const QString&);

      /**
       *
       *
       */
      void setNameField(const QString&);

      /**
       * With this the it is possible to set a custom logo for the device
       * @param the logo pixmap
       */
      void setLogo(const QPixmap &);

      /**
       * Prints the list of possible syncable features
       */
      void showList(QPtrList<ManipulatorPart>);
      /**
       *
       */
      void clearProgress(const Profile&,  Konnector* con,  const QString& id);
      void addProgress( ManipulatorPart* part );
      void currentDone();
    };

  /**
   * This class is used for the sync entrys in the lower
   * part.
   * It has a pixmap, the name and a progress indicator
   */
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
    /**
     * sets the icon
     * @param the pixmap
     */
    void setProgressItemPix(const QPixmap&);

    /**
     * sets the name
     * @param the name
     */
    void setProgressLabel(const QString& );

    void timerEvent(QTimerEvent *);

    /**
     * sets the icon on status.
     * @param 0 for working 1 for done
     */
    void setStatusLabel(int status);

  };

};

#endif
