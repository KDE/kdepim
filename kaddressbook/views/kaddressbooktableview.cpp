// $Id$

#include <qlayout.h>
#include <qheader.h>
#include <qvbox.h>
#include <qlistbox.h>
#include <qwidget.h>
#include <qfile.h>
#include <qimage.h>
#include <qcombobox.h>
#include <qapplication.h>
#include <qdragobject.h>
#include <qevent.h>
#include <qurl.h>
#include <qpixmap.h>

#include <kabc/addressbook.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kcolorbutton.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kurlrequester.h>

#include "configuretableviewdialog.h"
#include "contactlistview.h"
#include "core.h"
#include "kabprefs.h"
#include "undocmds.h"

#include "kaddressbooktableview.h"

class TableViewFactory : public ViewFactory
{
  public:
    KAddressBookView *view( KAB::Core *core, QWidget *parent, const char *name )
    {
      return new KAddressBookTableView( core, parent, name );
    }

    QString type() const { return I18N_NOOP("Table"); }

    QString description() const { return i18n( "A listing of contacts in a table. Each cell of "
                                  "the table holds a field of the contact." ); }

    ViewConfigureWidget *configureWidget( KABC::AddressBook *ab, QWidget *parent,
                                          const char *name = 0 )
    {
      return new ConfigureTableViewWidget( ab, parent, name );
    }
};

extern "C" {
  void *init_libkaddrbk_tableview()
  {
    return ( new TableViewFactory );
  }
}

KAddressBookTableView::KAddressBookTableView( KAB::Core *core,
                                              QWidget *parent, const char *name )
  : KAddressBookView( core, parent, name )
{
  mainLayout = new QVBoxLayout( viewWidget(), 2 );

  // The list view will be created when the config is read.
  mListView = 0;
}

KAddressBookTableView::~KAddressBookTableView()
{
}

void KAddressBookTableView::reconstructListView()
{
    if (mListView)
    {
        disconnect(mListView, SIGNAL(selectionChanged()),
                   this, SLOT(addresseeSelected()));
        disconnect(mListView, SIGNAL(executed(QListViewItem*)),
                   this, SLOT(addresseeExecuted(QListViewItem*)));
        disconnect(mListView, SIGNAL(doubleClicked(QListViewItem*)),
                   this, SLOT(addresseeExecuted(QListViewItem*)));
        disconnect(mListView, SIGNAL(startAddresseeDrag()), this,
                   SIGNAL(startDrag()));
        disconnect(mListView, SIGNAL(addresseeDropped(QDropEvent*)), this,
                   SIGNAL(dropped(QDropEvent*)));
        delete mListView;
    }

  mListView = new ContactListView( this, core()->addressBook(), viewWidget() );
  mListView->setFullWidth( true );

  // Add the columns
  KABC::Field::List fieldList = fields();
  KABC::Field::List::ConstIterator it;

  int c = 0;
  for( it = fieldList.begin(); it != fieldList.end(); ++it ) {
      mListView->addColumn( (*it)->label() );
      mListView->setColumnWidthMode(c++, QListView::Manual);
  }

  connect(mListView, SIGNAL(selectionChanged()),
          this, SLOT(addresseeSelected()));
  connect(mListView, SIGNAL(startAddresseeDrag()), this,
          SIGNAL(startDrag()));
  connect(mListView, SIGNAL(addresseeDropped(QDropEvent*)), this,
          SIGNAL(dropped(QDropEvent*)));
  connect( mListView, SIGNAL( contextMenu( KListView*, QListViewItem*, const QPoint& ) ),
           this, SLOT( rmbClicked( KListView*, QListViewItem*, const QPoint& ) ) );
  connect( mListView->header(), SIGNAL( clicked(int) ),
           SIGNAL( sortFieldChanged() ) );

  if (KABPrefs::instance()->mHonorSingleClick)
    connect(mListView, SIGNAL(executed(QListViewItem*)),
          this, SLOT(addresseeExecuted(QListViewItem*)));
  else
    connect(mListView, SIGNAL(doubleClicked(QListViewItem*)),
          this, SLOT(addresseeExecuted(QListViewItem*)));

  refresh();

  mListView->setSorting( 0, true );
  mainLayout->addWidget( mListView );
  mainLayout->activate();
  mListView->show();
}

KABC::Field *KAddressBookTableView::sortField() const
{
  // we have hardcoded sorting, so we have to return a hardcoded field :(
  return ( mListView->sortColumn() == -1 ? fields()[ 0 ] : fields()[ mListView->sortColumn() ] );
}

void KAddressBookTableView::writeConfig(KConfig *config)
{
  KAddressBookView::writeConfig(config);

  mListView->saveLayout(config, config->group());
}

