/*
 * kmail: KDE mail client
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
 * Copyright (c) 2001 Aaron J. Seigo <aseigo@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include <config.h>
#include "kmcommands.h"
#include "searchwindow.h"
#include "kmmainwidget.h"
#include "kmmsgdict.h"
#include "kmmsgpart.h"
#include "kmfolderimap.h"
#include "kmfoldermgr.h"
#include "kmfoldersearch.h"
#include "kmfoldertree.h"
#include "kmheaders.h"
#include "kmsearchpatternedit.h"
#include "kmsearchpattern.h"
#include "folderrequester.h"
#include "messagecopyhelper.h"
#include "textsource.h"

#include <kapplication.h>
#include <kdebug.h>
#include <kstatusbar.h>
#include <kwin.h>
#include <kconfig.h>
#include <kstdaction.h>
#include <kiconloader.h>

#include <tqcheckbox.h>
#include <tqlayout.h>
#include <klineedit.h>
#include <tqpushbutton.h>
#include <tqradiobutton.h>
#include <tqbuttongroup.h>
#include <tqcombobox.h>
#include <tqobjectlist.h> //for mPatternEdit->queryList( 0, "mRuleField" )->first();
#include <tqcursor.h>
#include <tqpopupmenu.h>

#include <maillistdrag.h>
using namespace KPIM;

#include <mimelib/enum.h>
#include <mimelib/boyermor.h>

#include <assert.h>
#include <stdlib.h>

namespace KMail {

const int SearchWindow::MSGID_COLUMN = 4;

// KListView sub-class for dnd support
class MatchListView : public KListView
{
  public:
    MatchListView( TQWidget *parent, SearchWindow* sw, const char* name = 0 ) :
      KListView( parent, name ),
      mSearchWindow( sw )
    {}

  protected:
    virtual TQDragObject* dragObject()
    {
      KMMessageList list = mSearchWindow->selectedMessages();
      MailList mailList;
      for ( KMMsgBase* msg = list.first(); msg; msg = list.next() ) {
        if ( !msg )
          continue;
        MailSummary mailSummary( msg->getMsgSerNum(), msg->msgIdMD5(),
                                 msg->subject(), msg->fromStrip(),
                                 msg->toStrip(), msg->date() );
        mailList.append( mailSummary );
      }
      MailListDrag *d = new MailListDrag( mailList, viewport(), new KMTextSource );

      TQPixmap pixmap;
      if( mailList.count() == 1 )
        pixmap = TQPixmap( DesktopIcon("message", KIcon::SizeSmall) );
      else
        pixmap = TQPixmap( DesktopIcon("kmultiple", KIcon::SizeSmall) );

      d->setPixmap( pixmap );
      return d;
    }

  private:
    SearchWindow* mSearchWindow;
};

//-----------------------------------------------------------------------------
SearchWindow::SearchWindow(KMMainWidget* w, const char* name,
                         KMFolder *curFolder, bool modal):
  KDialogBase(0, name, modal, i18n("Find Messages"),
              User1 | User2 | Close, User1, false,
              KGuiItem( i18n("&Search"), "find" ),
              KStdGuiItem::stop()),
  mStopped(false),
  mCloseRequested(false),
  mSortColumn(0),
  mSortOrder(Ascending),
  mFolder(0),
  mTimer(new TQTimer(this, "mTimer")),
  mLastFocus(0),
  mKMMainWidget(w)
{
#if !KDE_IS_VERSION( 3, 2, 91 )
  // HACK - KWin keeps all dialogs on top of their mainwindows, but that's probably
  // wrong (#76026), and should be done only for modals. CVS HEAD should get
  // proper fix in KWin (l.lunak@kde.org)
  XDeleteProperty( qt_xdisplay(), winId(), XA_WM_TRANSIENT_FOR );
#endif
  KWin::setIcons(winId(), kapp->icon(), kapp->miniIcon());

  KConfig* config = KMKernel::config();
  config->setGroup("SearchDialog");

  TQWidget* searchWidget = new TQWidget(this);
  TQVBoxLayout *vbl = new TQVBoxLayout( searchWidget, 0, spacingHint(), "kmfs_vbl" );

  TQButtonGroup * radioGroup = new TQButtonGroup( searchWidget );
  radioGroup->hide();

  mChkbxAllFolders = new TQRadioButton(i18n("Search in &all local folders"), searchWidget);
  vbl->addWidget( mChkbxAllFolders );
  radioGroup->insert( mChkbxAllFolders );

  TQHBoxLayout *hbl = new TQHBoxLayout( vbl, spacingHint(), "kmfs_hbl" );
  mChkbxSpecificFolders = new TQRadioButton(i18n("Search &only in:"), searchWidget);
  hbl->addWidget(mChkbxSpecificFolders);
  mChkbxSpecificFolders->setChecked(true);
  radioGroup->insert( mChkbxSpecificFolders );

  mCbxFolders = new FolderRequester( searchWidget,
      kmkernel->getKMMainWidget()->folderTree() );
  mCbxFolders->setMustBeReadWrite( false );
  mCbxFolders->setFolder(curFolder);
  hbl->addWidget(mCbxFolders);

  mChkSubFolders = new TQCheckBox(i18n("I&nclude sub-folders"), searchWidget);
  mChkSubFolders->setChecked(true);
  hbl->addWidget(mChkSubFolders);

  TQWidget *spacer = new TQWidget( searchWidget, "spacer" );
  spacer->setMinimumHeight( 2 );
  vbl->addWidget( spacer );

  mPatternEdit = new KMSearchPatternEdit( "", searchWidget , "spe", false, true );
  mPatternEdit->setFrameStyle( TQFrame::NoFrame | TQFrame::Plain );
  mPatternEdit->setInsideMargin( 0 );
  mSearchPattern = new KMSearchPattern();
  KMFolderSearch *searchFolder = 0;
  if (curFolder)
      searchFolder = dynamic_cast<KMFolderSearch*>(curFolder->storage());
  if (searchFolder) {
      KConfig config(curFolder->location());
      KMFolder *root = searchFolder->search()->root();
      config.setGroup("Search Folder");
      mSearchPattern->readConfig(&config);
      if (root) {
          mChkbxSpecificFolders->setChecked(true);
          mCbxFolders->setFolder(root);
          mChkSubFolders->setChecked(searchFolder->search()->recursive());
      } else {
          mChkbxAllFolders->setChecked(true);
      }
  }
  mPatternEdit->setSearchPattern( mSearchPattern );
  TQObjectList *list = mPatternEdit->queryList( 0, "mRuleField" );
  TQObject *object = 0;
  if ( list )
      object = list->first();
  delete list;
  if (!searchFolder && object && ::qt_cast<TQComboBox*>(object))
      static_cast<TQComboBox*>(object)->setCurrentText("Subject");

  vbl->addWidget( mPatternEdit );

  // enable/disable widgets depending on radio buttons:
  connect( mChkbxSpecificFolders, TQT_SIGNAL(toggled(bool)),
           mCbxFolders, TQT_SLOT(setEnabled(bool)) );
  connect( mChkbxSpecificFolders, TQT_SIGNAL(toggled(bool)),
           mChkSubFolders, TQT_SLOT(setEnabled(bool)) );
  connect( mChkbxAllFolders, TQT_SIGNAL(toggled(bool)),
           this, TQT_SLOT(setEnabledSearchButton(bool)) );

  mLbxMatches = new MatchListView(searchWidget, this, "Find Messages");

  /*
     Default is to sort by date. TODO: Unfortunately this sorts *while*
     inserting, which looks rather strange - the user cannot read
     the results so far as they are constantly re-sorted --dnaber

     Sorting is now disabled when a search is started and reenabled
     when it stops. Items are appended to the list. This not only
     solves the above problem, but speeds searches with many hits
     up considerably. - till

     TODO: subclass KListViewItem and do proper (and performant)
     comapare functions
  */
  mLbxMatches->setSorting(2, false);
  mLbxMatches->setShowSortIndicator(true);
  mLbxMatches->setAllColumnsShowFocus(true);
  mLbxMatches->setSelectionModeExt(KListView::Extended);
  mLbxMatches->addColumn(i18n("Subject"),
                         config->readNumEntry("SubjectWidth", 150));
  mLbxMatches->addColumn(i18n("Sender/Receiver"),
                         config->readNumEntry("SenderWidth", 120));
  mLbxMatches->addColumn(i18n("Date"),
                         config->readNumEntry("DateWidth", 120));
  mLbxMatches->addColumn(i18n("Folder"),
                         config->readNumEntry("FolderWidth", 100));

  mLbxMatches->addColumn(""); // should be hidden
  mLbxMatches->setColumnWidthMode( MSGID_COLUMN, TQListView::Manual );
  mLbxMatches->setColumnWidth(MSGID_COLUMN, 0);
  mLbxMatches->header()->setResizeEnabled(false, MSGID_COLUMN);

  mLbxMatches->setDragEnabled( true );

  connect( mLbxMatches, TQT_SIGNAL(clicked(TQListViewItem *)),
           this, TQT_SLOT(slotShowMsg(TQListViewItem *)) );
  connect( mLbxMatches, TQT_SIGNAL(doubleClicked(TQListViewItem *)),
           this, TQT_SLOT(slotViewMsg(TQListViewItem *)) );
  connect( mLbxMatches, TQT_SIGNAL(currentChanged(TQListViewItem *)),
           this, TQT_SLOT(slotCurrentChanged(TQListViewItem *)) );
  connect( mLbxMatches, TQT_SIGNAL(contextMenuRequested(TQListViewItem *,const TQPoint &,int)),
           this, TQT_SLOT(slotContextMenuRequested(TQListViewItem *,const TQPoint &,int)) );
  vbl->addWidget( mLbxMatches );

  TQHBoxLayout *hbl2 = new TQHBoxLayout( vbl, spacingHint(), "kmfs_hbl2" );
  mSearchFolderLbl = new TQLabel(i18n("Search folder &name:"), searchWidget);
  hbl2->addWidget(mSearchFolderLbl);
  mSearchFolderEdt = new KLineEdit(searchWidget);
  if (searchFolder)
    mSearchFolderEdt->setText(searchFolder->folder()->name());
  else
    mSearchFolderEdt->setText(i18n("Last Search"));

  mSearchFolderLbl->setBuddy(mSearchFolderEdt);
  hbl2->addWidget(mSearchFolderEdt);
  mSearchFolderOpenBtn = new TQPushButton(i18n("Op&en Search Folder"), searchWidget);
  mSearchFolderOpenBtn->setEnabled(false);
  hbl2->addWidget(mSearchFolderOpenBtn);
  connect( mSearchFolderEdt, TQT_SIGNAL( textChanged( const TQString &)),
           this, TQT_SLOT( scheduleRename( const TQString & )));
  connect( &mRenameTimer, TQT_SIGNAL( timeout() ),
           this, TQT_SLOT( renameSearchFolder() ));
  connect( mSearchFolderOpenBtn, TQT_SIGNAL( clicked() ),
           this, TQT_SLOT( openSearchFolder() ));
  mSearchResultOpenBtn = new TQPushButton(i18n("Open &Message"), searchWidget);
  mSearchResultOpenBtn->setEnabled(false);
  hbl2->addWidget(mSearchResultOpenBtn);
  connect( mSearchResultOpenBtn, TQT_SIGNAL( clicked() ),
           this, TQT_SLOT( slotViewSelectedMsg() ));
  mStatusBar = new KStatusBar(searchWidget);
  mStatusBar->insertFixedItem(i18n("AMiddleLengthText..."), 0, true);
  mStatusBar->changeItem(i18n("Ready."), 0);
  mStatusBar->setItemAlignment(0, AlignLeft | AlignVCenter);
  mStatusBar->insertItem(TQString::null, 1, 1, true);
  mStatusBar->setItemAlignment(1, AlignLeft | AlignVCenter);
  vbl->addWidget(mStatusBar);

  int mainWidth = config->readNumEntry("SearchWidgetWidth", 0);
  int mainHeight = config->readNumEntry("SearchWidgetHeight", 0);

  if (mainWidth || mainHeight)
    resize(mainWidth, mainHeight);

  setMainWidget(searchWidget);
  setButtonBoxOrientation(TQWidget::Vertical);

  mBtnSearch = actionButton(KDialogBase::User1);
  mBtnStop = actionButton(KDialogBase::User2);
  mBtnStop->setEnabled(false);

  connect(this, TQT_SIGNAL(user1Clicked()), TQT_SLOT(slotSearch()));
  connect(this, TQT_SIGNAL(user2Clicked()), TQT_SLOT(slotStop()));
  connect(this, TQT_SIGNAL(finished()), this, TQT_SLOT(deleteLater()));

  // give focus to the value field of the first search rule
  object = mPatternEdit->child( "regExpLineEdit" );
  if ( object && object->isWidgetType() ) {
      static_cast<TQWidget*>(object)->setFocus();
      //kdDebug(5006) << "SearchWindow: focus has been given to widget "
      //              << object->name() << endl;
  }
  else
      kdDebug(5006) << "SearchWindow: regExpLineEdit not found" << endl;

  //set up actions
  KActionCollection *ac = actionCollection();
  ac->setWidget( this );
  mReplyAction = new KAction( i18n("&Reply..."), "mail_reply", 0, this,
                              TQT_SLOT(slotReplyToMsg()), ac, "search_reply" );
  mReplyAllAction = new KAction( i18n("Reply to &All..."), "mail_replyall",
                                 0, this, TQT_SLOT(slotReplyAllToMsg()),
                                 ac, "search_reply_all" );
  mReplyListAction = new KAction( i18n("Reply to Mailing-&List..."),
                                  "mail_replylist", 0, this,
                                  TQT_SLOT(slotReplyListToMsg()), ac,
                                  "search_reply_list" );
  mForwardActionMenu = new KActionMenu( i18n("Message->","&Forward"),
                                        "mail_forward", ac,
                                        "search_message_forward" );
  connect( mForwardActionMenu, TQT_SIGNAL(activated()), this,
           TQT_SLOT(slotForwardInlineMsg()) );
  mForwardAttachedAction = new KAction( i18n("Message->Forward->","As &Attachment..."),
                                        "mail_forward", 0, this,
                                        TQT_SLOT(slotForwardAttachedMsg()), ac,
                                        "search_message_forward_as_attachment" );
  mForwardInlineAction = new KAction( i18n("&Inline..."),
                                      "mail_forward", 0, this,
                                      TQT_SLOT(slotForwardInlineMsg()), ac,
                                      "search_message_forward_inline" );
  if ( GlobalSettings::self()->forwardingInlineByDefault() ) {
    mForwardActionMenu->insert( mForwardInlineAction );
    mForwardActionMenu->insert( mForwardAttachedAction );
  } else {
    mForwardActionMenu->insert( mForwardAttachedAction );
    mForwardActionMenu->insert( mForwardInlineAction );
  }

  mForwardDigestAction = new KAction( i18n("Message->Forward->","As Di&gest..."),
                                      "mail_forward", 0, this,
                                      TQT_SLOT(slotForwardDigestMsg()), ac,
                                      "search_message_forward_as_digest" );
  mForwardActionMenu->insert( mForwardDigestAction );
  mRedirectAction = new KAction( i18n("Message->Forward->","&Redirect..."),
                                      "mail_forward", 0, this,
                                      TQT_SLOT(slotRedirectMsg()), ac,
                                      "search_message_forward_redirect" );
  mForwardActionMenu->insert( mRedirectAction );
  mSaveAsAction = KStdAction::saveAs( this, TQT_SLOT(slotSaveMsg()), ac, "search_file_save_as" );
  mSaveAtchAction = new KAction( i18n("Save Attachments..."), "attach", 0,
                                 this, TQT_SLOT(slotSaveAttachments()), ac, "search_save_attachments" );

  mPrintAction = KStdAction::print( this, TQT_SLOT(slotPrintMsg()), ac, "search_print" );
  mClearAction = new KAction( i18n("Clear Selection"), 0, 0, this,
                              TQT_SLOT(slotClearSelection()), ac, "search_clear_selection" );

  mCopyAction = KStdAction::copy( this, TQT_SLOT(slotCopyMsgs()), ac, "search_copy_messages" );
  mCutAction = KStdAction::cut( this, TQT_SLOT(slotCutMsgs()), ac, "search_cut_messages" );

  connect(mTimer, TQT_SIGNAL(timeout()), this, TQT_SLOT(updStatus()));
  connect(kmkernel->searchFolderMgr(), TQT_SIGNAL(folderInvalidated(KMFolder*)),
          this, TQT_SLOT(folderInvalidated(KMFolder*)));

  connect(mCbxFolders, TQT_SIGNAL(folderChanged(KMFolder*)),
          this, TQT_SLOT(slotFolderActivated()));

}

