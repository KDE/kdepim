
#ifndef ksync_manipulator_h
#define ksync_manipulator_h

#include <qpixmap.h>
#include <kparts/part.h>
#include <qptrlist.h>

#include <ksyncentry.h>

namespace KitchenSync {

  class ManipulatorPart : public KParts::Part {
    public:
     ManipulatorPart(const QString &name, QWidget *parent = 0, const char *name  = 0 );
     virtual ~ManipulatorPart();
     // the Type this Part understands/ is able to interpret
     virtual QString type()const = 0;

     virtual int progress()const = 0;
     virtual QString identifier()const = 0;
     virtual QString name()const = 0;

     virtual QString description()const = 0;
     virtual QPixmap *pixmap()const = 0;
    public slots:
     virtual void syncEntries( const QPtrList<KSyncEntry> & ) = 0;     

  };
};


#endif
