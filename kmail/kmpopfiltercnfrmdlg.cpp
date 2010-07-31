/***************************************************************************
                          kmpopheadersdlg.cpp
                             -------------------
    begin                : Mon Oct 22 2001
    copyright            : (C) 2001 by Heiko Hund
                                       Thorsten Zachmann
    email                : heiko@ist.eigentlich.net
                           T.Zachmann@zagge.de
 ***************************************************************************/

#include <config.h>
#include "kmpopfiltercnfrmdlg.h"
#include "kmheaders.h"

#include <tqlayout.h>
#include <tqlabel.h>
#include <tqheader.h>
#include <tqcheckbox.h>
#include <tqvgroupbox.h>
#include <tqtimer.h>

#include <klocale.h>
#include <kio/global.h>

#include <assert.h>

////////////////////////////////////////
///  view
KMPopHeadersView::KMPopHeadersView(TQWidget *aParent, KMPopFilterCnfrmDlg *aDialog)
      : KListView(aParent)
{
  mDialog=aDialog;
  int mDownIndex=addColumn(TQIconSet(TQPixmap(mDown)), TQString::null, 24);
  assert( mDownIndex == Down ); //This code relies on the fact that radiobuttons are the first three columns for easier Column-Action mapping
			        //it does not necessarily be true - you could redefine mapToColumn and mapToAction to eg. shift those numbers by 1
  addColumn(TQIconSet(TQPixmap(mLater)), TQString::null, 24);
  addColumn(TQIconSet(TQPixmap(mDel)), TQString::null, 24);

  /*int subjCol =*/ addColumn(i18n("Subject"), 180);
  /*int sendCol =*/ addColumn(i18n("Sender"), 150);
  /*int recvCol =*/ addColumn(i18n("Receiver"), 150);
  int dateCol = addColumn(i18n("Date"), 120);
  int sizeCol = addColumn(i18n("Size"), 80);

  setAllColumnsShowFocus(true);

  setColumnAlignment(Down, Qt::AlignHCenter);
  setColumnAlignment(Later, Qt::AlignHCenter);
  setColumnAlignment(Delete, Qt::AlignHCenter);
  setColumnAlignment(sizeCol, Qt::AlignRight);

  setSorting(dateCol, false);
  setShowSortIndicator(true);
  header()->setResizeEnabled(false, Down);
  header()->setResizeEnabled(false, Later);
  header()->setResizeEnabled(false, Delete);
  header()->setClickEnabled(false, Down);
  header()->setClickEnabled(false, Later);
  header()->setClickEnabled(false, Delete);

  //we rely on fixed column order, so we forbid this
  header()->setMovingEnabled(false);

  connect(this, TQT_SIGNAL(pressed(TQListViewItem*, const TQPoint&, int)),
        TQT_SLOT(slotPressed(TQListViewItem*, const TQPoint&, int)));
}

KMPopHeadersView::~KMPopHeadersView()
{
}

//Handle keystrokes - Left and Right key select previous/next action correspondingly
void KMPopHeadersView::keyPressEvent( TQKeyEvent *e )
{
    if (e->key() == Key_Left) {
	    KMPopHeadersViewItem *item = dynamic_cast<KMPopHeadersViewItem*>( selectedItem() );
	    if (item&&mDialog) {
		    if (item->action()) { //here we rely on the fact that the leftmost action is equal to 0!
			    item->setAction((KMPopFilterAction)((int)item->action()-1));
			    mDialog->setAction( selectedItem(), item->action());
		    }
	    }
    } else if (e->key() == Key_Right) {
	    KMPopHeadersViewItem *item = dynamic_cast<KMPopHeadersViewItem*>( selectedItem() );
	    if (item&&mDialog) {
		    if (item->action()<NoAction-1) { //here we rely on the fact that right most action is one less than NoAction!
			    item->setAction((KMPopFilterAction)((int)item->action()+1));
			    mDialog->setAction( selectedItem(), item->action());
		    }
	    }
    } else {
	    TQListView::keyPressEvent( e );
    }
}

