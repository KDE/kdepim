#ifndef ksync_organizerpart_h
#define ksync_organizerpart_h

#include <klocale.h>
#include <qpixmap.h>

#include <manipulatorpart.h>

#include "organizerbase.h"

class KAboutData;
class KConfig;
class KAlendarSyncEntry;
//class OrganizerDialogBase;
namespace KitchenSync {

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
    void processEntry( const QPtrList<KSyncEntry>&,  QPtrList<KSyncEntry>& );
  public:
      void slotConfigOk();
  private:
    KAlendarSyncEntry* meta();
    QPixmap m_pixmap;
    QWidget *m_widget;
    OrganizerDialogBase *m_config;
    bool m_evo:1;
    bool m_configured;
    QString m_path;
    KConfig *m_conf;
  };
};

#endif
