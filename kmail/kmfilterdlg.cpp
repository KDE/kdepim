// -*- mode: C++; c-file-style: "gnu" -*-
// kmfilterdlg.cpp
// Author: Marc Mutz <Marc@Mutz.com>
// based on work by Stefan Taferner <taferner@kde.org>
// This code is under the GPL

#include <config.h>
#include "kmfilterdlg.h"

// other KMail headers:
#include "kmsearchpatternedit.h"
#include "kmfiltermgr.h"
#include "kmmainwidget.h"
#include "accountmanager.h"
using KMail::AccountManager;
#include "filterimporterexporter.h"
using KMail::FilterImporterExporter;
#include "foldersetselector.h"
#include "globalsettings.h"

// other KDE headers:
#include <kmessagebox.h>
#include <kdebug.h>
#include <klocale.h>
#include <kinputdialog.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kwin.h>
#include <kconfig.h>
#include <kicondialog.h>
#include <kkeybutton.h>
#include <klistview.h>
#include <kpushbutton.h>

// other Qt headers:
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqcombobox.h>
#include <tqwidgetstack.h>
#include <tqtooltip.h>
#include <tqwhatsthis.h>
#include <tqcheckbox.h>
#include <tqhbox.h>
#include <tqvalidator.h>
#include <tqtabwidget.h>

// other headers:
#include <assert.h>

using namespace KMail;


// What's this help texts
const char * _wt_filterlist =
I18N_NOOP( "<qt><p>This is the list of defined filters. "
	   "They are processed top-to-bottom.</p>"
	   "<p>Click on any filter to edit it "
	   "using the controls in the right-hand half "
	   "of the dialog.</p></qt>" );
const char * _wt_filterlist_new =
I18N_NOOP( "<qt><p>Click this button to create a new filter.</p>"
	   "<p>The filter will be inserted just before the currently-"
	   "selected one, but you can always change that "
	   "later on.</p>"
	   "<p>If you have clicked this button accidentally, you can undo this "
	   "by clicking on the <em>Delete</em> button.</p></qt>" );
const char * _wt_filterlist_copy =
I18N_NOOP( "<qt><p>Click this button to copy a filter.</p>"
	   "<p>If you have clicked this button accidentally, you can undo this "
	   "by clicking on the <em>Delete</em> button.</p></qt>" );
const char * _wt_filterlist_delete =
I18N_NOOP( "<qt><p>Click this button to <em>delete</em> the currently-"
	   "selected filter from the list above.</p>"
	   "<p>There is no way to get the filter back once "
	   "it is deleted, but you can always leave the "
	   "dialog by clicking <em>Cancel</em> to discard the "
	   "changes made.</p></qt>" );
const char * _wt_filterlist_top =
I18N_NOOP( "<qt><p>Click this button to move the currently-"
	   "selected filter to the <em>top</em> of the list above.</p>"
	   "<p>This is useful since the order of the filters in the list "
	   "determines the order in which they are tried on messages: "
	   "The topmost filter gets tried first.</p></qt>" );
const char * _wt_filterlist_up =
I18N_NOOP( "<qt><p>Click this button to move the currently-"
	   "selected filter <em>up</em> one in the list above.</p>"
	   "<p>This is useful since the order of the filters in the list "
	   "determines the order in which they are tried on messages: "
	   "The topmost filter gets tried first.</p>"
	   "<p>If you have clicked this button accidentally, you can undo this "
	   "by clicking on the <em>Down</em> button.</p></qt>" );
const char * _wt_filterlist_down =
I18N_NOOP( "<qt><p>Click this button to move the currently-"
	   "selected filter <em>down</em> one in the list above.</p>"
	   "<p>This is useful since the order of the filters in the list "
	   "determines the order in which they are tried on messages: "
	   "The topmost filter gets tried first.</p>"
	   "<p>If you have clicked this button accidentally, you can undo this "
	   "by clicking on the <em>Up</em> button.</p></qt>" );
const char * _wt_filterlist_bot =
I18N_NOOP( "<qt><p>Click this button to move the currently-"
	   "selected filter to the <em>bottom</em> of the list above.</p>"
	   "<p>This is useful since the order of the filters in the list "
	   "determines the order in which they are tried on messages: "
	   "The topmost filter gets tried first.</p></qt>" );
const char * _wt_filterlist_rename =
I18N_NOOP( "<qt><p>Click this button to rename the currently-selected filter.</p>"
	   "<p>Filters are named automatically, as long as they start with "
	   "\"&lt;\".</p>"
	   "<p>If you have renamed a filter accidentally and want automatic "
	   "naming back, click this button and select <em>Clear</em> followed "
	   "by <em>OK</em> in the appearing dialog.</p></qt>" );
const char * _wt_filterdlg_showLater =
I18N_NOOP( "<qt><p>Check this button to force the confirmation dialog to be "
	   "displayed.</p><p>This is useful if you have defined a ruleset that tags "
           "messages to be downloaded later. Without the possibility to force "
           "the dialog popup, these messages could never be downloaded if no "
           "other large messages were waiting on the server, or if you wanted to "
           "change the ruleset to tag the messages differently.</p></qt>" );

// The anchor of the filter dialog's help.
const char * KMFilterDlgHelpAnchor =  "filters-id" ;
const char * KMPopFilterDlgHelpAnchor =  "popfilters-id" ;

//=============================================================================
//
// class KMFilterDlg (the filter dialog)
//
//=============================================================================

