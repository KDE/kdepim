#ifndef TEST_H
#define TEST_H

#include <qstring.h>

#include <dcopobject.h>

#include <kmainwindow.h>

#include <kab2/Entry.h>

class QListBox;
class QListView;
class QListViewItem;

class KAddressBookInterface_stub;

class TestMainWindow : public KMainWindow, virtual public DCOPObject
{
  Q_OBJECT
  K_DCOP

  public:

    TestMainWindow();

  protected slots:

    void slotSetAddressBook(const QString & name);
    void slotLoad();

  k_dcop:

    void slotEntryComplete(int id, KAB::Entry);
    void slotEntryListComplete(int id, QStringList);

    void slotInsertComplete(int, QString);
    void slotRemoveComplete(int, bool);
    void slotReplaceComplete(int, bool);
    void slotContainsComplete(int, bool);
    void slotReadEntryList();

  private:

    QListBox * addressBookListBox_;
    QListView * addressBookListView_;

    QListViewItem * abItem_;

    KAddressBookInterface_stub * abStub_;

    QStringList entryList_;
};

#endif

