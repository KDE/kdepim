#ifndef TEST_H
#define TEST_H

#include <qstring.h>

#include <kmainwindow.h>

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

  private:

    QListBox * addressBookListBox_;
    QListView * addressBookListView_;
};

#endif