KMFilterDlg::KMFilterDlg(TQWidget* parent, const char* name, bool popFilter, bool createDummyFilter )
  : KDialogBase( parent, name,  false  /* modality */,
		 (popFilter)? i18n("POP3 Filter Rules"): i18n("Filter Rules") /* caption*/,
		 Help|Ok|Apply|Cancel|User1|User2 /* button mask */,
		 Ok /* default btn */,  false  /* separator */),
  bPopFilter(popFilter)
{
  KWin::setIcons( winId(), kapp->icon(), kapp->miniIcon() );
  setHelp( (bPopFilter)? KMPopFilterDlgHelpAnchor: KMFilterDlgHelpAnchor );
  setButtonText( User1, i18n("Import") );
  setButtonText( User2, i18n("Export") );
  connect( this, TQT_SIGNAL(user1Clicked()),
           this, TQT_SLOT( slotImportFilters()) );
  connect( this, TQT_SIGNAL(user2Clicked()),
           this, TQT_SLOT( slotExportFilters()) );

  TQWidget *w = new TQWidget( this );
  setMainWidget( w );
  TQHBoxLayout *topLayout = new TQHBoxLayout( w, 0, spacingHint(), "topLayout" );
  TQHBoxLayout *hbl = topLayout;
  TQVBoxLayout *vbl2 = 0;
  TQWidget *page1 = 0;
  TQWidget *page2 = 0;

  mFilterList = new KMFilterListBox( i18n("Available Filters"), w, 0, bPopFilter);
  topLayout->addWidget( mFilterList, 1 /*stretch*/ );

  if(!bPopFilter) {
    TQTabWidget *tabWidget = new TQTabWidget( w, "kmfd_tab" );
    tabWidget->setMargin( KDialog::marginHint() );
    topLayout->addWidget( tabWidget );

    page1 = new TQWidget( tabWidget );
    tabWidget->addTab( page1, i18n("&General") );
    hbl = new TQHBoxLayout( page1, 0, spacingHint(), "kmfd_hbl" );

    page2 = new TQWidget( tabWidget );
    tabWidget->addTab( page2, i18n("A&dvanced") );
    vbl2 = new TQVBoxLayout( page2, 0, spacingHint(), "kmfd_vbl2" );
  }

  TQVBoxLayout *vbl = new TQVBoxLayout( hbl, spacingHint(), "kmfd_vbl" );
  hbl->setStretchFactor( vbl, 2 );

  mPatternEdit = new KMSearchPatternEdit( i18n("Filter Criteria"), bPopFilter ? w : page1 , "spe", bPopFilter);
  vbl->addWidget( mPatternEdit, 0, Qt::AlignTop );

  if(bPopFilter){
    mActionGroup = new KMPopFilterActionWidget( i18n("Filter Action"), w );
    vbl->addWidget( mActionGroup, 0, Qt::AlignTop );

    mGlobalsBox = new TQVGroupBox(i18n("Global Options"), w);
    mShowLaterBtn = new TQCheckBox(i18n("Always &show matched 'Download Later' messages in confirmation dialog"), mGlobalsBox);
    TQWhatsThis::add( mShowLaterBtn, i18n(_wt_filterdlg_showLater) );
    vbl->addWidget( mGlobalsBox, 0, Qt::AlignTop );
  }
  else {
    TQGroupBox *agb = new TQGroupBox( 1 /*column*/, Vertical, i18n("Filter Actions"), page1 );
    mActionLister = new KMFilterActionWidgetLister( agb );
    vbl->addWidget( agb, 0, Qt::AlignTop );

    mAdvOptsGroup = new TQGroupBox ( 1 /*columns*/, Vertical,
				    i18n("Advanced Options"), page2);
    {
      TQWidget *adv_w = new TQWidget( mAdvOptsGroup );
      TQGridLayout *gl = new TQGridLayout( adv_w, 8 /*rows*/, 3 /*cols*/,
      				         0 /*border*/, spacingHint() );

      TQVBoxLayout *vbl3 = new TQVBoxLayout( gl, spacingHint(), "vbl3" );
      vbl3->addStretch( 1 );
      mApplyOnIn = new TQCheckBox( i18n("Apply this filter to incoming messages:"), adv_w );
      vbl3->addWidget( mApplyOnIn );
      TQButtonGroup *bg = new TQButtonGroup( 0, "bg" );
      bg->setExclusive( true );
      mApplyOnForAll = new TQRadioButton( i18n("from all accounts"), adv_w );
      bg->insert( mApplyOnForAll );
      vbl3->addWidget( mApplyOnForAll );
      mApplyOnForTraditional = new TQRadioButton( i18n("from all but online IMAP accounts"), adv_w );
      bg->insert( mApplyOnForTraditional );
      vbl3->addWidget( mApplyOnForTraditional );
      mApplyOnForChecked = new TQRadioButton( i18n("from checked accounts only"), adv_w );
      bg->insert( mApplyOnForChecked );
      vbl3->addWidget( mApplyOnForChecked );
      vbl3->addStretch( 2 );

      mAccountList = new KListView( adv_w, "accountList" );
      mAccountList->addColumn( i18n("Account Name") );
      mAccountList->addColumn( i18n("Type") );
      mAccountList->setAllColumnsShowFocus( true );
      mAccountList->setFrameStyle( TQFrame::WinPanel + TQFrame::Sunken );
      mAccountList->setSorting( -1 );
      gl->addMultiCellWidget( mAccountList, 0, 3, 1, 3 );

      mApplyOnOut = new TQCheckBox( i18n("Apply this filter to &sent messages"), adv_w );
      gl->addMultiCellWidget( mApplyOnOut, 4, 4, 0, 3 );

      mApplyOnCtrlJ = new TQCheckBox( i18n("Apply this filter on manual &filtering"), adv_w );
      gl->addMultiCellWidget( mApplyOnCtrlJ, 5, 5, 0, 3 );

      mStopProcessingHere = new TQCheckBox( i18n("If this filter &matches, stop processing here"), adv_w );
      gl->addMultiCellWidget( mStopProcessingHere,
			      6, 6, /*from to row*/
  			      0, 3 /*from to col*/ );
      mConfigureShortcut = new TQCheckBox( i18n("Add this filter to the Apply Filter menu"), adv_w );
      gl->addMultiCellWidget( mConfigureShortcut, 7, 7, 0, 1 );
      TQLabel *keyButtonLabel = new TQLabel( i18n( "Shortcut:" ), adv_w );
      keyButtonLabel->setAlignment( AlignVCenter | AlignRight );
      gl->addMultiCellWidget( keyButtonLabel, 7, 7, 2, 2 );
      mKeyButton = new KKeyButton( adv_w, "FilterShortcutSelector" );
      gl->addMultiCellWidget( mKeyButton, 7, 7, 3, 3 );
      mKeyButton->setEnabled( false );
      mConfigureToolbar = new TQCheckBox( i18n("Additionally add this filter to the toolbar"), adv_w );
      gl->addMultiCellWidget( mConfigureToolbar, 8, 8, 0, 3 );
      mConfigureToolbar->setEnabled( false );

      TQHBox *hbox = new TQHBox( adv_w );
      mFilterActionLabel = new TQLabel( i18n( "Icon for this filter:" ),
                                       hbox );
      mFilterActionLabel->setEnabled( false );

      mFilterActionIconButton = new KIconButton( hbox );
      mFilterActionLabel->setBuddy( mFilterActionIconButton );
      mFilterActionIconButton->setIconType( KIcon::NoGroup, KIcon::Any, true );
      mFilterActionIconButton->setIconSize( 16 );
      mFilterActionIconButton->setIcon( "gear" );
      mFilterActionIconButton->setEnabled( false );

      gl->addMultiCellWidget( hbox, 9, 9, 0, 3 );
    }
    vbl2->addWidget( mAdvOptsGroup, 0, Qt::AlignTop );
  }
  // spacer:
  vbl->addStretch( 1 );

  // load the filter parts into the edit widgets
  connect( mFilterList, TQT_SIGNAL(filterSelected(KMFilter*)),
	   this, TQT_SLOT(slotFilterSelected(KMFilter*)) );

  if (bPopFilter){
    // set the state of the global setting 'show later msgs'
    connect( mShowLaterBtn, TQT_SIGNAL(toggled(bool)),
             mFilterList, TQT_SLOT(slotShowLaterToggled(bool)));

    // set the action in the filter when changed
    connect( mActionGroup, TQT_SIGNAL(actionChanged(const KMPopFilterAction)),
	     this, TQT_SLOT(slotActionChanged(const KMPopFilterAction)) );
  } else {
    // transfer changes from the 'Apply this filter on...'
    // combo box to the filter
    connect( mApplyOnIn, TQT_SIGNAL(clicked()),
  	     this, TQT_SLOT(slotApplicabilityChanged()) );
    connect( mApplyOnForAll, TQT_SIGNAL(clicked()),
  	     this, TQT_SLOT(slotApplicabilityChanged()) );
    connect( mApplyOnForTraditional, TQT_SIGNAL(clicked()),
  	     this, TQT_SLOT(slotApplicabilityChanged()) );
    connect( mApplyOnForChecked, TQT_SIGNAL(clicked()),
  	     this, TQT_SLOT(slotApplicabilityChanged()) );
    connect( mApplyOnOut, TQT_SIGNAL(clicked()),
  	     this, TQT_SLOT(slotApplicabilityChanged()) );
    connect( mApplyOnCtrlJ, TQT_SIGNAL(clicked()),
  	     this, TQT_SLOT(slotApplicabilityChanged()) );
    connect( mAccountList, TQT_SIGNAL(clicked(TQListViewItem*)),
  	     this, TQT_SLOT(slotApplicableAccountsChanged()) );
    connect( mAccountList, TQT_SIGNAL(spacePressed(TQListViewItem*)),
  	     this, TQT_SLOT(slotApplicableAccountsChanged()) );

    // transfer changes from the 'stop processing here'
    // check box to the filter
    connect( mStopProcessingHere, TQT_SIGNAL(toggled(bool)),
	     this, TQT_SLOT(slotStopProcessingButtonToggled(bool)) );

    connect( mConfigureShortcut, TQT_SIGNAL(toggled(bool)),
	     this, TQT_SLOT(slotConfigureShortcutButtonToggled(bool)) );

    connect( mKeyButton, TQT_SIGNAL( capturedShortcut( const KShortcut& ) ),
             this, TQT_SLOT( slotCapturedShortcutChanged( const KShortcut& ) ) );

    connect( mConfigureToolbar, TQT_SIGNAL(toggled(bool)),
	     this, TQT_SLOT(slotConfigureToolbarButtonToggled(bool)) );

    connect( mFilterActionIconButton, TQT_SIGNAL( iconChanged( TQString ) ),
             this, TQT_SLOT( slotFilterActionIconChanged( TQString ) ) );
  }

  // reset all widgets here
  connect( mFilterList, TQT_SIGNAL(resetWidgets()),
	   this, TQT_SLOT(slotReset()) );

  connect( mFilterList, TQT_SIGNAL( applyWidgets() ),
           this, TQT_SLOT( slotUpdateFilter() ) );

  // support auto-naming the filter
  connect( mPatternEdit, TQT_SIGNAL(maybeNameChanged()),
	   mFilterList, TQT_SLOT(slotUpdateFilterName()) );

  // apply changes on 'Apply'
  connect( this, TQT_SIGNAL(applyClicked()),
	   mFilterList, TQT_SLOT(slotApplyFilterChanges()) );

  // apply changes on 'OK'
  connect( this, TQT_SIGNAL(okClicked()),
	   mFilterList, TQT_SLOT(slotApplyFilterChanges()) );

  // save dialog size on 'OK'
  connect( this, TQT_SIGNAL(okClicked()),
	   this, TQT_SLOT(slotSaveSize()) );

  // destruct the dialog on OK, close and Cancel
  connect( this, TQT_SIGNAL(finished()),
	   this, TQT_SLOT(slotFinished()) );

  KConfigGroup geometry( KMKernel::config(), "Geometry");
  const char * configKey
    = bPopFilter ? "popFilterDialogSize" : "filterDialogSize";
  if ( geometry.hasKey( configKey ) )
    resize( geometry.readSizeEntry( configKey ) );
  else
    adjustSize();

  // load the filter list (emits filterSelected())
  mFilterList->loadFilterList( createDummyFilter );
}

