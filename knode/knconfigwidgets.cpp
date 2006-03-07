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


#include <qpainter.h>

#include <qlabel.h>
//Added by qt3to4:
#include <QGridLayout>
#include <QHBoxLayout>
#include <QBoxLayout>
#include <QVBoxLayout>
#include <QTextEdit>

#include <kcharsets.h>
#include <klocale.h>
#include <knumvalidator.h>
#include <kmessagebox.h>
#include <kcolordialog.h>
#include <kfontdialog.h>
#include <kfiledialog.h>
#include <kuserprofile.h>
#include <kopenwith.h>
#include <kscoringeditor.h>
#include <kspell.h>
#include <kcombobox.h>
#include <kpgpui.h>
#include <kurlcompletion.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>
#include <kvbox.h>

#include "knaccountmanager.h"
#include "knconfig.h"
#include "knconfigwidgets.h"
#include "knconfigmanager.h"
#include "kndisplayedheader.h"
#include "kngroupmanager.h"
#include "knglobals.h"
#include "knnntpaccount.h"
#include "utilities.h"
#include "knfiltermanager.h"
#include "knarticlefilter.h"
#include "knscoring.h"
#include "postnewscomposerwidget_base.h"
#include "readnewsnavigationwidget_base.h"
#include "readnewsviewerwidget_base.h"
#include "settings.h"
#include <kpgp.h>

KNode::IdentityWidget::IdentityWidget( Identity *d, KInstance *inst, QWidget *parent ) :
  KCModule( inst ,parent ),
  d_ata( d )
{
  QString msg;

  QGridLayout *topL=new QGridLayout(this,  11, 3, 5,5);

  n_ame=new KLineEdit(this);
  QLabel *l=new QLabel(n_ame, i18n("&Name:"), this);
  topL->addWidget(l, 0,0);
  topL->addMultiCellWidget(n_ame, 0,0, 1,2);
  msg = i18n("<qt><p>Your name as it will appear to others reading your articles.</p>"
      "<p>Ex: <b>John Stuart Masterson III</b>.</p></qt>");
  n_ame->setWhatsThis( msg );
  l->setWhatsThis( msg );
  connect( n_ame, SIGNAL(textChanged(const QString&)), SLOT(changed()) );

  o_rga=new KLineEdit(this);
  l=new QLabel(o_rga, i18n("Organi&zation:"), this);
  topL->addWidget(l, 1,0);
  topL->addMultiCellWidget(o_rga, 1,1, 1,2);
  msg = i18n( "<qt><p>The name of the organization you work for.</p>"
      "<p>Ex: <b>KNode, Inc</b>.</p></qt>" );
  o_rga->setWhatsThis( msg );
  l->setWhatsThis( msg );
  connect( o_rga, SIGNAL(textChanged(const QString&)), SLOT(changed()) );

  e_mail=new KLineEdit(this);
  l=new QLabel(e_mail, i18n("Email a&ddress:"), this);
  topL->addWidget(l, 2,0);
  topL->addMultiCellWidget(e_mail, 2,2, 1,2);
  msg = i18n( "<qt><p>Your email address as it will appear to others "
      "reading your articles</p><p>Ex: <b>nospam@please.com</b>.</qt>" );
  l->setWhatsThis( msg );
  e_mail->setWhatsThis( msg );
  connect( e_mail, SIGNAL(textChanged(const QString&)), SLOT(changed()) );

  r_eplyTo=new KLineEdit(this);
  l=new QLabel(r_eplyTo, i18n("&Reply-to address:"), this);
  topL->addWidget(l, 3,0);
  topL->addMultiCellWidget(r_eplyTo, 3,3, 1,2);
  msg = i18n( "<qt><p>When someone reply to your article by email, this is the address the message "
      "will be sent. If you fill in this field, please do it with a real "
      "email address.</p><p>Ex: <b>john@example.com</b>.</p></qt>" );
  l->setWhatsThis( msg );
  r_eplyTo->setWhatsThis( msg );
  connect( r_eplyTo, SIGNAL(textChanged(const QString&)), SLOT(changed()) );

  m_ailCopiesTo=new KLineEdit(this);
  l=new QLabel(m_ailCopiesTo, i18n("&Mail-copies-to:"), this);
  topL->addWidget(l, 4,0);
  topL->addMultiCellWidget(m_ailCopiesTo, 4,4, 1,2);
  connect( m_ailCopiesTo, SIGNAL(textChanged(const QString&)), SLOT(changed()) );

  s_igningKey = new Kpgp::SecretKeyRequester(this);
  s_igningKey->dialogButton()->setText(i18n("Chan&ge..."));
  s_igningKey->setDialogCaption(i18n("Your OpenPGP Key"));
  s_igningKey->setDialogMessage(i18n("Select the OpenPGP key which should be "
      "used for signing articles."));
  l=new QLabel(s_igningKey, i18n("Signing ke&y:"), this);
  topL->addWidget(l, 5,0);
  topL->addMultiCellWidget(s_igningKey, 5,5, 1,2);
  msg = i18n("<qt><p>The OpenPGP key you choose here will be "
      "used to sign your articles.</p></qt>");
  l->setWhatsThis( msg );
  s_igningKey->setWhatsThis( msg );
  connect( s_igningKey, SIGNAL(changed()), SLOT(changed()) );

  b_uttonGroup = new QButtonGroup( this );
  connect( b_uttonGroup, SIGNAL( buttonClicked( int ) ),
           this, SLOT( slotSignatureType( int ) ) );
  b_uttonGroup->setExclusive( true );

  s_igFile = new QRadioButton( i18n("&Use a signature from file"), this );
  b_uttonGroup->addButton( s_igFile, 0 );
  topL->addMultiCellWidget(s_igFile, 6, 6, 0, 2);
  s_igFile->setWhatsThis(
                   i18n( "<qt><p>Mark this to let KNode read the signature from a file.</p></qt>" ) );
  s_ig = new KLineEdit(this);

  f_ileName = new QLabel(s_ig, i18n("Signature &file:"), this);
  topL->addWidget(f_ileName, 7, 0 );
  topL->addWidget(s_ig, 7, 1 );
  c_ompletion = new KUrlCompletion();
  s_ig->setCompletionObject(c_ompletion);
  msg = i18n( "<qt><p>The file from which the signature will be read.</p>"
      "<p>Ex: <b>/home/robt/.sig</b>.</p></qt>" );
  f_ileName->setWhatsThis( msg );
  s_ig->setWhatsThis( msg );

  c_hooseBtn = new QPushButton( i18n("Choo&se..."), this);
  connect(c_hooseBtn, SIGNAL(clicked()),
          this, SLOT(slotSignatureChoose()));
  topL->addWidget(c_hooseBtn, 7, 2 );
  e_ditBtn = new QPushButton( i18n("&Edit File"), this);
  connect(e_ditBtn, SIGNAL(clicked()),
          this, SLOT(slotSignatureEdit()));
  topL->addWidget(e_ditBtn, 8, 2);

  s_igGenerator = new QCheckBox(i18n("&The file is a program"), this);
  topL->addMultiCellWidget(s_igGenerator, 8, 8, 0, 1);
  msg = i18n( "<qt><p>Mark this option if the signature will be generated by a program</p>"
      "<p>Ex: <b>/home/robt/gensig.sh</b>.</p></qt>" );
  s_igGenerator->setWhatsThis( msg );
  connect( s_igGenerator, SIGNAL(toggled(bool)), SLOT(changed()) );

  s_igEdit = new QRadioButton( i18n("Specify signature &below"), this);
  b_uttonGroup->addButton( s_igEdit, 1 );
  topL->addMultiCellWidget(s_igEdit, 9, 9, 0, 2);

  s_igEditor = new QTextEdit( this );
  topL->addMultiCellWidget(s_igEditor, 10, 10, 0, 2);
  connect( s_igEditor, SIGNAL(textChanged()), SLOT(changed()) );

  topL->setColStretch(1,1);
  topL->setRowStretch(7,1);
  topL->setResizeMode(QLayout::SetMinimumSize);
  connect(s_ig,SIGNAL(textChanged ( const QString & )),
          this,SLOT(textFileNameChanged(const QString &)));

  load();
}


