#include "ResultReceiver.h"
#include "Entry.h"

ResultReceiver::ResultReceiver
(
 const QString & abName,
 QObject * parent,
 const char * name
)
  : QObject(parent, name),
    DCOPObject(name)
{
  connectDCOPSignal
    (
     0, 0, //"KAddressBookServer",
     //abName.utf8(),
     "entryComplete(int,Entry)",
     "slotEntryComplete(int,Entry)",
     false
    );

  connectDCOPSignal
    (
     0, 0, //"KAddressBookServer",
     //abName.utf8(),
     "entryListComplete(int,QStringList)",
     "slotEntryListComplete(int,QStringList)",
     false
    );
}

  void
ResultReceiver::slotEntryListComplete(int id, QStringList l)
{
  qDebug("ResultReceiver::slotEntryListComplete");
  emit(entryListComplete(id, l));
}

  void
ResultReceiver::slotEntryComplete(int id, Entry e)
{
  qDebug("ResultReceiver::slotEntryComplete");
  if (e.isNull())
  {
    qDebug("ResultReceiver::slotEntryComplete: entry is null");
  }
  emit(entryComplete(id, e));
}

