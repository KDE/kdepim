#include <qstring.h>
#include <qcstring.h>
#include <qlistview.h>
#include <qdatetime.h>

#include <dcopclient.h>
#include <kapp.h>
#include <kcmdlineargs.h>

#include "KAddressBookInterface.h"
#include "KAddressBookInterface_stub.h"
#include "Entry.h"
#include "Field.h"

  int
main(int argc, char ** argv)
{
	KCmdLineArgs::init(argc, argv, "testing kab", "testing kab", "testing kab");

  KApplication * app = new KApplication;

	DCOPClient * client = new DCOPClient;

	if (!client->attach())
    qFatal("Can't attach to DCOP");

  QStringList addressBookList;

  {
    QByteArray args, retVal;
    QCString retType;

    bool ok =
      client->call
      (
       "KAddressBookServer",
       "KAddressBookServer",
       "list()",
       args,
       retType,
       retVal
      );

    if (!ok)
      qFatal("Can't talk to KAddressBook server");

    QDataStream str(retVal, IO_ReadOnly);

    str >> addressBookList;
  }

  QStringList::ConstIterator it(addressBookList.begin());

  QListView * lv = new QListView;

  lv->setRootIsDecorated(true);

  lv->addColumn("Name");
  lv->addColumn("Value");
  lv->addColumn("Type");
  lv->addColumn("Subtype");

  QTime begin;
  begin.start();
  int count;

  for (; it != addressBookList.end(); ++it)
  {
    KAddressBook_stub * ab =
      new KAddressBook_stub("KAddressBookServer", (*it).latin1());

    QListViewItem * abItem = new QListViewItem(lv, ab->name());

    QStringList el(ab->entryList());

    count = el.count();

    for (QStringList::ConstIterator eit(el.begin()); eit != el.end(); ++eit)
    {
      QString entryID = *eit;

      Entry e(ab->entry(entryID));

      if (e.isNull())
      {
        qDebug("Entry not found");
        continue;
      }

      QListViewItem * entryItem = new QListViewItem(abItem, e.name());

      FieldList fl(e.fieldList());

      for (FieldList::ConstIterator fit(fl.begin()); fit != fl.end(); ++fit)
      {
        Field f(*fit);

        QListViewItem * fieldItem = new QListViewItem(entryItem, f.name());

        QByteArray val(f.value());

        if
          (
           (f.type().isEmpty() || (f.type() == "text")) &&
           (f.subType().isEmpty() || (f.subType() == "unicode"))
          )
        {
          QString s;
          QDataStream str(val, IO_ReadOnly);
          str >> s;
          fieldItem->setText(1, s);
          fieldItem->setText(2, "text");
          fieldItem->setText(3, "unicode");
        }
        else
        {
          fieldItem->setText(1, "Can't display");
          fieldItem->setText(2, f.type());
          fieldItem->setText(3, f.subType());
        }
      }
    }
  }
  qDebug("Time to read %d entities: %d ms", count, begin.elapsed());

  app->setMainWidget(lv);

  lv->show();

  return app->exec();
}