void KMFilterDlg::slotFinished() {
	delayedDestruct();
}

void KMFilterDlg::slotSaveSize() {
  KConfigGroup geometry( KMKernel::config(), "Geometry" );
  geometry.writeEntry( bPopFilter ? "popFilterDialogSize" : "filterDialogSize", size() );
}

/** Set action of popFilter */
void KMFilterDlg::slotActionChanged(const KMPopFilterAction aAction)
{
  mFilter->setAction(aAction);
}

void KMFilterDlg::slotFilterSelected( KMFilter* aFilter )
{
  assert( aFilter );

  if (bPopFilter){
    mActionGroup->setAction( aFilter->action() );
    mGlobalsBox->setEnabled( true );
    mShowLaterBtn->setChecked(mFilterList->showLaterMsgs());
  } else {
    mActionLister->setActionList( aFilter->actions() );

    mAdvOptsGroup->setEnabled( true );
  }

  mPatternEdit->setSearchPattern( aFilter->pattern() );
  mFilter = aFilter;

  if (!bPopFilter) {
    kdDebug(5006) << "apply on inbound == "
		  << aFilter->applyOnInbound() << endl;
    kdDebug(5006) << "apply on outbound == "
		  << aFilter->applyOnOutbound() << endl;
    kdDebug(5006) << "apply on explicit == "
		  << aFilter->applyOnExplicit() << endl;

    // NOTE: setting these values activates the slot that sets them in
    // the filter! So make sure we have the correct values _before_ we
    // set the first one:
    const bool applyOnIn = aFilter->applyOnInbound();
    const bool applyOnForAll = aFilter->applicability() == KMFilter::All;
    const bool applyOnTraditional = aFilter->applicability() == KMFilter::ButImap;
    const bool applyOnOut = aFilter->applyOnOutbound();
    const bool applyOnExplicit = aFilter->applyOnExplicit();
    const bool stopHere = aFilter->stopProcessingHere();
    const bool configureShortcut = aFilter->configureShortcut();
    const bool configureToolbar = aFilter->configureToolbar();
    const TQString icon = aFilter->icon();
    const KShortcut shortcut( aFilter->shortcut() );

    mApplyOnIn->setChecked( applyOnIn );
    mApplyOnForAll->setEnabled( applyOnIn );
    mApplyOnForTraditional->setEnabled( applyOnIn );
    mApplyOnForChecked->setEnabled( applyOnIn );
    mApplyOnForAll->setChecked( applyOnForAll );
    mApplyOnForTraditional->setChecked( applyOnTraditional );
    mApplyOnForChecked->setChecked( !applyOnForAll && !applyOnTraditional );
    mAccountList->setEnabled( mApplyOnForChecked->isEnabled() && mApplyOnForChecked->isChecked() );
    slotUpdateAccountList();
    mApplyOnOut->setChecked( applyOnOut );
    mApplyOnCtrlJ->setChecked( applyOnExplicit );
    mStopProcessingHere->setChecked( stopHere );
    mConfigureShortcut->setChecked( configureShortcut );
    mKeyButton->setShortcut( shortcut, false );
    mConfigureToolbar->setChecked( configureToolbar );
    mFilterActionIconButton->setIcon( icon );
  }
}

