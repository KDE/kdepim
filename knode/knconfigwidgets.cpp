/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2006 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include "knconfigwidgets.h"

#include "configuration/identity_widget.h"
#include "knaccountmanager.h"
#include "knconfigmanager.h"
#include "kndisplayedheader.h"
#include "kngroupmanager.h"
#include "knglobals.h"
#include "knnntpaccount.h"
#include "utilities.h"
#include "knfiltermanager.h"
#include "knarticlefilter.h"
#include "knscoring.h"
#include "ui_postnewscomposerwidget_base.h"
#include "ui_readnewsnavigationwidget_base.h"
#include "ui_readnewsviewerwidget_base.h"
#include "settings.h"
#include "utils/locale.h"

#include <QPainter>
#include <kcharsets.h>
#include <kio/ioslave_defaults.h>
#include <kmessagebox.h>
#include <kcolordialog.h>
#include <kfontdialog.h>
#include <kscoringeditor.h>
#include <sonnet/configwidget.h>
#include <kcombobox.h>
#include <libkpgp/kpgpui.h>



//BEGIN: NNTP account configuration widgets ----------------------------------

KNode::NntpAccountListWidget::NntpAccountListWidget( const KComponentData &inst, QWidget *parent ) :
  KCModule( inst, parent )
{
  setupUi( this );

  // account listbox
  connect( mAccountList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(slotEditBtnClicked()) );
  connect( mAccountList, SIGNAL(itemSelectionChanged()), SLOT(slotSelectionChanged()) );

  // buttons
  connect( mAddButton, SIGNAL(clicked()), SLOT(slotAddBtnClicked()) );
  connect( mEditButton, SIGNAL(clicked()), SLOT(slotEditBtnClicked()) );
  connect( mDeleteButton, SIGNAL(clicked()), SLOT(slotDelBtnClicked()) );
  connect( mSubscribeButton, SIGNAL(clicked()), SLOT(slotSubBtnClicked()) );

  load();

  // the settings dialog is non-modal, so we have to react to changes
  // made outside of the dialog
  KNAccountManager *am = knGlobals.accountManager();
  connect( am, SIGNAL(accountAdded(KNNntpAccount::Ptr)), SLOT(slotAddItem(KNNntpAccount::Ptr)) );
  connect( am, SIGNAL(accountRemoved(KNNntpAccount::Ptr)), SLOT(slotRemoveItem(KNNntpAccount::Ptr)) );
  connect( am, SIGNAL(accountModified(KNNntpAccount::Ptr)), SLOT(slotUpdateItem(KNNntpAccount::Ptr)) );

  slotSelectionChanged();     // disable Delete & Edit initially
}


void KNode::NntpAccountListWidget::load()
{
  mAccountList->clear();
  KNNntpAccount::List list = knGlobals.accountManager()->accounts();
  for ( KNNntpAccount::List::Iterator it = list.begin(); it != list.end(); ++it )
    slotAddItem( *it );
}


void KNode::NntpAccountListWidget::slotAddItem( KNNntpAccount::Ptr a )
{
  AccountListItem *item;
  item = new AccountListItem( a );
  item->setText( a->name() );
  item->setIcon( SmallIcon( "network-server" ) );
  mAccountList->addItem( item );
  emit changed( true );
}


void KNode::NntpAccountListWidget::slotRemoveItem( KNNntpAccount::Ptr a )
{
  AccountListItem *item;
  for ( int i = 0; i < mAccountList->count(); ++i ) {
    item = static_cast<AccountListItem*>( mAccountList->item( i ) );
    if ( item && item->account() == a ) {
      delete mAccountList->takeItem( i );
      break;
    }
  }
  slotSelectionChanged();
  emit changed( true );
}


void KNode::NntpAccountListWidget::slotUpdateItem( KNNntpAccount::Ptr a )
{
  AccountListItem *item;
  for ( int i = 0; i < mAccountList->count(); ++i ) {
    item = static_cast<AccountListItem*>( mAccountList->item( i ) );
    if ( item && item->account() == a )
      item->setText( a->name() );
  }
  slotSelectionChanged();
  emit changed( true );
}



void KNode::NntpAccountListWidget::slotSelectionChanged()
{
  AccountListItem *item = static_cast<AccountListItem*>( mAccountList->currentItem() );
  mDeleteButton->setEnabled( item );
  mEditButton->setEnabled( item );
  mSubscribeButton->setEnabled( item );

  if ( item ) {
    mServerInfo->setText( i18n("Server: %1", item->account()->server() ) );
    mPortInfo->setText( i18n("Port: %1", item->account()->port() ) );
  } else {
    mServerInfo->setText( i18n("Server: ") );
    mPortInfo->setText( i18n("Port: ") );
  }
}



void KNode::NntpAccountListWidget::slotAddBtnClicked()
{
  KNNntpAccount::Ptr acc = KNNntpAccount::Ptr( new KNNntpAccount() );

  if(acc->editProperties(this)) {
    if(knGlobals.accountManager()->newAccount(acc))
      acc->writeConfig();
  }
}



void KNode::NntpAccountListWidget::slotDelBtnClicked()
{
  AccountListItem *item = static_cast<AccountListItem*>( mAccountList->currentItem() );
  if ( item )
    knGlobals.accountManager()->removeAccount( item->account() );
}



void KNode::NntpAccountListWidget::slotEditBtnClicked()
{
  AccountListItem *item = static_cast<AccountListItem*>( mAccountList->currentItem() );
  if ( item ) {
    item->account()->editProperties( this );
    slotUpdateItem( item->account() );
  }
}


void KNode::NntpAccountListWidget::slotSubBtnClicked()
{
  AccountListItem *item = static_cast<AccountListItem*>( mAccountList->currentItem() );
  if( item )
    knGlobals.groupManager()->showGroupDialog( item->account(), this );
}


//=======================================================================================


KNode::NntpAccountConfDialog::NntpAccountConfDialog( KNNntpAccount *a, QWidget *parent ) :
    KPageDialog( parent ),
    mAccount( a ),
    mUseServerForName( false )
{
  if ( a->id() != -1 )
    setCaption( i18n("Properties of %1", a->name()) );
  else
    setCaption( i18n("New Account") );
  setFaceType( Tabbed );
  setButtons( Ok | Cancel | Help );
  setDefaultButton( Ok );

  // server config tab
  QFrame* page = new QFrame( this );
  addPage( page, i18n("Ser&ver") );
  setupUi( page );

  mName->setText( a->name() );
  mServer->setText( a->server() );
  mPort->setValue( a->port() );
#ifndef Q_WS_WIN
// don't know how to set this in KDE4, where no related methods exists
  mPort->setSliderEnabled( false );
#endif
  mFetchDesc->setChecked( a->fetchDescriptions() );

  connect( mServer, SIGNAL(textChanged(QString)),
           this, SLOT(slotServerTextEdited()) );
  connect( mServer, SIGNAL(editingFinished()),
           this, SLOT(slotEditingFinished()) );

  mLogin->setChecked( a->needsLogon() );
  mUser->setText( a->user() );

  connect( knGlobals.accountManager(), SIGNAL(passwordsChanged()), SLOT(slotPasswordChanged()) );
  if ( a->readyForLogin() )
    mPassword->setText( a->pass() );
  else
    if ( a->needsLogon() )
      knGlobals.accountManager()->loadPasswordsAsync();

  switch ( mAccount->encryption() ) {
    case KNServerInfo::None:
      mEncNone->setChecked( true );
      break;
    case KNServerInfo::SSL:
      mEncSSL->setChecked( true );
      break;
    case KNServerInfo::TLS:
      mEncTLS->setChecked( true );
      break;
  }
  connect( mEncNone, SIGNAL(toggled(bool)),
           this, SLOT(encryptionChanged(bool)) );
  connect( mEncSSL, SIGNAL(toggled(bool)),
           this, SLOT(encryptionChanged(bool)) );
  connect( mEncTLS, SIGNAL(toggled(bool)),
           this, SLOT(encryptionChanged(bool)) );


  mIntervalChecking->setChecked( a->intervalChecking() );
  mInterval->setValue( a->checkInterval() );
  mInterval->setSuffix(ki18np(" minute", " minutes"));

  // identity tab
  mIdentityWidget = new KNode::IdentityWidget( a, knGlobals.componentData(), this );
  addPage( mIdentityWidget, i18n("&Identity") );

  // per server cleanup configuration
  mCleanupWidget = new GroupCleanupWidget( a->cleanupConfig(), this );
  addPage( mCleanupWidget, i18n("&Cleanup") );
  mCleanupWidget->load();

  KNHelper::restoreWindowSize("accNewsPropDLG", this, sizeHint());

  setHelp("anc-setting-the-news-account");
}


