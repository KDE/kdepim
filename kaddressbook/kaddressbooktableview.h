#ifndef KADDRESSBOOKTABLEVIEW_H
#define KADDRESSBOOKTABLEVIEW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

#include <qwidget.h>
#include <qlistview.h>
#include <qstring.h>
#include <qdialog.h>
#include <qpixmap.h>
#include <qtabdialog.h>
#include <qfileinfo.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qtooltip.h>

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
    KAddressBookTableView(KABC::AddressBook *doc, QWidget *parent, 
                          const char *name = 0L );
    virtual ~KAddressBookTableView();
  
    virtual void refresh(QString uid = QString::null);
    virtual QStringList selectedUids();
    virtual void setSelected(QString uid = QString::null, bool selected = false);
    virtual void readConfig(KConfig *config);
    virtual void writeConfig(KConfig *config);
    virtual QString type() const { return "Table"; }
    virtual void incrementalSearch(const QString &value, KABC::Field *field);
  
  public slots:
    virtual void reconstructListView();

  protected slots:
    /** Called whenever the user selects an addressee in the list view.
    */
    void addresseeSelected();
  
    /** Called whenever the user executes an addressee. In terms of the
    * list view, this is probably a double click
    */
    void addresseeExecuted(QListViewItem*);
 
  private:
    KABC::AddressBook *mDocument;
    QVBoxLayout *mainLayout;
    ContactListView *mListView;
};

#endif