//-----------------------------------------------------------------------------
SearchWindow::~SearchWindow()
{
  TQValueListIterator<TQGuardedPtr<KMFolder> > fit;
  for ( fit = mFolders.begin(); fit != mFolders.end(); ++fit ) {
    if (!(*fit))
      continue;
    (*fit)->close("searchwindow");
  }

  KConfig* config = KMKernel::config();
  config->setGroup("SearchDialog");
  config->writeEntry("SubjectWidth", mLbxMatches->columnWidth(0));
  config->writeEntry("SenderWidth", mLbxMatches->columnWidth(1));
  config->writeEntry("DateWidth", mLbxMatches->columnWidth(2));
  config->writeEntry("FolderWidth", mLbxMatches->columnWidth(3));
  config->writeEntry("SearchWidgetWidth", width());
  config->writeEntry("SearchWidgetHeight", height());
  config->sync();
}

void SearchWindow::setEnabledSearchButton(bool)
{
  //Make sure that button is enable
  //Before when we selected a folder == "Local Folder" as that it was not a folder
  //search button was disable, and when we select "Search in all local folder"
  //Search button was never enabled :(
  mBtnSearch->setEnabled( true );
}

//-----------------------------------------------------------------------------
void SearchWindow::updStatus(void)
{
    TQString genMsg, detailMsg, procMsg;
    int numMatches = 0, numProcessed = 0;
    KMSearch const *search = (mFolder) ? (mFolder->search()) : 0;
    TQString folderName;
    if (search) {
        numMatches = search->foundCount();
        numProcessed = search->searchCount();
        folderName = search->currentFolder();
    }

    if (search && !search->running()) {
        procMsg = i18n("%n message searched", "%n messages searched",
                       numProcessed);
        if(!mStopped) {
            genMsg = i18n("Done.");
            detailMsg = i18n("%n match in %1", "%n matches in %1",
                             numMatches).arg(procMsg);
        } else {
            genMsg = i18n("Search canceled.");
            detailMsg = i18n("%n match so far in %1", "%n matches so far in %1",
                             numMatches).arg(procMsg);
        }
    } else {
        procMsg = i18n("%n message", "%n messages", numProcessed);
        genMsg = i18n("%n match", "%n matches", numMatches);
        detailMsg = i18n("Searching in %1. %2 searched so far")
                    .arg(folderName).arg(procMsg);
    }

    mStatusBar->changeItem(genMsg, 0);
    mStatusBar->changeItem(detailMsg, 1);
}


