#ifndef KSYNC_OVERVIEWPART
#define KSYNC_OVERVIEWPART

#include <klocale.h>
#include <qpixmap.h>

#include <manipulatorpart.h>

namespace KSync {

  class OverviewWidget;
  
  class OverviewPart : public ManipulatorPart {
   Q_OBJECT
  public:
    OverviewPart(QWidget *parent, const char *name,
		 QObject *object=0, const char *name2 = 0, // make GenericFactory loading possible
		 const QStringList & = QStringList() );
    virtual ~OverviewPart();
    static KAboutData *createAboutData();
    QString type()const { return QString::fromLatin1("Overview"); };
    int progress()const { return 0; };
    QString name()const { return i18n("Overview" ); };
    QString description()const { return i18n("This part is the main widget of KitchenSync"); };
    QPixmap *pixmap();
    bool partIsVisible()const { return true; };
    bool configIsVisible()const { return false; };
    QWidget* widget();
//    QWidget* configWidget();
public slots:
      void startSync();
      void slotProgress( ManipulatorPart* part, int syncStatus,  int progress );
      void slotSyncPartActivated( ManipulatorPart* );
  private:
    QPixmap m_pixmap;
    OverviewWidget *m_widget;
      //QWidget *m_config;
  };
};


#endif