void KAddressBookTableView::readConfig(KConfig *config)
{
  KAddressBookView::readConfig( config );

  // The config could have changed the fields, so we need to reconstruct
  // the listview.
  reconstructListView();

  // Set the list view options
  mListView->setAlternateBackgroundEnabled(config->readBoolEntry("ABackground",
                                                                 true));
  mListView->setSingleLineEnabled(config->readBoolEntry("SingleLine", false));
  mListView->setToolTipsEnabled(config->readBoolEntry("ToolTips", true));

  if (config->readBoolEntry("Background", false))
    mListView->setBackgroundPixmap(config->readPathEntry("BackgroundName"));

  // Restore the layout of the listview
  mListView->restoreLayout(config, config->group());
}

void KAddressBookTableView::refresh(QString uid)
{
  // For now just repopulate. In reality this method should
  // check the value of uid, and if valid iterate through
  // the listview to find the entry, then tell it to refresh.

  if (uid.isNull()) {
    // Clear the list view
    QString currentUID, nextUID;
    ContactListViewItem *currentItem = dynamic_cast<ContactListViewItem*>( mListView->currentItem() );
    if ( currentItem ) {
      ContactListViewItem *nextItem = dynamic_cast<ContactListViewItem*>( currentItem->itemBelow() );
      if ( nextItem )
        nextUID = nextItem->addressee().uid();
      currentUID = currentItem->addressee().uid();
    }

    mListView->clear();

    currentItem = 0;
    KABC::Addressee::List addresseeList = addressees();
    KABC::Addressee::List::Iterator it;
    for (it = addresseeList.begin(); it != addresseeList.end(); ++it ) {
      ContactListViewItem *item = new ContactListViewItem(*it, mListView, 
                                        core()->addressBook(), fields());
      if ( (*it).uid() == currentUID )
        currentItem = item;
      else if ( (*it).uid() == nextUID && !currentItem )
        currentItem = item;
    }

    // Sometimes the background pixmap gets messed up when we add lots
    // of items.
    mListView->repaint();

    if ( currentItem ) {
      mListView->setCurrentItem( currentItem );
      mListView->ensureItemVisible( currentItem );
    }
  } else {
    // Only need to update on entry. Iterate through and try to find it
    ContactListViewItem *ceItem;
    QListViewItemIterator it( mListView );
    while ( it.current() ) {
      ceItem = dynamic_cast<ContactListViewItem*>( it.current() );
      if ( ceItem && ceItem->addressee().uid() == uid ) {
        ceItem->refresh();
        return;
      }
      ++it;
    }

    refresh( QString::null );
  }
}

QStringList KAddressBookTableView::selectedUids()
{
    QStringList uidList;
    QListViewItem *item;
    ContactListViewItem *ceItem;

    for(item = mListView->firstChild(); item; item = item->itemBelow())
    {
        if (mListView->isSelected( item ))
        {
            ceItem = dynamic_cast<ContactListViewItem*>(item);
            if (ceItem != 0L)
                uidList << ceItem->addressee().uid();
        }
    }

    return uidList;
}

void KAddressBookTableView::setSelected(QString uid, bool selected)
{
    QListViewItem *item;
    ContactListViewItem *ceItem;

    if (uid.isNull())
    {
        mListView->selectAll(selected);
    }
    else
    {
        for(item = mListView->firstChild(); item; item = item->itemBelow())
        {
            ceItem = dynamic_cast<ContactListViewItem*>(item);
            if ((ceItem != 0L) && (ceItem->addressee().uid() == uid))
            {
                mListView->setSelected(item, selected);

                if (selected)
                    mListView->ensureItemVisible(item);
            }
        }
    }
}

void KAddressBookTableView::addresseeSelected()
{
    // We need to try to find the first selected item. This might not be the
    // last selected item, but when QListView is in multiselection mode,
    // there is no way to figure out which one was
    // selected last.
    QListViewItem *item;
    bool found =false;
    for (item = mListView->firstChild(); item && !found;
         item = item->nextSibling())
    {
        if (item->isSelected())
        {
            found = true;
            ContactListViewItem *ceItem
                 = dynamic_cast<ContactListViewItem*>(item);
            if ( ceItem ) emit selected(ceItem->addressee().uid());
        }
    }

    if (!found)
        emit selected(QString::null);
}

void KAddressBookTableView::addresseeExecuted(QListViewItem *item)
{
    if (item)
    {
        ContactListViewItem *ceItem
               = dynamic_cast<ContactListViewItem*>(item);

       if (ceItem)
       {
           emit executed(ceItem->addressee().uid());
       }
    }
    else
    {
        emit executed(QString::null);
    }
}

void KAddressBookTableView::rmbClicked( KListView*, QListViewItem*, const QPoint &point )
{
  popup( point );
}

#include "kaddressbooktableview.moc"