void KMFilterDlg::slotReset()
{
  mFilter = 0;
  mPatternEdit->reset();

  if(bPopFilter) {
    mActionGroup->reset();
    mGlobalsBox->setEnabled( false );
  } else {
    mActionLister->reset();
    mAdvOptsGroup->setEnabled( false );
    slotUpdateAccountList();
  }
}

void KMFilterDlg::slotUpdateFilter()
{
  mPatternEdit->updateSearchPattern();
  if ( !bPopFilter ) {
    mActionLister->updateActionList();
  }
}

void KMFilterDlg::slotApplicabilityChanged()
{
  if ( mFilter ) {
    mFilter->setApplyOnInbound( mApplyOnIn->isChecked() );
    mFilter->setApplyOnOutbound( mApplyOnOut->isChecked() );
    mFilter->setApplyOnExplicit( mApplyOnCtrlJ->isChecked() );
    if ( mApplyOnForAll->isChecked() )
      mFilter->setApplicability( KMFilter::All );
    else if ( mApplyOnForTraditional->isChecked() )
      mFilter->setApplicability( KMFilter::ButImap );
    else if ( mApplyOnForChecked->isChecked() )
      mFilter->setApplicability( KMFilter::Checked );

    mApplyOnForAll->setEnabled( mApplyOnIn->isChecked() );
    mApplyOnForTraditional->setEnabled(  mApplyOnIn->isChecked() );
    mApplyOnForChecked->setEnabled( mApplyOnIn->isChecked() );
    mAccountList->setEnabled( mApplyOnForChecked->isEnabled() && mApplyOnForChecked->isChecked() );

    // Advanced tab functionality - Update list of accounts this filter applies to
    TQListViewItemIterator it( mAccountList );
    while ( it.current() ) {
      TQCheckListItem *item = dynamic_cast<TQCheckListItem*>( it.current() );
      if (item) {
	int id = item->text( 2 ).toInt();
	  item->setOn( mFilter->applyOnAccount( id ) );
      }
      ++it;
    }

    kdDebug(5006) << "KMFilterDlg: setting filter to be applied at "
                  << ( mFilter->applyOnInbound() ? "incoming " : "" )
                  << ( mFilter->applyOnOutbound() ? "outgoing " : "" )
                  << ( mFilter->applyOnExplicit() ? "explicit CTRL-J" : "" )
                  << endl;
  }
}

void KMFilterDlg::slotApplicableAccountsChanged()
{
  if ( mFilter && mApplyOnForChecked->isEnabled() && mApplyOnForChecked->isChecked() ) {
    // Advanced tab functionality - Update list of accounts this filter applies to
    TQListViewItemIterator it( mAccountList );
    while ( it.current() ) {
      TQCheckListItem *item = dynamic_cast<TQCheckListItem*>( it.current() );
      if (item) {
	int id = item->text( 2 ).toInt();
	mFilter->setApplyOnAccount( id, item->isOn() );
      }
      ++it;
    }
  }
}

void KMFilterDlg::slotStopProcessingButtonToggled( bool aChecked )
{
  if ( mFilter )
    mFilter->setStopProcessingHere( aChecked );
}

void KMFilterDlg::slotConfigureShortcutButtonToggled( bool aChecked )
{
  if ( mFilter ) {
    mFilter->setConfigureShortcut( aChecked );
    mKeyButton->setEnabled( aChecked );
    mConfigureToolbar->setEnabled( aChecked );
    mFilterActionIconButton->setEnabled( aChecked );
    mFilterActionLabel->setEnabled( aChecked );
  }
}

void KMFilterDlg::slotCapturedShortcutChanged( const KShortcut& sc )
{
  KShortcut mySc(sc);
  if ( mySc == mKeyButton->shortcut() ) return;
  // FIXME work around a problem when reseting the shortcut via the shortcut dialog
  // somehow the returned shortcut does not evaluate to true in KShortcut::isNull(),
  // so we additionally have to check for an empty string
  if ( mySc.isNull() || mySc.toString().isEmpty() )
    mySc.clear();
  if ( !mySc.isNull() && !( kmkernel->getKMMainWidget()->shortcutIsValid( mySc ) ) ) {
    TQString msg( i18n( "The selected shortcut is already used, "
          "please select a different one." ) );
    KMessageBox::sorry( this, msg );
  } else {
    mKeyButton->setShortcut( mySc, false );
    if ( mFilter )
      mFilter->setShortcut( mKeyButton->shortcut() );
  }
}

void KMFilterDlg::slotConfigureToolbarButtonToggled( bool aChecked )
{
  if ( mFilter )
    mFilter->setConfigureToolbar( aChecked );
}

void KMFilterDlg::slotFilterActionIconChanged( TQString icon )
{
  if ( mFilter )
    mFilter->setIcon( icon );
}

void KMFilterDlg::slotUpdateAccountList()
{
  mAccountList->clear();
  TQListViewItem *top = 0;
  for( KMAccount *a = kmkernel->acctMgr()->first(); a!=0;
       a = kmkernel->acctMgr()->next() ) {
    TQCheckListItem *listItem =
      new TQCheckListItem( mAccountList, top, a->name(), TQCheckListItem::CheckBox );
    listItem->setText( 1, a->type() );
    listItem->setText( 2, TQString( "%1" ).arg( a->id() ) );
    if ( mFilter )
      listItem->setOn( mFilter->applyOnAccount( a->id() ) );
    top = listItem;
  }

  TQListViewItem *listItem = mAccountList->firstChild();
  if ( listItem ) {
    mAccountList->setCurrentItem( listItem );
    mAccountList->setSelected( listItem, true );
  }
}

//=============================================================================
//
// class KMFilterListBox (the filter list manipulator)
//
//=============================================================================