KNode::NntpAccountConfDialog::~NntpAccountConfDialog()
{
  KNHelper::saveWindowSize("accNewsPropDLG", size());
}

void KNode::NntpAccountConfDialog::slotServerTextEdited()
{
  if ( mName->text().trimmed().isEmpty() ) {
    mUseServerForName = true;
  }

  if ( mUseServerForName ) {
    mName->setText( mServer->text() );
  }
}

void KNode::NntpAccountConfDialog::slotEditingFinished()
{
  mUseServerForName = false;
}

void KNode::NntpAccountConfDialog::slotButtonClicked( int button )
{
  if ( button == KDialog::Ok ) {
    if ( mName->text().isEmpty() || mServer->text().trimmed().isEmpty() ) {
      KMessageBox::sorry(this, i18n("Please enter an arbitrary name for the account and the\nhostname of the news server."));
      return;
    }

    mAccount->setName( mName->text() );
    mAccount->setServer( mServer->text().trimmed().remove(QLatin1String("news://")) );
    mAccount->setPort( mPort->value() );
    mAccount->setFetchDescriptions( mFetchDesc->isChecked() );
    mAccount->setNeedsLogon( mLogin->isChecked() );
    mAccount->setUser( mUser->text() );
    mAccount->setPass( mPassword->text() );

    if ( mEncNone->isChecked() )
      mAccount->setEncryption( KNServerInfo::None );
    if ( mEncSSL->isChecked() )
      mAccount->setEncryption( KNServerInfo::SSL );
    if ( mEncTLS->isChecked() )
      mAccount->setEncryption( KNServerInfo::TLS );

    mAccount->setIntervalChecking( mIntervalChecking->isChecked() );
    mAccount->setCheckInterval( mInterval->value() );

    if ( mAccount->id() != -1 ) // only save if account has a valid id
      mAccount->writeConfig();

    mIdentityWidget->save();
    mCleanupWidget->save();

    accept();
  } else {
    KDialog::slotButtonClicked( button );
  }
}


void KNode::NntpAccountConfDialog::slotPasswordChanged()
{
  if ( mPassword->text().isEmpty() )
    mPassword->setText( mAccount->pass() );
}


void KNode::NntpAccountConfDialog::encryptionChanged( bool checked )
{
  if ( checked ) { // All 3 buttons are connected to this slot, so only the checked one is taken into account.
    if ( mEncNone->isChecked() ) {
      mPort->setValue( DEFAULT_NNTP_PORT );
    } else if ( mEncSSL->isChecked() || mEncTLS->isChecked() ) {
      mPort->setValue( DEFAULT_NNTPS_PORT );
    }
  }
}


//END: NNTP account configuration widgets ------------------------------------

//===================================================================================

KNode::AppearanceWidget::ColorListItem::ColorListItem( const QString &text, const QColor &color, QListWidget *parent ) :
  QListWidgetItem( text, parent )
{
  setColor( color );
}


void KNode::AppearanceWidget::ColorListItem::setColor( const QColor &color )
{
  mColor = color;
  int height = QFontMetrics( font() ).height();
  QPixmap icon( height, height );
  QPainter p( &icon );
  p.setPen( Qt::black );
  p.drawRect( 0, 0, height - 1, height - 1 );
  p.fillRect( 1, 1, height - 2, height - 2, color );
  setIcon( icon );
  if ( listWidget() )
    listWidget()->update();
}


//===================================================================================


KNode::AppearanceWidget::FontListItem::FontListItem( const QString &text, const QFont &font, QListWidget *parent ) :
  QListWidgetItem( parent ),
  mText( text )
{
  setFont( font );
}


void KNode::AppearanceWidget::FontListItem::setFont( const QFont &font )
{
  mFont = font;
  setText( QString("[%1 %2] %3").arg( mFont.family() ).arg( mFont.pointSize() ).arg( mText ) );
  if ( listWidget() )
    listWidget()->update();
}


//===================================================================================


KNode::AppearanceWidget::AppearanceWidget( const KComponentData &inst, QWidget *parent ) :
  KCModule(inst, parent )
{
  QGridLayout *topL = new QGridLayout( this );

  //color-list
  mColorList = new QListWidget( this );
  topL->addWidget( mColorList, 1, 0, 3, 2 );
  connect( mColorList, SIGNAL(itemActivated(QListWidgetItem*)), SLOT(slotColItemActivated(QListWidgetItem*)) );
  connect( mColorList, SIGNAL(itemSelectionChanged()), SLOT(slotColSelectionChanged()) );

  c_olorCB = new QCheckBox(i18n("&Use custom colors"),this);
  c_olorCB->setObjectName( "kcfg_useCustomColors" );
  topL->addWidget( c_olorCB, 0, 0, 1, 3 );
  connect(c_olorCB, SIGNAL(toggled(bool)), this, SLOT(slotColCheckBoxToggled(bool)));

  c_olChngBtn = new QPushButton( i18nc( "@action:button Run a color selection dialog", "Cha&nge..." ), this );
  connect(c_olChngBtn, SIGNAL(clicked()), this, SLOT(slotColChangeBtnClicked()));
  topL->addWidget( c_olChngBtn, 1, 2, 1, 1 );

  //font-list
  mFontList = new QListWidget( this );
  topL->addWidget( mFontList, 5, 0, 3, 2 );
  connect( mFontList, SIGNAL(itemActivated(QListWidgetItem*)), SLOT(slotFontItemActivated(QListWidgetItem*)) );
  connect( mFontList, SIGNAL(itemSelectionChanged()), SLOT(slotFontSelectionChanged()) );

  f_ontCB = new QCheckBox(i18n("Use custom &fonts"),this);
  f_ontCB->setObjectName( "kcfg_useCustomFonts" );
  topL->addWidget(f_ontCB , 4, 0, 1, 3 );
  connect(f_ontCB, SIGNAL(toggled(bool)), this, SLOT(slotFontCheckBoxToggled(bool)));

  f_ntChngBtn = new QPushButton( i18nc( "@action:button Run a font selection dialog", "Chang&e..."), this);
  connect(f_ntChngBtn, SIGNAL(clicked()), this, SLOT(slotFontChangeBtnClicked()));
  topL->addWidget( f_ntChngBtn, 5, 2, 1, 1 );

  topL->setColumnStretch( 0, 1 );

  addConfig( knGlobals.settings(), this );
  load();
}


