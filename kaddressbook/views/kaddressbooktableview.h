#ifndef KADDRESSBOOKTABLEVIEW_H
#define KADDRESSBOOKTABLEVIEW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qwidget.h>
#include <qlistview.h>
#include <qstring.h>
#include <qdialog.h>
#include <qtabdialog.h>
#include <qstringlist.h>
#include <qvaluelist.h>

#include "undo.h"
#include "kaddressbookview.h"

class QListViewItem;
class QListBox;
class QVBoxLayout;
class KConfig;

class ContactListViewItem;
class ContactListView;

namespace KABC { class AddressBook; }

/**
 * This class is the table view for kaddressbook. This view is a KListView
 * with multiple columns for the selected fields.
 *
 * @short Table View
 * @author Don Sanders <dsanders@kde.org>
 * @version 0.1
 */
class KAddressBookTableView : public KAddressBookView
{
friend class ContactListView;

  Q_OBJECT

  public:
    KAddressBookTableView( KABC::AddressBook *ab, QWidget *parent,
                           const char *name = 0 );
    virtual ~KAddressBookTableView();

    virtual void refresh(QString uid = QString::null);
    virtual QStringList selectedUids();
    virtual void setSelected(QString uid = QString::null, bool selected = false);
    virtual void readConfig(KConfig *config);
    virtual void writeConfig(KConfig *config);
    virtual QString type() const { return "Table"; }

  public slots:
    virtual void reconstructListView();

  protected slots:
    /**
      Called whenever the user selects an addressee in the list view.
    */
    void addresseeSelected();

    /**
      Called whenever the user executes an addressee. In terms of the
      list view, this is probably a double click
    */
    void addresseeExecuted(QListViewItem*);

    /**
      RBM menu called.
     */
    void rmbClicked( KListView*, QListViewItem*, const QPoint& );

  private:
    QVBoxLayout *mainLayout;
    ContactListView *mListView;
};

#endif
