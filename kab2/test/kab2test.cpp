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

#include <kab2/KAddressBookServerInterface_stub.h>
#include <kab2/KAddressBookInterface_stub.h>
#include <kab2/Entry.h>
#include <kab2/Field.h>

#include "kab2test.h"

TestMainWindow::TestMainWindow()
  : KMainWindow(),
    DCOPObject("TestMainWindow"),
    abStub_(0)
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

  connectDCOPSignal
    (
     0, 0,
     "entryComplete(int,Entry)",
     "slotEntryComplete(int,Entry)",
     false
    );

  connectDCOPSignal
    (
     0, 0,
     "entryListComplete(int,QStringList)",
     "slotEntryListComplete(int,QStringList)",
     false
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

  delete abStub_;

  abStub_ =
    new KAddressBookInterface_stub("KAddressBookServer", name.utf8().data());

  abItem_ = new QListViewItem(addressBookListView_, abStub_->name());

  abStub_->entryList();
}

  void
TestMainWindow::slotEntryComplete(int, KAB::Entry e)
{
  qDebug("slotEntryComplete");

  if (e.isNull())
  {
    qDebug("Entry not found");
    return;
  }
  else
  {
    qDebug("Entry found. id == `%s'", e.id().ascii());
  }

  qDebug("Creating entry item");
  QListViewItem * entryItem = new QListViewItem(abItem_, e.id());

  KAB::FieldList fl(e.fieldList());

  qDebug("Creating field items");
  for (KAB::FieldList::ConstIterator fit(fl.begin()); fit != fl.end(); ++fit)
  {
    KAB::Field f(*fit);

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
  qDebug("slotEntryComplete: done");
}

  void
TestMainWindow::slotInsertComplete(int, QString)
{
  qDebug("slotInsertComplete");
}

  void
TestMainWindow::slotRemoveComplete(int, bool)
{
  qDebug("slotRemoveComplete");
}

  void
TestMainWindow::slotReplaceComplete(int, bool)
{
  qDebug("slotReplaceComplete");
}

  void
TestMainWindow::slotContainsComplete(int, bool)
{
  qDebug("slotContainsComplete");
}

  void
TestMainWindow::slotEntryListComplete(int, QStringList l)
{
  qDebug("slotEntryListComplete");

  qDebug("There are %d entries", l.count());

  entryList_ = l;

  QTimer::singleShot(0, this, SLOT(slotReadEntryList()));
}

  void
TestMainWindow::slotReadEntryList()
{
  QStringList::ConstIterator it(entryList_.begin());

  for (; it != entryList_.end(); ++it)
  {
    qDebug("Asking for entry `%s'", (*it).ascii());
    abStub_->entry(*it);
    qDebug("Asked for entry `%s'", (*it).ascii());
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