//-----------------------------------------------------------------------------
void SearchWindow::keyPressEvent(TQKeyEvent *evt)
{
    KMSearch const *search = (mFolder) ? mFolder->search() : 0;
    bool searching = (search) ? search->running() : false;
    if (evt->key() == Key_Escape && searching) {
        mFolder->stopSearch();
        return;
    }

    KDialogBase::keyPressEvent(evt);
}


//-----------------------------------------------------------------------------
void SearchWindow::slotFolderActivated()
{
    mChkbxSpecificFolders->setChecked(true);
}

//-----------------------------------------------------------------------------
void SearchWindow::activateFolder(KMFolder *curFolder)
{
    mChkbxSpecificFolders->setChecked(true);
    mCbxFolders->setFolder(curFolder);
}

//-----------------------------------------------------------------------------
void SearchWindow::slotSearch()
{
    mLastFocus = focusWidget();
    mBtnSearch->setFocus();     // set focus so we don't miss key event

    mStopped = false;
    mFetchingInProgress = 0;

    mSearchFolderOpenBtn->setEnabled(true);
    if ( mSearchFolderEdt->text().isEmpty() ) {
      mSearchFolderEdt->setText( i18n("Last Search") );
    }
    mBtnSearch->setEnabled(false);
    mBtnStop->setEnabled(true);

    mLbxMatches->clear();

    mSortColumn = mLbxMatches->sortColumn();
    mSortOrder = mLbxMatches->sortOrder();
    mLbxMatches->setSorting(-1);
    mLbxMatches->setShowSortIndicator(false);

    // If we haven't openend an existing search folder, find or
    // create one.
    if (!mFolder) {
      KMFolderMgr *mgr = kmkernel->searchFolderMgr();
      TQString baseName = mSearchFolderEdt->text();
      TQString fullName = baseName;
      int count = 0;
      KMFolder *folder;
      while ((folder = mgr->find(fullName))) {
        if (folder->storage()->inherits("KMFolderSearch"))
          break;
        fullName = TQString("%1 %2").arg(baseName).arg(++count);
      }

      if (!folder)
        folder = mgr->createFolder(fullName, false, KMFolderTypeSearch,
            &mgr->dir());

      mFolder = dynamic_cast<KMFolderSearch*>( folder->storage() );
    }
    mFolder->stopSearch();
    disconnect(mFolder, TQT_SIGNAL(msgAdded(int)),
            this, TQT_SLOT(slotAddMsg(int)));
    disconnect(mFolder, TQT_SIGNAL(msgRemoved(KMFolder*, Q_UINT32)),
            this, TQT_SLOT(slotRemoveMsg(KMFolder*, Q_UINT32)));
    connect(mFolder, TQT_SIGNAL(msgAdded(int)),
            this, TQT_SLOT(slotAddMsg(int)));
    connect(mFolder, TQT_SIGNAL(msgRemoved(KMFolder*, Q_UINT32)),
            this, TQT_SLOT(slotRemoveMsg(KMFolder*, Q_UINT32)));
    mSearchFolderEdt->setEnabled(false);
    KMSearch *search = new KMSearch();
    connect(search, TQT_SIGNAL(finished(bool)),
            this, TQT_SLOT(searchDone()));
    if (mChkbxAllFolders->isChecked()) {
        search->setRecursive(true);
    } else {
        search->setRoot(mCbxFolders->folder());
        search->setRecursive(mChkSubFolders->isChecked());
    }

    mPatternEdit->updateSearchPattern();
    KMSearchPattern *searchPattern = new KMSearchPattern();
    *searchPattern = *mSearchPattern; //deep copy
    searchPattern->purify();
    search->setSearchPattern(searchPattern);
    mFolder->setSearch(search);
    enableGUI();

    mTimer->start(200);
}