void KNode::AppearanceWidget::load()
{
  KCModule::load();

  slotColCheckBoxToggled( c_olorCB->isChecked() );
  slotFontCheckBoxToggled( f_ontCB->isChecked() );

  KConfigSkeletonItem::List items = knGlobals.settings()->items();
  mColorList->clear();
  for ( KConfigSkeletonItem::List::Iterator it = items.begin(); it != items.end(); ++it ) {
    KConfigSkeleton::ItemColor *item = dynamic_cast<KConfigSkeleton::ItemColor*>( *it );
    if ( item )
      mColorList->addItem( new ColorListItem( item->label(), item->value() ) );
  }

  mFontList->clear();
  for ( KConfigSkeletonItem::List::Iterator it = items.begin(); it != items.end(); ++it ) {
    KConfigSkeleton::ItemFont *item = dynamic_cast<KConfigSkeleton::ItemFont*>( *it );
    if ( item )
      mFontList->addItem( new FontListItem( item->label(), item->value() ) );
  }
}


void KNode::AppearanceWidget::save()
{
  KConfigSkeletonItem::List items = knGlobals.settings()->items();
  int row = 0;
  for ( KConfigSkeletonItem::List::Iterator it = items.begin(); it != items.end(); ++it ) {
    KConfigSkeleton::ItemColor *item = dynamic_cast<KConfigSkeleton::ItemColor*>( *it );
    if ( !item )
      continue;
    item->setValue( static_cast<ColorListItem*>( mColorList->item( row ) )->color() );
    ++row;
  }

  row = 0;
  for ( KConfigSkeletonItem::List::Iterator it = items.begin(); it != items.end(); ++it ) {
    KConfigSkeleton::ItemFont *item = dynamic_cast<KConfigSkeleton::ItemFont*>( *it );
    if ( !item )
      continue;
    item->setValue( static_cast<FontListItem*>( mFontList->item( row ) )->font() );
    ++row;
  }

  KCModule::save();

  knGlobals.configManager()->appearance()->recreateLVIcons();
}


void KNode::AppearanceWidget::defaults()
{
  KCModule::defaults();

  KConfigSkeletonItem::List items = knGlobals.settings()->items();
  int row = 0;
  for ( KConfigSkeletonItem::List::Iterator it = items.begin(); it != items.end(); ++it ) {
    KConfigSkeleton::ItemColor *item = dynamic_cast<KConfigSkeleton::ItemColor*>( *it );
    if ( !item )
      continue;
    item->setDefault();
    static_cast<ColorListItem*>( mColorList->item( row ) )->setColor( item->value() );
    ++row;
  }

  row = 0;
  for ( KConfigSkeletonItem::List::Iterator it = items.begin(); it != items.end(); ++it ) {
    KConfigSkeleton::ItemFont *item = dynamic_cast<KConfigSkeleton::ItemFont*>( *it );
    if ( !item )
      continue;
    item->setDefault();
    static_cast<FontListItem*>( mFontList->item( row ) )->setFont( item->value() );
    ++row;
  }

  emit changed(true);
}


void KNode::AppearanceWidget::slotColCheckBoxToggled(bool b)
{
  mColorList->setEnabled( b );
  c_olChngBtn->setEnabled( b && mColorList->currentItem() );
  if (b) mColorList->setFocus();
}


// show color dialog for the entry
void KNode::AppearanceWidget::slotColItemActivated( QListWidgetItem *item )
{
  if ( item ) {
    ColorListItem *colorItem = static_cast<ColorListItem*>( item );
    QColor col = colorItem->color();
    int result = KColorDialog::getColor(col,this);

    if (result == KColorDialog::Accepted) {
      colorItem->setColor(col);
    }
  }
  emit changed(true);
}


void KNode::AppearanceWidget::slotColChangeBtnClicked()
{
  if ( mColorList->currentItem() )
    slotColItemActivated( mColorList->currentItem() );
}


void KNode::AppearanceWidget::slotColSelectionChanged()
{
  c_olChngBtn->setEnabled( mColorList->currentItem() );
}


void KNode::AppearanceWidget::slotFontCheckBoxToggled(bool b)
{
  mFontList->setEnabled( b );
  f_ntChngBtn->setEnabled( b && mFontList->currentItem() );
  if (b) mFontList->setFocus();
}


// show font dialog for the entry
void KNode::AppearanceWidget::slotFontItemActivated( QListWidgetItem *item )
{
  if ( item ) {
    FontListItem *fontItem = static_cast<FontListItem*>( item );
    QFont font = fontItem->font();
    const int result = KFontDialog::getFont( font /** by-ref*/,
                                             KFontChooser::NoDisplayFlags, this );

    if (result == KFontDialog::Accepted)
      fontItem->setFont(font);
  }
  emit changed(true);
}


void KNode::AppearanceWidget::slotFontChangeBtnClicked()
{
  if ( mFontList->currentItem() )
    slotFontItemActivated( mFontList->currentItem() );
}


void KNode::AppearanceWidget::slotFontSelectionChanged()
{
  f_ntChngBtn->setEnabled( mFontList->currentItem() );
}


//=============================================================================================


KNode::ReadNewsGeneralWidget::ReadNewsGeneralWidget( const KComponentData &inst, QWidget *parent ) :
  KCModule( inst, parent )
{
  setupUi( this );
  addConfig( knGlobals.settings(), this );
  load();
}


void KNode::ReadNewsGeneralWidget::load()
{
  KCModule::load();
  switch ( knGlobals.settings()->dateFormat() ) {
    case KMime::DateFormatter::CTime: mStandardDateFormat->setChecked( true ); break;
    case KMime::DateFormatter::Localized: mLocalizedDateFormat->setChecked( true ); break;
    case KMime::DateFormatter::Fancy: mFancyDateFormat->setChecked( true ); break;
    case KMime::DateFormatter::Custom: mCustomDateFormat->setChecked( true ); break;
    case KMime::DateFormatter::Iso: break; // not used
    case KMime::DateFormatter::Rfc: break; // not used

  }
}

void KNode::ReadNewsGeneralWidget::save()
{
  if ( mStandardDateFormat->isChecked() )
    knGlobals.settings()->setDateFormat( KMime::DateFormatter::CTime );
  if ( mLocalizedDateFormat->isChecked() )
    knGlobals.settings()->setDateFormat( KMime::DateFormatter::Localized );
  if ( mFancyDateFormat->isChecked() )
    knGlobals.settings()->setDateFormat( KMime::DateFormatter::Fancy );
  if ( mCustomDateFormat->isChecked() )
    knGlobals.settings()->setDateFormat( KMime::DateFormatter::Custom );
  KCModule::save();
}

//=============================================================================================


KNode::ReadNewsNavigationWidget::ReadNewsNavigationWidget( const KComponentData &inst, QWidget *parent ) :
  KCModule( inst, parent )
{
  KNode::Ui::ReadNewsNavigationWidgetBase ui;
  ui.setupUi( this );
  addConfig( knGlobals.settings(), this );
  load();
}


//=============================================================================================