KMFilterListBox::KMFilterListBox( const TQString & title, TQWidget *parent, const char* name, bool popFilter )
  : TQGroupBox( 1, Horizontal, title, parent, name ),
    bPopFilter(popFilter)
{
  mFilterList.setAutoDelete( true );
  mIdxSelItem = -1;

  //----------- the list box
  mListBox = new TQListBox(this);
  mListBox->setMinimumWidth(150);
  TQWhatsThis::add( mListBox, i18n(_wt_filterlist) );

  //----------- the first row of buttons
  TQHBox *hb = new TQHBox(this);
  hb->setSpacing(4);
  mBtnTop = new KPushButton( TQString::null, hb );
  mBtnTop->setAutoRepeat( true );
  mBtnTop->setIconSet( BarIconSet( "top", KIcon::SizeSmall ) );
  mBtnTop->setMinimumSize( mBtnTop->sizeHint() * 1.2 );
  mBtnUp = new KPushButton( TQString::null, hb );
  mBtnUp->setAutoRepeat( true );
  mBtnUp->setIconSet( BarIconSet( "up", KIcon::SizeSmall ) );
  mBtnUp->setMinimumSize( mBtnUp->sizeHint() * 1.2 );
  mBtnDown = new KPushButton( TQString::null, hb );
  mBtnDown->setAutoRepeat( true );
  mBtnDown->setIconSet( BarIconSet( "down", KIcon::SizeSmall ) );
  mBtnDown->setMinimumSize( mBtnDown->sizeHint() * 1.2 );
  mBtnBot = new KPushButton( TQString::null, hb );
  mBtnBot->setAutoRepeat( true );
  mBtnBot->setIconSet( BarIconSet( "bottom", KIcon::SizeSmall ) );
  mBtnBot->setMinimumSize( mBtnBot->sizeHint() * 1.2 );
  TQToolTip::add( mBtnTop, i18n("Top") );
  TQToolTip::add( mBtnUp, i18n("Up") );
  TQToolTip::add( mBtnDown, i18n("Down") );
  TQToolTip::add( mBtnBot, i18n("Bottom") );
  TQWhatsThis::add( mBtnTop, i18n(_wt_filterlist_top) );
  TQWhatsThis::add( mBtnUp, i18n(_wt_filterlist_up) );
  TQWhatsThis::add( mBtnDown, i18n(_wt_filterlist_down) );
  TQWhatsThis::add( mBtnBot, i18n(_wt_filterlist_bot) );

  //----------- the second row of buttons
  hb = new TQHBox(this);
  hb->setSpacing(4);
  mBtnNew = new TQPushButton( TQString::null, hb );
  mBtnNew->setPixmap( BarIcon( "filenew", KIcon::SizeSmall ) );
  mBtnNew->setMinimumSize( mBtnNew->sizeHint() * 1.2 );
  mBtnCopy = new TQPushButton( TQString::null, hb );
  mBtnCopy->setIconSet( BarIconSet( "editcopy", KIcon::SizeSmall ) );
  mBtnCopy->setMinimumSize( mBtnCopy->sizeHint() * 1.2 );
  mBtnDelete = new TQPushButton( TQString::null, hb );
  mBtnDelete->setIconSet( BarIconSet( "editdelete", KIcon::SizeSmall ) );
  mBtnDelete->setMinimumSize( mBtnDelete->sizeHint() * 1.2 );
  mBtnRename = new TQPushButton( i18n("Rename..."), hb );
  TQToolTip::add( mBtnNew, i18n("New") );
  TQToolTip::add( mBtnCopy, i18n("Copy") );
  TQToolTip::add( mBtnDelete, i18n("Delete"));
  TQWhatsThis::add( mBtnNew, i18n(_wt_filterlist_new) );
  TQWhatsThis::add( mBtnCopy, i18n(_wt_filterlist_copy) );
  TQWhatsThis::add( mBtnDelete, i18n(_wt_filterlist_delete) );
  TQWhatsThis::add( mBtnRename, i18n(_wt_filterlist_rename) );

  // third row
  if ( !popFilter ) {
    hb = new TQHBox( this );
    hb->setSpacing( 4 );
    TQPushButton *btn = new TQPushButton( i18n("Select Source Folders"), hb );
    connect( btn, TQT_SIGNAL(clicked()), TQT_SLOT(slotSelectSourceFolders()) );
  }


  //----------- now connect everything
  connect( mListBox, TQT_SIGNAL(highlighted(int)),
	   this, TQT_SLOT(slotSelected(int)) );
  connect( mListBox, TQT_SIGNAL( doubleClicked ( TQListBoxItem * )),
           this, TQT_SLOT( slotRename()) );
  connect( mBtnTop, TQT_SIGNAL(clicked()),
	   this, TQT_SLOT(slotTop()) );
  connect( mBtnUp, TQT_SIGNAL(clicked()),
	   this, TQT_SLOT(slotUp()) );
  connect( mBtnDown, TQT_SIGNAL(clicked()),
	   this, TQT_SLOT(slotDown()) );
  connect( mBtnBot, TQT_SIGNAL(clicked()),
	   this, TQT_SLOT(slotBottom()) );
  connect( mBtnNew, TQT_SIGNAL(clicked()),
	   this, TQT_SLOT(slotNew()) );
  connect( mBtnCopy, TQT_SIGNAL(clicked()),
	   this, TQT_SLOT(slotCopy()) );
  connect( mBtnDelete, TQT_SIGNAL(clicked()),
	   this, TQT_SLOT(slotDelete()) );
  connect( mBtnRename, TQT_SIGNAL(clicked()),
	   this, TQT_SLOT(slotRename()) );

  // the dialog should call loadFilterList()
  // when all signals are connected.
  enableControls();
}


void KMFilterListBox::createFilter( const TQCString & field,
				    const TQString & value )
{
  KMSearchRule *newRule = KMSearchRule::createInstance( field, KMSearchRule::FuncContains, value );

  KMFilter *newFilter = new KMFilter(0, bPopFilter);
  newFilter->pattern()->append( newRule );
  newFilter->pattern()->setName( TQString("<%1>:%2").arg( field ).arg( value) );

  KMFilterActionDesc *desc = (*kmkernel->filterActionDict())["transfer"];
  if ( desc )
    newFilter->actions()->append( desc->create() );

  insertFilter( newFilter );
  enableControls();
}

bool KMFilterListBox::showLaterMsgs()
{
	return mShowLater;
}

void KMFilterListBox::slotUpdateFilterName()
{
  KMSearchPattern *p = mFilterList.at(mIdxSelItem)->pattern();
  if ( !p ) return;

  TQString shouldBeName = p->name();
  TQString displayedName = mListBox->text( mIdxSelItem );

  if ( shouldBeName.stripWhiteSpace().isEmpty() ) {
    mFilterList.at(mIdxSelItem)->setAutoNaming( true );
  }

  if ( mFilterList.at(mIdxSelItem)->isAutoNaming() ) {
    // auto-naming of patterns
    if ( !p->isEmpty() && p->first() && !p->first()->field().stripWhiteSpace().isEmpty() )
      shouldBeName = TQString( "<%1>: %2" ).arg( p->first()->field() ).arg( p->first()->contents() );
    else
      shouldBeName = "<" + i18n("unnamed") + ">";
    p->setName( shouldBeName );
  }

  if ( displayedName == shouldBeName ) return;

  mListBox->blockSignals( true );
  mListBox->changeItem( shouldBeName, mIdxSelItem );
  mListBox->blockSignals( false );
}

void KMFilterListBox::slotShowLaterToggled(bool aOn)
{
  mShowLater = aOn;
}

