#include "managesievescriptsdialog.h"
#include "managesievescriptsdialog_p.h"

#include "sieveconfig.h"
#include "accountmanager.h"
#include "imapaccountbase.h"
#include "sievejob.h"
#include "kmkernel.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kwin.h>
#include <kapplication.h>
#include <kinputdialog.h>
#include <kglobalsettings.h>
#include <kmessagebox.h>

#include <tqlayout.h>
#include <tqlistview.h>
#include <tqtextedit.h>
#include <tqpopupmenu.h>

#include <cassert>

inline TQCheckListItem * qcli_cast( TQListViewItem * lvi ) {
  return lvi && lvi->rtti() == 1 ? static_cast<TQCheckListItem*>( lvi ) : 0 ;
}
inline const TQCheckListItem * qcli_cast( const TQListViewItem * lvi ) {
  return lvi && lvi->rtti() == 1 ? static_cast<const TQCheckListItem*>( lvi ) : 0 ;
}

KMail::ManageSieveScriptsDialog::ManageSieveScriptsDialog( TQWidget * parent, const char * name )
  : KDialogBase( Plain, i18n( "Manage Sieve Scripts" ), Close, Close,
    parent, name, false ),
    mSieveEditor( 0 ),
    mContextMenuItem( 0 ),
    mWasActive( false )
{
  setWFlags( WGroupLeader|WDestructiveClose );
  KWin::setIcons( winId(), kapp->icon(), kapp->miniIcon() );

  TQVBoxLayout * vlay = new TQVBoxLayout( plainPage(), 0, 0 );

  mListView = new TQListView( plainPage() );
  mListView->addColumn( i18n( "Available Scripts" ) );
  mListView->setResizeMode( TQListView::LastColumn );
  mListView->setRootIsDecorated( true );
  mListView->setSelectionMode( TQListView::Single );
  connect( mListView, TQT_SIGNAL(contextMenuRequested(TQListViewItem*,const TQPoint&,int)),
           this, TQT_SLOT(slotContextMenuRequested(TQListViewItem*, const TQPoint&)) );
  connect( mListView, TQT_SIGNAL(doubleClicked(TQListViewItem*,const TQPoint&,int)),
           this, TQT_SLOT(slotDoubleClicked(TQListViewItem*)) );
  connect( mListView, TQT_SIGNAL(selectionChanged(TQListViewItem*)),
           this, TQT_SLOT(slotSelectionChanged(TQListViewItem*)) );
  vlay->addWidget( mListView );

  resize( 2 * sizeHint().width(), sizeHint().height() );

  slotRefresh();
}

KMail::ManageSieveScriptsDialog::~ManageSieveScriptsDialog() {
  killAllJobs();
}

void KMail::ManageSieveScriptsDialog::killAllJobs() {
  for ( TQMap<SieveJob*,TQCheckListItem*>::const_iterator it = mJobs.constBegin(), end = mJobs.constEnd() ; it != end ; ++it )
    it.key()->kill();
  mJobs.clear();
}

static KURL findUrlForAccount( const KMail::ImapAccountBase * a ) {
  assert( a );
  const KMail::SieveConfig sieve = a->sieveConfig();
  if ( !sieve.managesieveSupported() )
    return KURL();
  if ( sieve.reuseConfig() ) {
    // assemble Sieve url from the settings of the account:
    KURL u;
    u.setProtocol( "sieve" );
    u.setHost( a->host() );
    u.setUser( a->login() );
    u.setPass( a->passwd() );
    u.setPort( sieve.port() );
    // Translate IMAP LOGIN to PLAIN:
    u.addQueryItem( "x-mech", a->auth() == "*" ? "PLAIN" : a->auth() );
    if ( !a->useSSL() && !a->useTLS() )
        u.addQueryItem( "x-allow-unencrypted", "true" );
    return u;
  } else {
    KURL u = sieve.alternateURL();
    if ( u.protocol().lower() == "sieve" && !a->useSSL() && !a->useTLS() && u.queryItem("x-allow-unencrypted").isEmpty() )
        u.addQueryItem( "x-allow-unencrypted", "true" );
    return u;
  }
}

void KMail::ManageSieveScriptsDialog::slotRefresh() {
  killAllJobs();
  mUrls.clear();
  mListView->clear();

  KMail::AccountManager * am = kmkernel->acctMgr();
  assert( am );
  TQCheckListItem * last = 0;
  for ( KMAccount * a = am->first() ; a ; a = am->next() ) {
    last = new TQCheckListItem( mListView, last, a->name(), TQCheckListItem::Controller );
    last->setPixmap( 0, SmallIcon( "server" ) );
    if ( ImapAccountBase * iab = dynamic_cast<ImapAccountBase*>( a ) ) {
      const KURL u = ::findUrlForAccount( iab );
      if ( u.isEmpty() )
        continue;
      SieveJob * job = SieveJob::list( u );
      connect( job, TQT_SIGNAL(item(KMail::SieveJob*,const TQString&,bool)),
               this, TQT_SLOT(slotItem(KMail::SieveJob*,const TQString&,bool)) );
      connect( job, TQT_SIGNAL(result(KMail::SieveJob*,bool,const TQString&,bool)),
               this, TQT_SLOT(slotResult(KMail::SieveJob*,bool,const TQString&,bool)) );
      mJobs.insert( job, last );
      mUrls.insert( last, u );
    } else {
      TQListViewItem * item = new TQListViewItem( last, i18n( "No Sieve URL configured" ) );
      item->setEnabled( false );
      last->setOpen( true );
    }
  }
}