KNode::ReadNewsViewerWidget::ReadNewsViewerWidget( const KComponentData &inst, QWidget *parent ) :
  KCModule( inst, parent )
{
  KNode::Ui::ReadNewsViewerWidgetBase ui;
  ui.setupUi( this );
  addConfig( knGlobals.settings(), this );
  load();
}


//=============================================================================================


KNode::DisplayedHeadersWidget::DisplayedHeadersWidget( DisplayedHeaders *d, const KComponentData &inst, QWidget *parent ) :
  KCModule( inst, parent ),
  s_ave( false ),
  d_ata( d )
{
  QGridLayout *topL=new QGridLayout(this);
  topL->setSpacing(5);
  topL->setMargin(5);

  //listbox
  mHeaderList = new QListWidget( this );
  connect( mHeaderList, SIGNAL(itemActivated(QListWidgetItem*)), SLOT(slotEditBtnClicked()) );
  connect( mHeaderList, SIGNAL(itemSelectionChanged()), SLOT(slotSelectionChanged()) );
  topL->addWidget( mHeaderList, 0, 0, 7, 1);

  // buttons
  a_ddBtn = new QPushButton( i18nc( "@action:button Add a new message header field (open dialog)", "&Add..." ), this );
  connect(a_ddBtn, SIGNAL(clicked()), this, SLOT(slotAddBtnClicked()));
  topL->addWidget(a_ddBtn, 0,1);

  d_elBtn = new QPushButton( i18nc( "@action:button Delete a message header field", "&Delete" ), this);
  connect(d_elBtn, SIGNAL(clicked()), this, SLOT(slotDelBtnClicked()));
  topL->addWidget(d_elBtn, 1,1);

  e_ditBtn=new QPushButton(i18nc("modify something","&Edit..."), this);
  connect(e_ditBtn, SIGNAL(clicked()), this, SLOT(slotEditBtnClicked()));
  topL->addWidget(e_ditBtn, 2,1);

  u_pBtn = new QPushButton( i18nc( "@action:button Move an element of a list up", "&Up" ), this );
  connect(u_pBtn, SIGNAL(clicked()), this, SLOT(slotUpBtnClicked()));
  topL->addWidget(u_pBtn, 4,1);

  d_ownBtn = new QPushButton( i18nc( "@action:button Move an element of a list down", "Do&wn" ), this );
  connect(d_ownBtn, SIGNAL(clicked()), this, SLOT(slotDownBtnClicked()));
  topL->addWidget(d_ownBtn, 5,1);

  topL->addItem( new QSpacerItem( 0,20), 3, 0 );        // separate up/down buttons
  topL->setRowStretch(6,1);         // stretch the listbox

  slotSelectionChanged();     // disable buttons initially

  load();
}



void KNode::DisplayedHeadersWidget::load()
{
  mHeaderList->clear();
  KNDisplayedHeader::List list = d_ata->headers();
  for ( KNDisplayedHeader::List::Iterator it = list.begin(); it != list.end(); ++it )
    mHeaderList->addItem( generateItem( (*it) ) );
}

void KNode::DisplayedHeadersWidget::save()
{
  if(s_ave) {
    d_ata->setDirty(true);
    d_ata->save();
  }
  s_ave = false;
}



KNode::DisplayedHeadersWidget::HdrItem* KNode::DisplayedHeadersWidget::generateItem(KNDisplayedHeader *h)
{
  QString text;
  if(h->hasName()) {
    text=h->translatedName();
    text+=": <";
  } else {
    text = '<';
  }
  text+=h->header();
  text+='>';
  return new HdrItem(text,h);
}



void KNode::DisplayedHeadersWidget::slotSelectionChanged()
{
  int curr = mHeaderList->currentRow();
  d_elBtn->setEnabled(curr!=-1);
  e_ditBtn->setEnabled(curr!=-1);
  u_pBtn->setEnabled(curr>0);
  d_ownBtn->setEnabled( ( curr != -1 )  && ( curr + 1 != mHeaderList->count() ) );
}



void KNode::DisplayedHeadersWidget::slotAddBtnClicked()
{
  KNDisplayedHeader *h=d_ata->createNewHeader();

  DisplayedHeaderConfDialog* dlg=new DisplayedHeaderConfDialog(h, this);
  if(dlg->exec()) {
    mHeaderList->addItem( generateItem( h ) );
    h->createTags();
    s_ave=true;
  } else
    d_ata->remove(h);
  emit changed(true);
}



void KNode::DisplayedHeadersWidget::slotDelBtnClicked()
{
  if ( !mHeaderList->currentItem() )
    return;

  if(KMessageBox::warningContinueCancel(this, i18n("Really delete this header?"),"",KGuiItem(i18n("&Delete"),"edit-delete"))==KMessageBox::Continue) {
    KNDisplayedHeader *h = ( static_cast<HdrItem*>( mHeaderList->currentItem() ) )->header();
    d_ata->remove(h);
    delete mHeaderList->takeItem( mHeaderList->currentRow() );
    s_ave=true;
  }
  emit changed(true);
}



void KNode::DisplayedHeadersWidget::slotEditBtnClicked()
{
  if ( !mHeaderList->currentItem() )
    return;
  KNDisplayedHeader *h = ( static_cast<HdrItem*>( mHeaderList->currentItem() ) )->header();

  DisplayedHeaderConfDialog* dlg=new DisplayedHeaderConfDialog(h, this);
  if(dlg->exec()) {
    int row = mHeaderList->currentRow();
    delete mHeaderList->takeItem( row );
    mHeaderList->insertItem( row, generateItem( h ) );
    mHeaderList->setCurrentRow( row );
    h->createTags();
    s_ave=true;
  }
  emit changed(true);
}



void KNode::DisplayedHeadersWidget::slotUpBtnClicked()
{
  int row =  mHeaderList->currentRow();
  if ( row <= 0 )
    return;

  KNDisplayedHeader *h = static_cast<HdrItem*>( mHeaderList->currentItem() )->header();

  d_ata->up(h);
  mHeaderList->insertItem( row -1, mHeaderList->takeItem( row ) );
  mHeaderList->setCurrentRow( row - 1 );
  s_ave=true;
  emit changed(true);
}



void KNode::DisplayedHeadersWidget::slotDownBtnClicked()
{
  int row = mHeaderList->currentRow();
  if ( row < 0 || row >= mHeaderList->count() )
    return;

  KNDisplayedHeader *h = static_cast<HdrItem*>( mHeaderList->currentItem() )->header();

  d_ata->down(h);
  mHeaderList->insertItem( row + 1, mHeaderList->takeItem( row ) );
  mHeaderList->setCurrentRow( row + 1 );
  s_ave=true;
  emit changed(true);
}


//=============================================================================================