void KMPopHeadersView::slotPressed(TQListViewItem* aItem, const TQPoint&, int aColumn) {
  if ( !( aItem && aColumn>=0 && aColumn<NoAction ) ) return;
  KMPopHeadersViewItem *item = dynamic_cast<KMPopHeadersViewItem*>(aItem);
  assert( item );
  item->setAction(mapToAction(aColumn));
}

const char *KMPopHeadersView::mUnchecked[26] = {
"19 16 9 1",
"  c None",
"# c #000000",
". c #ffffff",
"a c #4a4c4a",
"b c #524c52",
"c c #efefef",
"e c #fff2ff",
"f c #f6f2f6",
"g c #fff6ff",
"                   ",
"                   ",
"         aaaa      ",
"       ba####aa    ",
"      a##....aac   ",
"      a#......ec   ",
"     a#........fc  ",
"     a#........gc  ",
"     a#........fc  ",
"     b#........gc  ",
"      a#......gc   ",
"      age....gec   ",
"       ccfgfgcc    ",
"         cccc      ",
"                   ",
"                   ",};

const char *KMPopHeadersView::mChecked[26] = {
"19 16 9 1",
"  c None",
"# c #000000",
". c #ffffff",
"a c #4a4c4a",
"b c #524c52",
"c c #efefef",
"e c #fff2ff",
"f c #f6f2f6",
"g c #fff6ff",
"                   ",
"                   ",
"         aaaa      ",
"       ba####aa    ",
"      a##....aac   ",
"      a#......ec   ",
"     a#...##...fc  ",
"     a#..####..gc  ",
"     a#..####..fc  ",
"     b#...##...gc  ",
"      a#......gc   ",
"      age....gec   ",
"       ccfgfgcc    ",
"         cccc      ",
"                   ",
"                   ",};

const char *KMPopHeadersView::mLater[25] = {
"16 16 8 1",
". c None",
"g c #303030",
"c c #585858",
"f c #a0a0a0",
"b c #c0c000",
"e c #dcdcdc",
"a c #ffff00",
"d c #ffffff",
"................",
"...........eaa..",
"..........eaaa..",
".........ebaab..",
".........eaaa...",
"........eaaab...",
"........eaaa....",
".......eaaab....",
"eaae..ebaccccc..",
"eaaae.eaacdedc..",
"ebaaabaaabcdc...",
".ebaaaaaa.fgf...",
"..ebaaaa..cec...",
"...ebaab.cdedc..",
"........eccccc..",
"................"};

const char *KMPopHeadersView::mDown[20] = {
"16 16 3 1",
". c None",
"a c #008000",
"b c #00c000",
"................",
"............aa..",
"...........aaa..",
"..........baab..",
"..........aaa...",
".........baab...",
".........aaa....",
"........aaab....",
".aa....baaa.....",
".aaa...aaa......",
".baaabaaab......",
"..baaaaaa.......",
"...baaaa........",
"....baab........",
"................",
"................"};

const char *KMPopHeadersView::mDel[19] = {
"16 16 2 1",
". c None",
"# c #c00000",
"................",
"................",
"..##.......##...",
"..###.....###...",
"...###...###....",
"....###.###.....",
".....#####......",
"......###.......",
".....#####......",
"....###.###.....",
"...###...###....",
"..###.....###...",
"..##.......##...",
"................",
"................",
"................"};


/////////////////////////////////////////
/////////////////////////////////////////
///  viewitem
/////////////////////////////////////////
/////////////////////////////////////////
KMPopHeadersViewItem::KMPopHeadersViewItem(KMPopHeadersView *aParent, KMPopFilterAction aAction)
      : KListViewItem(aParent)
{
  mParent = aParent;
  mAction = NoAction;

  setPixmap(mParent->mapToColumn(Delete), TQPixmap(KMPopHeadersView::mUnchecked));
  setPixmap(mParent->mapToColumn(Down), TQPixmap(KMPopHeadersView::mUnchecked));
  setPixmap(mParent->mapToColumn(Later), TQPixmap(KMPopHeadersView::mUnchecked));

  setAction( aAction );
}

KMPopHeadersViewItem::~KMPopHeadersViewItem()
{
}