//-----------------------------------------------------------------------------
void SearchWindow::searchDone()
{
    mTimer->stop();
    updStatus();

    TQTimer::singleShot(0, this, TQT_SLOT(enableGUI()));
    if(mLastFocus)
        mLastFocus->setFocus();
    if (mCloseRequested)
        close();

    mLbxMatches->setSorting(mSortColumn, mSortOrder == Ascending);
    mLbxMatches->setShowSortIndicator(true);

    mSearchFolderEdt->setEnabled(true);
}

void SearchWindow::slotAddMsg(int idx)
{
    if (!mFolder)
        return;
    bool unget = !mFolder->isMessage(idx);
    KMMessage *msg = mFolder->getMsg(idx);
    TQString from, fName;
    KMFolder *pFolder = msg->parent();
    if (!mFolders.contains(pFolder)) {
        mFolders.append(pFolder);
        pFolder->open("searchwindow");
    }
    if(pFolder->whoField() == "To")
        from = msg->to();
    else
        from = msg->from();
    if (pFolder->isSystemFolder())
        fName = i18n(pFolder->name().utf8());
    else
        fName = pFolder->name();

    (void)new KListViewItem(mLbxMatches, mLbxMatches->lastItem(),
                            msg->subject(), from, msg->dateIsoStr(),
                            fName,
                            TQString::number(mFolder->serNum(idx)));
    if (unget)
        mFolder->unGetMsg(idx);
}