KNode::DisplayedHeaderConfDialog::DisplayedHeaderConfDialog( KNDisplayedHeader *h, QWidget *parent )
  : KDialog( parent ),
    h_dr(h)
{
  setCaption( i18n("Header Properties") );
  setButtons( Ok | Cancel | Help );

  QWidget* page=new QWidget( this );
  setMainWidget( page );
  QGridLayout *topL=new QGridLayout(page);
  topL->setSpacing(5);
  topL->setMargin(0);

  QWidget *nameW = new QWidget(page);
  QGridLayout *nameL=new QGridLayout(nameW);
  nameL->setSpacing(5);

  h_drC=new KComboBox(true, nameW);
  h_drC->lineEdit()->setMaxLength(64);
  connect(h_drC, SIGNAL(activated(int)), this, SLOT(slotActivated(int)));
  QLabel *label = new QLabel( i18nc( "@label:textbox Edition of a message header name", "H&eader:" ), nameW );
  label->setBuddy(h_drC);
  nameL->addWidget(label,0,0);
  nameL->addWidget(h_drC,0,1);

  n_ameE=new KLineEdit(nameW);

  n_ameE->setMaxLength(64);
  label = new QLabel( i18nc( "@label:textbox Edition of the displayed name in the UI of a message header", "Displayed na&me:" ), nameW );
  label->setBuddy(n_ameE);
  nameL->addWidget(label,1,0);
  nameL->addWidget(n_ameE,1,1);
  nameL->setColumnStretch(1,1);

  topL->addWidget(nameW,0,0, 1, 2 );

  QGroupBox *ngb = new QGroupBox(i18n("Name"), page);
  // ### hide style settings for now, the new viewer doesn't support this yet
  ngb->hide();
  QVBoxLayout *ngbL = new QVBoxLayout(ngb);
  ngbL->setSpacing(5);
  ngbL->setMargin(8);
  ngbL->addSpacing(fontMetrics().lineSpacing()-4);
  n_ameCB[0]=new QCheckBox(i18n("&Large"), ngb);
  n_ameCB[1]=new QCheckBox(i18n("&Bold"), ngb);
  n_ameCB[2]=new QCheckBox(i18n("&Italic"), ngb);
  n_ameCB[3]=new QCheckBox(i18n("&Underlined"), ngb);
  for( int i = 0 ; i < 4 ; ++i) {
    ngbL->addWidget( n_ameCB[i] );
  }
  topL->addWidget(ngb,1,0);

  QGroupBox *vgb=new QGroupBox(i18n("Value"), page);
  // ### hide style settings for now, the new viewer doen't support this yet
  vgb->hide();
  QVBoxLayout *vgbL = new QVBoxLayout(vgb);
  vgbL->setSpacing(5);
  vgbL->setMargin(8);
  vgbL->addSpacing(fontMetrics().lineSpacing()-4);
  v_alueCB[0]=new QCheckBox(i18n("L&arge"), vgb);
  v_alueCB[1]=new QCheckBox(i18n("Bol&d"), vgb);
  v_alueCB[2]=new QCheckBox(i18n("I&talic"), vgb);
  v_alueCB[3]=new QCheckBox(i18n("U&nderlined"), vgb);
  for( int i = 0 ; i < 4 ; ++i) {
    vgbL->addWidget( v_alueCB[i] );
  }
  topL->addWidget(vgb,1,1);

  topL->setColumnStretch(0,1);
  topL->setColumnStretch(1,1);

  // preset values...
  h_drC->addItems( KNDisplayedHeader::predefs() );
  h_drC->lineEdit()->setText(h->header());
  n_ameE->setText(h->translatedName());
  for(int i=0; i<4; ++i) {
    n_ameCB[i]->setChecked(h->flag(i));
    v_alueCB[i]->setChecked(h->flag(i+4));
  }

  setFixedHeight(sizeHint().height());
  KNHelper::restoreWindowSize("accReadHdrPropDLG", this, sizeHint());

  connect(n_ameE, SIGNAL(textChanged(QString)), SLOT(slotNameChanged(QString)));

  setHelp("anc-knode-headers");
  slotNameChanged( n_ameE->text() );
  connect(this,SIGNAL(okClicked()),SLOT(slotOk()));
}


KNode::DisplayedHeaderConfDialog::~DisplayedHeaderConfDialog()
{
  KNHelper::saveWindowSize("accReadHdrPropDLG", size());
}


void KNode::DisplayedHeaderConfDialog::slotOk()
{
  h_dr->setHeader(h_drC->currentText());
  h_dr->setTranslatedName(n_ameE->text());
  for(int i=0; i<4; ++i) {
    if(h_dr->hasName())
      h_dr->setFlag(i, n_ameCB[i]->isChecked());
    else
      h_dr->setFlag(i,false);
    h_dr->setFlag(i+4, v_alueCB[i]->isChecked());
  }
  accept();
}


// the user selected one of the presets, insert the *translated* string as display name:
void KNode::DisplayedHeaderConfDialog::slotActivated(int pos)
{
  n_ameE->setText(i18n(h_drC->itemText(pos).toLocal8Bit()));  // I think it's save here, the combobox has only english defaults
}


// disable the name format options when the name is empty
void KNode::DisplayedHeaderConfDialog::slotNameChanged(const QString& str)
{
  for(int i=0; i<4; ++i)
      n_ameCB[i]->setEnabled(!str.isEmpty());
}

//=============================================================================================


KNode::ScoringWidget::ScoringWidget( const KComponentData &inst, QWidget *parent ) :
  KCModule( inst, parent )
{
  QGridLayout *topL = new QGridLayout(this);
  topL->setSpacing(5);
  topL->setMargin(5);
  mKsc = new KScoringEditorWidget( knGlobals.scoringManager(), this );
  topL->addWidget( mKsc, 0, 0, 1, 2 );

  topL->addItem( new QSpacerItem( 0, 10), 1, 0 );

  mIgnored = new KIntSpinBox( -100000, 100000, 1, 0, this );
  mIgnored->setObjectName( "kcfg_ignoredThreshold" );
  QLabel *l = new QLabel( i18n("Default score for &ignored threads:"), this );
  l->setBuddy( mIgnored );
  topL->addWidget(l, 2, 0);
  topL->addWidget( mIgnored, 2, 1 );

  mWatched = new KIntSpinBox( -100000, 100000, 1, 0, this );
  mWatched->setObjectName( "kcfg_watchedThreshold" );
  l = new QLabel( i18n("Default score for &watched threads:"), this );
  l->setBuddy( mWatched );
  topL->addWidget(l, 3, 0);
  topL->addWidget( mWatched, 3, 1);

  topL->setColumnStretch(0, 1);

  addConfig( knGlobals.settings(), this );
  load();
}


//=============================================================================================