KNode::IdentityWidget::~IdentityWidget()
{
  delete c_ompletion;
}

void KNode::IdentityWidget::textFileNameChanged(const QString &text)
{
    e_ditBtn->setEnabled(!text.isEmpty());
    emit changed( true );
}

void KNode::IdentityWidget::load()
{
  kDebug() << "void KNConfig::IdentityWidget::load()" << endl;
  n_ame->setText(d_ata->n_ame);
  o_rga->setText(d_ata->o_rga);
  e_mail->setText(d_ata->e_mail);
  r_eplyTo->setText(d_ata->r_eplyTo);
  m_ailCopiesTo->setText(d_ata->m_ailCopiesTo);
  s_igningKey->setKeyIDs( Kpgp::KeyIDList() << d_ata->s_igningKey.toLatin1() );
  s_ig->setText(d_ata->s_igPath);
  s_igGenerator->setChecked(d_ata->useSigGenerator());
  s_igEditor->setPlainText( d_ata->s_igText );
  slotSignatureType(d_ata->useSigFile()? 0:1);
}

void KNode::IdentityWidget::save()
{
  d_ata->n_ame=n_ame->text();
  d_ata->o_rga=o_rga->text();
  d_ata->e_mail=e_mail->text();
  d_ata->r_eplyTo=r_eplyTo->text();
  d_ata->m_ailCopiesTo=m_ailCopiesTo->text();
  d_ata->s_igningKey = s_igningKey->keyIDs().first();
  d_ata->u_seSigFile=s_igFile->isChecked();
  d_ata->u_seSigGenerator=s_igGenerator->isChecked();
  d_ata->s_igPath=c_ompletion->replacedPath(s_ig->text());
  d_ata->s_igText = s_igEditor->toPlainText();

  if(d_ata->isGlobal())
    d_ata->save();
}

void KNode::IdentityWidget::slotSignatureType(int type)
{
  bool sigFromFile = (type==0);

  b_uttonGroup->button( type )->setChecked( true );
  f_ileName->setEnabled(sigFromFile);
  s_ig->setEnabled(sigFromFile);
  c_hooseBtn->setEnabled(sigFromFile);
  e_ditBtn->setEnabled(sigFromFile && !s_ig->text().isEmpty());
  s_igGenerator->setEnabled(sigFromFile);
  s_igEditor->setEnabled(!sigFromFile);

  if (sigFromFile)
    f_ileName->setFocus();
  else
    s_igEditor->setFocus();
  emit changed( true );
}


