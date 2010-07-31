/*
    ksubscription.cpp

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include "ksubscription.h"
#include "kaccount.h"

#include <tqlayout.h>
#include <tqtimer.h>
#include <tqlabel.h>
#include <tqpushbutton.h>
#include <tqheader.h>
#include <tqtoolbutton.h>

#include <kseparator.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>
#include <klineedit.h>


//=============================================================================

KGroupInfo::KGroupInfo(const TQString &name, const TQString &description,
    bool newGroup, bool subscribed,
    Status status, TQString path)
  : name(name), description(description),
    newGroup(newGroup), subscribed(subscribed),
    status(status), path(path)
{
}

//-----------------------------------------------------------------------------
bool KGroupInfo::operator== (const KGroupInfo &gi2)
{
  return (name == gi2.name);
}

//-----------------------------------------------------------------------------
bool KGroupInfo::operator< (const KGroupInfo &gi2)
{
  return (name < gi2.name);
}

//=============================================================================

GroupItem::GroupItem( TQListView *v, const KGroupInfo &gi, KSubscription* browser,
    bool isCheckItem )
  : TQCheckListItem( v, gi.name, isCheckItem ? CheckBox : CheckBoxController ),
    mInfo( gi ), mBrowser( browser ), mIsCheckItem( isCheckItem ),
    mIgnoreStateChange( false )
{
  if (listView()->columns() > 1)
    setDescription();
}

//-----------------------------------------------------------------------------
GroupItem::GroupItem( TQListViewItem *i, const KGroupInfo &gi, KSubscription* browser,
    bool isCheckItem )
  : TQCheckListItem( i, gi.name, isCheckItem ? CheckBox : CheckBoxController ),
    mInfo( gi ), mBrowser( browser ), mIsCheckItem( isCheckItem ),
    mIgnoreStateChange( false )
{
  if (listView()->columns() > 1)
    setDescription();
}

//-----------------------------------------------------------------------------
void GroupItem::setInfo( KGroupInfo info )
{
  mInfo = info;
  setText(0, mInfo.name);
  if (listView()->columns() > 1)
    setDescription();
}

//-----------------------------------------------------------------------------
void GroupItem::setDescription()
{
  setText(1, mInfo.description);
}

//-----------------------------------------------------------------------------
void GroupItem::setOn( bool on )
{
  if (mBrowser->isLoading())
  {
    // set this only if we're loading/creating items
    // otherwise changes are only permanent when the dialog is saved
    mInfo.subscribed = on;
  }
  if (isCheckItem())
    TQCheckListItem::setOn(on);
}

//------------------------------------------------------------------------------
void GroupItem::stateChange( bool on )
{
  // delegate to parent
  if ( !mIgnoreStateChange )
    mBrowser->changeItemState(this, on);
}

//------------------------------------------------------------------------------
void GroupItem::setVisible( bool b )
{
  if (b)
  {
    TQListViewItem::setVisible(b);
    setEnabled(true);
  }
  else
  {
    if (isCheckItem())
    {
      bool setInvisible = true;
      for (TQListViewItem * lvchild = firstChild(); lvchild != 0;
          lvchild = lvchild->nextSibling())
      {
        if (lvchild->isVisible()) // item has a visible child
          setInvisible = false;
      }
      if (setInvisible)
        TQListViewItem::setVisible(b);
      else
      {
        // leave it visible so that children remain visible
        setOpen(true);
        setEnabled(false);
      }
    }
    else
    {
      // non-checkable item
      TQPtrList<TQListViewItem> moveItems;

      for (TQListViewItem * lvchild = firstChild(); lvchild != 0;
          lvchild = lvchild->nextSibling())
      {
        if (static_cast<GroupItem*>(lvchild)->isCheckItem())
        {
          // remember the items
          moveItems.append(lvchild);
        }
      }
      TQPtrListIterator<TQListViewItem> it( moveItems );
      for ( ; it.current(); ++it)
      {
        // move the checkitem to top
        TQListViewItem* parent = it.current()->parent();
        if (parent) parent->takeItem(it.current());
        listView()->insertItem(it.current());
      }
      TQListViewItem::setVisible(false);
    }
  }
}

//-----------------------------------------------------------------------------
void GroupItem::paintCell( TQPainter * p, const TQColorGroup & cg,
    int column, int width, int align )
{
  if (mIsCheckItem)
    return TQCheckListItem::paintCell( p, cg, column, width, align );
  else
    return TQListViewItem::paintCell( p, cg, column, width, align );
}

//-----------------------------------------------------------------------------
void GroupItem::paintFocus( TQPainter * p, const TQColorGroup & cg,
    const TQRect & r )
{
  if (mIsCheckItem)
    TQCheckListItem::paintFocus(p, cg, r);
  else
    TQListViewItem::paintFocus(p, cg, r);
}

//-----------------------------------------------------------------------------
int GroupItem::width( const TQFontMetrics& fm, const TQListView* lv, int column) const
{
  if (mIsCheckItem)
    return TQCheckListItem::width(fm, lv, column);
  else
    return TQListViewItem::width(fm, lv, column);
}

//-----------------------------------------------------------------------------
void GroupItem::setup()
{
  if (mIsCheckItem)
    TQCheckListItem::setup();
  else
    TQListViewItem::setup();
}


//=============================================================================

KSubscription::KSubscription( TQWidget *parent, const TQString &caption,
    KAccount * acct, int buttons, const TQString &user1, bool descriptionColumn )
  : KDialogBase( parent, 0, true, caption, buttons | Help | Ok | Cancel, Ok,
      true, i18n("Reload &List"), user1 ),
    mAcct( acct )
{
  mLoading = true;
  setWFlags( getWFlags() | WDestructiveClose );

  // create Widgets
  page = new TQWidget(this);
  setMainWidget(page);

  TQLabel *comment = new TQLabel("<p>"+
          i18n("Manage which mail folders you want to see in your folder view") + "</p>", page);

  TQToolButton *clearButton = new TQToolButton( page );
  clearButton->setIconSet( KGlobal::iconLoader()->loadIconSet(
              KApplication::reverseLayout() ? "clear_left":"locationbar_erase", KIcon::Small, 0 ) );
  filterEdit = new KLineEdit(page);
  TQLabel *l = new TQLabel(filterEdit,i18n("S&earch:"), page);
  connect( clearButton, TQT_SIGNAL( clicked() ), filterEdit, TQT_SLOT( clear() ) );

  // checkboxes
  noTreeCB = new TQCheckBox(i18n("Disable &tree view"), page);
  noTreeCB->setChecked(false);
  subCB = new TQCheckBox(i18n("&Subscribed only"), page);
  subCB->setChecked(false);
  newCB = new TQCheckBox(i18n("&New only"), page);
  newCB->setChecked(false);


  KSeparator *sep = new KSeparator(KSeparator::HLine, page);

  // init the labels
  TQFont fnt = font();
  fnt.setBold(true);
  leftLabel = new TQLabel(i18n("Loading..."), page);
  rightLabel = new TQLabel(i18n("Current changes:"), page);
  leftLabel->setFont(fnt);
  rightLabel->setFont(fnt);

  // icons
  pmRight = BarIconSet("forward");
  pmLeft = BarIconSet("back");

  arrowBtn1 = new TQPushButton(page);
  arrowBtn1->setEnabled(false);
  arrowBtn2 = new TQPushButton(page);
  arrowBtn2->setEnabled(false);
  arrowBtn1->setIconSet(pmRight);
  arrowBtn2->setIconSet(pmRight);
  arrowBtn1->setFixedSize(35,30);
  arrowBtn2->setFixedSize(35,30);

  // the main listview
  groupView = new TQListView(page);
  groupView->setRootIsDecorated(true);
  groupView->addColumn(i18n("Name"));
  groupView->setAllColumnsShowFocus(true);
  if (descriptionColumn)
    mDescrColumn = groupView->addColumn(i18n("Description"));
  else
    groupView->header()->setStretchEnabled(true, 0);

  // layout
  TQGridLayout *topL = new TQGridLayout(page,4,1,0, KDialog::spacingHint());
  TQHBoxLayout *filterL = new TQHBoxLayout(KDialog::spacingHint());
  TQVBoxLayout *arrL = new TQVBoxLayout(KDialog::spacingHint());
  listL = new TQGridLayout(2, 3, KDialog::spacingHint());

  topL->addWidget(comment, 0,0);
  topL->addLayout(filterL, 1,0);
  topL->addWidget(sep,2,0);
  topL->addLayout(listL, 3,0);

  filterL->addWidget(clearButton);
  filterL->addWidget(l);
  filterL->addWidget(filterEdit, 1);
  filterL->addWidget(noTreeCB);
  filterL->addWidget(subCB);
  filterL->addWidget(newCB);

  listL->addWidget(leftLabel, 0,0);
  listL->addWidget(rightLabel, 0,2);
  listL->addWidget(groupView, 1,0);
  listL->addLayout(arrL, 1,1);
  listL->setRowStretch(1,1);
  listL->setColStretch(0,5);
  listL->setColStretch(2,2);

  arrL->addWidget(arrowBtn1, AlignCenter);
  arrL->addWidget(arrowBtn2, AlignCenter);

  // listviews
  subView = new TQListView(page);
  subView->addColumn(i18n("Subscribe To"));
  subView->header()->setStretchEnabled(true, 0);
  unsubView = new TQListView(page);
  unsubView->addColumn(i18n("Unsubscribe From"));
  unsubView->header()->setStretchEnabled(true, 0);

  TQVBoxLayout *protL = new TQVBoxLayout(3);
  listL->addLayout(protL, 1,2);
  protL->addWidget(subView);
  protL->addWidget(unsubView);

  // disable some widgets as long we're loading
  enableButton(User1, false);
  enableButton(User2, false);
  newCB->setEnabled(false);
  noTreeCB->setEnabled(false);
  subCB->setEnabled(false);

  filterEdit->setFocus();

   // items clicked
  connect(groupView, TQT_SIGNAL(clicked(TQListViewItem *)),
      this, TQT_SLOT(slotChangeButtonState(TQListViewItem*)));
  connect(subView, TQT_SIGNAL(clicked(TQListViewItem *)),
      this, TQT_SLOT(slotChangeButtonState(TQListViewItem*)));
  connect(unsubView, TQT_SIGNAL(clicked(TQListViewItem *)),
      this, TQT_SLOT(slotChangeButtonState(TQListViewItem*)));

  // connect buttons
  connect(arrowBtn1, TQT_SIGNAL(clicked()), TQT_SLOT(slotButton1()));
  connect(arrowBtn2, TQT_SIGNAL(clicked()), TQT_SLOT(slotButton2()));
  connect(this, TQT_SIGNAL(user1Clicked()), TQT_SLOT(slotLoadFolders()));

  // connect checkboxes
  connect(subCB, TQT_SIGNAL(clicked()), TQT_SLOT(slotCBToggled()));
  connect(newCB, TQT_SIGNAL(clicked()), TQT_SLOT(slotCBToggled()));
  connect(noTreeCB, TQT_SIGNAL(clicked()), TQT_SLOT(slotCBToggled()));

  // connect textfield
  connect(filterEdit, TQT_SIGNAL(textChanged(const TQString&)),
          TQT_SLOT(slotFilterTextChanged(const TQString&)));

  // update status
  connect(this, TQT_SIGNAL(listChanged()), TQT_SLOT(slotUpdateStatusLabel()));
}

//-----------------------------------------------------------------------------
KSubscription::~KSubscription()
{
}

//-----------------------------------------------------------------------------
void KSubscription::setStartItem( const KGroupInfo &info )
{
  TQListViewItemIterator it(groupView);

  for ( ; it.current(); ++it)
  {
    if (static_cast<GroupItem*>(it.current())->info() == info)
    {
      it.current()->setSelected(true);
      it.current()->setOpen(true);
    }
  }
}

//-----------------------------------------------------------------------------
void KSubscription::removeListItem( TQListView *view, const KGroupInfo &gi )
{
  if(!view) return;
  TQListViewItemIterator it(view);

  for ( ; it.current(); ++it)
  {
    if (static_cast<GroupItem*>(it.current())->info() == gi)
    {
      delete it.current();
      break;
    }
  }
  if (view == groupView)
    emit listChanged();
}

//-----------------------------------------------------------------------------
TQListViewItem* KSubscription::getListItem( TQListView *view, const KGroupInfo &gi )
{
  if(!view) return 0;
  TQListViewItemIterator it(view);

  for ( ; it.current(); ++it)
  {
    if (static_cast<GroupItem*>(it.current())->info() == gi)
      return (it.current());
  }
  return 0;
}

//-----------------------------------------------------------------------------
bool KSubscription::itemInListView( TQListView *view, const KGroupInfo &gi )
{
  if(!view) return false;
  TQListViewItemIterator it(view);

  for ( ; it.current(); ++it)
    if (static_cast<GroupItem*>(it.current())->info() == gi)
      return true;

  return false;
}

//------------------------------------------------------------------------------
void KSubscription::setDirectionButton1( Direction dir )
{
  mDirButton1 = dir;
  if (dir == Left)
    arrowBtn1->setIconSet(pmLeft);
  else
    arrowBtn1->setIconSet(pmRight);
}

//------------------------------------------------------------------------------
void KSubscription::setDirectionButton2( Direction dir )
{
  mDirButton2 = dir;
  if (dir == Left)
    arrowBtn2->setIconSet(pmLeft);
  else
    arrowBtn2->setIconSet(pmRight);
}

//------------------------------------------------------------------------------
void KSubscription::changeItemState( GroupItem* item, bool on )
{
  // is this a checkable item
  if (!item->isCheckItem()) return;

  // if we're currently loading the items ignore changes
  if (mLoading) return;
  if (on)
  {
    if (!itemInListView(unsubView, item->info()))
    {
      TQListViewItem *p = item->parent();
      while (p)
      {
        // make sure all parents are subscribed
        GroupItem* pi = static_cast<GroupItem*>(p);
        if (pi->isCheckItem() && !pi->isOn())
        {
          pi->setIgnoreStateChange(true);
          pi->setOn(true);
          pi->setIgnoreStateChange(false);
          new GroupItem(subView, pi->info(), this);
        }
        p = p->parent();
      }
      new GroupItem(subView, item->info(), this);
    }
    // eventually remove it from the other listview
    removeListItem(unsubView, item->info());
  }
  else {
    if (!itemInListView(subView, item->info()))
    {
      new GroupItem(unsubView, item->info(), this);
    }
    // eventually remove it from the other listview
    removeListItem(subView, item->info());
  }
  // update the buttons
  slotChangeButtonState(item);
}

//------------------------------------------------------------------------------
void KSubscription::filterChanged( TQListViewItem* item, const TQString & text )
{
  if ( !item && groupView )
    item = groupView->firstChild();
  if ( !item )
    return;

  do
  {
    if ( item->firstChild() ) // recursive descend
      filterChanged(item->firstChild(), text);

    GroupItem* gr = static_cast<GroupItem*>(item);
    if (subCB->isChecked() || newCB->isChecked() || !text.isEmpty() ||
        noTreeCB->isChecked())
    {
      // set it invisible
      if ( subCB->isChecked() &&
           (!gr->isCheckItem() ||
            (gr->isCheckItem() && !gr->info().subscribed)) )
      {
        // only subscribed
        gr->setVisible(false);
        continue;
      }
      if ( newCB->isChecked() &&
           (!gr->isCheckItem() ||
            (gr->isCheckItem() && !gr->info().newGroup)) )
      {
        // only new
        gr->setVisible(false);
        continue;
      }
      if ( !text.isEmpty() &&
           gr->text(0).find(text, 0, false) == -1)
      {
        // searchfield
        gr->setVisible(false);
        continue;
      }
      if ( noTreeCB->isChecked() &&
           !gr->isCheckItem() )
      {
        // disable treeview
        gr->setVisible(false);
        continue;
      }

      gr->setVisible(true);

    } else {
      gr->setVisible(true);
    }

  } while ((item = item->nextSibling()));

}

//------------------------------------------------------------------------------
uint KSubscription::activeItemCount()
{
  TQListViewItemIterator it(groupView);

  uint count = 0;
  for ( ; it.current(); ++it)
  {
    if (static_cast<GroupItem*>(it.current())->isCheckItem() &&
        it.current()->isVisible() && it.current()->isEnabled())
      count++;
  }

  return count;
}

//------------------------------------------------------------------------------
void KSubscription::restoreOriginalParent()
{
  TQPtrList<TQListViewItem> move;
  TQListViewItemIterator it(groupView);
  for ( ; it.current(); ++it)
  {
    TQListViewItem* origParent = static_cast<GroupItem*>(it.current())->
      originalParent();
    if (origParent && origParent != it.current()->parent())
    {
      // remember this to avoid messing up the iterator
      move.append(it.current());
    }
  }
  TQPtrListIterator<TQListViewItem> it2( move );
  for ( ; it2.current(); ++it2)
  {
    // restore the original parent
    TQListViewItem* origParent = static_cast<GroupItem*>(it2.current())->
      originalParent();
    groupView->takeItem(it2.current());
    origParent->insertItem(it2.current());
  }
}

//-----------------------------------------------------------------------------
void KSubscription::saveOpenStates()
{
  TQListViewItemIterator it(groupView);

  for ( ; it.current(); ++it)
  {
    static_cast<GroupItem*>(it.current())->setLastOpenState(
        it.current()->isOpen() );
  }
}

//-----------------------------------------------------------------------------
void KSubscription::restoreOpenStates()
{
  TQListViewItemIterator it(groupView);

  for ( ; it.current(); ++it)
  {
    it.current()->setOpen(
        static_cast<GroupItem*>(it.current())->lastOpenState() );
  }
}

//-----------------------------------------------------------------------------
void KSubscription::slotLoadingComplete()
{
  mLoading = false;

  enableButton(User1, true);
  enableButton(User2, true);
  newCB->setEnabled(true);
  noTreeCB->setEnabled(true);
  subCB->setEnabled(true);

  // remember the correct parent
  TQListViewItemIterator it(groupView);
  for ( ; it.current(); ++it)
  {
    static_cast<GroupItem*>(it.current())->
      setOriginalParent( it.current()->parent() );
  }

  emit listChanged();
}

//------------------------------------------------------------------------------
void KSubscription::slotChangeButtonState( TQListViewItem *item )
{
  if (!item ||
      (item->listView() == groupView &&
       !static_cast<GroupItem*>(item)->isCheckItem()))
  {
    // disable and return
    arrowBtn1->setEnabled(false);
    arrowBtn2->setEnabled(false);
    return;
  }
  // set the direction of the buttons and enable/disable them
  TQListView* currentView = item->listView();
  if (currentView == groupView)
  {
    setDirectionButton1(Right);
    setDirectionButton2(Right);
    if (static_cast<GroupItem*>(item)->isOn())
    {
      // already subscribed
      arrowBtn1->setEnabled(false);
      arrowBtn2->setEnabled(true);
    } else {
      // unsubscribed
      arrowBtn1->setEnabled(true);
      arrowBtn2->setEnabled(false);
    }
  } else if (currentView == subView)
  {
    // undo possible
    setDirectionButton1(Left);

    arrowBtn1->setEnabled(true);
    arrowBtn2->setEnabled(false);
  } else if (currentView == unsubView)
  {
    // undo possible
    setDirectionButton2(Left);

    arrowBtn1->setEnabled(false);
    arrowBtn2->setEnabled(true);
  }
}

//------------------------------------------------------------------------------
void KSubscription::slotButton1()
{
  if (mDirButton1 == Right)
  {
    if (groupView->currentItem() &&
        static_cast<GroupItem*>(groupView->currentItem())->isCheckItem())
    {
      // activate
      GroupItem* item = static_cast<GroupItem*>(groupView->currentItem());
      item->setOn(true);
    }
  }
  else {
    if (subView->currentItem())
    {
      GroupItem* item = static_cast<GroupItem*>(subView->currentItem());
      // get the corresponding item from the groupView
      TQListViewItem* listitem = getListItem(groupView, item->info());
      if (listitem)
      {
        // deactivate
        GroupItem* chk = static_cast<GroupItem*>(listitem);
        chk->setOn(false);
      }
    }
  }
}

//------------------------------------------------------------------------------
void KSubscription::slotButton2()
{
  if (mDirButton2 == Right)
  {
    if (groupView->currentItem() &&
        static_cast<GroupItem*>(groupView->currentItem())->isCheckItem())
    {
      // deactivate
      GroupItem* item = static_cast<GroupItem*>(groupView->currentItem());
      item->setOn(false);
    }
  }
  else {
    if (unsubView->currentItem())
    {
      GroupItem* item = static_cast<GroupItem*>(unsubView->currentItem());
      // get the corresponding item from the groupView
      TQListViewItem* listitem = getListItem(groupView, item->info());
      if (listitem)
      {
        // activate
        GroupItem* chk = static_cast<GroupItem*>(listitem);
        chk->setOn(true);
      }
    }
  }
}

//------------------------------------------------------------------------------
void KSubscription::slotCBToggled()
{
  if (!noTreeCB->isChecked() && !newCB->isChecked() && !subCB->isChecked())
  {
    restoreOriginalParent();
  }
  // set items {in}visible
  filterChanged(groupView->firstChild());
  emit listChanged();
}

//------------------------------------------------------------------------------
void KSubscription::slotFilterTextChanged( const TQString & text )
{
  // remember is the items are open
  if (mLastText.isEmpty())
    saveOpenStates();

  if (!mLastText.isEmpty() && text.length() < mLastText.length())
  {
    // reset
    restoreOriginalParent();
    TQListViewItemIterator it(groupView);
    for ( ; it.current(); ++it)
    {
      it.current()->setVisible(true);
      it.current()->setEnabled(true);
    }
  }
  // set items {in}visible
  filterChanged(groupView->firstChild(), text);
  // restore the open-states
  if (text.isEmpty())
    restoreOpenStates();

  emit listChanged();
  mLastText = text;
}

//------------------------------------------------------------------------------
void KSubscription::slotUpdateStatusLabel()
{
  TQString text;
  if (mLoading)
    text = i18n("Loading... (1 matching)", "Loading... (%n matching)",
                activeItemCount());
  else
    text = i18n("%1: (1 matching)", "%1: (%n matching)", activeItemCount())
           .arg(account()->name());

  leftLabel->setText(text);
}

//------------------------------------------------------------------------------
void KSubscription::slotLoadFolders()
{
  enableButton(User1, false);
  mLoading = true;
  subView->clear();
  unsubView->clear();
  groupView->clear();
}

#include "ksubscription.moc"
