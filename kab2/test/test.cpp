#include <qstring.h>
#include <qcstring.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qtimer.h>
#include <qlayout.h>

#include <dcopclient.h>
#include <kapp.h>
#include <kcmdlineargs.h>
#include <kmainwindow.h>

#include "KAddressBookServerInterface.h"
#include "KAddressBookServerInterface_stub.h"
#include "KAddressBookInterface.h"
#include "KAddressBookInterface_stub.h"
#include "Entry.h"
#include "Field.h"

#include "test.h"

TestMainWindow::TestMainWindow()
  : KMainWindow()
{
  QWidget * w = new QWidget(this);

  setCentralWidget(w);

  addressBookListBox_ = new QListBox(w);
  addressBookListView_ = new QListView(w);

  addressBookListView_->setRootIsDecorated(true);

  addressBookListView_->addColumn("Name");
  addressBookListView_->addColumn("Value");
  addressBookListView_->addColumn("Type");
  addressBookListView_->addColumn("Subtype");

  QVBoxLayout * l = new QVBoxLayout(w);

  l->addWidget(addressBookListBox_);
  l->addWidget(addressBookListView_);

  connect
    (
     addressBookListBox_,
     SIGNAL(highlighted(const QString &)),
     this,
     SLOT(slotSetAddressBook(const QString &))
    );

  QTimer::singleShot(0, this, SLOT(slotLoad()));
}

  void
TestMainWindow::slotLoad()
{
  DCOPClient * client = kapp->dcopClient();

  if (!client->attach())
  {
    qWarning("Can't attach to DCOP");
    return;
  }

  KAddressBookServerInterface_stub server
    ("KAddressBookServer", "KAddressBookServer");

  QStringList addressBookList = server.list();

  addressBookListBox_->insertStringList(addressBookList);
}

  void
TestMainWindow::slotSetAddressBook(const QString & name)
{
  addressBookListView_->clear();

  KAddressBookInterface_stub ab("KAddressBookServer", name.utf8().data());

  QListViewItem * abItem =
    new QListViewItem(addressBookListView_, ab.name());

  QStringList el(ab.entryList());

  for (QStringList::ConstIterator eit(el.begin()); eit != el.end(); ++eit)
  {
    QString entryID = *eit;

    Entry e(ab.entry(entryID));

    if (e.isNull())
    {
      qDebug("Entry not found");
      continue;
    }

    QListViewItem * entryItem = new QListViewItem(abItem, e.id());

    FieldList fl(e.fieldList());

    for (FieldList::ConstIterator fit(fl.begin()); fit != fl.end(); ++fit)
    {
      Field f(*fit);

      QListViewItem * fieldItem = new QListViewItem(entryItem, f.name());

      QByteArray val(f.value());

      if
        (
         (f.type().isEmpty() || (f.type() == "text")) &&
         (f.subType().isEmpty() || (f.subType() == "UCS-2"))
        )
        {
          QString s;
          QDataStream str(val, IO_ReadOnly);
          str >> s;
          fieldItem->setText(1, s);
          fieldItem->setText(2, "text");
          fieldItem->setText(3, "UCS-2");
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

  int
main(int argc, char ** argv)
{
	KCmdLineArgs::init(argc, argv, "testing kab", "testing kab", "testing kab");

  KApplication * app = new KApplication;

  TestMainWindow * w = new TestMainWindow;
  app->setMainWidget(w);

  w->show();

  return app->exec();
}

#include "test.moc"