void KNode::IdentityWidget::slotSignatureChoose()
{
  QString tmp = KFileDialog::getOpenFileName( c_ompletion->replacedPath( s_ig->text() ), QString(), this, i18n("Choose Signature") );
  if(!tmp.isEmpty()) s_ig->setText(tmp);
  emit changed( true );
}


void KNode::IdentityWidget::slotSignatureEdit()
{
  QString fileName = c_ompletion->replacedPath(s_ig->text()).trimmed();

  if (fileName.isEmpty()) {
    KMessageBox::sorry(this, i18n("You must specify a filename."));
    return;
  }

  QFileInfo fileInfo( fileName );
  if (fileInfo.isDir()) {
    KMessageBox::sorry(this, i18n("You have specified a folder."));
    return;
  }

  KService::Ptr offer = KServiceTypeProfile::preferredService("text/plain", "Application");
  KUrl u(fileName);

  if (offer)
    KRun::run(*offer, u);
  else
    KRun::displayOpenWithDialog(u);
  emit changed( true );
}



//==========================================================================================

//BEGIN: NNTP account configuration widgets ----------------------------------

KNode::NntpAccountListWidget::NntpAccountListWidget( KInstance *inst, QWidget *parent ) :
  KCModule( inst, parent )
{
  setupUi( this );

  // account listbox
  connect( mAccountList, SIGNAL( itemActivated( QListWidgetItem* ) ), SLOT( slotEditBtnClicked() ) );
  connect( mAccountList, SIGNAL( itemSelectionChanged() ), SLOT( slotSelectionChanged() ) );

  // buttons
  connect( mAddButton, SIGNAL( clicked() ), SLOT( slotAddBtnClicked() ) );
  connect( mEditButton, SIGNAL( clicked() ), SLOT( slotEditBtnClicked() ) );
  connect( mDeleteButton, SIGNAL( clicked() ), SLOT( slotDelBtnClicked() ) );
  connect( mSubscribeButton, SIGNAL( clicked() ), SLOT( slotSubBtnClicked() ) );

  load();

  // the settings dialog is non-modal, so we have to react to changes
  // made outside of the dialog
  KNAccountManager *am = knGlobals.accountManager();
  connect( am, SIGNAL( accountAdded( KNNntpAccount* ) ), SLOT( slotAddItem( KNNntpAccount* ) ) );
  connect( am, SIGNAL( accountRemoved( KNNntpAccount* ) ), SLOT( slotRemoveItem( KNNntpAccount* ) ) );
  connect( am, SIGNAL( accountModified( KNNntpAccount* ) ), SLOT( slotUpdateItem( KNNntpAccount* ) ) );

  slotSelectionChanged();     // disable Delete & Edit initially
}


void KNode::NntpAccountListWidget::load()
{
  mAccountList->clear();
  KNAccountManager::List list = knGlobals.accountManager()->accounts();
  for ( KNAccountManager::List::Iterator it = list.begin(); it != list.end(); ++it )
    slotAddItem( *it );
}


void KNode::NntpAccountListWidget::slotAddItem(KNNntpAccount *a)
{
  AccountListItem *item;
  item = new AccountListItem( a );
  item->setText( a->name() );
  item->setIcon( SmallIcon( "server" ) );
  mAccountList->addItem( item );
  emit changed( true );
}


void KNode::NntpAccountListWidget::slotRemoveItem(KNNntpAccount *a)
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


void KNode::NntpAccountListWidget::slotUpdateItem(KNNntpAccount *a)
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
    mServerInfo->setText( i18n("Server: %1").arg( item->account()->server() ) );
    mPortInfo->setText( i18n("Port: %1").arg( item->account()->port() ) );
  } else {
    mServerInfo->setText( i18n("Server: ") );
    mPortInfo->setText( i18n("Port: ") );
  }
}



