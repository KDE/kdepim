/*                                                                      
    This file is part of KAddressBook.                                  
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>                   
                                                                        
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or   
    (at your option) any later version.                                 
                                                                        
    This program is distributed in the hope that it will be useful,     
    but WITHOUT ANY WARRANTY; without even the implied warranty of      
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the        
    GNU General Public License for more details.                        
                                                                        
    You should have received a copy of the GNU General Public License   
    along with this program; if not, write to the Free Software         
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#include "kaddressbookiconview.h"

#include <qlayout.h>
#include <qiconview.h>
#include <qstringlist.h>

#include <kconfig.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kabc/addressbook.h>
#include <kabc/addressee.h>

#include "kabprefs.h"

////////////////////////////////
// AddresseeIconView (internal class)
AddresseeIconView::AddresseeIconView(QWidget *parent, const char *name)
  : KIconView(parent, name)
{
  setSelectionMode(QIconView::Extended);
  setResizeMode(QIconView::Adjust);
  setWordWrapIconText(true);
  setMaxItemTextLength(8);
  setItemsMovable(false);
  setSorting(true, true);
  
  connect(this, SIGNAL(dropped(QDropEvent*, 
                       const QValueList<QIconDragItem>&)),
          this, SLOT(itemDropped(QDropEvent*, 
                                 const QValueList<QIconDragItem>&)));
}

AddresseeIconView::~AddresseeIconView()
{
}

void AddresseeIconView::itemDropped(QDropEvent *e, 
                                    const QValueList<QIconDragItem> &)
{
  emit addresseeDropped(e);
}

QDragObject *AddresseeIconView::dragObject()
{
  emit startAddresseeDrag();
  
  // We never want IconView to start the drag
  return 0;
}
////////////////////////////////
// AddresseeIconViewItem  (internal class)
class AddresseeIconViewItem : public KIconViewItem
{
  public:
    AddresseeIconViewItem(const KABC::Field::List &fields,
                          KABC::AddressBook *doc, const KABC::Addressee &a, 
                          QIconView *parent)
      : KIconViewItem(parent), mFields( fields ), mDocument(doc), mAddressee(a)
      {
          if ( mFields.isEmpty() ) {
            mFields = KABC::Field::defaultFields();
          }
          refresh();
      }
      
    const KABC::Addressee &addressee() const { return mAddressee; }
    
    void refresh()
    {
        // Update our addressee, since it may have changed elsewhere
        mAddressee = mDocument->findByUid(mAddressee.uid());
        
        if (!mAddressee.isEmpty())
        {
          QString text = "";
          
          // Try all the selected fields until we find one with text.
          // This will limit the number of unlabeled icons in the view
          KABC::Field::List::Iterator iter;
          for (iter = mFields.begin(); iter != mFields.end() && text.isEmpty();
               ++iter)
          {
            text = (*iter)->value( mAddressee );
          }
       
          setText(text);
        }
    }
    
  private:
    KABC::Field::List mFields;
    KABC::AddressBook *mDocument;
    KABC::Addressee mAddressee;
};

///////////////////////////////
// KAddressBookView

KAddressBookIconView::KAddressBookIconView(KABC::AddressBook *doc, 
                                           QWidget *parent,
                                           const char *name)
    : KAddressBookView(doc, parent, name)
{
    mDocument = doc;
    
    // Init the GUI
    QVBoxLayout *layout = new QVBoxLayout(viewWidget());
    
    mIconView = new AddresseeIconView(viewWidget(), "mIconView");
    layout->addWidget(mIconView);
    
    // Connect up the signals
    connect(mIconView, SIGNAL(executed(QIconViewItem *)),
            this, SLOT(addresseeExecuted(QIconViewItem *)));
    connect(mIconView, SIGNAL(selectionChanged()),
            this, SLOT(addresseeSelected()));
    connect(mIconView, SIGNAL(addresseeDropped(QDropEvent*)),
            this, SIGNAL(dropped(QDropEvent*)));
    connect(mIconView, SIGNAL(startAddresseeDrag()),
            this, SIGNAL(startDrag()));
}

KAddressBookIconView::~KAddressBookIconView()
{
}

void KAddressBookIconView::readConfig(KConfig *config)
{
  KAddressBookView::readConfig(config);
  
  disconnect(mIconView, SIGNAL(executed(QIconViewItem *)),
             this, SLOT(addresseeExecuted(QIconViewItem *)));
             
  if (KABPrefs::instance()->mHonorSingleClick)
    connect(mIconView, SIGNAL(executed(QIconViewItem *)),
            this, SLOT(addresseeExecuted(QIconViewItem *)));
  else
    connect(mIconView, SIGNAL(doubleClicked(QIconViewItem *)),
            this, SLOT(addresseeExecuted(QIconViewItem *)));
}

QStringList KAddressBookIconView::selectedUids()
{
    QStringList uidList;
    QIconViewItem *item;
    AddresseeIconViewItem *aItem;
    
    for (item = mIconView->firstItem(); item; item = item->nextItem())
    {
        if (item->isSelected())
        {
            aItem = dynamic_cast<AddresseeIconViewItem*>(item);
            if (aItem)
                uidList << aItem->addressee().uid();
        }
    }
    
    return uidList;
}
    
void KAddressBookIconView::refresh(QString uid)
{
    QIconViewItem *item;
    AddresseeIconViewItem *aItem;
    
    if (uid == QString::null)
    {
        // Rebuild the view
        mIconView->clear();
        
        QPixmap icon(KGlobal::iconLoader()->loadIcon("vcard",
                                                     KIcon::Desktop));
        KABC::Addressee::List addresseeList = addressees();
        KABC::Addressee::List::Iterator iter;
        for (iter = addresseeList.begin(); iter != addresseeList.end(); ++iter)
        {
            aItem = new AddresseeIconViewItem(fields(), mDocument, *iter,
                                              mIconView);
            aItem->setPixmap(icon);
        }
        
        // by default nothing is selected
        emit selected(QString::null);
        
    }
    else
    {
        // Try to find the one to refresh
        bool found = false;
        for (item = mIconView->firstItem(); item && !found; 
             item = item->nextItem())
        {
            aItem = dynamic_cast<AddresseeIconViewItem*>(item);
            if ((aItem) && (aItem->addressee().uid() == uid))
            {
                aItem->refresh();
                found = true;
            }    
        }
    }
}

void KAddressBookIconView::setSelected(QString uid, bool selected)
{
    QIconViewItem *item;
    AddresseeIconViewItem *aItem;
    
    if (uid == QString::null)
    {
        mIconView->selectAll(selected);
    }
    else
    {
        bool found = false;
        for (item = mIconView->firstItem(); item && !found; 
             item = item->nextItem())
         {
             aItem = dynamic_cast<AddresseeIconViewItem*>(item);
             
             if ((aItem) && (aItem->addressee().uid() == uid))
             {
                 mIconView->setSelected(aItem, selected);
                 found = true;
             }
         }
    }
}
   
void KAddressBookIconView::addresseeExecuted(QIconViewItem *item)
{
    AddresseeIconViewItem *aItem = dynamic_cast<AddresseeIconViewItem*>(item);
    
    if (aItem)
        emit executed(aItem->addressee().uid());
}

void KAddressBookIconView::addresseeSelected()
{
    QIconViewItem *item;
    AddresseeIconViewItem *aItem;
    
    bool found = false;
    for (item = mIconView->firstItem(); item && !found; 
         item = item->nextItem())
    {
        if (item->isSelected())
        {
            aItem = dynamic_cast<AddresseeIconViewItem*>(item);
            emit selected(aItem->addressee().uid()); 
            found = true;
        }
    }
    
    if (!found)
        emit selected(QString::null);
}

void KAddressBookIconView::incrementalSearch(const QString &value, 
                                             KABC::Field *field)
{
  // Only supports searching on the first field, since that is all that
  // is visible
  if ( field->equals( fields().first() ) )
  {
    QIconViewItem *item = mIconView->findItem(value);
    
    if (item)
    {
      mIconView->setSelected(item, true, false);
      mIconView->ensureItemVisible(item);
    }
  }
}

#include "kaddressbookiconview.moc"