void KMail::ManageSieveScriptsDialog::slotResult( KMail::SieveJob * job, bool success, const TQString &, bool ) {
  TQCheckListItem * parent = mJobs[job];
  if ( !parent )
    return;

  mJobs.remove( job );

  parent->setOpen( true );

  if ( success )
    return;

  TQListViewItem * item = new TQListViewItem( parent, i18n( "Failed to fetch the list of scripts" ) );
  item->setEnabled( false );
}

void KMail::ManageSieveScriptsDialog::slotItem( KMail::SieveJob * job, const TQString & filename, bool isActive ) {
  TQCheckListItem * parent = mJobs[job];
  if ( !parent )
    return;
  TQCheckListItem * item = new TQCheckListItem( parent, filename, TQCheckListItem::RadioButton );
  if ( isActive ) {
    item->setOn( true );
    mSelectedItems[parent] = item;
  }
}

void KMail::ManageSieveScriptsDialog::slotContextMenuRequested( TQListViewItem * i, const TQPoint & p ) {
  TQCheckListItem * item = qcli_cast( i );
  if ( !item )
    return;
  if ( !item->depth() && !mUrls.count( item ) )
    return;
  TQPopupMenu menu;
  mContextMenuItem = item;
  if ( item->depth() ) {
    // script items:
    menu.insertItem( i18n( "Delete Script" ), this, TQT_SLOT(slotDeleteScript()) );
    menu.insertItem( i18n( "Edit Script..." ), this, TQT_SLOT(slotEditScript()) );
    menu.insertItem( i18n( "Deactivate Script" ), this, TQT_SLOT(slotDeactivateScript()) );
  } else {
    // top-levels:
    menu.insertItem( i18n( "New Script..." ), this, TQT_SLOT(slotNewScript()) );
  }
  menu.exec( p );
  mContextMenuItem = 0;
}


void KMail::ManageSieveScriptsDialog::slotDeactivateScript() {
  if ( !mContextMenuItem )
    return;

  TQCheckListItem * parent = qcli_cast( mContextMenuItem->parent() );
  if ( !parent )
    return;
  if ( mContextMenuItem->isOn()) {
    mSelectedItems[parent] = mContextMenuItem;
    changeActiveScript( parent,false );
  }
}

void KMail::ManageSieveScriptsDialog::slotSelectionChanged( TQListViewItem * i ) {
  TQCheckListItem * item = qcli_cast( i );
  if ( !item )
    return;
  TQCheckListItem * parent = qcli_cast( item->parent() );
  if ( !parent )
    return;
  if ( item->isOn() && mSelectedItems[parent] != item ) {
    mSelectedItems[parent] = item;
    changeActiveScript( parent,true );
  }
}

void KMail::ManageSieveScriptsDialog::changeActiveScript( TQCheckListItem * item , bool activate) {
  if ( !item )
    return;
  if ( !mUrls.count( item ) )
    return;
  if ( !mSelectedItems.count( item ) )
    return;
  KURL u = mUrls[item];
  if ( u.isEmpty() )
    return;
  TQCheckListItem * selected = mSelectedItems[item];
  if ( !selected )
    return;
  u.setFileName( selected->text( 0 ) );
  SieveJob * job;
  if ( activate )
    job = SieveJob::activate( u );
  else
    job = SieveJob::desactivate( u );
  connect( job, TQT_SIGNAL(result(KMail::SieveJob*,bool,const TQString&,bool)),
           this, TQT_SLOT(slotRefresh()) );
}

void KMail::ManageSieveScriptsDialog::slotDoubleClicked( TQListViewItem * i ) {
  TQCheckListItem * item = qcli_cast( i );
  if ( !item )
    return;
  if ( !item->depth() )
    return;
  mContextMenuItem = item;
  slotEditScript();
  mContextMenuItem = 0;
}

void KMail::ManageSieveScriptsDialog::slotDeleteScript() {
  if ( !mContextMenuItem )
    return;
  if ( !mContextMenuItem->depth() )
    return;

  TQCheckListItem * parent = qcli_cast( mContextMenuItem->parent() );
  if ( !parent )
    return;

  if ( !mUrls.count( parent ) )
    return;

  KURL u = mUrls[parent];
  if ( u.isEmpty() )
    return;

  u.setFileName( mContextMenuItem->text( 0 ) );

  if ( KMessageBox::warningContinueCancel( this, i18n( "Really delete script \"%1\" from the server?" ).arg( u.fileName() ),
                                   i18n( "Delete Sieve Script Confirmation" ),
                                   KStdGuiItem::del() )
       != KMessageBox::Continue )
    return;
  SieveJob * job = SieveJob::del( u );
  connect( job, TQT_SIGNAL(result(KMail::SieveJob*,bool,const TQString&,bool)),
           this, TQT_SLOT(slotRefresh()) );
}

