#ifndef RESULT_RECEIVER_H
#define RESULT_RECEIVER_H

#include <qstring.h>
#include <qstringlist.h>
#include <qobject.h>

#include <dcopobject.h>

#include "Entry.h"

class ResultReceiver : public QObject, virtual public DCOPObject
{
  Q_OBJECT
  K_DCOP

  public:

    ResultReceiver(const QString & abName, QObject * parent, const char * name);

  k_dcop:

    void slotEntryComplete(int id, Entry);
    void slotEntryListComplete(int id, QStringList);

  signals:

    void entryComplete(int, Entry);
    void entryListComplete(int, QStringList);
};

#endif