void KMFilterListBox::slotApplyFilterChanges()
{
  if ( mIdxSelItem >= 0 ) {
    emit applyWidgets();
    slotSelected( mListBox->currentItem() );
  }

  // by now all edit widgets should have written back
  // their widget's data into our filter list.

  KMFilterMgr *fm;
  if (bPopFilter)
    fm = kmkernel->popFilterMgr();
  else
    fm = kmkernel->filterMgr();

  TQValueList<KMFilter*> newFilters = filtersForSaving();

  if (bPopFilter)
    fm->setShowLaterMsgs(mShowLater);

  fm->setFilters( newFilters );
  if (fm->atLeastOneOnlineImapFolderTarget()) {
    TQString str = i18n("At least one filter targets a folder on an online "
		       "IMAP account. Such filters will only be applied "
		       "when manually filtering and when filtering "
		       "incoming online IMAP mail.");
    KMessageBox::information( this, str, TQString::null,
			      "filterDlgOnlineImapCheck" );
  }
}

TQValueList<KMFilter*> KMFilterListBox::filtersForSaving() const
{
      const_cast<KMFilterListBox*>( this )->applyWidgets(); // signals aren't const
      TQValueList<KMFilter*> filters;
      TQStringList emptyFilters;
      TQPtrListIterator<KMFilter> it( mFilterList );
      for ( it.toFirst() ; it.current() ; ++it ) {
        KMFilter *f = new KMFilter( **it ); // deep copy
        f->purify();
        if ( !f->isEmpty() )
          // the filter is valid:
          filters.append( f );
        else {
          // the filter is invalid:
          emptyFilters << f->name();
          delete f;
        }
      }

      // report on invalid filters:
      if ( !emptyFilters.empty() ) {
        TQString msg = i18n("The following filters have not been saved because they "
                   "were invalid (e.g. containing no actions or no search "
                   "rules).");
        KMessageBox::informationList( 0, msg, emptyFilters, TQString::null,
                      "ShowInvalidFilterWarning" );
      }
      return filters;
}

void KMFilterListBox::slotSelected( int aIdx )
{
  mIdxSelItem = aIdx;
  // TQPtrList::at(i) will return 0 if i is out of range.
  KMFilter *f = mFilterList.at(aIdx);
  if ( f )
    emit filterSelected( f );
  else
    emit resetWidgets();
  enableControls();
}

void KMFilterListBox::slotNew()
{
  // just insert a new filter.
  insertFilter( new KMFilter(0, bPopFilter) );
  enableControls();
}

void KMFilterListBox::slotCopy()
{
  if ( mIdxSelItem < 0 ) {
    kdDebug(5006) << "KMFilterListBox::slotCopy called while no filter is selected, ignoring." << endl;
    return;
  }

  // make sure that all changes are written to the filter before we copy it
  emit applyWidgets();

  KMFilter *filter = mFilterList.at( mIdxSelItem );

  // enableControls should make sure this method is
  // never called when no filter is selected.
  assert( filter );

  // inserts a copy of the current filter.
  insertFilter( new KMFilter( *filter ) );
  enableControls();
}

void KMFilterListBox::slotDelete()
{
  if ( mIdxSelItem < 0 ) {
    kdDebug(5006) << "KMFilterListBox::slotDelete called while no filter is selected, ignoring." << endl;
    return;
  }

  int oIdxSelItem = mIdxSelItem;
  mIdxSelItem = -1;
  // unselect all
  mListBox->selectAll( false );
  // broadcast that all widgets let go
  // of the filter
  emit resetWidgets();

  // remove the filter from both the filter list...
  mFilterList.remove( oIdxSelItem );
  // and the listbox
  mListBox->removeItem( oIdxSelItem );

  int count = (int)mListBox->count();
  // and set the new current item.
  if ( count > oIdxSelItem )
    // oIdxItem is still a valid index
    mListBox->setSelected( oIdxSelItem, true );
  else if ( count )
    // oIdxSelIdx is no longer valid, but the
    // list box isn't empty
    mListBox->setSelected( count - 1, true );
  // the list is empty - keep index -1

  enableControls();
}

void KMFilterListBox::slotTop()
{
  if ( mIdxSelItem < 0 ) {
    kdDebug(5006) << "KMFilterListBox::slotTop called while no filter is selected, ignoring." << endl;
    return;
  }
  if ( mIdxSelItem == 0 ) {
    kdDebug(5006) << "KMFilterListBox::slotTop called while the _topmost_ filter is selected, ignoring." << endl;
    return;
  }

  swapFilters( mIdxSelItem, 0 );
  enableControls();
}

void KMFilterListBox::slotUp()
{
  if ( mIdxSelItem < 0 ) {
    kdDebug(5006) << "KMFilterListBox::slotUp called while no filter is selected, ignoring." << endl;
    return;
  }
  if ( mIdxSelItem == 0 ) {
    kdDebug(5006) << "KMFilterListBox::slotUp called while the _topmost_ filter is selected, ignoring." << endl;
    return;
  }

  swapNeighbouringFilters( mIdxSelItem, mIdxSelItem - 1 );
  enableControls();
}

void KMFilterListBox::slotDown()
{
  if ( mIdxSelItem < 0 ) {
    kdDebug(5006) << "KMFilterListBox::slotDown called while no filter is selected, ignoring." << endl;
    return;
  }
  if ( mIdxSelItem == (int)mListBox->count() - 1 ) {
    kdDebug(5006) << "KMFilterListBox::slotDown called while the _last_ filter is selected, ignoring." << endl;
    return;
  }

  swapNeighbouringFilters( mIdxSelItem, mIdxSelItem + 1);
  enableControls();
}

void KMFilterListBox::slotBottom()
{
  if ( mIdxSelItem < 0 ) {
    kdDebug(5006) << "KMFilterListBox::slotBottom called while no filter is selected, ignoring." << endl;
    return;
  }
  if ( mIdxSelItem == (int)mListBox->count() - 1 ) {
    kdDebug(5006) << "KMFilterListBox::slotBottom called while the _last_ filter is selected, ignoring." << endl;
    return;
  }

  swapFilters( mIdxSelItem, mListBox->count()-1 );
  enableControls();
}

void KMFilterListBox::slotRename()
{
  if ( mIdxSelItem < 0 ) {
    kdDebug(5006) << "KMFilterListBox::slotRename called while no filter is selected, ignoring." << endl;
    return;
  }

  bool okPressed =  false ;
  KMFilter *filter = mFilterList.at( mIdxSelItem );

  // enableControls should make sure this method is
  // never called when no filter is selected.
  assert( filter );

  // allow empty names - those will turn auto-naming on again
  TQValidator *validator = new TQRegExpValidator( TQRegExp( ".*" ), 0 );
  TQString newName = KInputDialog::getText
    (
     i18n("Rename Filter"),
     i18n("Rename filter \"%1\" to:\n(leave the field empty for automatic naming)")
        .arg( filter->pattern()->name() ) /*label*/,
     filter->pattern()->name() /* initial value */,
     &okPressed, topLevelWidget(), 0, validator
     );
  delete validator;

  if ( !okPressed ) return;

  if ( newName.isEmpty() ) {
    // bait for slotUpdateFilterName to
    // use automatic naming again.
    filter->pattern()->setName( "<>" );
    filter->setAutoNaming( true );
  } else {
    filter->pattern()->setName( newName );
    filter->setAutoNaming( false );
  }

  slotUpdateFilterName();
}