KNode::FilterListWidget::FilterListWidget( const KComponentData &inst, QWidget *parent ) :
  KCModule( inst, parent ),
  f_ilManager( knGlobals.filterManager() )
{
  QGridLayout *topL=new QGridLayout(this);
  topL->setSpacing(5);
  topL->setMargin(5);

  // == Filters =================================================

  mFilterList = new QListWidget( this );
  QLabel *label = new QLabel( i18nc("@title", "&Filters:" ), this );
  label->setBuddy(mFilterList);
  topL->addWidget( label, 0, 0 );

  connect( mFilterList, SIGNAL(itemSelectionChanged()), SLOT(slotSelectionChangedFilter()) );
  connect( mFilterList, SIGNAL(itemActivated(QListWidgetItem*)), SLOT(slotEditBtnClicked()) );
  topL->addWidget( mFilterList, 1, 0, 5, 1);

  a_ddBtn = new QPushButton( i18nc("@action:button Add a new filter", "&Add..." ), this );
  connect(a_ddBtn, SIGNAL(clicked()), this, SLOT(slotAddBtnClicked()));
  topL->addWidget(a_ddBtn,1,1);

  e_ditBtn=new QPushButton(i18nc("modify something","&Edit..."), this);
  connect(e_ditBtn, SIGNAL(clicked()), this, SLOT(slotEditBtnClicked()));
  topL->addWidget(e_ditBtn,2,1);

  c_opyBtn = new QPushButton( i18nc( "@action:button Copy a filter", "Co&py..."), this);
  connect(c_opyBtn, SIGNAL(clicked()), this, SLOT(slotCopyBtnClicked()));
  topL->addWidget(c_opyBtn,3,1);

  d_elBtn = new QPushButton( i18nc( "@action:button Delete a filter", "&Delete"), this);
  connect(d_elBtn, SIGNAL(clicked()), this, SLOT(slotDelBtnClicked()));
  topL->addWidget(d_elBtn,4,1);

  // == Menu ====================================================

  mMenuList = new QListWidget( this );
  label = new QLabel( i18nc( "@title", "&Menu:"), this );
  label->setBuddy(mMenuList);
  topL->addWidget( label, 6, 0 );

  connect( mMenuList, SIGNAL(itemSelectionChanged()), SLOT(slotSelectionChangedMenu()) );
  topL->addWidget( mMenuList, 7, 0, 5, 1);

  u_pBtn = new QPushButton( i18nc( "@action:button move something up in a list", "&Up"), this );
  connect(u_pBtn, SIGNAL(clicked()), this, SLOT(slotUpBtnClicked()));
  topL->addWidget(u_pBtn,7,1);

  d_ownBtn = new QPushButton( i18nc( "@action:button move something down in a list", "Do&wn"), this );
  connect(d_ownBtn, SIGNAL(clicked()), this, SLOT(slotDownBtnClicked()));
  topL->addWidget(d_ownBtn,8,1);

  s_epAddBtn = new QPushButton( i18nc( "@action:button", "Add\n&Separator" ), this );
  connect(s_epAddBtn, SIGNAL(clicked()), this, SLOT(slotSepAddBtnClicked()));
  topL->addWidget(s_epAddBtn,9,1);

  s_epRemBtn = new QPushButton( i18nc( "@action:button", "&Remove\nSeparator" ), this );
  connect(s_epRemBtn, SIGNAL(clicked()), this, SLOT(slotSepRemBtnClicked()));
  topL->addWidget(s_epRemBtn,10,1);

  topL->setRowStretch(5,1);
  topL->setRowStretch(11,1);

  a_ctive = SmallIcon("view-filter",16);
  d_isabled = SmallIcon("view-filter",16,KIconLoader::DisabledState);

  load();

  slotSelectionChangedFilter();
  slotSelectionChangedMenu();
}


KNode::FilterListWidget::~FilterListWidget()
{
  f_ilManager->endConfig();
}


void KNode::FilterListWidget::load()
{
  mFilterList->clear();
  mMenuList->clear();
  f_ilManager->startConfig(this);
}

void KNode::FilterListWidget::save()
{
  f_ilManager->commitChanges();
}


void KNode::FilterListWidget::addItem(KNArticleFilter *f)
{
  FilterListItem *item = new FilterListItem( f , f->translatedName() );
  if(f->isEnabled())
    item->setIcon( a_ctive );
  else
    item->setIcon( d_isabled );
  mFilterList->addItem( item );
  slotSelectionChangedFilter();
  emit changed(true);
}


void KNode::FilterListWidget::removeItem(KNArticleFilter *f)
{
  int i = findItem( mFilterList, f );
  if ( i >= 0 )
    delete mFilterList->takeItem( i );
  slotSelectionChangedFilter();
  emit changed(true);
}


void KNode::FilterListWidget::updateItem(KNArticleFilter *f)
{
  int i = findItem( mFilterList, f );

  if ( i >= 0 ) {
    FilterListItem *item = static_cast<FilterListItem*>( mFilterList->item( i ) );
    item->setText( f->translatedName() );
    if ( f->isEnabled() ) {
      item->setIcon( a_ctive );
      i = findItem( mMenuList, f );
      if ( i >= 0 )
        mMenuList->item( i )->setText( f->translatedName() );
    } else
      item->setIcon( d_isabled );
  }
  slotSelectionChangedFilter();
  emit changed(true);
}


void KNode::FilterListWidget::addMenuItem(KNArticleFilter *f)
{
  if (f) {
    if ( findItem( mMenuList, f) < 0 )
      mMenuList->addItem( new FilterListItem( f, f->translatedName() ) );
  } else   // separator
    mMenuList->addItem( new FilterListItem( 0, "===" ) );
  slotSelectionChangedMenu();
  emit changed(true);
}


void KNode::FilterListWidget::removeMenuItem(KNArticleFilter *f)
{
  int i = findItem( mMenuList, f );
  if ( i >= 0 )
    delete mMenuList->takeItem( i );
  slotSelectionChangedMenu();
  emit changed(true);
}


QList<int> KNode::FilterListWidget::menuOrder()
{
  KNArticleFilter *f;
  QList<int> lst;

  for( int i = 0; i < mMenuList->count(); ++i ) {
    f = static_cast<FilterListItem*>( mMenuList->item( i ) )->filter();
    if ( f )
      lst << f->id();
    else
      lst << -1;
  }
 return lst;
}


int KNode::FilterListWidget::findItem( QListWidget *l, KNArticleFilter *f )
{
  for ( int i = 0; i < l->count(); ++i )
    if ( static_cast<FilterListItem*>( l->item( i ) )->filter() == f )
      return i;
  return -1;
}


void KNode::FilterListWidget::slotAddBtnClicked()
{
  f_ilManager->newFilter();
}


void KNode::FilterListWidget::slotDelBtnClicked()
{
  if ( mFilterList->currentItem() )
    f_ilManager->deleteFilter( static_cast<FilterListItem*>( mFilterList->currentItem() )->filter() );
}


void KNode::FilterListWidget::slotEditBtnClicked()
{
  if ( mFilterList->currentItem() )
    f_ilManager->editFilter( static_cast<FilterListItem*>( mFilterList->currentItem() )->filter() );
}


void KNode::FilterListWidget::slotCopyBtnClicked()
{
  if ( mFilterList->currentItem() )
    f_ilManager->copyFilter( static_cast<FilterListItem*>( mFilterList->currentItem() )->filter() );
}


void KNode::FilterListWidget::slotUpBtnClicked()
{
  int row = mMenuList->currentRow();
  if ( row <= 0)
    return;
  mMenuList->insertItem( row - 1, mMenuList->takeItem( row ) );
  mMenuList->setCurrentRow( row - 1 );
  emit changed(true);
}


void KNode::FilterListWidget::slotDownBtnClicked()
{
  int row = mMenuList->currentRow();
  if ( row < 0 || row > mMenuList->count() - 1 )
    return;
  mMenuList->insertItem( row + 1, mMenuList->takeItem( row ) );
  mMenuList->setCurrentRow( row + 1 );
  emit changed(true);
}


void KNode::FilterListWidget::slotSepAddBtnClicked()
{
  mMenuList->insertItem( mMenuList->currentRow(), new FilterListItem( 0, "===" ) );
  slotSelectionChangedMenu();
  emit changed(true);
}


void KNode::FilterListWidget::slotSepRemBtnClicked()
{
  FilterListItem *item = static_cast<FilterListItem*>( mMenuList->currentItem() );
  if ( item && item->filter() == 0 )
    delete item;
  slotSelectionChangedMenu();
  emit changed(true);
}


void KNode::FilterListWidget::slotSelectionChangedFilter()
{
  QListWidgetItem *item = mFilterList->currentItem();

  d_elBtn->setEnabled( item );
  e_ditBtn->setEnabled( item );
  c_opyBtn->setEnabled( item );
}