void KMPopHeadersViewItem::setAction(KMPopFilterAction aAction)
{
  if(aAction != NoAction && aAction!=mAction)
  {
    if ( mAction!=NoAction ) setPixmap(mParent->mapToColumn(mAction), TQPixmap(KMPopHeadersView::mUnchecked));
    setPixmap(mParent->mapToColumn(aAction), TQPixmap(KMPopHeadersView::mChecked));
    mAction=aAction;
  }
}

void KMPopHeadersViewItem::paintFocus(TQPainter *, const TQColorGroup &, const TQRect &)
{
}

TQString KMPopHeadersViewItem::key(int col, bool) const
{
  if (col == 3) return KMMsgBase::skipKeyword(text(col).lower());
  if (col == 6) return text(8);
  if (col == 7)
    return text(col).rightJustify( 10, '0', false);
  return text(col);
}

/////////////////////////////////////////
/////////////////////////////////////////
///  dlg
/////////////////////////////////////////
/////////////////////////////////////////
KMPopFilterCnfrmDlg::KMPopFilterCnfrmDlg(TQPtrList<KMPopHeaders> *aHeaders, const TQString &aAccount, bool aShowLaterMsgs, TQWidget *aParent, const char *aName)
      : KDialogBase(aParent, aName, true, i18n("POP Filter"), Ok | Help, Ok, false)
{
  unsigned int rulesetCount = 0;
  //mHeaders = aHeaders;
  mShowLaterMsgs = aShowLaterMsgs;
  mLowerBoxVisible = false;

  TQWidget *w = new TQWidget(this);
  setMainWidget(w);

  TQVBoxLayout *vbl = new TQVBoxLayout(w, 0, spacingHint());

  TQLabel *l = new TQLabel(i18n("Messages to filter found on POP Account: <b>%1</b><p>"
      "The messages shown exceed the maximum size limit you defined for this account.<br>You can select "
      "what you want to do with them by checking the appropriate button.").arg(aAccount), w);
  vbl->addWidget(l);

  TQVGroupBox *upperBox = new TQVGroupBox(i18n("Messages Exceeding Size"), w);
  upperBox->hide();
  KMPopHeadersView *lv = new KMPopHeadersView(upperBox, this);
  vbl->addWidget(upperBox);

  TQVGroupBox *lowerBox = new TQVGroupBox(i18n("Ruleset Filtered Messages: none"), w);
  TQString checkBoxText((aShowLaterMsgs)?
      i18n("Show messages matched by a ruleset and tagged 'Download' or 'Delete'"):
      i18n("Show messages matched by a filter ruleset"));
  TQCheckBox* cb = new TQCheckBox(checkBoxText, lowerBox);
  cb->setEnabled(false);
  mFilteredHeaders = new KMPopHeadersView(lowerBox, this);
  mFilteredHeaders->hide();
  vbl->addWidget(lowerBox);

  mFilteredHeaders->header()->setResizeEnabled(false, 8);
  mFilteredHeaders->setColumnWidth(8, 0);

  // fill the listviews with data from the headers
  KMPopHeaders *headers;
  for(headers = aHeaders->first(); headers; headers = aHeaders->next())
  {
    KMPopHeadersViewItem *lvi = 0;

    if(headers->ruleMatched())
    {
      if(aShowLaterMsgs && headers->action() == Later)
      {
        // insert messages tagged 'later' only
        lvi = new KMPopHeadersViewItem(mFilteredHeaders, headers->action());
        mFilteredHeaders->show();
        mLowerBoxVisible = true;
      }
      else if(aShowLaterMsgs)
      {
        // enable checkbox to show 'delete' and 'download' msgs
        // but don't insert them into the listview yet
        mDDLList.append(headers);
        cb->setEnabled(true);
      }
      else if(!aShowLaterMsgs)
      {
        // insert all messaged tagged by a ruleset, enable
        // the checkbox, but don't show the listview yet
        lvi = new KMPopHeadersViewItem(mFilteredHeaders, headers->action());
        cb->setEnabled(true);
      }
      rulesetCount++;
    }
    else
    {
      // insert all messages not tagged by a ruleset
      // into the upper listview
      lvi = new KMPopHeadersViewItem(lv, headers->action());
      upperBox->show();
    }

    if(lvi)
    {
      mItemMap[lvi] = headers;
      setupLVI(lvi,headers->header());
    }
  }

  if(rulesetCount)
      lowerBox->setTitle(i18n("Ruleset Filtered Messages: %1").arg(rulesetCount));

  // connect signals and slots
  connect(lv, TQT_SIGNAL(pressed(TQListViewItem*, const TQPoint&, int)),
      this, TQT_SLOT(slotPressed(TQListViewItem*, const TQPoint&, int)));
  connect(mFilteredHeaders, TQT_SIGNAL(pressed(TQListViewItem*, const TQPoint&, int)),
      this, TQT_SLOT(slotPressed(TQListViewItem*, const TQPoint&, int)));
  connect(cb, TQT_SIGNAL(toggled(bool)),
      this, TQT_SLOT(slotToggled(bool)));

  adjustSize();
  TQTimer::singleShot(0, this, TQT_SLOT(slotUpdateMinimumSize()));
}

