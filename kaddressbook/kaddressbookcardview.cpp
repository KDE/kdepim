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

#include "kaddressbookcardview.h"

#include <qevent.h>
#include <qdragobject.h>
#include <qlayout.h>
#include <qiconview.h>
#include <qstringlist.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kabc/addressbook.h>
#include <kabc/addressee.h>

#include "kabprefs.h"

////////////////////////////////
// AddresseeCardViewItem  (internal class)
class AddresseeCardViewItem : public CardViewItem
{
  public:
    AddresseeCardViewItem(const KABC::Field::List &fields,
                          bool showEmptyFields,
                          KABC::AddressBook *doc, const KABC::Addressee &a, 
                          CardView *parent)
      : CardViewItem(parent, a.formattedName()), 
        mFields( fields ), mShowEmptyFields(showEmptyFields),
        mDocument(doc), mAddressee(a)
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
          clearFields();
          
          // We might want to make this the first field. hmm... -mpilone
          setCaption( mAddressee.realName() );
          
          // Try all the selected fields until we find one with text.
          // This will limit the number of unlabeled icons in the view
          KABC::Field::List::Iterator iter;
          for (iter = mFields.begin(); iter != mFields.end(); ++iter)
          {
            if (mShowEmptyFields || !(*iter)->value( mAddressee ).isEmpty())
              insertField((*iter)->label(), (*iter)->value( mAddressee ));
          }
        }
    }
    
  private:
    KABC::Field::List mFields;
    bool mShowEmptyFields;
    KABC::AddressBook *mDocument;
    KABC::Addressee mAddressee;
};

///////////////////////////////
// AddresseeCardView

AddresseeCardView::AddresseeCardView(QWidget *parent, const char *name)
  : CardView(parent, name)
{
  setAcceptDrops(true);
}
  
AddresseeCardView::~AddresseeCardView()
{
}

void AddresseeCardView::dragEnterEvent(QDragEnterEvent *e)
{
  if (QTextDrag::canDecode(e))
    e->accept();
}

void AddresseeCardView::dropEvent(QDropEvent *e)
{
  emit addresseeDropped(e);
}

void AddresseeCardView::startDrag()
{
  emit startAddresseeDrag();
}

    
///////////////////////////////
// KAddressBookCardView

KAddressBookCardView::KAddressBookCardView(KABC::AddressBook *doc, 
                                           QWidget *parent,
                                           const char *name)
    : KAddressBookView(doc, parent, name)
{
    mShowEmptyFields = true;
    
    // Init the GUI
    QVBoxLayout *layout = new QVBoxLayout(viewWidget());
    
    mCardView = new AddresseeCardView(viewWidget(), "mCardView");
    mCardView->setSelectionMode(CardView::Extended);
    layout->addWidget(mCardView);
    
    // Connect up the signals
    connect(mCardView, SIGNAL(executed(CardViewItem *)),
            this, SLOT(addresseeExecuted(CardViewItem *)));
    connect(mCardView, SIGNAL(selectionChanged()),
            this, SLOT(addresseeSelected()));
    connect(mCardView, SIGNAL(addresseeDropped(QDropEvent*)),
            this, SIGNAL(dropped(QDropEvent*)));
    connect(mCardView, SIGNAL(startAddresseeDrag()),
            this, SIGNAL(startDrag()));
}

KAddressBookCardView::~KAddressBookCardView()
{
}
    
