
#ifndef ksync_manipulator_h
#define ksync_manipulator_h

#include <qpixmap.h>
#include <qstring.h>
#include <kparts/part.h>
#include <qptrlist.h>

#include <ksyncentry.h>

namespace KitchenSync {

  class ManipulatorPart : public KParts::Part {
    public:
     ManipulatorPart(const QString &name = QString::null, QWidget *parent = 0, const char *name  = 0 );
     virtual ~ManipulatorPart();
     // the Type this Part understands/ is able to interpret
     virtual QString type()const {return QString::null; };

     virtual int progress()const { return 0; };
     virtual QString identifier()const { return QString::null; };
     virtual QString name()const { return QString::null; };

     virtual QString description()const { return QString::null; };
     virtual const QPixmap *pixmap()const { return 0l; };
    public slots:
     virtual void syncEntries( const QPtrList<KSyncEntry> & ){ };     

  };
};


#endif
