#ifndef KSYNC_DEBUGGER
#define KSYNC_DEBUGGER

#include <klocale.h>
#include <qpixmap.h>

#include <manipulatorpart.h>

class KAboutData;

namespace KSync {

class Debugger : public ManipulatorPart
{
   Q_OBJECT
  public:
    Debugger( QWidget *parent, const char *name,
              QObject *object=0, const char *name2 = 0, // make GenericFactory loading possible
              const QStringList & = QStringList() );
    virtual ~Debugger();

    static KAboutData *createAboutData();

    QString type() const;
    QString name() const;
    QString description() const;
    bool partIsVisible() const;
    QPixmap *pixmap();
    QString iconName() const;
    QWidget* widget();

  private:
    QPixmap m_pixmap;
    QWidget *m_widget;
};

}

#endif