void KMFilterListBox::slotSelectSourceFolders()
{
  FolderSetSelector dlg( kmkernel->getKMMainWidget()->folderTree(), this );
  dlg.setCaption( i18n( "Select Folders to Filter" ) );
  if ( !GlobalSettings::filterSourceFolders().isEmpty() )
    dlg.setSelectedFolders( GlobalSettings::filterSourceFolders() );
  if ( dlg.exec() == TQDialog::Accepted ) {
    GlobalSettings::setFilterSourceFolders( dlg.selectedFolders() );
  }
}

void KMFilterListBox::enableControls()
{
  bool theFirst = ( mIdxSelItem == 0 );
  bool theLast = ( mIdxSelItem >= (int)mFilterList.count() - 1 );
  bool aFilterIsSelected = ( mIdxSelItem >= 0 );

  mBtnTop->setEnabled( aFilterIsSelected && !theFirst );
  mBtnUp->setEnabled( aFilterIsSelected && !theFirst );
  mBtnDown->setEnabled( aFilterIsSelected && !theLast );
  mBtnBot->setEnabled( aFilterIsSelected && !theLast );
  mBtnCopy->setEnabled( aFilterIsSelected );
  mBtnDelete->setEnabled( aFilterIsSelected );
  mBtnRename->setEnabled( aFilterIsSelected );

  if ( aFilterIsSelected )
    mListBox->ensureCurrentVisible();
}

void KMFilterListBox::loadFilterList( bool createDummyFilter )
{
  assert(mListBox);
  setEnabled( false );
  emit resetWidgets();
  // we don't want the insertion to
  // cause flicker in the edit widgets.
  blockSignals( true );

  // clear both lists
  mFilterList.clear();
  mListBox->clear();

  const KMFilterMgr *manager = 0;
  if(bPopFilter)
  {
    mShowLater = kmkernel->popFilterMgr()->showLaterMsgs();
    manager = kmkernel->popFilterMgr();
  }
  else
  {
    manager = kmkernel->filterMgr();
  }
  Q_ASSERT( manager );

  TQValueListConstIterator<KMFilter*> it;
  for ( it = manager->filters().constBegin() ; it != manager->filters().constEnd() ; ++it ) {
    mFilterList.append( new KMFilter( **it ) ); // deep copy
    mListBox->insertItem( (*it)->pattern()->name() );
  }

  blockSignals( false );
  setEnabled( true );

  // create an empty filter when there's none, to avoid a completely
  // disabled dialog (usability tests indicated that the new-filter
  // button is too hard to find that way):
  if ( !mListBox->count() && createDummyFilter )
    slotNew();

  if ( mListBox->count() > 0 )
    mListBox->setSelected( 0, true );

  enableControls();
}

void KMFilterListBox::insertFilter( KMFilter* aFilter )
{
  // must be really a filter...
  assert( aFilter );

  // if mIdxSelItem < 0, TQListBox::insertItem will append.
  mListBox->insertItem( aFilter->pattern()->name(), mIdxSelItem );
  if ( mIdxSelItem < 0 ) {
    // none selected -> append
    mFilterList.append( aFilter );
    mListBox->setSelected( mListBox->count() - 1, true );
    //    slotSelected( mListBox->count() - 1 );
  } else {
    // insert just before selected
    mFilterList.insert( mIdxSelItem, aFilter );
    mListBox->setSelected( mIdxSelItem, true );
    //    slotSelected( mIdxSelItem );
  }

}

void KMFilterListBox::appendFilter( KMFilter* aFilter )
{
    mFilterList.append( aFilter );
    mListBox->insertItem( aFilter->pattern()->name(), -1 );
}

void KMFilterListBox::swapNeighbouringFilters( int untouchedOne, int movedOne )
{
  // must be neighbours...
  assert( untouchedOne - movedOne == 1 || movedOne - untouchedOne == 1 );

  // untouchedOne is at idx. to move it down(up),
  // remove item at idx+(-)1 w/o deleting it.
  TQListBoxItem *item = mListBox->item( movedOne );
  mListBox->takeItem( item );
  // now selected item is at idx(idx-1), so
  // insert the other item at idx, ie. above(below).
  mListBox->insertItem( item, untouchedOne );

  KMFilter* filter = mFilterList.take( movedOne );
  mFilterList.insert( untouchedOne, filter );

  mIdxSelItem += movedOne - untouchedOne;
}

void KMFilterListBox::swapFilters( int from, int to )
{
  TQListBoxItem *item = mListBox->item( from );
  mListBox->takeItem( item );
  mListBox->insertItem( item, to );

  KMFilter* filter = mFilterList.take( from );
  mFilterList.insert( to, filter );

  mIdxSelItem = to;
  mListBox->setCurrentItem( mIdxSelItem );
  mListBox->setSelected( mIdxSelItem, true );
}

//=============================================================================
//
// class KMFilterActionWidget
//
//=============================================================================

KMFilterActionWidget::KMFilterActionWidget( TQWidget *parent, const char* name )
  : TQHBox( parent, name )
{
  int i;
  mActionList.setAutoDelete( true );

  mComboBox = new TQComboBox(  false , this );
  assert( mComboBox );
  mWidgetStack = new TQWidgetStack(this);
  assert( mWidgetStack );

  setSpacing( 4 );

  TQPtrListIterator<KMFilterActionDesc> it ( kmkernel->filterActionDict()->list() );
  for ( i=0, it.toFirst() ; it.current() ; ++it, ++i ) {
    //create an instance:
    KMFilterAction *a = (*it)->create();
    // append to the list of actions:
    mActionList.append( a );
    // add parameter widget to widget stack:
    mWidgetStack->addWidget( a->createParamWidget( mWidgetStack ), i );
    // add (i18n-ized) name to combo box
    mComboBox->insertItem( (*it)->label );
  }
  // widget for the case where no action is selected.
  mWidgetStack->addWidget( new TQLabel( i18n("Please select an action."), mWidgetStack ), i );
  mWidgetStack->raiseWidget(i);
  mComboBox->insertItem( " " );
  mComboBox->setCurrentItem(i);

  // don't show scroll bars.
  mComboBox->setSizeLimit( mComboBox->count() );
  // layout management:
  // o the combo box is not to be made larger than it's sizeHint(),
  //   the parameter widget should grow instead.
  // o the whole widget takes all space horizontally, but is fixed vertically.
  mComboBox->adjustSize();
  mComboBox->setSizePolicy( TQSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed ) );
  setSizePolicy( TQSizePolicy( TQSizePolicy::Preferred, TQSizePolicy::Fixed ) );
  updateGeometry();

  // redirect focus to the filter action combo box
  setFocusProxy( mComboBox );

  // now connect the combo box and the widget stack
  connect( mComboBox, TQT_SIGNAL(activated(int)),
	   mWidgetStack, TQT_SLOT(raiseWidget(int)) );
}

