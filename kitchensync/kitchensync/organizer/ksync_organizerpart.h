#ifndef ksync_organizerpart_h
#define ksync_organizerpart_h

#include <klocale.h>
#include <qpixmap.h>
#include <kurlrequester.h>

#include <manipulatorpart.h>

#include "organizerbase.h"

class KAboutData;
class KConfig;
class KSimpleConfig;
//class OrganizerDialogBase;

namespace KCal {
    class CalendarLocal;
};

namespace KSync {
    class Syncee;
    class EventSyncee;
    class TodoSyncee;
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
    enum Data{ Calendar = 0,
               Todo };

    QPixmap m_pixmap;
    QWidget *m_widget;
    OrganizerDialogBase *m_config;
    TodoSyncee* loadTodos(const QString& path, const QString& timeZoneId );
    EventSyncee* loadEvents( const QString& path, const QString& timeZoneId );
    void doMeta( EventSyncee*, TodoSyncee*,  const QString& path);
    void doMetaIntern( Syncee*, KSimpleConfig*, const QString& key);
    void writeMeta( EventSyncee*, TodoSyncee*, const QString& path );
    void writeMetaIntern( Syncee*, KSimpleConfig*, const QString& key );
    void save( EventSyncee*, TodoSyncee*, const QString& path, const QString& timeZoneId );
    bool isEvolutionSync()const;
    QString path( Data d, const QString& str );
  };
};

#endif