void KMail::ManageSieveScriptsDialog::slotEditScript() {
  if ( !mContextMenuItem )
    return;
  if ( !mContextMenuItem->depth() )
    return;
  TQCheckListItem * parent = qcli_cast( mContextMenuItem->parent() );
  if ( !mUrls.count( parent ) )
    return;
  KURL url = mUrls[parent];
  if ( url.isEmpty() )
    return;
  url.setFileName( mContextMenuItem->text( 0 ) );
  mCurrentURL = url;
  SieveJob * job = SieveJob::get( url );
  connect( job, TQT_SIGNAL(result(KMail::SieveJob*,bool,const TQString&,bool)),
           this, TQT_SLOT(slotGetResult(KMail::SieveJob*,bool,const TQString&,bool)) );
}

void KMail::ManageSieveScriptsDialog::slotNewScript() {
  if ( !mContextMenuItem )
    return;
  if ( mContextMenuItem->depth() )
    mContextMenuItem = qcli_cast( mContextMenuItem->parent() );
  if ( !mContextMenuItem )
    return;

  if ( !mUrls.count( mContextMenuItem ) )
    return;

  KURL u = mUrls[mContextMenuItem];
  if ( u.isEmpty() )
    return;

  bool ok = false;
  const TQString name = KInputDialog::getText( i18n( "New Sieve Script" ),
                                              i18n( "Please enter a name for the new Sieve script:" ),
                                              i18n( "unnamed" ), &ok, this );
  if ( !ok || name.isEmpty() )
    return;

  u.setFileName( name );

  (void) new TQCheckListItem( mContextMenuItem, name, TQCheckListItem::RadioButton );

  mCurrentURL = u;
  slotGetResult( 0, true, TQString::null, false );
}

KMail::SieveEditor::SieveEditor( TQWidget * parent, const char * name )
  : KDialogBase( Plain, i18n( "Edit Sieve Script" ), Ok|Cancel, Ok, parent, name )
{
  TQVBoxLayout * vlay = new TQVBoxLayout( plainPage(), 0, spacingHint() );
  mTextEdit = new TQTextEdit( plainPage() );
  vlay->addWidget( mTextEdit );
  mTextEdit->setFocus();
  mTextEdit->setTextFormat( TQTextEdit::PlainText );
  mTextEdit->setWordWrap( TQTextEdit::NoWrap );
  mTextEdit->setFont( KGlobalSettings::fixedFont() );
  connect( mTextEdit, TQT_SIGNAL( textChanged () ), TQT_SLOT( slotTextChanged() ) );
  resize( 3 * sizeHint() );
}

KMail::SieveEditor::~SieveEditor() {}


void KMail::SieveEditor::slotTextChanged()
{
  enableButtonOK( !script().isEmpty() );
}

void KMail::ManageSieveScriptsDialog::slotGetResult( KMail::SieveJob *, bool success, const TQString & script, bool isActive ) {
  if ( !success )
    return;

  if ( mSieveEditor )
    return;

  mSieveEditor = new SieveEditor( this );
  mSieveEditor->setScript( script );
  connect( mSieveEditor, TQT_SIGNAL(okClicked()), this, TQT_SLOT(slotSieveEditorOkClicked()) );
  connect( mSieveEditor, TQT_SIGNAL(cancelClicked()), this, TQT_SLOT(slotSieveEditorCancelClicked()) );
  mSieveEditor->show();
  mWasActive = isActive;
}

void KMail::ManageSieveScriptsDialog::slotSieveEditorOkClicked() {
  if ( !mSieveEditor )
    return;
  SieveJob * job = SieveJob::put( mCurrentURL,mSieveEditor->script(), mWasActive, mWasActive );
  connect( job, TQT_SIGNAL(result(KMail::SieveJob*,bool,const TQString&,bool)),
           this, TQT_SLOT(slotPutResult(KMail::SieveJob*,bool)) );
}

void KMail::ManageSieveScriptsDialog::slotSieveEditorCancelClicked() {
  mSieveEditor->deleteLater(); mSieveEditor = 0;
  mCurrentURL = KURL();
  slotRefresh();
}

void KMail::ManageSieveScriptsDialog::slotPutResult( KMail::SieveJob *, bool success ) {
  if ( success ) {
    KMessageBox::information( this, i18n( "The Sieve script was successfully uploaded." ),
                              i18n( "Sieve Script Upload" ) );
    mSieveEditor->deleteLater(); mSieveEditor = 0;
    mCurrentURL = KURL();
  } else {
    mSieveEditor->show();
  }
}

#include "managesievescriptsdialog.moc"
#include "managesievescriptsdialog_p.moc"
