#ifndef TEST_H
#define TEST_H

#include <qstring.h>

#include <kmainwindow.h>

class ResultReceiver;
class QListBox;
class QListView;

class TestMainWindow : public KMainWindow
{
  Q_OBJECT

  public:

    TestMainWindow();

  protected slots:

    void slotSetAddressBook(const QString & name);
    void slotLoad();

    void slotEntryComplete(int, Entry);
    void slotInsertComplete(int, QString);
    void slotRemoveComplete(int, bool);
    void slotReplaceComplete(int, bool);
    void slotContainsComplete(int, bool);
    void slotEntryListComplete(int, QStringList);
    void slotReadEntryList();

  private:

    QListBox * addressBookListBox_;
    QListView * addressBookListView_;

    QListViewItem * abItem_;

    KAddressBookInterface_stub * abStub_;

    ResultReceiver * resultReceiver_;

    QStringList entryList_;
};

#endif