void KNode::NntpAccountListWidget::slotAddBtnClicked()
{
  KNNntpAccount *acc = new KNNntpAccount();

  if(acc->editProperties(this)) {
    knGlobals.accountManager()->newAccount(acc);
    acc->saveInfo();
  }
  else
    delete acc;
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
    KDialogBase( Tabbed, (a->id() != -1) ? i18n("Properties of %1").arg(a->name()) : i18n("New Account"),
                 Ok | Cancel | Help, Ok, parent ),
    mAccount( a )
{
  // server config tab
  QFrame* page = addPage( i18n("Ser&ver") );
  setupUi( page );

  mName->setText( a->name() );
  mServer->setText( a->server() );
  mPort->setValue( a->port() );
  mFetchDesc->setChecked( a->fetchDescriptions() );

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

  mIntervalChecking->setChecked( a->intervalChecking() );
  mInterval->setValue( a->checkInterval() );

  // identity tab
  mIdentityWidget = new KNode::IdentityWidget( a->identity(), knGlobals.instance(), addVBoxPage(i18n("&Identity") ) );

  // per server cleanup configuration
  QFrame* cleanupPage = addPage( i18n("&Cleanup") );
  QVBoxLayout *cleanupLayout = new QVBoxLayout( cleanupPage, KDialog::spacingHint() );
  mCleanupWidget = new GroupCleanupWidget( a->cleanupConfig(), cleanupPage );
  mCleanupWidget->load();
  cleanupLayout->addWidget( mCleanupWidget );
  cleanupLayout->addStretch( 1 );


  KNHelper::restoreWindowSize("accNewsPropDLG", this, sizeHint());

  setHelp("anc-setting-the-news-account");
}


KNode::NntpAccountConfDialog::~NntpAccountConfDialog()
{
  KNHelper::saveWindowSize("accNewsPropDLG", size());
}


void KNode::NntpAccountConfDialog::slotOk()
{
  if ( mName->text().isEmpty() || mServer->text().trimmed().isEmpty() ) {
    KMessageBox::sorry(this, i18n("Please enter an arbitrary name for the account and the\nhostname of the news server."));
    return;
  }

  mAccount->setName( mName->text() );
  mAccount->setServer( mServer->text().trimmed() );
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
    mAccount->saveInfo();

  mIdentityWidget->save();
  mCleanupWidget->save();

  accept();
}


void KNode::NntpAccountConfDialog::slotPasswordChanged()
{
  if ( mPassword->text().isEmpty() )
    mPassword->setText( mAccount->pass() );
}

//END: NNTP account configuration widgets ------------------------------------

//=============================================================================================

KNode::SmtpAccountWidget::SmtpAccountWidget( KInstance *inst, QWidget *parent ) :
    KCModule( inst, parent )
{
  setupUi( this );

  connect( mUseExternalMailer, SIGNAL( toggled(bool) ), SLOT( useExternalMailerToggled(bool) ) );
  connect( mLogin, SIGNAL( toggled(bool) ), SLOT( loginToggled(bool) ) );

  mAccount = knGlobals.accountManager()->smtp();
  connect( knGlobals.accountManager(), SIGNAL(passwordsChanged()), SLOT(slotPasswordChanged()) );
  load();
}


void KNode::SmtpAccountWidget::load()
{
  mUseExternalMailer->setChecked( knGlobals.settings()->useExternalMailer() );
  useExternalMailerToggled( knGlobals.settings()->useExternalMailer() );
  mServer->setText( mAccount->server() );
  mPort->setValue( mAccount->port() );
  mLogin->setChecked( mAccount->needsLogon() );
  loginToggled( mAccount->needsLogon() );
  mUser->setText( mAccount->user() );
  if ( mAccount->readyForLogin() )
    mPassword->setText( mAccount->pass() );
  else
    if ( mAccount->needsLogon() )
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
}


void KNode::SmtpAccountWidget::save()
{
  knGlobals.settings()->setUseExternalMailer( mUseExternalMailer->isChecked() );

  mAccount->setServer( mServer->text() );
  mAccount->setPort( mPort->value() );
  mAccount->setNeedsLogon( mLogin->isChecked() );
  if ( mAccount->needsLogon() ) {
    mAccount->setUser( mUser->text() );
    mAccount->setPass( mPassword->text() );
  }
  if ( mEncNone->isChecked() )
    mAccount->setEncryption( KNServerInfo::None );
  if ( mEncSSL->isChecked() )
    mAccount->setEncryption( KNServerInfo::SSL );
  if ( mEncTLS->isChecked() )
    mAccount->setEncryption( KNServerInfo::TLS );

  KConfig *conf = knGlobals.config();
  conf->setGroup("MAILSERVER");
  mAccount->saveConf( conf );
}


void KNode::SmtpAccountWidget::useExternalMailerToggled( bool b )
{
  mServer->setEnabled( !b );
  mPort->setEnabled( !b );
  mServerLabel->setEnabled( !b );
  mPortLabel->setEnabled( !b );
  mLogin->setEnabled( !b );
  if ( !b )
    loginToggled( mLogin->isChecked() );
  else
    loginToggled( false );
  mEncGroup->setEnabled( !b );
  emit changed( true );
}


void KNode::SmtpAccountWidget::loginToggled( bool b )
{
  mUser->setEnabled( b );
  mUserLabel->setEnabled( b );
  mPassword->setEnabled( b );
  mPasswordLabel->setEnabled( b );
  emit changed( true );
}


void KNode::SmtpAccountWidget::slotPasswordChanged()
{
  if ( mPassword->text().isEmpty() )
    mPassword->setText( mAccount->pass() );
}


//=============================================================================================


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


KNode::AppearanceWidget::AppearanceWidget( KInstance *inst, QWidget *parent ) :
  KCModule(inst, parent )
{
  QGridLayout *topL = new QGridLayout( this );

  //color-list
  mColorList = new QListWidget( this );
  topL->addWidget( mColorList, 1, 0, 3, 2 );
  connect( mColorList, SIGNAL( itemActivated( QListWidgetItem* ) ), SLOT( slotColItemActivated( QListWidgetItem* ) ) );
  connect( mColorList, SIGNAL( itemSelectionChanged() ), SLOT( slotColSelectionChanged() ) );

  c_olorCB = new QCheckBox(i18n("&Use custom colors"),this);
  c_olorCB->setObjectName( "kcfg_useCustomColors" );
  topL->addWidget( c_olorCB, 0, 0, 1, 3 );
  connect(c_olorCB, SIGNAL(toggled(bool)), this, SLOT(slotColCheckBoxToggled(bool)));

  c_olChngBtn=new QPushButton(i18n("Cha&nge..."), this);
  connect(c_olChngBtn, SIGNAL(clicked()), this, SLOT(slotColChangeBtnClicked()));
  topL->addWidget( c_olChngBtn, 1, 2, 1, 1 );

  //font-list
  mFontList = new QListWidget( this );
  topL->addWidget( mFontList, 5, 0, 3, 2 );
  connect( mFontList, SIGNAL( itemActivated( QListWidgetItem* ) ), SLOT( slotFontItemActivated( QListWidgetItem* ) ) );
  connect( mFontList, SIGNAL( itemSelectionChanged() ), SLOT( slotFontSelectionChanged() ) );

  f_ontCB = new QCheckBox(i18n("Use custom &fonts"),this);
  f_ontCB->setObjectName( "kcfg_useCustomFonts" );
  topL->addWidget(f_ontCB , 4, 0, 1, 3 );
  connect(f_ontCB, SIGNAL(toggled(bool)), this, SLOT(slotFontCheckBoxToggled(bool)));

  f_ntChngBtn=new QPushButton(i18n("Chang&e..."), this);
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
    int result = KFontDialog::getFont(font,false,this);

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


KNode::ReadNewsGeneralWidget::ReadNewsGeneralWidget( KInstance *inst, QWidget *parent ) :
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
    case KNode::Settings::EnumDateFormat::CTime: mStandardDateFormat->setChecked( true ); break;
    case KNode::Settings::EnumDateFormat::Localized: mLocalizedDateFormat->setChecked( true ); break;
    case KNode::Settings::EnumDateFormat::Fancy: mFancyDateFormat->setChecked( true ); break;
    case KNode::Settings::EnumDateFormat::Custom: mCustomDateFormat->setChecked( true ); break;
  }
}