KMPopFilterCnfrmDlg::~KMPopFilterCnfrmDlg()
{
}

void KMPopFilterCnfrmDlg::setupLVI(KMPopHeadersViewItem *lvi, KMMessage *msg)
{
      // set the subject
      TQString tmp = msg->subject();
      if(tmp.isEmpty())
        tmp = i18n("no subject");
      lvi->setText(3, tmp);

      // set the sender
      tmp = msg->fromStrip();
      if(tmp.isEmpty())
        tmp = i18n("unknown");
      lvi->setText(4, tmp);

      // set the receiver
      tmp = msg->toStrip();
      if(tmp.isEmpty())
        tmp = i18n("unknown");
      lvi->setText(5, tmp);

      // set the date
      lvi->setText(6, KMime::DateFormatter::formatDate( KMime::DateFormatter::Fancy, msg->date() ) );
      // set the size
      lvi->setText(7, KIO::convertSize(msg->msgLength()));
      // Date for sorting
      lvi->setText(8, msg->dateIsoStr());
}

void KMPopFilterCnfrmDlg::setAction(TQListViewItem *aItem, KMPopFilterAction aAction)
{
    mItemMap[aItem]->setAction(aAction);
}
/**
  This Slot is called whenever a ListView item was pressed.
  It checks for the column the button was pressed in and changes the action if the
  click happened over a radio button column.
  Of course the radio button state is changed as well if the above is true.
*/
void KMPopFilterCnfrmDlg::slotPressed(TQListViewItem *aItem, const TQPoint &, int aColumn)
{
  if ( aColumn>=0 && aColumn<NoAction ) setAction(aItem,KMPopHeadersView::mapToAction(aColumn));
}

void KMPopFilterCnfrmDlg::slotToggled(bool aOn)
{
  if(aOn)
  {
    if(mShowLaterMsgs)
    {
      // show download and delete msgs in the list view too
      for(KMPopHeaders* headers = mDDLList.first(); headers; headers = mDDLList.next())
      {
        KMPopHeadersViewItem *lvi = new KMPopHeadersViewItem(mFilteredHeaders, headers->action());
        mItemMap[lvi] = headers;
        mDelList.append(lvi);
        setupLVI(lvi,headers->header());
      }
    }

    if(!mLowerBoxVisible)
    {
      mFilteredHeaders->show();
    }
  }
  else
  {
    if(mShowLaterMsgs)
    {
      // delete download and delete msgs from the lower listview
      for(KMPopHeadersViewItem* item = mDelList.first(); item; item = mDelList.next())
      {
        mFilteredHeaders->takeItem(item);
      }
      mDelList.clear();
    }

    if(!mLowerBoxVisible)
    {
      mFilteredHeaders->hide();
    }
  }
  TQTimer::singleShot(0, this, TQT_SLOT(slotUpdateMinimumSize()));
}

void KMPopFilterCnfrmDlg::slotUpdateMinimumSize()
{
  mainWidget()->setMinimumSize(mainWidget()->sizeHint());
}

#include "kmpopfiltercnfrmdlg.moc"