void KNode::FilterListWidget::slotSelectionChangedMenu()
{
  int current = mMenuList->currentRow();

  u_pBtn->setEnabled( current > 0 );
  d_ownBtn->setEnabled( current >= 0 && ( current < mMenuList->count() - 1 ) );
  s_epRemBtn->setEnabled( current >= 0 && ( static_cast<FilterListItem*>( mMenuList->item( current ) )->filter() == 0 ) );
}


//=============================================================================================


KNode::PostNewsTechnicalWidget::PostNewsTechnicalWidget( const KComponentData &inst, QWidget *parent ) :
  KCModule( inst, parent )
{
  setupUi( this );

  mCharset->addItems( KNode::Utilities::Locale::encodings() );
  mEncoding->addItem( i18n("Allow 8-bit") );
  mEncoding->addItem( i18n("7-bit (Quoted-Printable)") );

  connect( mHeaderList, SIGNAL(itemActivated(QListWidgetItem*)), SLOT(slotEditBtnClicked()) );
  connect( mHeaderList, SIGNAL(itemSelectionChanged()), SLOT(slotSelectionChanged()) );

  connect( mAddButton, SIGNAL(clicked()), SLOT(slotAddBtnClicked()) );
  connect( mEditButton, SIGNAL(clicked()), SLOT(slotEditBtnClicked()) );
  connect( mDeleteButton, SIGNAL(clicked()), SLOT(slotDelBtnClicked()) );

  addConfig( knGlobals.settings(), this );
  load();

  slotSelectionChanged();
}


void KNode::PostNewsTechnicalWidget::load()
{
  KCModule::load();

  QString charsetDesc = KGlobal::charsets()->descriptionForEncoding( knGlobals.settings()->charset() );
  mCharset->setCurrentIndex( mCharset->findText( charsetDesc ) );
  mEncoding->setCurrentIndex( knGlobals.settings()->allow8BitBody() ? 0 : 1 );

  mHeaderList->clear();
  XHeader::List list = knGlobals.settings()->xHeaders();
  for ( XHeader::List::Iterator it = list.begin(); it != list.end(); ++it )
    mHeaderList->addItem( (*it).header() );
}


void KNode::PostNewsTechnicalWidget::save()
{
  QString charset = KGlobal::charsets()->encodingForName( mCharset->currentText() );
  knGlobals.settings()->setCharset( charset );
  knGlobals.settings()->setAllow8BitBody( mEncoding->currentIndex() == 0 );

  XHeader::List list;
  for ( int i = 0; i < mHeaderList->count(); ++i )
    list.append( XHeader( mHeaderList->item( i )->text() ) );
  knGlobals.settings()->setXHeaders( list );

  KCModule::save();
}


void KNode::PostNewsTechnicalWidget::slotSelectionChanged()
{
  mDeleteButton->setEnabled( mHeaderList->currentItem() != 0 );
  mEditButton->setEnabled( mHeaderList->currentItem() != 0 );
}


void KNode::PostNewsTechnicalWidget::slotAddBtnClicked()
{
  XHeaderConfDialog *dlg = new XHeaderConfDialog( QString(), this );
  if ( dlg->exec() )
    mHeaderList->addItem( dlg->result() );

  delete dlg;

  slotSelectionChanged();
  emit changed( true );
}


void KNode::PostNewsTechnicalWidget::slotDelBtnClicked()
{
  QListWidgetItem *item = mHeaderList->currentItem();
  if ( !item )
    return;
  delete item;
  slotSelectionChanged();
  emit changed( true );
}


void KNode::PostNewsTechnicalWidget::slotEditBtnClicked()
{
  QListWidgetItem *item = mHeaderList->currentItem();
  if ( !item )
    return;

  XHeaderConfDialog *dlg = new XHeaderConfDialog( item->text(), this );
  if ( dlg->exec() )
    item->setText( dlg->result() );

  delete dlg;

  slotSelectionChanged();
  emit changed( true );
}


//===================================================================================================


KNode::XHeaderConfDialog::XHeaderConfDialog( const QString &h, QWidget *parent ) :
  KDialog( parent )
{
  setCaption( i18n("Additional Header") );
  setButtons( Ok | Cancel );

  KHBox* page = new KHBox( this );
  setMainWidget( page );

  mNameEdit = new KLineEdit( page );
  new QLabel( ":", page );
  mValueEdit = new KLineEdit( page );

  int pos = h.indexOf( ": " );
  if ( pos != -1 ) {
    mNameEdit->setText( h.left( pos ) );
    pos += 2;
    mValueEdit->setText( h.right( h.length() - pos ) );
  }

  setFixedHeight(sizeHint().height());
  KNHelper::restoreWindowSize("XHeaderDlg", this, sizeHint());

  mNameEdit->setFocus();
}


KNode::XHeaderConfDialog::~XHeaderConfDialog()
{
  KNHelper::saveWindowSize("XHeaderDlg", size());
}


QString KNode::XHeaderConfDialog::result() const
{
  QString value = mValueEdit->text();
  // just in case someone pastes a newline
  value.replace( '\n', ' ' );
  return mNameEdit->text() + ": " + value;
}


//===================================================================================================


KNode::PostNewsComposerWidget::PostNewsComposerWidget( const KComponentData &inst, QWidget *parent ) :
  KCModule( inst, parent )
{
  KNode::Ui::PostNewsComposerWidgetBase ui;
  ui.setupUi( this );
  addConfig( knGlobals.settings(), this );
  load();
}


//===================================================================================================


KNode::PostNewsSpellingWidget::PostNewsSpellingWidget( const KComponentData &inst, QWidget *parent ) :
  KCModule( inst, parent )
{
  QVBoxLayout *topL=new QVBoxLayout(this);
  topL->setSpacing(5);

  c_conf = new Sonnet::ConfigWidget(KNGlobals::self()->config(), this );
  topL->addWidget(c_conf);
  connect(c_conf, SIGNAL(configChanged()), SLOT(changed()));

  topL->addStretch(1);
}


KNode::PostNewsSpellingWidget::~PostNewsSpellingWidget()
{
}


void KNode::PostNewsSpellingWidget::save()
{
  c_conf->save();
}


//==============================================================================================================

KNode::PrivacyWidget::PrivacyWidget( const KComponentData &inst,QWidget *parent ) :
  KCModule(inst, parent )
{
  QBoxLayout *topLayout = new QVBoxLayout(this);
  topLayout->setSpacing(5);
  c_onf = new Kpgp::Config( this, false );
  c_onf->setObjectName( "knode pgp config" );
  topLayout->addWidget(c_onf);
  connect(c_onf, SIGNAL(changed()), SLOT(changed()));

  topLayout->addStretch(1);

  load();
}


KNode::PrivacyWidget::~PrivacyWidget()
{
}


void KNode::PrivacyWidget::save()
{
  c_onf->applySettings();
}


//==============================================================================================================


//BEGIN: Cleanup configuration widgets ---------------------------------------