void SearchWindow::slotRemoveMsg(KMFolder *, Q_UINT32 serNum)
{
    if (!mFolder)
        return;
    TQListViewItemIterator it(mLbxMatches);
    while (it.current()) {
        TQListViewItem *item = *it;
        if (serNum == (*it)->text(MSGID_COLUMN).toUInt()) {
            delete item;
            return;
        }
        ++it;
    }
}

//-----------------------------------------------------------------------------
void SearchWindow::slotStop()
{
    if (mFolder)
      mFolder->stopSearch();
    mStopped = true;
    mBtnStop->setEnabled(false);
}

//-----------------------------------------------------------------------------
void SearchWindow::slotClose()
{
    accept();
}


//-----------------------------------------------------------------------------
void SearchWindow::closeEvent(TQCloseEvent *e)
{
    if (mFolder && mFolder->search() && mFolder->search()->running()) {
      mCloseRequested = true;
      //Cancel search in progress by setting the search folder search to
      //the null search
      mFolder->setSearch(new KMSearch());
      TQTimer::singleShot(0, this, TQT_SLOT(slotClose()));
    } else {
      KDialogBase::closeEvent(e);
    }
}

//-----------------------------------------------------------------------------
void SearchWindow::scheduleRename( const TQString &s)
{
    if (!s.isEmpty() ) {
      mRenameTimer.start(250, true);
      mSearchFolderOpenBtn->setEnabled(false);
    } else {
      mRenameTimer.stop();
      mSearchFolderOpenBtn->setEnabled(!s.isEmpty());
    }
}

