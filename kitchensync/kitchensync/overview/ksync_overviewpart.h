#ifndef KSYNC_OVERVIEWPART
#define KSYNC_OVERVIEWPART

#include <klocale.h>
#include <qpixmap.h>

#include <manipulatorpart.h>

namespace KitchenSync {

  class OverviewPart : public ManipulatorPart {
   Q_OBJECT
  public:
    OverviewPart(QWidget *parent, const char *name, const QStringList & = QStringList() );
    virtual ~OverviewPart();
    QString type()const { return QString::fromLatin1("Overview"); };
    int progress()const { return 0; };
    QString name()const { return i18n("Overview" ); };
    QString description()const { return i18n("This part is the main window of KitchenSync"); };
    QPixmap *pixmap();
    bool partIsVisible()const { return false; };
    QWidget* widget();
    QWidget* configWidget();
  private:
    QPixmap m_pixmap;
    QWidget *m_widget;
    QWidget *m_config;
  };
};

 
#endif