KNode::GroupCleanupWidget::GroupCleanupWidget( Cleanup *data, QWidget *parent )
  : QWidget( parent ), mData( data )
{
  QVBoxLayout *top = new QVBoxLayout( this );

  if (!mData->isGlobal()) {
    mDefault = new QCheckBox( i18n("&Use global cleanup configuration"), this );
    connect( mDefault, SIGNAL(toggled(bool)), SLOT(slotDefaultToggled(bool)) );
    top->addWidget( mDefault );
  }

  mExpGroup = new QGroupBox( i18n("Newsgroup Cleanup Settings"), this );
  top->addWidget( mExpGroup );
  QGridLayout *grid = new QGridLayout( mExpGroup );
  grid->setSpacing( KDialog::spacingHint() );
  grid->setMargin( KDialog::marginHint() );

  grid->setRowMinimumHeight( 0, KDialog::spacingHint() );

  mExpEnabled = new QCheckBox( i18n("&Expire old articles automatically"), mExpGroup );
  grid->addWidget( mExpEnabled, 1, 0, 1, 2 );
  connect( mExpEnabled, SIGNAL(toggled(bool)), SIGNAL(changed()) );

  mExpDays = new KIntSpinBox( 0, 99999, 1, 0, mExpGroup );
  mExpDays->setSuffix(ki18np(" day", " days"));
  QLabel *label = new QLabel( i18n("&Purge groups every:"), mExpGroup );
  label->setBuddy( mExpDays );
  grid->addWidget( label, 2, 0 );
  grid->addWidget( mExpDays, 2, 1, Qt::AlignRight );
  connect( mExpDays, SIGNAL(valueChanged(int)), SIGNAL(changed()) );
  connect( mExpEnabled, SIGNAL(toggled(bool)), label, SLOT(setEnabled(bool)) );
  connect( mExpEnabled, SIGNAL(toggled(bool)), mExpDays, SLOT(setEnabled(bool)) );

  mExpReadDays = new KIntSpinBox( 0, 99999, 1, 0, mExpGroup );
  mExpReadDays->setSuffix(ki18np(" day", " days"));
  label = new QLabel( i18n("&Keep read articles:"), mExpGroup );
  label->setBuddy( mExpReadDays );
  grid->addWidget( label, 3, 0 );
  grid->addWidget( mExpReadDays, 3, 1, Qt::AlignRight );
  connect( mExpReadDays, SIGNAL(valueChanged(int)), SIGNAL(changed()) );

  mExpUnreadDays = new KIntSpinBox( 0, 99999, 1, 0, mExpGroup );
  mExpUnreadDays->setSuffix(ki18np(" day", " days"));
  label = new QLabel( i18n("Keep u&nread articles:"), mExpGroup );
  label->setBuddy( mExpUnreadDays );
  grid->addWidget( label, 4, 0 );
  grid->addWidget( mExpUnreadDays, 4, 1, Qt::AlignRight );
  connect( mExpUnreadDays, SIGNAL(valueChanged(int)), SIGNAL(changed()) );

  mExpUnavailable = new QCheckBox( i18n("&Remove articles that are not available on the server"), mExpGroup );
  grid->addWidget( mExpUnavailable, 5, 0, 1, 2 );
  connect( mExpUnavailable, SIGNAL(toggled(bool)), SIGNAL(changed()) );

  mPreserveThreads = new QCheckBox( i18n("Preser&ve threads"), mExpGroup );
  grid->addWidget( mPreserveThreads, 6, 0, 1, 2 );
  connect( mPreserveThreads, SIGNAL(toggled(bool)), SIGNAL(changed()) );

  grid->setColumnStretch(1,1);
}


void KNode::GroupCleanupWidget::load()
{
  if (!mData->isGlobal()) {
    mDefault->setChecked( mData->useDefault() );
    slotDefaultToggled( mData->useDefault() );
  }
  mExpEnabled->setChecked( !mData->d_oExpire ); // make sure the toggled(bool) signal is emitted at least once
  mExpEnabled->setChecked( mData->d_oExpire );
  mExpDays->setValue( mData->e_xpireInterval );
  mExpReadDays->setValue( mData->maxAgeForRead() );
  mExpUnreadDays->setValue( mData->maxAgeForUnread() );
  mExpUnavailable->setChecked( mData->removeUnavailable() );
  mPreserveThreads->setChecked( mData->preserveThreads() );
}


void KNode::GroupCleanupWidget::save()
{
  if (!mData->isGlobal())
    mData->setUseDefault( mDefault->isChecked() );
  mData->d_oExpire = mExpEnabled->isChecked();
  mData->e_xpireInterval = mExpDays->value();
  mData->r_eadMaxAge = mExpReadDays->value();
  mData->u_nreadMaxAge = mExpUnreadDays->value();
  mData->r_emoveUnavailable = mExpUnavailable->isChecked();
  mData->p_reserveThr = mPreserveThreads->isChecked();
}


void KNode::GroupCleanupWidget::slotDefaultToggled( bool state )
{
    mExpGroup->setEnabled( !state );
}


KNode::CleanupWidget::CleanupWidget( const KComponentData &inst,QWidget *parent ) :
  KCModule(inst, parent ),
  d_ata( knGlobals.configManager()->cleanup() )
{
  QVBoxLayout *topL=new QVBoxLayout(this);
  topL->setSpacing(5);

  mGroupCleanup = new GroupCleanupWidget( d_ata, this );
  topL->addWidget( mGroupCleanup );
  connect( mGroupCleanup, SIGNAL(changed()), SLOT(changed()) );

  // === folders =========================================================

  QGroupBox *foldersB = new QGroupBox( i18n("Folders"), this );
  topL->addWidget(foldersB);
  QGridLayout *foldersL = new QGridLayout( foldersB );
  foldersL->setSpacing( KDialog::spacingHint() );
  foldersL->setMargin( KDialog::marginHint() );

  foldersL->setRowMinimumHeight( 0, KDialog::spacingHint() );

  f_olderCB=new QCheckBox(i18n("Co&mpact folders automatically"), foldersB);
  connect(f_olderCB, SIGNAL(toggled(bool)), this, SLOT(slotFolderCBtoggled(bool)));
  foldersL->addWidget(f_olderCB,1,0, 1, 2 );

  f_olderDays=new KIntSpinBox( 0, 99999, 1, 0, foldersB );
  f_olderDays->setSuffix(ki18np(" day", " days"));
  f_olderDaysL=new QLabel(i18n("P&urge folders every:"),foldersB);
  f_olderDaysL->setBuddy(f_olderDays);
  foldersL->addWidget(f_olderDaysL,2,0);
  foldersL->addWidget(f_olderDays,2,1,Qt::AlignRight);
  connect(f_olderDays, SIGNAL(valueChanged(int)), SLOT(changed()));

  foldersL->setColumnStretch(1,1);

  topL->addStretch(1);

  load();
}


KNode::CleanupWidget::~CleanupWidget()
{
}


void KNode::CleanupWidget::load()
{
  f_olderCB->setChecked(d_ata->d_oCompact);
  slotFolderCBtoggled(d_ata->d_oCompact);
  f_olderDays->setValue(d_ata->c_ompactInterval);
  mGroupCleanup->load();
}


void KNode::CleanupWidget::save()
{
  d_ata->d_oCompact=f_olderCB->isChecked();
  d_ata->c_ompactInterval=f_olderDays->value();

  mGroupCleanup->save();

  d_ata->setDirty(true);
}


void KNode::CleanupWidget::slotFolderCBtoggled(bool b)
{
  f_olderDaysL->setEnabled(b);
  f_olderDays->setEnabled(b);
  emit changed(true);
}

//END: Cleanup configuration widgets -----------------------------------------

//------------------------