//-----------------------------------------------------------------------------
void SearchWindow::renameSearchFolder()
{
    if (mFolder && (mFolder->folder()->name() != mSearchFolderEdt->text())) {
        int i = 1;
        TQString name =  mSearchFolderEdt->text();
        while (i < 100) {
            if (!kmkernel->searchFolderMgr()->find( name )) {
                mFolder->rename( name );
                kmkernel->searchFolderMgr()->contentsChanged();
                break;
            }
            name.setNum( i );
            name = mSearchFolderEdt->text() + " " + name;
            ++i;
        }
    }
    if ( mFolder )
      mSearchFolderOpenBtn->setEnabled(true);
}

void SearchWindow::openSearchFolder()
{
  Q_ASSERT( mFolder );
    renameSearchFolder();
    mKMMainWidget->slotSelectFolder( mFolder->folder() );
    slotClose();
}

//-----------------------------------------------------------------------------
void SearchWindow::folderInvalidated(KMFolder *folder)
{
    if (folder->storage() == mFolder) {
        mLbxMatches->clear();
        if (mFolder->search())
            connect(mFolder->search(), TQT_SIGNAL(finished(bool)),
                    this, TQT_SLOT(searchDone()));
        mTimer->start(200);
        enableGUI();
    }
}

//-----------------------------------------------------------------------------
KMMessage *SearchWindow::indexToMessage( TQListViewItem *item )
{
  if( !item ) {
    return 0;
  }

  KMFolder *folder;
  int msgIndex;
  KMMsgDict::instance()->getLocation( item->text( MSGID_COLUMN ).toUInt(),
                                      &folder, &msgIndex );

  if ( !folder || msgIndex < 0 ) {
    return 0;
  }

  mKMMainWidget->slotSelectFolder( folder );
  return folder->getMsg( msgIndex );
}