void KMFilterActionWidget::setAction( const KMFilterAction* aAction )
{
  int i=0;
  bool found =  false ;
  int count = mComboBox->count() - 1 ; // last entry is the empty one
  TQString label = ( aAction ) ? aAction->label() : TQString::null ;

  // find the index of typeOf(aAction) in mComboBox
  // and clear the other widgets on the way.
  for ( ; i < count ; i++ )
    if ( aAction && mComboBox->text(i) == label ) {
      //...set the parameter widget to the settings
      // of aAction...
      aAction->setParamWidgetValue( mWidgetStack->widget(i) );
      //...and show the correct entry of
      // the combo box
      mComboBox->setCurrentItem(i); // (mm) also raise the widget, but doesn't
      mWidgetStack->raiseWidget(i);
      found = true;
    } else // clear the parameter widget
      mActionList.at(i)->clearParamWidget( mWidgetStack->widget(i) );
  if ( found ) return;

  // not found, so set the empty widget
  mComboBox->setCurrentItem( count ); // last item
  mWidgetStack->raiseWidget( count) ;
}

KMFilterAction * KMFilterActionWidget::action()
{
  // look up the action description via the label
  // returned by TQComboBox::currentText()...
  KMFilterActionDesc *desc = (*kmkernel->filterActionDict())[ mComboBox->currentText() ];
  if ( desc ) {
    // ...create an instance...
    KMFilterAction *fa = desc->create();
    if ( fa ) {
      // ...and apply the setting of the parameter widget.
      fa->applyParamWidgetValue( mWidgetStack->visibleWidget() );
      return fa;
    }
  }

  return 0;
}

//=============================================================================
//
// class KMFilterActionWidgetLister (the filter action editor)
//
//=============================================================================

KMFilterActionWidgetLister::KMFilterActionWidgetLister( TQWidget *parent, const char* name )
  : KWidgetLister( 1, FILTER_MAX_ACTIONS, parent, name )
{
  mActionList = 0;
}

KMFilterActionWidgetLister::~KMFilterActionWidgetLister()
{
}

void KMFilterActionWidgetLister::setActionList( TQPtrList<KMFilterAction> *aList )
{
  assert ( aList );

  if ( mActionList )
    regenerateActionListFromWidgets();

  mActionList = aList;

  ((TQWidget*)parent())->setEnabled( true );

  if ( aList->count() == 0 ) {
    slotClear();
    return;
  }

  int superfluousItems = (int)mActionList->count() - mMaxWidgets ;
  if ( superfluousItems > 0 ) {
    kdDebug(5006) << "KMFilterActionWidgetLister: Clipping action list to "
	      << mMaxWidgets << " items!" << endl;

    for ( ; superfluousItems ; superfluousItems-- )
      mActionList->removeLast();
  }

  // set the right number of widgets
  setNumberOfShownWidgetsTo( mActionList->count() );

  // load the actions into the widgets
  TQPtrListIterator<KMFilterAction> aIt( *mActionList );
  TQPtrListIterator<TQWidget> wIt( mWidgetList );
  for ( aIt.toFirst(), wIt.toFirst() ;
	aIt.current() && wIt.current() ; ++aIt, ++wIt )
    ((KMFilterActionWidget*)(*wIt))->setAction( (*aIt) );
}

void KMFilterActionWidgetLister::reset()
{
  if ( mActionList )
    regenerateActionListFromWidgets();

  mActionList = 0;
  slotClear();
  ((TQWidget*)parent())->setEnabled(  false  );
}

TQWidget* KMFilterActionWidgetLister::createWidget( TQWidget *parent )
{
  return new KMFilterActionWidget(parent);
}

void KMFilterActionWidgetLister::clearWidget( TQWidget *aWidget )
{
  if ( aWidget )
    ((KMFilterActionWidget*)aWidget)->setAction(0);
}

void KMFilterActionWidgetLister::regenerateActionListFromWidgets()
{
  if ( !mActionList ) return;

  mActionList->clear();

  TQPtrListIterator<TQWidget> it( mWidgetList );
  for ( it.toFirst() ; it.current() ; ++it ) {
    KMFilterAction *a = ((KMFilterActionWidget*)(*it))->action();
    if ( a )
      mActionList->append( a );
  }

}

//=============================================================================
//
// class KMPopFilterActionWidget
//
//=============================================================================

KMPopFilterActionWidget::KMPopFilterActionWidget( const TQString& title, TQWidget *parent, const char* name )
  : TQVButtonGroup( title, parent, name )
{
  mActionMap[Down] = new TQRadioButton( i18n("&Download mail"), this );
  mActionMap[Later] = new TQRadioButton( i18n("Download mail la&ter"), this );
  mActionMap[Delete] = new TQRadioButton( i18n("D&elete mail from server"), this );
  mIdMap[id(mActionMap[Later])] = Later;
  mIdMap[id(mActionMap[Down])] = Down;
  mIdMap[id(mActionMap[Delete])] = Delete;

  connect( this, TQT_SIGNAL(clicked(int)),
	   this, TQT_SLOT( slotActionClicked(int)) );
}

void KMPopFilterActionWidget::setAction( KMPopFilterAction aAction )
{
  if( aAction == NoAction)
  {
    aAction = Later;
  }

  mAction = aAction;

  blockSignals( true );
  if(!mActionMap[aAction]->isChecked())
  {
    mActionMap[aAction]->setChecked( true );
  }
  blockSignals( false );

  setEnabled( true );
}

KMPopFilterAction  KMPopFilterActionWidget::action()
{
  return mAction;
}

void KMPopFilterActionWidget::slotActionClicked(int aId)
{
  emit actionChanged(mIdMap[aId]);
  setAction(mIdMap[aId]);
}

void KMPopFilterActionWidget::reset()
{
  blockSignals( true );
  mActionMap[Down]->setChecked( true );
  blockSignals( false );

  setEnabled(  false  );
}

void KMFilterDlg::slotImportFilters()
{
    FilterImporterExporter importer( this, bPopFilter );
    TQValueList<KMFilter*> filters = importer.importFilters();
    // FIXME message box how many were imported?
    if (filters.isEmpty()) return;

    TQValueListConstIterator<KMFilter*> it;

    for ( it = filters.constBegin() ; it != filters.constEnd() ; ++it ) {
        mFilterList->appendFilter( *it ); // no need to deep copy, ownership passes to the list
    }
}

void KMFilterDlg::slotExportFilters()
{
    FilterImporterExporter exporter( this, bPopFilter );
    TQValueList<KMFilter*> filters = mFilterList->filtersForSaving();
    exporter.exportFilters( filters );
    TQValueList<KMFilter*>::iterator it;
    for ( it = filters.begin(); it != filters.end(); ++it )
        delete *it;
}

#include "kmfilterdlg.moc"