void KAddressBookCardView::readConfig(KConfig *config)
{
  KAddressBookView::readConfig(config);
  
  mCardView->setDrawCardBorder(config->readBoolEntry("DrawBorder", true));
  mCardView->setDrawColSeparators(config->readBoolEntry("DrawSeparators", 
                                                        true));
  mCardView->setDrawFieldLabels(config->readBoolEntry("DrawFieldLabels",true));
  mShowEmptyFields = config->readBoolEntry("ShowEmptyFields", true);
  
  disconnect(mCardView, SIGNAL(executed(CardViewItem *)),
            this, SLOT(addresseeExecuted(CardViewItem *)));
            
  if (KABPrefs::instance()->mHonorSingleClick)
    connect(mCardView, SIGNAL(executed(CardViewItem *)),
            this, SLOT(addresseeExecuted(CardViewItem *)));
  else
    connect(mCardView, SIGNAL(doubleClicked(CardViewItem *)),
            this, SLOT(addresseeExecuted(CardViewItem *)));
}
  
QStringList KAddressBookCardView::selectedUids()
{
    QStringList uidList;
    CardViewItem *item;
    AddresseeCardViewItem *aItem;
    
    for (item = mCardView->firstItem(); item; item = item->nextItem())
    {
        if (item->isSelected())
        {
            aItem = dynamic_cast<AddresseeCardViewItem*>(item);
            if (aItem)
                uidList << aItem->addressee().uid();
        }
    }

    return uidList;
}
    
void KAddressBookCardView::refresh(QString uid)
{
    CardViewItem *item;
    AddresseeCardViewItem *aItem;
    
    if (uid == QString::null)
    {
        // Rebuild the view
        mCardView->viewport()->setUpdatesEnabled( false );
        mCardView->clear();
        
        KABC::Addressee::List addresseeList = addressees();
        KABC::Addressee::List::Iterator iter;
        for (iter = addresseeList.begin(); iter != addresseeList.end(); ++iter)
        {
            aItem = new AddresseeCardViewItem(fields(), mShowEmptyFields,
                                              addressBook(), *iter, mCardView);
        }
        mCardView->viewport()->setUpdatesEnabled( true );
        mCardView->viewport()->update();
        
        // by default nothing is selected
        emit selected(QString::null);
    }
    else
    {
        // Try to find the one to refresh
        bool found = false;
        for (item = mCardView->firstItem(); item && !found; 
             item = item->nextItem())
        {
            aItem = dynamic_cast<AddresseeCardViewItem*>(item);
            if ((aItem) && (aItem->addressee().uid() == uid))
            {
                aItem->refresh();
                found = true;
            }    
        }
    }
}

void KAddressBookCardView::setSelected(QString uid, bool selected)
{
    CardViewItem *item;
    AddresseeCardViewItem *aItem;
    
    if (uid == QString::null)
    {
        mCardView->selectAll(selected);
    }
    else
    {
        bool found = false;
        for (item = mCardView->firstItem(); item && !found; 
             item = item->nextItem())
         {
             aItem = dynamic_cast<AddresseeCardViewItem*>(item);
             
             if ((aItem) && (aItem->addressee().uid() == uid))
             {
                 mCardView->setSelected(aItem, selected);
                 found = true;
             }
         }
    }
}
   
void KAddressBookCardView::addresseeExecuted(CardViewItem *item)
{
    AddresseeCardViewItem *aItem = dynamic_cast<AddresseeCardViewItem*>(item);
    
    if (aItem)
    {
      emit executed(aItem->addressee().uid());
    }     
}

void KAddressBookCardView::addresseeSelected()
{
    CardViewItem *item;
    AddresseeCardViewItem *aItem;
    
    bool found = false;
    for (item = mCardView->firstItem(); item && !found; 
         item = item->nextItem())
    {
        if (item->isSelected())
        {
            aItem = dynamic_cast<AddresseeCardViewItem*>(item);
            emit selected(aItem->addressee().uid()); 
            found = true;
        }
    }
    
    if (!found)
        emit selected(QString::null);
        
}

void KAddressBookCardView::incrementalSearch(const QString &value, 
                                             KABC::Field *field)
{
  CardViewItem *item = mCardView->findItem(value, field->label());
    
  if (item)
  {
    mCardView->setSelected(item, true);
    mCardView->ensureItemVisible(item);
  }
}


#include "kaddressbookcardview.moc"