//-----------------------------------------------------------------------------
bool SearchWindow::slotShowMsg( TQListViewItem *item )
{
  KMMessage *message = indexToMessage( item );
  if ( message ) {
    mKMMainWidget->slotSelectMessage( message );
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------
void SearchWindow::slotViewSelectedMsg()
{
  slotViewMsg( mLbxMatches->currentItem() );
}

//-----------------------------------------------------------------------------
bool SearchWindow::slotViewMsg( TQListViewItem *item )
{
  KMMessage *message = indexToMessage( item );
  if ( message ) {
    mKMMainWidget->slotMsgActivated( message );
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------
void SearchWindow::slotCurrentChanged(TQListViewItem *item)
{
  mSearchResultOpenBtn->setEnabled(item!=0);
}

//-----------------------------------------------------------------------------
void SearchWindow::enableGUI()
{
    KMSearch const *search = (mFolder) ? (mFolder->search()) : 0;
    bool searching = (search) ? (search->running()) : false;
    actionButton(KDialogBase::Close)->setEnabled(!searching);
    mCbxFolders->setEnabled(!searching && !mChkbxAllFolders->isChecked());
    mChkSubFolders->setEnabled(!searching && !mChkbxAllFolders->isChecked());
    mChkbxAllFolders->setEnabled(!searching);
    mChkbxSpecificFolders->setEnabled(!searching);
    mPatternEdit->setEnabled(!searching);
    mBtnSearch->setEnabled(!searching);
    mBtnStop->setEnabled(searching);
}


//-----------------------------------------------------------------------------
KMMessageList SearchWindow::selectedMessages()
{
    KMMessageList msgList;
    KMFolder* folder = 0;
    int msgIndex = -1;
    for (TQListViewItemIterator it(mLbxMatches); it.current(); it++)
        if (it.current()->isSelected()) {
            KMMsgDict::instance()->getLocation((*it)->text(MSGID_COLUMN).toUInt(),
                                           &folder, &msgIndex);
            if (folder && msgIndex >= 0)
                msgList.append(folder->getMsgBase(msgIndex));
        }
    return msgList;
}

//-----------------------------------------------------------------------------
KMMessage* SearchWindow::message()
{
    TQListViewItem *item = mLbxMatches->currentItem();
    KMFolder* folder = 0;
    int msgIndex = -1;
    if (!item)
        return 0;
    KMMsgDict::instance()->getLocation(item->text(MSGID_COLUMN).toUInt(),
                                   &folder, &msgIndex);
    if (!folder || msgIndex < 0)
        return 0;

    return folder->getMsg(msgIndex);
}

//-----------------------------------------------------------------------------
void SearchWindow::moveSelectedToFolder( int menuId )
{
    KMFolder *dest = mMenuToFolder[menuId];
    if (!dest)
        return;

    KMMessageList msgList = selectedMessages();
    KMCommand *command = new KMMoveCommand( dest, msgList );
    command->start();
}

//-----------------------------------------------------------------------------
void SearchWindow::copySelectedToFolder( int menuId )
{
    KMFolder *dest = mMenuToFolder[menuId];
    if (!dest)
        return;

    KMMessageList msgList = selectedMessages();
    KMCommand *command = new KMCopyCommand( dest, msgList );
    command->start();
}

//-----------------------------------------------------------------------------
void SearchWindow::updateContextMenuActions()
{
    int count = selectedMessages().count();
    bool single_actions = count == 1;
    mReplyAction->setEnabled( single_actions );
    mReplyAllAction->setEnabled( single_actions );
    mReplyListAction->setEnabled( single_actions );
    mPrintAction->setEnabled( single_actions );
    mForwardDigestAction->setEnabled( !single_actions );
    mRedirectAction->setEnabled( single_actions );
    mCopyAction->setEnabled( count > 0 );
    mCutAction->setEnabled( count > 0 );
}

//-----------------------------------------------------------------------------
void SearchWindow::slotContextMenuRequested( TQListViewItem *lvi, const TQPoint &, int )
{
    if (!lvi)
        return;
    mLbxMatches->setSelected( lvi, true );
    mLbxMatches->setCurrentItem( lvi );
    // FIXME is this ever unGetMsg()'d?
    if (!message())
        return;
    TQPopupMenu *menu = new TQPopupMenu(this);
    updateContextMenuActions();

    mMenuToFolder.clear();
    TQPopupMenu *msgMoveMenu = new TQPopupMenu(menu);
    mKMMainWidget->folderTree()->folderToPopupMenu( KMFolderTree::MoveMessage,
        this, &mMenuToFolder, msgMoveMenu );
    TQPopupMenu *msgCopyMenu = new TQPopupMenu(menu);
    mKMMainWidget->folderTree()->folderToPopupMenu( KMFolderTree::CopyMessage,
        this, &mMenuToFolder, msgCopyMenu );

    // show most used actions
    mReplyAction->plug(menu);
    mReplyAllAction->plug(menu);
    mReplyListAction->plug(menu);
    mForwardActionMenu->plug(menu);
    menu->insertSeparator();
    mCopyAction->plug(menu);
    mCutAction->plug(menu);
    menu->insertItem(i18n("&Copy To"), msgCopyMenu);
    menu->insertItem(i18n("&Move To"), msgMoveMenu);
    menu->insertSeparator();
    mSaveAsAction->plug(menu);
    mSaveAtchAction->plug(menu);
    mPrintAction->plug(menu);
    menu->insertSeparator();
    mClearAction->plug(menu);
    menu->exec (TQCursor::pos(), 0);
    delete menu;
}

//-----------------------------------------------------------------------------
void SearchWindow::slotClearSelection()
{
    mLbxMatches->clearSelection();
}

//-----------------------------------------------------------------------------
void SearchWindow::slotReplyToMsg()
{
    KMCommand *command = new KMReplyToCommand(this, message());
    command->start();
}

//-----------------------------------------------------------------------------
void SearchWindow::slotReplyAllToMsg()
{
    KMCommand *command = new KMReplyToAllCommand(this, message());
    command->start();
}

//-----------------------------------------------------------------------------
void SearchWindow::slotReplyListToMsg()
{
    KMCommand *command = new KMReplyListCommand(this, message());
    command->start();
}

//-----------------------------------------------------------------------------
void SearchWindow::slotForwardInlineMsg()
{
    KMCommand *command = new KMForwardInlineCommand(this, selectedMessages());
    command->start();
}

//-----------------------------------------------------------------------------
void SearchWindow::slotForwardAttachedMsg()
{
    KMCommand *command = new KMForwardAttachedCommand(this, selectedMessages());
    command->start();
}

//-----------------------------------------------------------------------------
void SearchWindow::slotForwardDigestMsg()
{
    KMCommand *command = new KMForwardDigestCommand(this, selectedMessages());
    command->start();
}

//-----------------------------------------------------------------------------
void SearchWindow::slotRedirectMsg()
{
    KMCommand *command = new KMRedirectCommand(this, message());
    command->start();
}

//-----------------------------------------------------------------------------
void SearchWindow::slotSaveMsg()
{
    KMSaveMsgCommand *saveCommand = new KMSaveMsgCommand(this,
                                                         selectedMessages());
    if (saveCommand->url().isEmpty())
        delete saveCommand;
    else
        saveCommand->start();
}
//-----------------------------------------------------------------------------
void SearchWindow::slotSaveAttachments()
{
    KMSaveAttachmentsCommand *saveCommand = new KMSaveAttachmentsCommand(this,
                                                                         selectedMessages());
    saveCommand->start();
}


//-----------------------------------------------------------------------------
void SearchWindow::slotPrintMsg()
{
    KMCommand *command = new KMPrintCommand(this, message());
    command->start();
}

void SearchWindow::slotCopyMsgs()
{
  TQValueList<Q_UINT32> list = MessageCopyHelper::serNumListFromMsgList( selectedMessages() );
  mKMMainWidget->headers()->setCopiedMessages( list, false );
}

void SearchWindow::slotCutMsgs()
{
  TQValueList<Q_UINT32> list = MessageCopyHelper::serNumListFromMsgList( selectedMessages() );
  mKMMainWidget->headers()->setCopiedMessages( list, true );
}


void SearchWindow::setSearchPattern( const KMSearchPattern& pattern )
{
    *mSearchPattern = pattern;
    mPatternEdit->setSearchPattern( mSearchPattern );
}

} // namespace KMail
#include "searchwindow.moc"
