
#ifndef ksync_manipulator_h
#define ksync_manipulator_h

#include <qpixmap.h>
#include <qstring.h>
#include <kparts/part.h>
#include <qptrlist.h>
#include <qstringlist.h>


#include <ksyncentry.h>

namespace KitchenSync {

  class ManipulatorPart : public KParts::Part {
    Q_OBJECT
    public:
     ManipulatorPart(QObject *parent = 0, const char *name  = 0 );
     virtual ~ManipulatorPart() {};
     // the Type this Part understands/ is able to interpret
     virtual QString type()const {return QString::null; };

     virtual int progress()const { return 0; };
     //virtual QString identifier()const { return QString::null; };
     virtual QString name()const { return QString::null; };

     virtual QString description()const { return QString::null; };
     virtual QPixmap *pixmap() { return 0l; };
     virtual QString iconName() const {return QString::null; };

     virtual bool partIsVisible()const { return false; }
     virtual bool configIsVisible()const { return true; }

     virtual QWidget *configWidget(){ return 0l; };

     virtual QPtrList<KSyncEntry> processEntry(QPtrList<KSyncEntry>* ) {
       QPtrList<KSyncEntry> ent;  return ent;
     };
    public slots:
     virtual void slotConfigOk() { };
  };
};

#endif
