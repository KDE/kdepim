#ifndef ksync_organizerpart_h
#define ksync_organizerpart_h

#include <klocale.h>
#include <qpixmap.h>

#include <manipulatorpart.h>

namespace KitchenSync {

  class OrganizerPart : public ManipulatorPart {
  public:
    OrganizerPart(QWidget *parent, const char *name, const QStringList & );
    virtual ~OrganizerPart();
    QString type()const { return QString::fromLatin1("Organizer"); };
    int progress()const { return 0; };
    QString name()const { return i18n("Organizer" ); };
    QString description()const { return i18n("This part is responsible for syncing your\n Calendar."); };
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