void KNode::ReadNewsGeneralWidget::save()
{
  if ( mStandardDateFormat->isChecked() )
    knGlobals.settings()->setDateFormat( KNode::Settings::EnumDateFormat::CTime );
  if ( mLocalizedDateFormat->isChecked() )
    knGlobals.settings()->setDateFormat( KNode::Settings::EnumDateFormat::Localized );
  if ( mFancyDateFormat->isChecked() )
    knGlobals.settings()->setDateFormat( KNode::Settings::EnumDateFormat::Fancy );
  if ( mCustomDateFormat->isChecked() )
    knGlobals.settings()->setDateFormat( KNode::Settings::EnumDateFormat::Custom );
  KCModule::save();
}

//=============================================================================================


KNode::ReadNewsNavigationWidget::ReadNewsNavigationWidget( KInstance *inst, QWidget *parent ) :
  KCModule( inst, parent )
{
  KNode::Ui::ReadNewsNavigationWidgetBase ui;
  ui.setupUi( this );
  addConfig( knGlobals.settings(), this );
  load();
}


//=============================================================================================


KNode::ReadNewsViewerWidget::ReadNewsViewerWidget( KInstance *inst, QWidget *parent ) :
  KCModule( inst, parent )
{
  KNode::Ui::ReadNewsViewerWidgetBase ui;
  ui.setupUi( this );
  addConfig( knGlobals.settings(), this );
  load();
}


//=============================================================================================


