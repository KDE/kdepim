#ifndef ksync_organizerpart_h
#define ksync_organizerpart_h

#include <klocale.h>
#include <qpixmap.h>
#include <kurlrequester.h>

#include <manipulatorpart.h>

#include "organizerbase.h"

class KAboutData;
class KConfig;
//class OrganizerDialogBase;
namespace KSync {

  class OrganizerPart : public ManipulatorPart {
  Q_OBJECT
  public:
    OrganizerPart(QWidget *parent, const char *name,
		  QObject *obj = 0, const char *na=0,
		  const QStringList & = QStringList() );
    virtual ~OrganizerPart();

    static KAboutData *createAboutData();

    QString type()const { return QString::fromLatin1("Organizer"); };
    int progress()const { return 0; };
    QString name()const { return i18n("Organizer" ); };
    QString description()const { return i18n("This part is responsible for syncing your\n Calendar."); };
    QPixmap *pixmap();
    bool partIsVisible()const { return false; };
    QWidget* widget();
    QWidget* configWidget();
    void processEntry( const Syncee::PtrList&,  Syncee::PtrList& );
  public:
      void slotConfigOk();
  private:
    QPixmap m_pixmap;
    QWidget *m_widget;
    OrganizerDialogBase *m_config;
    bool m_evo:1;
    bool m_configured:1;
    QString m_path;
    KConfig *m_conf;
  };
};

#endif