KNode::DisplayedHeadersWidget::DisplayedHeadersWidget( DisplayedHeaders *d, KInstance *inst, QWidget *parent ) :
  KCModule( inst, parent ),
  s_ave( false ),
  d_ata( d )
{
  QGridLayout *topL=new QGridLayout(this, 7,2, 5,5);

  //listbox
  mHeaderList = new QListWidget( this );
  connect( mHeaderList, SIGNAL( itemActivated(QListWidgetItem* ) ), SLOT( slotEditBtnClicked() ) );
  connect( mHeaderList, SIGNAL( itemSelectionChanged() ), SLOT( slotSelectionChanged() ) );
  topL->addMultiCellWidget( mHeaderList, 0,6, 0,0 );

  // buttons
  a_ddBtn=new QPushButton(i18n("&Add..."), this);
  connect(a_ddBtn, SIGNAL(clicked()), this, SLOT(slotAddBtnClicked()));
  topL->addWidget(a_ddBtn, 0,1);

  d_elBtn=new QPushButton(i18n("&Delete"), this);
  connect(d_elBtn, SIGNAL(clicked()), this, SLOT(slotDelBtnClicked()));
  topL->addWidget(d_elBtn, 1,1);

  e_ditBtn=new QPushButton(i18n("modify something","&Edit..."), this);
  connect(e_ditBtn, SIGNAL(clicked()), this, SLOT(slotEditBtnClicked()));
  topL->addWidget(e_ditBtn, 2,1);

  u_pBtn=new QPushButton(i18n("&Up"), this);
  connect(u_pBtn, SIGNAL(clicked()), this, SLOT(slotUpBtnClicked()));
  topL->addWidget(u_pBtn, 4,1);

  d_ownBtn=new QPushButton(i18n("Do&wn"), this);
  connect(d_ownBtn, SIGNAL(clicked()), this, SLOT(slotDownBtnClicked()));
  topL->addWidget(d_ownBtn, 5,1);

  topL->addRowSpacing(3,20);        // separate up/down buttons
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
  } else
    text="<";
  text+=h->header();
  text+=">";
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

  if(KMessageBox::warningContinueCancel(this, i18n("Really delete this header?"),"",KGuiItem(i18n("&Delete"),"editdelete"))==KMessageBox::Continue) {
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
  : KDialogBase( Plain, i18n("Header Properties"), Ok | Cancel | Help, Ok, parent ),
    h_dr(h)
{
  QFrame* page=plainPage();
  QGridLayout *topL=new QGridLayout(page, 2, 2, 0, 5);

  QWidget *nameW = new QWidget(page);
  QGridLayout *nameL=new QGridLayout(nameW, 2, 2, 5);

  h_drC=new KComboBox(true, nameW);
  h_drC->lineEdit()->setMaxLength(64);
  connect(h_drC, SIGNAL(activated(int)), this, SLOT(slotActivated(int)));
  nameL->addWidget(new QLabel(h_drC, i18n("H&eader:"),nameW),0,0);
  nameL->addWidget(h_drC,0,1);

  n_ameE=new KLineEdit(nameW);

  n_ameE->setMaxLength(64);
  nameL->addWidget(new QLabel(n_ameE, i18n("Displayed na&me:"),nameW),1,0);
  nameL->addWidget(n_ameE,1,1);
  nameL->setColStretch(1,1);

  topL->addMultiCellWidget(nameW,0,0,0,1);

  QGroupBox *ngb=new QGroupBox(i18n("Name"), page);
  // ### hide style settings for now, the new viewer doesn't support this yet
  ngb->hide();
  QVBoxLayout *ngbL = new QVBoxLayout(ngb, 8, 5);
  ngbL->setAutoAdd(true);
  ngbL->addSpacing(fontMetrics().lineSpacing()-4);
  n_ameCB[0]=new QCheckBox(i18n("&Large"), ngb);
  n_ameCB[1]=new QCheckBox(i18n("&Bold"), ngb);
  n_ameCB[2]=new QCheckBox(i18n("&Italic"), ngb);
  n_ameCB[3]=new QCheckBox(i18n("&Underlined"), ngb);
  topL->addWidget(ngb,1,0);

  QGroupBox *vgb=new QGroupBox(i18n("Value"), page);
  // ### hide style settings for now, the new viewer doen't support this yet
  vgb->hide();
  QVBoxLayout *vgbL = new QVBoxLayout(vgb, 8, 5);
  vgbL->setAutoAdd(true);
  vgbL->addSpacing(fontMetrics().lineSpacing()-4);
  v_alueCB[0]=new QCheckBox(i18n("L&arge"), vgb);
  v_alueCB[1]=new QCheckBox(i18n("Bol&d"), vgb);
  v_alueCB[2]=new QCheckBox(i18n("I&talic"), vgb);
  v_alueCB[3]=new QCheckBox(i18n("U&nderlined"), vgb);
  topL->addWidget(vgb,1,1);

  topL->setColStretch(0,1);
  topL->setColStretch(1,1);

  // preset values...
  h_drC->addItems( KNDisplayedHeader::predefs() );
  h_drC->lineEdit()->setText(h->header());
  n_ameE->setText(h->translatedName());
  for(int i=0; i<4; i++) {
    n_ameCB[i]->setChecked(h->flag(i));
    v_alueCB[i]->setChecked(h->flag(i+4));
  }

  setFixedHeight(sizeHint().height());
  KNHelper::restoreWindowSize("accReadHdrPropDLG", this, sizeHint());

  connect(n_ameE, SIGNAL(textChanged(const QString&)), SLOT(slotNameChanged(const QString&)));

  setHelp("anc-knode-headers");
  slotNameChanged( n_ameE->text() );
}


KNode::DisplayedHeaderConfDialog::~DisplayedHeaderConfDialog()
{
  KNHelper::saveWindowSize("accReadHdrPropDLG", size());
}


void KNode::DisplayedHeaderConfDialog::slotOk()
{
  h_dr->setHeader(h_drC->currentText());
  h_dr->setTranslatedName(n_ameE->text());
  for(int i=0; i<4; i++) {
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
  n_ameE->setText(i18n(h_drC->text(pos).toLocal8Bit()));  // I think it's save here, the combobox has only english defaults
}


// disable the name format options when the name is empty
void KNode::DisplayedHeaderConfDialog::slotNameChanged(const QString& str)
{
  for(int i=0; i<4; i++)
      n_ameCB[i]->setEnabled(!str.isEmpty());
}

//=============================================================================================


KNode::ScoringWidget::ScoringWidget( KInstance *inst, QWidget *parent ) :
  KCModule( inst, parent )
{
  QGridLayout *topL = new QGridLayout(this,4,2, 5,5);
  mKsc = new KScoringEditorWidget( knGlobals.scoringManager(), this );
  topL->addMultiCellWidget( mKsc, 0, 0, 0, 1 );

  topL->addRowSpacing(1, 10);

  mIgnored = new KIntSpinBox( -100000, 100000, 1, 0, this );
  mIgnored->setObjectName( "kcfg_ignoredThreshold" );
  QLabel *l = new QLabel( mIgnored, i18n("Default score for &ignored threads:"), this );
  topL->addWidget(l, 2, 0);
  topL->addWidget( mIgnored, 2, 1 );

  mWatched = new KIntSpinBox( -100000, 100000, 1, 0, this );
  mWatched->setObjectName( "kcfg_watchedThreshold" );
  l = new QLabel( mWatched, i18n("Default score for &watched threads:"), this );
  topL->addWidget(l, 3, 0);
  topL->addWidget( mWatched, 3, 1);

  topL->setColStretch(0, 1);

  addConfig( knGlobals.settings(), this );
  load();
}


//=============================================================================================


KNode::FilterListWidget::FilterListWidget( KInstance *inst, QWidget *parent ) :
  KCModule( inst, parent ),
  f_ilManager( knGlobals.filterManager() )
{
  QGridLayout *topL=new QGridLayout(this, 6,2, 5,5);

  // == Filters =================================================

  mFilterList = new QListWidget( this );
  topL->addWidget( new QLabel( mFilterList, i18n("&Filters:"), this ), 0, 0 );

  connect( mFilterList, SIGNAL( itemSelectionChanged() ), SLOT( slotSelectionChangedFilter() ) );
  connect( mFilterList, SIGNAL( itemActivated( QListWidgetItem* ) ), SLOT( slotEditBtnClicked() ) );
  topL->addMultiCellWidget( mFilterList, 1, 5, 0, 0 );

  a_ddBtn=new QPushButton(i18n("&Add..."), this);
  connect(a_ddBtn, SIGNAL(clicked()), this, SLOT(slotAddBtnClicked()));
  topL->addWidget(a_ddBtn,1,1);

  e_ditBtn=new QPushButton(i18n("modify something","&Edit..."), this);
  connect(e_ditBtn, SIGNAL(clicked()), this, SLOT(slotEditBtnClicked()));
  topL->addWidget(e_ditBtn,2,1);

  c_opyBtn=new QPushButton(i18n("Co&py..."), this);
  connect(c_opyBtn, SIGNAL(clicked()), this, SLOT(slotCopyBtnClicked()));
  topL->addWidget(c_opyBtn,3,1);

  d_elBtn=new QPushButton(i18n("&Delete"), this);
  connect(d_elBtn, SIGNAL(clicked()), this, SLOT(slotDelBtnClicked()));
  topL->addWidget(d_elBtn,4,1);

  // == Menu ====================================================

  mMenuList = new QListWidget( this );
  topL->addWidget( new QLabel( mMenuList, i18n("&Menu:"), this ), 6, 0 );

  connect( mMenuList, SIGNAL( itemSelectionChanged() ), SLOT( slotSelectionChangedMenu() ) );
  topL->addMultiCellWidget( mMenuList, 7, 11, 0, 0 );

  u_pBtn=new QPushButton(i18n("&Up"), this);
  connect(u_pBtn, SIGNAL(clicked()), this, SLOT(slotUpBtnClicked()));
  topL->addWidget(u_pBtn,7,1);

  d_ownBtn=new QPushButton(i18n("Do&wn"), this);
  connect(d_ownBtn, SIGNAL(clicked()), this, SLOT(slotDownBtnClicked()));
  topL->addWidget(d_ownBtn,8,1);

  s_epAddBtn=new QPushButton(i18n("Add\n&Separator"), this);
  connect(s_epAddBtn, SIGNAL(clicked()), this, SLOT(slotSepAddBtnClicked()));
  topL->addWidget(s_epAddBtn,9,1);

  s_epRemBtn=new QPushButton(i18n("&Remove\nSeparator"), this);
  connect(s_epRemBtn, SIGNAL(clicked()), this, SLOT(slotSepRemBtnClicked()));
  topL->addWidget(s_epRemBtn,10,1);

  topL->setRowStretch(5,1);
  topL->setRowStretch(11,1);

  a_ctive = SmallIcon("filter",16);
  d_isabled = SmallIcon("filter",16,KIcon::DisabledState);

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


KNode::PostNewsTechnicalWidget::PostNewsTechnicalWidget( KInstance *inst, QWidget *parent ) :
  KCModule( inst, parent )
{
  setupUi( this );

  mCharset->insertStringList( KGlobal::charsets()->availableEncodingNames() );
  mEncoding->insertItem( i18n("Allow 8-bit") );
  mEncoding->insertItem( i18n("7-bit (Quoted-Printable)") );

  connect( mHeaderList, SIGNAL( itemActivated(QListWidgetItem*) ), SLOT( slotEditBtnClicked() ) );
  connect( mHeaderList, SIGNAL( itemSelectionChanged() ), SLOT( slotSelectionChanged() ) );

  connect( mAddButton, SIGNAL( clicked() ), SLOT( slotAddBtnClicked() ) );
  connect( mEditButton, SIGNAL( clicked() ), SLOT( slotEditBtnClicked() ) );
  connect( mDeleteButton, SIGNAL( clicked() ), SLOT( slotDelBtnClicked() ) );

  addConfig( knGlobals.settings(), this );
  load();

  slotSelectionChanged();
}


void KNode::PostNewsTechnicalWidget::load()
{
  KCModule::load();

  kDebug(5003) << k_funcinfo << knGlobals.settings()->charset() << endl;
  mCharset->setCurrentIndex( mCharset->findText( knGlobals.settings()->charset() ) );
  mEncoding->setCurrentIndex( knGlobals.settings()->allow8BitBody() ? 0 : 1 );

  mHeaderList->clear();
  XHeader::List list = knGlobals.settings()->xHeaders();
  for ( XHeader::List::Iterator it = list.begin(); it != list.end(); ++it )
    mHeaderList->addItem( (*it).header() );
}


void KNode::PostNewsTechnicalWidget::save()
{
  knGlobals.settings()->setCharset( mCharset->currentText() );
  knGlobals.settings()->setAllow8BitBody( mEncoding->currentItem() == 0 );

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
  KDialogBase( parent, 0, true, i18n("Additional Header"), Ok | Cancel, Ok )
{
  KHBox* page = makeHBoxMainWidget();

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
  return mNameEdit->text() + ": " + mValueEdit->text();
}


//===================================================================================================


KNode::PostNewsComposerWidget::PostNewsComposerWidget( KInstance *inst, QWidget *parent ) :
  KCModule( inst, parent )
{
  KNode::Ui::PostNewsComposerWidgetBase ui;
  ui.setupUi( this );
  addConfig( knGlobals.settings(), this );
  load();
}


//===================================================================================================


KNode::PostNewsSpellingWidget::PostNewsSpellingWidget( KInstance *inst, QWidget *parent ) :
  KCModule( inst, parent )
{
  QVBoxLayout *topL=new QVBoxLayout(this, 5);

  c_onf = new KSpellConfig( this, 0, false );
  topL->addWidget(c_onf);
  connect(c_onf, SIGNAL(configChanged()), SLOT(changed()));

  topL->addStretch(1);
}


KNode::PostNewsSpellingWidget::~PostNewsSpellingWidget()
{
}


void KNode::PostNewsSpellingWidget::save()
{
  c_onf->writeGlobalSettings();
}


//==============================================================================================================

KNode::PrivacyWidget::PrivacyWidget( KInstance *inst,QWidget *parent ) :
  KCModule(inst, parent )
{
  QBoxLayout *topLayout = new QVBoxLayout(this, 5);
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
  QGridLayout *grid = new QGridLayout( mExpGroup, 7, 2, KDialog::marginHint(), KDialog::spacingHint() );

  grid->setRowSpacing( 0, KDialog::spacingHint() );

  mExpEnabled = new QCheckBox( i18n("&Expire old articles automatically"), mExpGroup );
  grid->addMultiCellWidget( mExpEnabled, 1, 1, 0, 1 );
  connect( mExpEnabled, SIGNAL(toggled(bool)), SIGNAL(changed()) );

  mExpDays = new KIntSpinBox( 0, 99999, 1, 0, mExpGroup );
  mExpDays->setSuffix( i18n(" days") );
  QLabel *label = new QLabel( mExpDays, i18n("&Purge groups every:"), mExpGroup );
  grid->addWidget( label, 2, 0 );
  grid->addWidget( mExpDays, 2, 1, Qt::AlignRight );
  connect( mExpDays, SIGNAL(valueChanged(int)), SIGNAL(changed()) );
  connect( mExpEnabled, SIGNAL(toggled(bool)), label, SLOT(setEnabled(bool)) );
  connect( mExpEnabled, SIGNAL(toggled(bool)), mExpDays, SLOT(setEnabled(bool)) );

  mExpReadDays = new KIntSpinBox( 0, 99999, 1, 0, mExpGroup );
  mExpReadDays->setSuffix( i18n(" days") );
  label = new QLabel( mExpReadDays, i18n("&Keep read articles:"), mExpGroup );
  grid->addWidget( label, 3, 0 );
  grid->addWidget( mExpReadDays, 3, 1, Qt::AlignRight );
  connect( mExpReadDays, SIGNAL(valueChanged(int)), SIGNAL(changed()) );

  mExpUnreadDays = new KIntSpinBox( 0, 99999, 1, 0, mExpGroup );
  mExpUnreadDays->setSuffix( i18n(" days") );
  label = new QLabel( mExpUnreadDays, i18n("Keep u&nread articles:"), mExpGroup );
  grid->addWidget( label, 4, 0 );
  grid->addWidget( mExpUnreadDays, 4, 1, Qt::AlignRight );
  connect( mExpUnreadDays, SIGNAL(valueChanged(int)), SIGNAL(changed()) );

  mExpUnavailable = new QCheckBox( i18n("&Remove articles that are not available on the server"), mExpGroup );
  grid->addMultiCellWidget( mExpUnavailable, 5, 5, 0, 1 );
  connect( mExpUnavailable, SIGNAL(toggled(bool)), SIGNAL(changed()) );

  mPreserveThreads = new QCheckBox( i18n("Preser&ve threads"), mExpGroup );
  grid->addMultiCellWidget( mPreserveThreads, 6, 6, 0, 1 );
  connect( mPreserveThreads, SIGNAL(toggled(bool)), SIGNAL(changed()) );

  grid->setColStretch(1,1);
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


KNode::CleanupWidget::CleanupWidget( KInstance *inst,QWidget *parent ) :
  KCModule(inst, parent ),
  d_ata( knGlobals.configManager()->cleanup() )
{
  QVBoxLayout *topL=new QVBoxLayout(this, 5);

  mGroupCleanup = new GroupCleanupWidget( d_ata, this );
  topL->addWidget( mGroupCleanup );
  connect( mGroupCleanup, SIGNAL(changed()), SLOT(changed()) );

  // === folders =========================================================

  QGroupBox *foldersB = new QGroupBox( i18n("Folders"), this );
  topL->addWidget(foldersB);
  QGridLayout *foldersL = new QGridLayout( foldersB, 3, 2, KDialog::marginHint(), KDialog::spacingHint() );

  foldersL->setRowSpacing( 0, KDialog::spacingHint() );

  f_olderCB=new QCheckBox(i18n("Co&mpact folders automatically"), foldersB);
  connect(f_olderCB, SIGNAL(toggled(bool)), this, SLOT(slotFolderCBtoggled(bool)));
  foldersL->addMultiCellWidget(f_olderCB,1,1,0,1);

  f_olderDays=new KIntSpinBox( 0, 99999, 1, 0, foldersB );
  f_olderDays->setSuffix(i18n(" days"));
  f_olderDaysL=new QLabel(f_olderDays,i18n("P&urge folders every:"), foldersB);
  foldersL->addWidget(f_olderDaysL,2,0);
  foldersL->addWidget(f_olderDays,2,1,Qt::AlignRight);
  connect(f_olderDays, SIGNAL(valueChanged(int)), SLOT(changed()));

  foldersL->setColStretch(1,1);

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
#include "knconfigwidgets.moc"
