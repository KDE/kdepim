/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/


#include <tqvbox.h>
#include <tqpainter.h>
#include <tqwhatsthis.h>
#include <tqlabel.h>

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
#include <kpgp.h>


KNConfig::IdentityWidget::IdentityWidget( Identity *d, TQWidget *p, const char *n ) :
  KCModule( p, n ),
  d_ata( d )
{
  TQString msg;

  TQGridLayout *topL=new TQGridLayout(this,  11, 3, 5,5);

  n_ame=new KLineEdit(this);
  TQLabel *l=new TQLabel(n_ame, i18n("&Name:"), this);
  topL->addWidget(l, 0,0);
  topL->addMultiCellWidget(n_ame, 0,0, 1,2);
  msg = i18n("<qt><p>Your name as it will appear to others reading your articles.</p>"
      "<p>Ex: <b>John Stuart Masterson III</b>.</p></qt>");
  TQWhatsThis::add( n_ame, msg );
  TQWhatsThis::add( l, msg );
  connect( n_ame, TQT_SIGNAL(textChanged(const TQString&)), TQT_SLOT(changed()) );

  o_rga=new KLineEdit(this);
  l=new TQLabel(o_rga, i18n("Organi&zation:"), this);
  topL->addWidget(l, 1,0);
  topL->addMultiCellWidget(o_rga, 1,1, 1,2);
  msg = i18n( "<qt><p>The name of the organization you work for.</p>"
      "<p>Ex: <b>KNode, Inc</b>.</p></qt>" );
  TQWhatsThis::add( o_rga, msg );
  TQWhatsThis::add( l, msg );
  connect( o_rga, TQT_SIGNAL(textChanged(const TQString&)), TQT_SLOT(changed()) );

  e_mail=new KLineEdit(this);
  l=new TQLabel(e_mail, i18n("Email a&ddress:"), this);
  topL->addWidget(l, 2,0);
  topL->addMultiCellWidget(e_mail, 2,2, 1,2);
  msg = i18n( "<qt><p>Your email address as it will appear to others "
      "reading your articles</p><p>Ex: <b>nospam@please.com</b>.</qt>" );
  TQWhatsThis::add( l, msg );
  TQWhatsThis::add( e_mail, msg );
  connect( e_mail, TQT_SIGNAL(textChanged(const TQString&)), TQT_SLOT(changed()) );

  r_eplyTo=new KLineEdit(this);
  l=new TQLabel(r_eplyTo, i18n("&Reply-to address:"), this);
  topL->addWidget(l, 3,0);
  topL->addMultiCellWidget(r_eplyTo, 3,3, 1,2);
  msg = i18n( "<qt><p>When someone reply to your article by email, this is the address the message "
      "will be sent. If you fill in this field, please do it with a real "
      "email address.</p><p>Ex: <b>john@example.com</b>.</p></qt>" );
  TQWhatsThis::add( l, msg );
  TQWhatsThis::add( r_eplyTo, msg );
  connect( r_eplyTo, TQT_SIGNAL(textChanged(const TQString&)), TQT_SLOT(changed()) );

  m_ailCopiesTo=new KLineEdit(this);
  l=new TQLabel(m_ailCopiesTo, i18n("&Mail-copies-to:"), this);
  topL->addWidget(l, 4,0);
  topL->addMultiCellWidget(m_ailCopiesTo, 4,4, 1,2);
  connect( m_ailCopiesTo, TQT_SIGNAL(textChanged(const TQString&)), TQT_SLOT(changed()) );

  s_igningKey = new Kpgp::SecretKeyRequester(this);
  s_igningKey->dialogButton()->setText(i18n("Chan&ge..."));
  s_igningKey->setDialogCaption(i18n("Your OpenPGP Key"));
  s_igningKey->setDialogMessage(i18n("Select the OpenPGP key which should be "
      "used for signing articles."));
  l=new TQLabel(s_igningKey, i18n("Signing ke&y:"), this);
  topL->addWidget(l, 5,0);
  topL->addMultiCellWidget(s_igningKey, 5,5, 1,2);
  msg = i18n("<qt><p>The OpenPGP key you choose here will be "
      "used to sign your articles.</p></qt>");
  TQWhatsThis::add( l, msg );
  TQWhatsThis::add( s_igningKey, msg );
  connect( s_igningKey, TQT_SIGNAL(changed()), TQT_SLOT(changed()) );

  b_uttonGroup = new TQButtonGroup(this);
  connect( b_uttonGroup, TQT_SIGNAL(clicked(int)),
           this, TQT_SLOT(slotSignatureType(int)) );
  b_uttonGroup->setExclusive(true);
  b_uttonGroup->hide();

  s_igFile = new TQRadioButton( i18n("&Use a signature from file"), this );
  b_uttonGroup->insert(s_igFile, 0);
  topL->addMultiCellWidget(s_igFile, 6, 6, 0, 2);
  TQWhatsThis::add( s_igFile,
                   i18n( "<qt><p>Mark this to let KNode read the signature from a file.</p></qt>" ) );
  s_ig = new KLineEdit(this);

  f_ileName = new TQLabel(s_ig, i18n("Signature &file:"), this);
  topL->addWidget(f_ileName, 7, 0 );
  topL->addWidget(s_ig, 7, 1 );
  c_ompletion = new KURLCompletion();
  s_ig->setCompletionObject(c_ompletion);
  msg = i18n( "<qt><p>The file from which the signature will be read.</p>"
      "<p>Ex: <b>/home/robt/.sig</b>.</p></qt>" );
  TQWhatsThis::add( f_ileName, msg );
  TQWhatsThis::add( s_ig, msg );

  c_hooseBtn = new TQPushButton( i18n("Choo&se..."), this);
  connect(c_hooseBtn, TQT_SIGNAL(clicked()),
          this, TQT_SLOT(slotSignatureChoose()));
  topL->addWidget(c_hooseBtn, 7, 2 );
  e_ditBtn = new TQPushButton( i18n("&Edit File"), this);
  connect(e_ditBtn, TQT_SIGNAL(clicked()),
          this, TQT_SLOT(slotSignatureEdit()));
  topL->addWidget(e_ditBtn, 8, 2);

  s_igGenerator = new TQCheckBox(i18n("&The file is a program"), this);
  topL->addMultiCellWidget(s_igGenerator, 8, 8, 0, 1);
  msg = i18n( "<qt><p>Mark this option if the signature will be generated by a program</p>"
      "<p>Ex: <b>/home/robt/gensig.sh</b>.</p></qt>" );
  TQWhatsThis::add( s_igGenerator, msg );
  connect( s_igGenerator, TQT_SIGNAL(toggled(bool)), TQT_SLOT(changed()) );

  s_igEdit = new TQRadioButton( i18n("Specify signature &below"), this);
  b_uttonGroup->insert(s_igEdit, 1);
  topL->addMultiCellWidget(s_igEdit, 9, 9, 0, 2);

  s_igEditor = new TQTextEdit(this);
  s_igEditor->setTextFormat(Qt::PlainText);
  topL->addMultiCellWidget(s_igEditor, 10, 10, 0, 2);
  connect( s_igEditor, TQT_SIGNAL(textChanged()), TQT_SLOT(changed()) );

  topL->setColStretch(1,1);
  topL->setRowStretch(7,1);
  topL->setResizeMode(TQLayout::Minimum);
  connect(s_ig,TQT_SIGNAL(textChanged ( const TQString & )),
          this,TQT_SLOT(textFileNameChanged(const TQString &)));

  load();
}


KNConfig::IdentityWidget::~IdentityWidget()
{
  delete c_ompletion;
}

void KNConfig::IdentityWidget::textFileNameChanged(const TQString &text)
{
    e_ditBtn->setEnabled(!text.isEmpty());
    emit changed( true );
}

void KNConfig::IdentityWidget::load()
{
  kdDebug() << "void KNConfig::IdentityWidget::load()" << endl;
  n_ame->setText(d_ata->n_ame);
  o_rga->setText(d_ata->o_rga);
  e_mail->setText(d_ata->e_mail);
  r_eplyTo->setText(d_ata->r_eplyTo);
  m_ailCopiesTo->setText(d_ata->m_ailCopiesTo);
  s_igningKey->setKeyIDs(Kpgp::KeyIDList() << d_ata->s_igningKey);
  s_ig->setText(d_ata->s_igPath);
  s_igGenerator->setChecked(d_ata->useSigGenerator());
  s_igEditor->setText(d_ata->s_igText);
  slotSignatureType(d_ata->useSigFile()? 0:1);
}

void KNConfig::IdentityWidget::save()
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
  d_ata->s_igText=s_igEditor->text();

  if(d_ata->isGlobal())
    d_ata->save();
}

void KNConfig::IdentityWidget::slotSignatureType(int type)
{
  bool sigFromFile = (type==0);

  b_uttonGroup->setButton(type);
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


void KNConfig::IdentityWidget::slotSignatureChoose()
{
  TQString tmp=KFileDialog::getOpenFileName(c_ompletion->replacedPath(s_ig->text()),TQString::null,this,i18n("Choose Signature"));
  if(!tmp.isEmpty()) s_ig->setText(tmp);
  emit changed( true );
}


void KNConfig::IdentityWidget::slotSignatureEdit()
{
  TQString fileName = c_ompletion->replacedPath(s_ig->text()).stripWhiteSpace();

  if (fileName.isEmpty()) {
    KMessageBox::sorry(this, i18n("You must specify a filename."));
    return;
  }

  TQFileInfo fileInfo( fileName );
  if (fileInfo.isDir()) {
    KMessageBox::sorry(this, i18n("You have specified a folder."));
    return;
  }

  KService::Ptr offer = KServiceTypeProfile::preferredService("text/plain", "Application");
  KURL u(fileName);

  if (offer)
    KRun::run(*offer, u);
  else
    KRun::displayOpenWithDialog(u);
  emit changed( true );
}



//==========================================================================================

//BEGIN: NNTP account configuration widgets ----------------------------------

KNConfig::NntpAccountListWidget::NntpAccountListWidget(TQWidget *p, const char *n) :
  KCModule( p, n ),
  a_ccManager( knGlobals.accountManager() )
{
  p_ixmap = SmallIcon("server");

  TQGridLayout *topL=new TQGridLayout(this, 6,2, 5,5);

  // account listbox
  l_box=new KNDialogListBox(false, this);
  connect(l_box, TQT_SIGNAL(selected(int)), this, TQT_SLOT(slotItemSelected(int)));
  connect(l_box, TQT_SIGNAL(selectionChanged()), this, TQT_SLOT(slotSelectionChanged()));
  topL->addMultiCellWidget(l_box, 0,4, 0,0);

  // info box
  TQGroupBox *gb = new TQGroupBox(2,Qt::Vertical,TQString::null,this);
  topL->addWidget(gb,5,0);

  s_erverInfo = new TQLabel(gb);
  p_ortInfo = new TQLabel(gb);

  // buttons
  a_ddBtn=new TQPushButton(i18n("&Add..."), this);
  connect(a_ddBtn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotAddBtnClicked()));
  topL->addWidget(a_ddBtn, 0,1);

  e_ditBtn=new TQPushButton(i18n("modify something","&Edit..."), this);
  connect(e_ditBtn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotEditBtnClicked()));
  topL->addWidget(e_ditBtn, 1,1);

  d_elBtn=new TQPushButton(i18n("&Delete"), this);
  connect(d_elBtn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotDelBtnClicked()));
  topL->addWidget(d_elBtn, 2,1);

  s_ubBtn=new TQPushButton(i18n("&Subscribe..."), this);
  connect(s_ubBtn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotSubBtnClicked()));
  topL->addWidget(s_ubBtn, 3,1);

  topL->setRowStretch(4,1);   // stretch the server listbox

  load();

  // the settings dialog is non-modal, so we have to react to changes
  // made outside of the dialog
  connect(a_ccManager, TQT_SIGNAL(accountAdded(KNNntpAccount*)), this, TQT_SLOT(slotAddItem(KNNntpAccount*)));
  connect(a_ccManager, TQT_SIGNAL(accountRemoved(KNNntpAccount*)), this, TQT_SLOT(slotRemoveItem(KNNntpAccount*)));
  connect(a_ccManager, TQT_SIGNAL(accountModified(KNNntpAccount*)), this, TQT_SLOT(slotUpdateItem(KNNntpAccount*)));

  slotSelectionChanged();     // disable Delete & Edit initially
}


KNConfig::NntpAccountListWidget::~NntpAccountListWidget()
{
}


void KNConfig::NntpAccountListWidget::load()
{
  l_box->clear();
  TQValueList<KNNntpAccount*>::Iterator it;
  for ( it = a_ccManager->begin(); it != a_ccManager->end(); ++it )
    slotAddItem( *it );
}


void KNConfig::NntpAccountListWidget::slotAddItem(KNNntpAccount *a)
{
  LBoxItem *it;
  it=new LBoxItem(a, a->name(), &p_ixmap);
  l_box->insertItem(it);
  emit changed(true);
}


void KNConfig::NntpAccountListWidget::slotRemoveItem(KNNntpAccount *a)
{
  LBoxItem *it;
  for(uint i=0; i<l_box->count(); i++) {
    it=static_cast<LBoxItem*>(l_box->item(i));
    if(it && it->account==a) {
      l_box->removeItem(i);
      break;
    }
  }
  slotSelectionChanged();
  emit changed(true);
}


void KNConfig::NntpAccountListWidget::slotUpdateItem(KNNntpAccount *a)
{
  LBoxItem *it;
  for(uint i=0; i<l_box->count(); i++) {
    it=static_cast<LBoxItem*>(l_box->item(i));
    if(it && it->account==a) {
      it=new LBoxItem(a, a->name(), &p_ixmap);
      l_box->changeItem(it, i);
      break;
    }
  }
  slotSelectionChanged();
  emit changed(true);
}



void KNConfig::NntpAccountListWidget::slotSelectionChanged()
{
  int curr=l_box->currentItem();
  d_elBtn->setEnabled(curr!=-1);
  e_ditBtn->setEnabled(curr!=-1);
  s_ubBtn->setEnabled(curr!=-1);

  LBoxItem *it = static_cast<LBoxItem*>(l_box->item(curr));
  if(it) {
    s_erverInfo->setText(i18n("Server: %1").arg(it->account->server()));
    p_ortInfo->setText(i18n("Port: %1").arg(it->account->port()));
  }
  else {
    s_erverInfo->setText(i18n("Server: "));
    p_ortInfo->setText(i18n("Port: "));
  }
}



void KNConfig::NntpAccountListWidget::slotItemSelected(int)
{
  slotEditBtnClicked();
}



void KNConfig::NntpAccountListWidget::slotAddBtnClicked()
{
  KNNntpAccount *acc = new KNNntpAccount();

  if(acc->editProperties(this)) {
    if (a_ccManager->newAccount(acc))
      acc->saveInfo();
  }
  else
    delete acc;
}



void KNConfig::NntpAccountListWidget::slotDelBtnClicked()
{
  LBoxItem *it = static_cast<LBoxItem*>(l_box->item(l_box->currentItem()));

  if(it)
    a_ccManager->removeAccount(it->account);
}



void KNConfig::NntpAccountListWidget::slotEditBtnClicked()
{
  LBoxItem *it = static_cast<LBoxItem*>(l_box->item(l_box->currentItem()));

  if(it) {
    it->account->editProperties(this);
    slotUpdateItem(it->account);
  }
}


void KNConfig::NntpAccountListWidget::slotSubBtnClicked()
{
  LBoxItem *it = static_cast<LBoxItem*>(l_box->item(l_box->currentItem()));

  if(it)
    knGlobals.groupManager()->showGroupDialog(it->account, this);
}


//=======================================================================================


KNConfig::NntpAccountConfDialog::NntpAccountConfDialog(KNNntpAccount *a, TQWidget *p, const char *n)
  : KDialogBase(Tabbed, (a->id()!=-1)? i18n("Properties of %1").arg(a->name()):i18n("New Account"),
                Ok|Cancel|Help, Ok, p, n),
    a_ccount(a)
{
  TQFrame* page=addPage(i18n("Ser&ver"));
  TQGridLayout *topL=new TQGridLayout(page, 11, 3, 5);

  n_ame=new KLineEdit(page);
  TQLabel *l=new TQLabel(n_ame,i18n("&Name:"),page);
  topL->addWidget(l, 0,0);
  n_ame->setText(a->name());
  topL->addMultiCellWidget(n_ame, 0, 0, 1, 2);

  s_erver=new KLineEdit(page);
  l=new TQLabel(s_erver,i18n("&Server:"), page);
  s_erver->setText(a->server());
  topL->addWidget(l, 1,0);
  topL->addMultiCellWidget(s_erver, 1, 1, 1, 2);

  p_ort=new KLineEdit(page);
  l=new TQLabel(p_ort, i18n("&Port:"), page);
  p_ort->setValidator(new KIntValidator(0,65536,this));
  p_ort->setText(TQString::number(a->port()));
  topL->addWidget(l, 2,0);
  topL->addWidget(p_ort, 2,1);

  h_old = new KIntSpinBox(5,1800,5,5,10,page);
  l = new TQLabel(h_old,i18n("Hol&d connection for:"), page);
  h_old->setSuffix(i18n(" sec"));
  h_old->setValue(a->hold());
  topL->addWidget(l,3,0);
  topL->addWidget(h_old,3,1);

  t_imeout = new KIntSpinBox(15,600,5,15,10,page);
  l = new TQLabel(t_imeout, i18n("&Timeout:"), page);
  t_imeout->setValue(a->timeout());
  t_imeout->setSuffix(i18n(" sec"));
  topL->addWidget(l,4,0);
  topL->addWidget(t_imeout,4,1);

  f_etchDes=new TQCheckBox(i18n("&Fetch group descriptions"), page);
  f_etchDes->setChecked(a->fetchDescriptions());
  topL->addMultiCellWidget(f_etchDes, 5,5, 0,3);

  /*u_seDiskCache=new TQCheckBox(i18n("&Cache articles on disk"), page);
  u_seDiskCache->setChecked(a->useDiskCache());
  topL->addMultiCellWidget(u_seDiskCache, 6,6, 0,3);*/

  a_uth=new TQCheckBox(i18n("Server requires &authentication"), page);
  connect(a_uth, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(slotAuthChecked(bool)));
  topL->addMultiCellWidget(a_uth, 6,6, 0,3);

  u_ser=new KLineEdit(page);
  u_serLabel=new TQLabel(u_ser,i18n("&User:"), page);
  u_ser->setText(a->user());
  topL->addWidget(u_serLabel, 7,0);
  topL->addMultiCellWidget(u_ser, 7,7, 1,2);

  p_ass=new KLineEdit(page);
  p_assLabel=new TQLabel(p_ass, i18n("Pass&word:"), page);
  p_ass->setEchoMode(KLineEdit::Password);
  if ( a->readyForLogin() )
    p_ass->setText(a->pass());
  else
    if ( a->needsLogon() )
      knGlobals.accountManager()->loadPasswordsAsync();
  topL->addWidget(p_assLabel, 8,0);
  topL->addMultiCellWidget(p_ass, 8,8, 1,2);

  i_nterval=new TQCheckBox(i18n("Enable &interval news checking"), page);
  connect(i_nterval, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(slotIntervalChecked(bool)));
  topL->addMultiCellWidget(i_nterval, 9,9, 0,3);

  c_heckInterval=new KIntSpinBox(1,10000,1,1,10,page);
  c_heckIntervalLabel=new TQLabel(c_heckInterval, i18n("Check inter&val:"), page);
  c_heckInterval->setSuffix(i18n(" min") );
  c_heckInterval->setValue(a->checkInterval());
  c_heckIntervalLabel->setBuddy(c_heckInterval);
  topL->addWidget(c_heckIntervalLabel, 10,0);
  topL->addMultiCellWidget(c_heckInterval, 10,10, 1,2);

  slotAuthChecked(a->needsLogon());
  slotIntervalChecked(a->intervalChecking());

  topL->setColStretch(1, 1);
  topL->setColStretch(2, 1);

  // Specfic Identity tab =========================================
  i_dWidget=new KNConfig::IdentityWidget(a->identity(), addVBoxPage(i18n("&Identity")));

  // per server cleanup configuration
  TQFrame* cleanupPage = addPage( i18n("&Cleanup") );
  TQVBoxLayout *cleanupLayout = new TQVBoxLayout( cleanupPage, KDialog::spacingHint() );
  mCleanupWidget = new GroupCleanupWidget( a->cleanupConfig(), cleanupPage );
  mCleanupWidget->load();
  cleanupLayout->addWidget( mCleanupWidget );
  cleanupLayout->addStretch( 1 );

  connect( knGlobals.accountManager(), TQT_SIGNAL(passwordsChanged()), TQT_SLOT(slotPasswordChanged()) );

  KNHelper::restoreWindowSize("accNewsPropDLG", this, sizeHint());

  setHelp("anc-setting-the-news-account");
}



KNConfig::NntpAccountConfDialog::~NntpAccountConfDialog()
{
  KNHelper::saveWindowSize("accNewsPropDLG", size());
}


void KNConfig::NntpAccountConfDialog::slotOk()
{
  if (n_ame->text().isEmpty() || s_erver->text().stripWhiteSpace().isEmpty()) {
    KMessageBox::sorry(this, i18n("Please enter an arbitrary name for the account and the\nhostname of the news server."));
    return;
  }

  a_ccount->setName(n_ame->text());
  a_ccount->setServer(s_erver->text().stripWhiteSpace());
  a_ccount->setPort(p_ort->text().toInt());
  a_ccount->setHold(h_old->value());
  a_ccount->setTimeout(t_imeout->value());
  a_ccount->setFetchDescriptions(f_etchDes->isChecked());
  //a_ccount->setUseDiskCache(u_seDiskCache->isChecked());
  a_ccount->setNeedsLogon(a_uth->isChecked());
  a_ccount->setUser(u_ser->text());
  a_ccount->setPass(p_ass->text());
  a_ccount->setIntervalChecking(i_nterval->isChecked());
  a_ccount->setCheckInterval(c_heckInterval->value());
  if (a_ccount->id() != -1) // only save if account has a valid id
    a_ccount->saveInfo();

  i_dWidget->save();
  mCleanupWidget->save();

  accept();
}


void KNConfig::NntpAccountConfDialog::slotAuthChecked(bool b)
{
  a_uth->setChecked(b);
  u_ser->setEnabled(b);
  u_serLabel->setEnabled(b);
  p_ass->setEnabled(b);
  p_assLabel->setEnabled(b);
}

void KNConfig::NntpAccountConfDialog::slotIntervalChecked(bool b)
{
  i_nterval->setChecked(b);
  c_heckInterval->setEnabled(b);
  c_heckIntervalLabel->setEnabled(b);
}

void KNConfig::NntpAccountConfDialog::slotPasswordChanged()
{
  if ( p_ass->text().isEmpty() )
    p_ass->setText( a_ccount->pass() );
}

//END: NNTP account configuration widgets ------------------------------------

//=============================================================================================

KNConfig::SmtpAccountWidget::SmtpAccountWidget( TQWidget *p, const char *n ) :
  SmtpAccountWidgetBase( p, n )
{
  mAccount = knGlobals.accountManager()->smtp();
  connect( knGlobals.accountManager(), TQT_SIGNAL(passwordsChanged()), TQT_SLOT(slotPasswordChanged()) );
  load();
}


void KNConfig::SmtpAccountWidget::load()
{
  mUseExternalMailer->setChecked( knGlobals.configManager()->postNewsTechnical()->useExternalMailer() );
  useExternalMailerToggled( knGlobals.configManager()->postNewsTechnical()->useExternalMailer() );
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


void KNConfig::SmtpAccountWidget::save()
{
  knGlobals.configManager()->postNewsTechnical()->u_seExternalMailer = mUseExternalMailer->isChecked();
  knGlobals.configManager()->postNewsTechnical()->setDirty(true);

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


void KNConfig::SmtpAccountWidget::useExternalMailerToggled( bool b )
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


void KNConfig::SmtpAccountWidget::loginToggled( bool b )
{
  bool canEnable = ( b && !mUseExternalMailer->isChecked() );
  mUser->setEnabled( canEnable );
  mUserLabel->setEnabled( canEnable );
  mPassword->setEnabled( canEnable );
  mPasswordLabel->setEnabled( canEnable );
  emit changed( true );
}


void KNConfig::SmtpAccountWidget::slotPasswordChanged()
{
  if ( mPassword->text().isEmpty() )
    mPassword->setText( mAccount->pass() );
}


//=============================================================================================


//===================================================================================
// code taken from KMail, Copyright (C) 2000 Espen Sand, espen@kde.org

KNConfig::AppearanceWidget::ColorListItem::ColorListItem( const TQString &text, const TQColor &color )
  : TQListBoxText(text), mColor( color )
{
}


KNConfig::AppearanceWidget::ColorListItem::~ColorListItem()
{
}


void KNConfig::AppearanceWidget::ColorListItem::paint( TQPainter *p )
{
  TQFontMetrics fm = p->fontMetrics();
  int h = fm.height();

  p->drawText( 30+3*2, fm.ascent() + fm.leading()/2, text() );

  p->setPen( Qt::black );
  p->drawRect( 3, 1, 30, h-1 );
  p->fillRect( 4, 2, 28, h-3, mColor );
}


int KNConfig::AppearanceWidget::ColorListItem::height(const TQListBox *lb ) const
{
  return( lb->fontMetrics().lineSpacing()+1 );
}


int KNConfig::AppearanceWidget::ColorListItem::width(const TQListBox *lb ) const
{
  return( 30 + lb->fontMetrics().width( text() ) + 6 );
}


//===================================================================================


KNConfig::AppearanceWidget::FontListItem::FontListItem( const TQString &name, const TQFont &font )
  : TQListBoxText(name), f_ont(font)
{
  fontInfo = TQString("[%1 %2]").arg(f_ont.family()).arg(f_ont.pointSize());
}


KNConfig::AppearanceWidget::FontListItem::~FontListItem()
{
}


void KNConfig::AppearanceWidget::FontListItem::setFont(const TQFont &font)
{
  f_ont = font;
  fontInfo = TQString("[%1 %2]").arg(f_ont.family()).arg(f_ont.pointSize());
}


void KNConfig::AppearanceWidget::FontListItem::paint( TQPainter *p )
{
  TQFont fnt = p->font();
  fnt.setWeight(TQFont::Bold);
  p->setFont(fnt);
  int fontInfoWidth = p->fontMetrics().width(fontInfo);
  int h = p->fontMetrics().ascent() + p->fontMetrics().leading()/2;
  p->drawText(2, h, fontInfo );
  fnt.setWeight(TQFont::Normal);
  p->setFont(fnt);
  p->drawText(5 + fontInfoWidth, h, text() );
}


int KNConfig::AppearanceWidget::FontListItem::width(const TQListBox *lb ) const
{
  return( lb->fontMetrics().width(fontInfo) + lb->fontMetrics().width(text()) + 20 );
}


//===================================================================================


KNConfig::AppearanceWidget::AppearanceWidget( TQWidget *p, const char *n ) :
  KCModule( p, n ),
  d_ata( knGlobals.configManager()->appearance() )
{
  TQGridLayout *topL=new TQGridLayout(this, 8,2, 5,5);

  //color-list
  c_List = new KNDialogListBox(false, this);
  topL->addMultiCellWidget(c_List,1,3,0,0);
  connect(c_List, TQT_SIGNAL(selected(TQListBoxItem*)),TQT_SLOT(slotColItemSelected(TQListBoxItem*)));
  connect(c_List, TQT_SIGNAL(selectionChanged()), TQT_SLOT(slotColSelectionChanged()));

  c_olorCB = new TQCheckBox(i18n("&Use custom colors"),this);
  topL->addWidget(c_olorCB,0,0);
  connect(c_olorCB, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(slotColCheckBoxToggled(bool)));

  c_olChngBtn=new TQPushButton(i18n("Cha&nge..."), this);
  connect(c_olChngBtn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotColChangeBtnClicked()));
  topL->addWidget(c_olChngBtn,1,1);

  //font-list
  f_List = new KNDialogListBox(false, this);
  topL->addMultiCellWidget(f_List,5,7,0,0);
  connect(f_List, TQT_SIGNAL(selected(TQListBoxItem*)),TQT_SLOT(slotFontItemSelected(TQListBoxItem*)));
  connect(f_List, TQT_SIGNAL(selectionChanged()),TQT_SLOT(slotFontSelectionChanged()));

  f_ontCB = new TQCheckBox(i18n("Use custom &fonts"),this);
  topL->addWidget(f_ontCB,4,0);
  connect(f_ontCB, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(slotFontCheckBoxToggled(bool)));

  f_ntChngBtn=new TQPushButton(i18n("Chang&e..."), this);
  connect(f_ntChngBtn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotFontChangeBtnClicked()));
  topL->addWidget(f_ntChngBtn,5,1);

  load();
}


KNConfig::AppearanceWidget::~AppearanceWidget()
{
}


void KNConfig::AppearanceWidget::load()
{
  c_olorCB->setChecked(d_ata->u_seColors);
  slotColCheckBoxToggled(d_ata->u_seColors);
  c_List->clear();
  for(int i=0; i < d_ata->colorCount(); i++)
    c_List->insertItem(new ColorListItem(d_ata->colorName(i), d_ata->color(i)));

  f_ontCB->setChecked(d_ata->u_seFonts);
  slotFontCheckBoxToggled(d_ata->u_seFonts);
  f_List->clear();
  for(int i=0; i < d_ata->fontCount(); i++)
    f_List->insertItem(new FontListItem(d_ata->fontName(i), d_ata->font(i)));
}


void KNConfig::AppearanceWidget::save()
{
  d_ata->u_seColors=c_olorCB->isChecked();
  for(int i=0; i<d_ata->colorCount(); i++)
    d_ata->c_olors[i] = (static_cast<ColorListItem*>(c_List->item(i)))->color();

  d_ata->u_seFonts=f_ontCB->isChecked();
  for(int i=0; i<d_ata->fontCount(); i++)
    d_ata->f_onts[i] = (static_cast<FontListItem*>(f_List->item(i)))->font();

  d_ata->setDirty(true);

  d_ata->recreateLVIcons();
}


void KNConfig::AppearanceWidget::defaults()
{
  // default colors
  ColorListItem *colorItem;
  for(int i=0; i < d_ata->colorCount(); i++) {
    colorItem=static_cast<ColorListItem*>(c_List->item(i));
    colorItem->setColor(d_ata->defaultColor(i));
  }
  c_List->triggerUpdate(true);
  c_List->repaint(true);

  // default fonts
  FontListItem *fontItem;
  for(int i=0; i < d_ata->fontCount(); i++) {
    fontItem=static_cast<FontListItem*>(f_List->item(i));
    fontItem->setFont(d_ata->defaultFont(i));
  }
  f_List->triggerUpdate(false);

  emit changed(true);
}


void KNConfig::AppearanceWidget::slotColCheckBoxToggled(bool b)
{
  c_List->setEnabled(b);
  c_olChngBtn->setEnabled(b && (c_List->currentItem()!=-1));
  if (b) c_List->setFocus();
  emit changed(true);
}


// show color dialog for the entry
void KNConfig::AppearanceWidget::slotColItemSelected(TQListBoxItem *it)
{
  if (it) {
    ColorListItem *colorItem = static_cast<ColorListItem*>(it);
    TQColor col = colorItem->color();
    int result = KColorDialog::getColor(col,this);

    if (result == KColorDialog::Accepted) {
      colorItem->setColor(col);
      c_List->triggerUpdate(false);
    }
  }
  emit changed(true);
}


void KNConfig::AppearanceWidget::slotColChangeBtnClicked()
{
  if(c_List->currentItem()!=-1)
    slotColItemSelected(c_List->item(c_List->currentItem()));
}


void KNConfig::AppearanceWidget::slotColSelectionChanged()
{
  c_olChngBtn->setEnabled(c_List->currentItem()!=-1);
}


void KNConfig::AppearanceWidget::slotFontCheckBoxToggled(bool b)
{
  f_List->setEnabled(b);
  f_ntChngBtn->setEnabled(b && (f_List->currentItem()!=-1));
  if (b) f_List->setFocus();
  emit changed(true);
}


// show font dialog for the entry
void KNConfig::AppearanceWidget::slotFontItemSelected(TQListBoxItem *it)
{
  if (it) {
    FontListItem *fontItem = static_cast<FontListItem*>(it);
    TQFont font = fontItem->font();
    int result = KFontDialog::getFont(font,false,this);

    if (result == KFontDialog::Accepted) {
      fontItem->setFont(font);
      f_List->triggerUpdate(false);
    }
  }
  emit changed(true);
}


void KNConfig::AppearanceWidget::slotFontChangeBtnClicked()
{
  if(f_List->currentItem()!=-1)
    slotFontItemSelected(f_List->item(f_List->currentItem()));
}


void KNConfig::AppearanceWidget::slotFontSelectionChanged()
{
  f_ntChngBtn->setEnabled(f_List->currentItem()!=-1);
}


//=============================================================================================


KNConfig::ReadNewsGeneralWidget::ReadNewsGeneralWidget( ReadNewsGeneral *d, TQWidget *p, const char *n ) :
  KCModule( p, n ),
  d_ata( d )
{
  TQGroupBox *hgb=new TQGroupBox(i18n("Article Handling"), this);
  TQGroupBox *lgb=new TQGroupBox(i18n("Article List"), this);
  TQGroupBox *cgb=new TQGroupBox(i18n("Memory Consumption"), this);
  TQLabel *l1, *l2, *l3;

  a_utoCB=new TQCheckBox(i18n("Check for new articles a&utomatically"), hgb);
  m_axFetch=new KIntSpinBox(0, 100000, 1, 0, 10, hgb);
  l1=new TQLabel(m_axFetch, i18n("&Maximum number of articles to fetch:"), hgb);
  m_arkCB=new TQCheckBox(i18n("Mar&k article as read after:"), hgb);
  m_arkSecs=new KIntSpinBox(0, 9999, 1, 0, 10, hgb);
  connect(m_arkCB, TQT_SIGNAL(toggled(bool)), m_arkSecs, TQT_SLOT(setEnabled(bool)));
  m_arkSecs->setSuffix(i18n(" sec"));
  m_arkCrossCB=new TQCheckBox(i18n("Mark c&rossposted articles as read"), hgb);

  s_martScrollingCB=new TQCheckBox(i18n("Smart scrolli&ng"), lgb);
  e_xpThrCB=new TQCheckBox(i18n("Show &whole thread on expanding"), lgb);
  d_efaultExpandCB=new TQCheckBox(i18n("Default to e&xpanded threads"), lgb);
  s_coreCB=new TQCheckBox(i18n("Show article &score"), lgb);
  l_inesCB=new TQCheckBox(i18n("Show &line count"), lgb);
  u_nreadCB=new TQCheckBox(i18n("Show unread count in &thread"), lgb);

  c_ollCacheSize=new KIntSpinBox(0, 99999, 1, 1, 10, cgb);
  c_ollCacheSize->setSuffix(" KB");
  l2=new TQLabel(c_ollCacheSize, i18n("Cach&e size for headers:"), cgb);
  a_rtCacheSize=new KIntSpinBox(0, 99999, 1, 1, 10, cgb);
  a_rtCacheSize->setSuffix(" KB");
  l3=new TQLabel(a_rtCacheSize, i18n("Cache si&ze for articles:"), cgb);

  TQVBoxLayout *topL=new TQVBoxLayout(this, 5);
  TQGridLayout *hgbL=new TQGridLayout(hgb, 5,2, 8,5);
  TQVBoxLayout *lgbL=new TQVBoxLayout(lgb, 8, 5);
  TQGridLayout *cgbL=new TQGridLayout(cgb, 3,2, 8,5);

  topL->addWidget(hgb);
  topL->addWidget(lgb);
  topL->addWidget(cgb);
  topL->addStretch(1);

  hgbL->addRowSpacing(0, fontMetrics().lineSpacing()-4);
  hgbL->addMultiCellWidget(a_utoCB, 1,1, 0,1);
  hgbL->addWidget(l1, 2, 0);
  hgbL->addWidget(m_axFetch, 2,1);
  hgbL->addWidget(m_arkCB, 3,0);
  hgbL->addWidget(m_arkSecs, 3,1);
  hgbL->addMultiCellWidget(m_arkCrossCB, 4,4, 0,1);
  hgbL->setColStretch(0,1);

  lgbL->addSpacing(fontMetrics().lineSpacing()-4);
  lgbL->addWidget(s_martScrollingCB);
  lgbL->addWidget(e_xpThrCB);
  lgbL->addWidget(d_efaultExpandCB);
  lgbL->addWidget(s_coreCB);
  lgbL->addWidget(l_inesCB);
  lgbL->addWidget(u_nreadCB);

  cgbL->addRowSpacing(0, fontMetrics().lineSpacing()-4);
  cgbL->addWidget(l2, 1,0);
  cgbL->addWidget(c_ollCacheSize, 1,1);
  cgbL->addWidget(l3, 2,0);
  cgbL->addWidget(a_rtCacheSize, 2,1);
  cgbL->setColStretch(0,1);

  topL->setResizeMode(TQLayout::Minimum);

  connect(a_utoCB,           TQT_SIGNAL(toggled(bool)),     TQT_SLOT(changed()));
  connect(m_axFetch,         TQT_SIGNAL(valueChanged(int)), TQT_SLOT(changed()));
  connect(m_arkCB,           TQT_SIGNAL(toggled(bool)),     TQT_SLOT(changed()));
  connect(m_arkSecs,         TQT_SIGNAL(valueChanged(int)), TQT_SLOT(changed()));
  connect(m_arkCrossCB,      TQT_SIGNAL(toggled(bool)),     TQT_SLOT(changed()));
  connect(s_martScrollingCB, TQT_SIGNAL(toggled(bool)),     TQT_SLOT(changed()));
  connect(e_xpThrCB,         TQT_SIGNAL(toggled(bool)),     TQT_SLOT(changed()));
  connect(d_efaultExpandCB,  TQT_SIGNAL(toggled(bool)),     TQT_SLOT(changed()));
  connect(l_inesCB,          TQT_SIGNAL(toggled(bool)),     TQT_SLOT(changed()));
  connect(s_coreCB,          TQT_SIGNAL(toggled(bool)),     TQT_SLOT(changed()));
  connect(u_nreadCB,         TQT_SIGNAL(toggled(bool)),     TQT_SLOT(changed()));
  connect(c_ollCacheSize,    TQT_SIGNAL(valueChanged(int)), TQT_SLOT(changed()));
  connect(a_rtCacheSize,     TQT_SIGNAL(valueChanged(int)), TQT_SLOT(changed()));

  load();
}


KNConfig::ReadNewsGeneralWidget::~ReadNewsGeneralWidget()
{
}


void KNConfig::ReadNewsGeneralWidget::load()
{
  a_utoCB->setChecked(d_ata->a_utoCheck);
  m_axFetch->setValue(d_ata->m_axFetch);
  m_arkCB->setChecked(d_ata->a_utoMark);
  m_arkSecs->setValue(d_ata->m_arkSecs);
  m_arkSecs->setEnabled(d_ata->a_utoMark);
  m_arkCrossCB->setChecked(d_ata->m_arkCrossposts);
  s_martScrollingCB->setChecked(d_ata->s_martScrolling);
  e_xpThrCB->setChecked(d_ata->t_otalExpand);
  d_efaultExpandCB->setChecked(d_ata->d_efaultExpand);
  l_inesCB->setChecked(d_ata->s_howLines);
  s_coreCB->setChecked(d_ata->s_howScore);
  u_nreadCB->setChecked(d_ata->s_howUnread);
  c_ollCacheSize->setValue(d_ata->c_ollCacheSize);
  a_rtCacheSize->setValue(d_ata->a_rtCacheSize);
}

void KNConfig::ReadNewsGeneralWidget::save()
{
  d_ata->a_utoCheck=a_utoCB->isChecked();
  d_ata->m_axFetch=m_axFetch->value();
  d_ata->a_utoMark=m_arkCB->isChecked();
  d_ata->m_arkSecs=m_arkSecs->value();
  d_ata->m_arkCrossposts=m_arkCrossCB->isChecked();
  d_ata->s_martScrolling=s_martScrollingCB->isChecked();
  d_ata->t_otalExpand=e_xpThrCB->isChecked();
  d_ata->d_efaultExpand=d_efaultExpandCB->isChecked();
  d_ata->s_howLines=l_inesCB->isChecked();
  d_ata->s_howScore=s_coreCB->isChecked();
  d_ata->s_howUnread=u_nreadCB->isChecked();
  d_ata->c_ollCacheSize=c_ollCacheSize->value();
  d_ata->a_rtCacheSize=a_rtCacheSize->value();

  d_ata->setDirty(true);
}

//=============================================================================================


KNConfig::ReadNewsNavigationWidget::ReadNewsNavigationWidget( ReadNewsNavigation *d, TQWidget *p, const char *n ) :
  KCModule( p, n ),
  d_ata( d )
{
  TQVBoxLayout *topL=new TQVBoxLayout(this, 5);

  // ==== Mark All as Read ====================================================

  TQGroupBox *gb=new TQGroupBox(i18n("\"Mark All as Read\" Triggers Following Actions"), this);
  TQVBoxLayout *gbL=new TQVBoxLayout(gb, 8, 5);
  topL->addWidget(gb);

  gbL->addSpacing(fontMetrics().lineSpacing()-4);
  m_arkAllReadGoNextCB=new TQCheckBox(i18n("&Switch to the next group"), gb);
  gbL->addWidget(m_arkAllReadGoNextCB);

  connect(m_arkAllReadGoNextCB, TQT_SIGNAL(toggled(bool)), TQT_SLOT(changed()));

  // ==== Mark Thread as Read =================================================

  gb=new TQGroupBox(i18n("\"Mark Thread as Read\" Triggers Following Actions"), this);
  gbL=new TQVBoxLayout(gb, 8, 5);
  topL->addWidget(gb);

  gbL->addSpacing(fontMetrics().lineSpacing()-4);
  m_arkThreadReadCloseThreadCB=new TQCheckBox(i18n("Clos&e the current thread"), gb);
  gbL->addWidget(m_arkThreadReadCloseThreadCB);
  m_arkThreadReadGoNextCB=new TQCheckBox(i18n("Go &to the next unread thread"), gb);
  gbL->addWidget(m_arkThreadReadGoNextCB);

  connect(m_arkThreadReadCloseThreadCB, TQT_SIGNAL(toggled(bool)), TQT_SLOT(changed()));
  connect(m_arkThreadReadGoNextCB, TQT_SIGNAL(toggled(bool)), TQT_SLOT(changed()));

  // ==== Ignore Thread =======================================================

  gb=new TQGroupBox(i18n("\"Ignore Thread\" Triggers Following Actions"), this);
  gbL=new TQVBoxLayout(gb, 8, 5);
  topL->addWidget(gb);

  gbL->addSpacing(fontMetrics().lineSpacing()-4);
  i_gnoreThreadCloseThreadCB=new TQCheckBox(i18n("Close the cu&rrent thread"), gb);
  gbL->addWidget(i_gnoreThreadCloseThreadCB);
  i_gnoreThreadGoNextCB=new TQCheckBox(i18n("Go to the next &unread thread"), gb);
  gbL->addWidget(i_gnoreThreadGoNextCB);

  connect(i_gnoreThreadCloseThreadCB, TQT_SIGNAL(toggled(bool)), TQT_SLOT(changed()));
  connect(i_gnoreThreadGoNextCB, TQT_SIGNAL(toggled(bool)), TQT_SLOT(changed()));

  topL->addStretch(1);
  topL->setResizeMode(TQLayout::Minimum);

  load();
}


KNConfig::ReadNewsNavigationWidget::~ReadNewsNavigationWidget()
{
}


void KNConfig::ReadNewsNavigationWidget::load()
{
  m_arkAllReadGoNextCB->setChecked(d_ata->m_arkAllReadGoNext);
  m_arkThreadReadGoNextCB->setChecked(d_ata->m_arkThreadReadGoNext);
  m_arkThreadReadCloseThreadCB->setChecked(d_ata->m_arkThreadReadCloseThread);
  i_gnoreThreadGoNextCB->setChecked(d_ata->i_gnoreThreadGoNext);
  i_gnoreThreadCloseThreadCB->setChecked(d_ata->i_gnoreThreadCloseThread);
}

void KNConfig::ReadNewsNavigationWidget::save()
{
  d_ata->m_arkAllReadGoNext = m_arkAllReadGoNextCB->isChecked();
  d_ata->m_arkThreadReadGoNext = m_arkThreadReadGoNextCB->isChecked();
  d_ata->m_arkThreadReadCloseThread = m_arkThreadReadCloseThreadCB->isChecked();
  d_ata->i_gnoreThreadGoNext = i_gnoreThreadGoNextCB->isChecked();
  d_ata->i_gnoreThreadCloseThread = i_gnoreThreadCloseThreadCB->isChecked();

  d_ata->setDirty(true);
}


//=============================================================================================


KNConfig::ReadNewsViewerWidget::ReadNewsViewerWidget( ReadNewsViewer *d, TQWidget *p, const char *n ) :
  KCModule( p, n ),
  d_ata( d )
{
  TQGroupBox *appgb=new TQGroupBox(i18n("Appearance"), this);
  TQGroupBox *agb=new TQGroupBox(i18n("Attachments"), this);
  TQGroupBox *secbox = new TQGroupBox( i18n("Security"), this );

  r_ewrapCB=new TQCheckBox(i18n("Re&wrap text when necessary"), appgb);
  r_emoveTrailingCB=new TQCheckBox(i18n("Re&move trailing empty lines"), appgb);
  s_igCB=new TQCheckBox(i18n("Show sig&nature"), appgb);
  mShowRefBar = new TQCheckBox( i18n("Show reference bar"), appgb );
  q_uoteCharacters=new KLineEdit(appgb);
  TQLabel *quoteCharL = new TQLabel(q_uoteCharacters, i18n("Recognized q&uote characters:"), appgb);

  o_penAttCB=new TQCheckBox(i18n("Open a&ttachments on click"), agb);
  a_ltAttCB=new TQCheckBox(i18n("Show alternati&ve contents as attachments"), agb);

  mAlwaysShowHTML = new TQCheckBox( i18n("Prefer HTML to plain text"), secbox );

  TQVBoxLayout *topL=new TQVBoxLayout(this, 5);
  TQGridLayout *appgbL=new TQGridLayout(appgb, 5,2, 8,5);
  TQVBoxLayout *agbL=new TQVBoxLayout(agb, 8, 5);
  TQVBoxLayout *secLayout = new TQVBoxLayout( secbox, 8, 5 );

  topL->addWidget(appgb);
  topL->addWidget(agb);
  topL->addWidget( secbox );
  topL->addStretch(1);

  appgbL->addRowSpacing(0, fontMetrics().lineSpacing()-4);
  appgbL->addMultiCellWidget(r_ewrapCB, 2,2, 0,1);
  appgbL->addMultiCellWidget(r_emoveTrailingCB, 3,3, 0,1);
  appgbL->addMultiCellWidget(s_igCB, 4,4, 0,1);
  appgbL->addMultiCellWidget( mShowRefBar, 5,5, 0,1 );
  appgbL->addWidget(quoteCharL, 6,0);
  appgbL->addWidget(q_uoteCharacters, 6,1);

  agbL->addSpacing(fontMetrics().lineSpacing()-4);
  agbL->addWidget(o_penAttCB);
  agbL->addWidget(a_ltAttCB);

  secLayout->addSpacing( fontMetrics().lineSpacing() - 4 );
  secLayout->addWidget( mAlwaysShowHTML );

  topL->setResizeMode(TQLayout::Minimum);

  connect(r_ewrapCB, TQT_SIGNAL(toggled(bool)), TQT_SLOT(changed()));
  connect(r_emoveTrailingCB, TQT_SIGNAL(toggled(bool)), TQT_SLOT(changed()));
  connect(s_igCB, TQT_SIGNAL(toggled(bool)), TQT_SLOT(changed()));
  connect(q_uoteCharacters, TQT_SIGNAL(textChanged(const TQString&)), TQT_SLOT(changed()));
  connect(o_penAttCB, TQT_SIGNAL(toggled(bool)), TQT_SLOT(changed()));
  connect(a_ltAttCB, TQT_SIGNAL(toggled(bool)), TQT_SLOT(changed()));
  connect( mShowRefBar, TQT_SIGNAL(toggled(bool)), TQT_SLOT(changed()) );
  connect( mAlwaysShowHTML, TQT_SIGNAL(toggled(bool)), TQT_SLOT(changed()) );

  load();
}


KNConfig::ReadNewsViewerWidget::~ReadNewsViewerWidget()
{
}


void KNConfig::ReadNewsViewerWidget::load()
{
  r_ewrapCB->setChecked(d_ata->r_ewrapBody);
  r_emoveTrailingCB->setChecked(d_ata->r_emoveTrailingNewlines);
  s_igCB->setChecked(d_ata->s_howSig);
  q_uoteCharacters->setText(d_ata->q_uoteCharacters);
  o_penAttCB->setChecked(d_ata->o_penAtt);
  a_ltAttCB->setChecked(d_ata->s_howAlts);
  mShowRefBar->setChecked( d_ata->showRefBar() );
  mAlwaysShowHTML->setChecked( d_ata->alwaysShowHTML() );
}


void KNConfig::ReadNewsViewerWidget::save()
{
  d_ata->r_ewrapBody=r_ewrapCB->isChecked();
  d_ata->r_emoveTrailingNewlines=r_emoveTrailingCB->isChecked();
  d_ata->s_howSig=s_igCB->isChecked();
  d_ata->q_uoteCharacters=q_uoteCharacters->text();
  d_ata->o_penAtt=o_penAttCB->isChecked();
  d_ata->s_howAlts=a_ltAttCB->isChecked();
  d_ata->setShowRefBar( mShowRefBar->isChecked() );
  d_ata->setAlwaysShowHTML( mAlwaysShowHTML->isChecked() );

  d_ata->setDirty(true);
}


//=============================================================================================


KNConfig::DisplayedHeadersWidget::DisplayedHeadersWidget( DisplayedHeaders *d, TQWidget *p, const char *n ) :
  KCModule( p, n ),
  s_ave( false ),
  d_ata( d )
{
  TQGridLayout *topL=new TQGridLayout(this, 7,2, 5,5);

  //listbox
  l_box=new KNDialogListBox(false, this);
  connect(l_box, TQT_SIGNAL(selected(int)), this, TQT_SLOT(slotItemSelected(int)));
  connect(l_box, TQT_SIGNAL(selectionChanged()), this, TQT_SLOT(slotSelectionChanged()));
  topL->addMultiCellWidget(l_box, 0,6, 0,0);

  // buttons
  a_ddBtn=new TQPushButton(i18n("&Add..."), this);
  connect(a_ddBtn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotAddBtnClicked()));
  topL->addWidget(a_ddBtn, 0,1);

  d_elBtn=new TQPushButton(i18n("&Delete"), this);
  connect(d_elBtn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotDelBtnClicked()));
  topL->addWidget(d_elBtn, 1,1);

  e_ditBtn=new TQPushButton(i18n("modify something","&Edit..."), this);
  connect(e_ditBtn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotEditBtnClicked()));
  topL->addWidget(e_ditBtn, 2,1);

  u_pBtn=new TQPushButton(i18n("&Up"), this);
  connect(u_pBtn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotUpBtnClicked()));
  topL->addWidget(u_pBtn, 4,1);

  d_ownBtn=new TQPushButton(i18n("Do&wn"), this);
  connect(d_ownBtn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotDownBtnClicked()));
  topL->addWidget(d_ownBtn, 5,1);

  topL->addRowSpacing(3,20);        // separate up/down buttons
  topL->setRowStretch(6,1);         // stretch the listbox

  slotSelectionChanged();     // disable buttons initially

  load();
}



KNConfig::DisplayedHeadersWidget::~DisplayedHeadersWidget()
{
}


void KNConfig::DisplayedHeadersWidget::load()
{
  l_box->clear();
  TQValueList<KNDisplayedHeader*> list = d_ata->headers();
  for ( TQValueList<KNDisplayedHeader*>::Iterator it = list.begin(); it != list.end(); ++it )
    l_box->insertItem( generateItem( (*it) ) );
}

void KNConfig::DisplayedHeadersWidget::save()
{
  if(s_ave) {
    d_ata->setDirty(true);
    d_ata->save();
  }
  s_ave = false;
}



KNConfig::DisplayedHeadersWidget::HdrItem* KNConfig::DisplayedHeadersWidget::generateItem(KNDisplayedHeader *h)
{
  TQString text;
  if(h->hasName()) {
    text=h->translatedName();
    text+=": <";
  } else
    text="<";
  text+=h->header();
  text+=">";
  return new HdrItem(text,h);
}



void KNConfig::DisplayedHeadersWidget::slotItemSelected(int)
{
  slotEditBtnClicked();
}



void KNConfig::DisplayedHeadersWidget::slotSelectionChanged()
{
  int curr = l_box->currentItem();
  d_elBtn->setEnabled(curr!=-1);
  e_ditBtn->setEnabled(curr!=-1);
  u_pBtn->setEnabled(curr>0);
  d_ownBtn->setEnabled((curr!=-1)&&(curr+1!=(int)(l_box->count())));
}



void KNConfig::DisplayedHeadersWidget::slotAddBtnClicked()
{
  KNDisplayedHeader *h=d_ata->createNewHeader();

  DisplayedHeaderConfDialog* dlg=new DisplayedHeaderConfDialog(h, this);
  if(dlg->exec()) {
    l_box->insertItem(generateItem(h));
    h->createTags();
    s_ave=true;
  } else
    d_ata->remove(h);
  emit changed(true);
}



void KNConfig::DisplayedHeadersWidget::slotDelBtnClicked()
{
  if(l_box->currentItem()==-1)
    return;

  if(KMessageBox::warningContinueCancel(this, i18n("Really delete this header?"),"",KGuiItem(i18n("&Delete"),"editdelete"))==KMessageBox::Continue) {
    KNDisplayedHeader *h = (static_cast<HdrItem*>(l_box->item(l_box->currentItem())))->hdr;
    d_ata->remove(h);
    l_box->removeItem(l_box->currentItem());
    s_ave=true;
  }
  emit changed(true);
}



void KNConfig::DisplayedHeadersWidget::slotEditBtnClicked()
{
  if (l_box->currentItem()==-1) return;
  KNDisplayedHeader *h = (static_cast<HdrItem*>(l_box->item(l_box->currentItem())))->hdr;

  DisplayedHeaderConfDialog* dlg=new DisplayedHeaderConfDialog(h, this);
  if(dlg->exec()) {
    l_box->changeItem(generateItem(h), l_box->currentItem());
    h->createTags();
    s_ave=true;
  }
  emit changed(true);
}



void KNConfig::DisplayedHeadersWidget::slotUpBtnClicked()
{
  int c=l_box->currentItem();
  if(c==0 || c==-1) return;

  KNDisplayedHeader *h = (static_cast<HdrItem*>(l_box->item(c)))->hdr;

  d_ata->up(h);
  l_box->insertItem(generateItem(h), c-1);
  l_box->removeItem(c+1);
  l_box->setCurrentItem(c-1);
  s_ave=true;
  emit changed(true);
}



void KNConfig::DisplayedHeadersWidget::slotDownBtnClicked()
{
  int c=l_box->currentItem();
  if(c==-1 || c==(int) l_box->count()-1) return;

  KNDisplayedHeader *h = (static_cast<HdrItem*>(l_box->item(c)))->hdr;

  d_ata->down(h);
  l_box->insertItem(generateItem(h), c+2);
  l_box->removeItem(c);
  l_box->setCurrentItem(c+1);
  s_ave=true;
  emit changed(true);
}


//=============================================================================================


KNConfig::DisplayedHeaderConfDialog::DisplayedHeaderConfDialog(KNDisplayedHeader *h, TQWidget *p, char *n)
  : KDialogBase(Plain, i18n("Header Properties"),Ok|Cancel|Help, Ok, p, n),
    h_dr(h)
{
  TQFrame* page=plainPage();
  TQGridLayout *topL=new TQGridLayout(page, 2, 2, 0, 5);

  TQWidget *nameW = new TQWidget(page);
  TQGridLayout *nameL=new TQGridLayout(nameW, 2, 2, 5);

  h_drC=new KComboBox(true, nameW);
  h_drC->lineEdit()->setMaxLength(64);
  connect(h_drC, TQT_SIGNAL(activated(int)), this, TQT_SLOT(slotActivated(int)));
  nameL->addWidget(new TQLabel(h_drC, i18n("H&eader:"),nameW),0,0);
  nameL->addWidget(h_drC,0,1);

  n_ameE=new KLineEdit(nameW);

  n_ameE->setMaxLength(64);
  nameL->addWidget(new TQLabel(n_ameE, i18n("Displayed na&me:"),nameW),1,0);
  nameL->addWidget(n_ameE,1,1);
  nameL->setColStretch(1,1);

  topL->addMultiCellWidget(nameW,0,0,0,1);

  TQGroupBox *ngb=new TQGroupBox(i18n("Name"), page);
  // ### hide style settings for now, the new viewer doesn't support this yet
  ngb->hide();
  TQVBoxLayout *ngbL = new TQVBoxLayout(ngb, 8, 5);
  ngbL->setAutoAdd(true);
  ngbL->addSpacing(fontMetrics().lineSpacing()-4);
  n_ameCB[0]=new TQCheckBox(i18n("&Large"), ngb);
  n_ameCB[1]=new TQCheckBox(i18n("&Bold"), ngb);
  n_ameCB[2]=new TQCheckBox(i18n("&Italic"), ngb);
  n_ameCB[3]=new TQCheckBox(i18n("&Underlined"), ngb);
  topL->addWidget(ngb,1,0);

  TQGroupBox *vgb=new TQGroupBox(i18n("Value"), page);
  // ### hide style settings for now, the new viewer doen't support this yet
  vgb->hide();
  TQVBoxLayout *vgbL = new TQVBoxLayout(vgb, 8, 5);
  vgbL->setAutoAdd(true);
  vgbL->addSpacing(fontMetrics().lineSpacing()-4);
  v_alueCB[0]=new TQCheckBox(i18n("L&arge"), vgb);
  v_alueCB[1]=new TQCheckBox(i18n("Bol&d"), vgb);
  v_alueCB[2]=new TQCheckBox(i18n("I&talic"), vgb);
  v_alueCB[3]=new TQCheckBox(i18n("U&nderlined"), vgb);
  topL->addWidget(vgb,1,1);

  topL->setColStretch(0,1);
  topL->setColStretch(1,1);

  // preset values...
  h_drC->insertStrList(KNDisplayedHeader::predefs());
  h_drC->lineEdit()->setText(h->header());
  n_ameE->setText(h->translatedName());
  for(int i=0; i<4; i++) {
    n_ameCB[i]->setChecked(h->flag(i));
    v_alueCB[i]->setChecked(h->flag(i+4));
  }

  setFixedHeight(sizeHint().height());
  KNHelper::restoreWindowSize("accReadHdrPropDLG", this, sizeHint());

  connect(n_ameE, TQT_SIGNAL(textChanged(const TQString&)), TQT_SLOT(slotNameChanged(const TQString&)));

  setHelp("anc-knode-headers");
  slotNameChanged( n_ameE->text() );
}


KNConfig::DisplayedHeaderConfDialog::~DisplayedHeaderConfDialog()
{
  KNHelper::saveWindowSize("accReadHdrPropDLG", size());
}


void KNConfig::DisplayedHeaderConfDialog::slotOk()
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
void KNConfig::DisplayedHeaderConfDialog::slotActivated(int pos)
{
  n_ameE->setText(i18n(h_drC->text(pos).local8Bit()));  // I think it's save here, the combobox has only english defaults
}


// disable the name format options when the name is empty
void KNConfig::DisplayedHeaderConfDialog::slotNameChanged(const TQString& str)
{
  for(int i=0; i<4; i++)
      n_ameCB[i]->setEnabled(!str.isEmpty());
}

//=============================================================================================


KNConfig::ScoringWidget::ScoringWidget( Scoring *d, TQWidget *p, const char *n ) :
  KCModule( p, n ),
  d_ata( d )
{
  TQGridLayout *topL = new TQGridLayout(this,4,2, 5,5);
  ksc = new KScoringEditorWidget(knGlobals.scoringManager(), this);
  topL->addMultiCellWidget(ksc, 0,0, 0,1);

  topL->addRowSpacing(1, 10);

  i_gnored=new KIntSpinBox(-100000, 100000, 1, 0, 10, this);
  TQLabel *l=new TQLabel(i_gnored, i18n("Default score for &ignored threads:"), this);
  topL->addWidget(l, 2, 0);
  topL->addWidget(i_gnored, 2, 1);
  connect(i_gnored, TQT_SIGNAL(valueChanged(int)), TQT_SLOT(changed()));

  w_atched=new KIntSpinBox(-100000, 100000, 1, 0, 10, this);
  l=new TQLabel(w_atched, i18n("Default score for &watched threads:"), this);
  topL->addWidget(l, 3, 0);
  topL->addWidget(w_atched, 3, 1);
  connect(w_atched, TQT_SIGNAL(valueChanged(int)), TQT_SLOT(changed()));

  topL->setColStretch(0, 1);

  load();
}


KNConfig::ScoringWidget::~ScoringWidget()
{
}


void KNConfig::ScoringWidget::load()
{
  i_gnored->setValue(d_ata->i_gnoredThreshold);
  w_atched->setValue(d_ata->w_atchedThreshold);
}

void KNConfig::ScoringWidget::save()
{
  d_ata->i_gnoredThreshold = i_gnored->value();
  d_ata->w_atchedThreshold = w_atched->value();

  d_ata->setDirty(true);
}


//=============================================================================================


KNConfig::FilterListWidget::FilterListWidget( TQWidget *p, const char *n ) :
  KCModule( p, n ),
  f_ilManager( knGlobals.filterManager() )
{
  TQGridLayout *topL=new TQGridLayout(this, 6,2, 5,5);

  // == Filters =================================================

  f_lb=new KNDialogListBox(false, this);
  topL->addWidget(new TQLabel(f_lb, i18n("&Filters:"),this),0,0);

  connect(f_lb, TQT_SIGNAL(selectionChanged()), TQT_SLOT(slotSelectionChangedFilter()));
  connect(f_lb, TQT_SIGNAL(selected(int)), TQT_SLOT(slotItemSelectedFilter(int)));
  topL->addMultiCellWidget(f_lb,1,5,0,0);

  a_ddBtn=new TQPushButton(i18n("&Add..."), this);
  connect(a_ddBtn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotAddBtnClicked()));
  topL->addWidget(a_ddBtn,1,1);

  e_ditBtn=new TQPushButton(i18n("modify something","&Edit..."), this);
  connect(e_ditBtn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotEditBtnClicked()));
  topL->addWidget(e_ditBtn,2,1);

  c_opyBtn=new TQPushButton(i18n("Co&py..."), this);
  connect(c_opyBtn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotCopyBtnClicked()));
  topL->addWidget(c_opyBtn,3,1);

  d_elBtn=new TQPushButton(i18n("&Delete"), this);
  connect(d_elBtn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotDelBtnClicked()));
  topL->addWidget(d_elBtn,4,1);

  // == Menu ====================================================

  m_lb=new KNDialogListBox(false, this);
  topL->addWidget(new TQLabel(m_lb, i18n("&Menu:"),this),6,0);

  connect(m_lb, TQT_SIGNAL(selectionChanged()), TQT_SLOT(slotSelectionChangedMenu()));
  topL->addMultiCellWidget(m_lb,7,11,0,0);

  u_pBtn=new TQPushButton(i18n("&Up"), this);
  connect(u_pBtn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotUpBtnClicked()));
  topL->addWidget(u_pBtn,7,1);

  d_ownBtn=new TQPushButton(i18n("Do&wn"), this);
  connect(d_ownBtn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotDownBtnClicked()));
  topL->addWidget(d_ownBtn,8,1);

  s_epAddBtn=new TQPushButton(i18n("Add\n&Separator"), this);
  connect(s_epAddBtn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotSepAddBtnClicked()));
  topL->addWidget(s_epAddBtn,9,1);

  s_epRemBtn=new TQPushButton(i18n("&Remove\nSeparator"), this);
  connect(s_epRemBtn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotSepRemBtnClicked()));
  topL->addWidget(s_epRemBtn,10,1);

  topL->setRowStretch(5,1);
  topL->setRowStretch(11,1);

  a_ctive = SmallIcon("filter",16);
  d_isabled = SmallIcon("filter",16,KIcon::DisabledState);

  load();

  slotSelectionChangedFilter();
  slotSelectionChangedMenu();
}


KNConfig::FilterListWidget::~FilterListWidget()
{
  f_ilManager->endConfig();
}


void KNConfig::FilterListWidget::load()
{
  f_lb->clear();
  m_lb->clear();
  f_ilManager->startConfig(this);
}

void KNConfig::FilterListWidget::save()
{
  f_ilManager->commitChanges();
}


void KNConfig::FilterListWidget::addItem(KNArticleFilter *f)
{
  if(f->isEnabled())
    f_lb->insertItem(new LBoxItem(f, f->translatedName(), &a_ctive));
  else
    f_lb->insertItem(new LBoxItem(f, f->translatedName(), &d_isabled));
  slotSelectionChangedFilter();
  emit changed(true);
}


void KNConfig::FilterListWidget::removeItem(KNArticleFilter *f)
{
  int i=findItem(f_lb, f);
  if (i!=-1) f_lb->removeItem(i);
  slotSelectionChangedFilter();
  emit changed(true);
}


void KNConfig::FilterListWidget::updateItem(KNArticleFilter *f)
{
  int i=findItem(f_lb, f);

  if(i!=-1) {
    if(f->isEnabled()) {
      f_lb->changeItem(new LBoxItem(f, f->translatedName(), &a_ctive), i);
      m_lb->changeItem(new LBoxItem(f, f->translatedName()), findItem(m_lb, f));
    } else
      f_lb->changeItem(new LBoxItem(f, f->translatedName(), &d_isabled), i);
  }
  slotSelectionChangedFilter();
  emit changed(true);
}


void KNConfig::FilterListWidget::addMenuItem(KNArticleFilter *f)
{
  if (f) {
    if (findItem(m_lb, f)==-1)
      m_lb->insertItem(new LBoxItem(f, f->translatedName()));
  } else   // separator
    m_lb->insertItem(new LBoxItem(0, "==="));
  slotSelectionChangedMenu();
  emit changed(true);
}


void KNConfig::FilterListWidget::removeMenuItem(KNArticleFilter *f)
{
  int i=findItem(m_lb, f);
  if(i!=-1) m_lb->removeItem(i);
  slotSelectionChangedMenu();
  emit changed(true);
}


TQValueList<int> KNConfig::FilterListWidget::menuOrder()
{
  KNArticleFilter *f;
  TQValueList<int> lst;

  for(uint i=0; i<m_lb->count(); i++) {
    f= (static_cast<LBoxItem*>(m_lb->item(i)))->filter;
    if(f)
      lst << f->id();
    else
      lst << -1;
  }
 return lst;
}


int KNConfig::FilterListWidget::findItem(TQListBox *l, KNArticleFilter *f)
{
  int idx=0;
  bool found=false;
  while(!found && idx < (int) l->count()) {
    found=( (static_cast<LBoxItem*>(l->item(idx)))->filter==f );
    if(!found) idx++;
  }
  if(found) return idx;
  else return -1;
}


void KNConfig::FilterListWidget::slotAddBtnClicked()
{
  f_ilManager->newFilter();
}


void KNConfig::FilterListWidget::slotDelBtnClicked()
{
  if (f_lb->currentItem()!=-1)
    f_ilManager->deleteFilter( (static_cast<LBoxItem*>(f_lb->item(f_lb->currentItem())))->filter );
}


void KNConfig::FilterListWidget::slotEditBtnClicked()
{
  if (f_lb->currentItem()!=-1)
    f_ilManager->editFilter( (static_cast<LBoxItem*>(f_lb->item(f_lb->currentItem())))->filter );
}


void KNConfig::FilterListWidget::slotCopyBtnClicked()
{
  if (f_lb->currentItem()!=-1)
    f_ilManager->copyFilter( (static_cast<LBoxItem*>(f_lb->item(f_lb->currentItem())))->filter );
}


void KNConfig::FilterListWidget::slotUpBtnClicked()
{
  int c=m_lb->currentItem();
  KNArticleFilter *f=0;

  if(c==0 || c==-1) return;
  f=(static_cast<LBoxItem*>(m_lb->item(c)))->filter;
  if(f)
    m_lb->insertItem(new LBoxItem(f, f->translatedName()), c-1);
  else
    m_lb->insertItem(new LBoxItem(0, "==="), c-1);
  m_lb->removeItem(c+1);
  m_lb->setCurrentItem(c-1);
  emit changed(true);
}


void KNConfig::FilterListWidget::slotDownBtnClicked()
{
  int c=m_lb->currentItem();
  KNArticleFilter *f=0;

  if(c==-1 || c+1==(int)m_lb->count()) return;
  f=(static_cast<LBoxItem*>(m_lb->item(c)))->filter;
  if(f)
    m_lb->insertItem(new LBoxItem(f, f->translatedName()), c+2);
  else
    m_lb->insertItem(new LBoxItem(0, "==="), c+2);
  m_lb->removeItem(c);
  m_lb->setCurrentItem(c+1);
  emit changed(true);
}


void KNConfig::FilterListWidget::slotSepAddBtnClicked()
{
  m_lb->insertItem(new LBoxItem(0, "==="), m_lb->currentItem());
  slotSelectionChangedMenu();
  emit changed(true);
}


void KNConfig::FilterListWidget::slotSepRemBtnClicked()
{
  int c=m_lb->currentItem();

  if( (c!=-1) && ( (static_cast<LBoxItem*>(m_lb->item(c)))->filter==0 ) )
     m_lb->removeItem(c);
  slotSelectionChangedMenu();
  emit changed(true);
}


void KNConfig::FilterListWidget::slotItemSelectedFilter(int)
{
  slotEditBtnClicked();
}


void KNConfig::FilterListWidget::slotSelectionChangedFilter()
{
  int curr = f_lb->currentItem();

  d_elBtn->setEnabled(curr!=-1);
  e_ditBtn->setEnabled(curr!=-1);
  c_opyBtn->setEnabled(curr!=-1);
}


void KNConfig::FilterListWidget::slotSelectionChangedMenu()
{
  int curr = m_lb->currentItem();

  u_pBtn->setEnabled(curr>0);
  d_ownBtn->setEnabled((curr!=-1)&&(curr+1!=(int)m_lb->count()));
  s_epRemBtn->setEnabled((curr!=-1) && ( (static_cast<LBoxItem*>(m_lb->item(curr)))->filter==0 ) );
}


//=============================================================================================


KNConfig::PostNewsTechnicalWidget::PostNewsTechnicalWidget( PostNewsTechnical *d, TQWidget *p, const char *n ) :
  KCModule( p, n ),
  d_ata( d )
{
  TQVBoxLayout *topL=new TQVBoxLayout(this, 5);

  // ==== General =============================================================

  TQGroupBox *ggb=new TQGroupBox(i18n("General"), this);
  TQGridLayout *ggbL=new TQGridLayout(ggb, 6,2, 8,5);
  topL->addWidget(ggb);

  ggbL->addRowSpacing(0, fontMetrics().lineSpacing()-4);
  c_harset=new TQComboBox(ggb);
  c_harset->insertStringList(d->composerCharsets());
  ggbL->addWidget(new TQLabel(c_harset, i18n("Cha&rset:"), ggb), 1,0);
  ggbL->addWidget(c_harset, 1,1);
  connect(c_harset, TQT_SIGNAL(activated(int)), TQT_SLOT(changed()));

  e_ncoding=new TQComboBox(ggb);
  e_ncoding->insertItem(i18n("Allow 8-bit"));
  e_ncoding->insertItem(i18n("7-bit (Quoted-Printable)"));
  ggbL->addWidget(new TQLabel(e_ncoding, i18n("Enco&ding:"), ggb), 2,0);
  ggbL->addWidget(e_ncoding, 2,1);
  connect(e_ncoding, TQT_SIGNAL(activated(int)), TQT_SLOT(changed()));

  u_seOwnCSCB=new TQCheckBox(i18n("Use o&wn default charset when replying"), ggb);
  ggbL->addMultiCellWidget(u_seOwnCSCB, 3,3, 0,1);
  connect(u_seOwnCSCB, TQT_SIGNAL(toggled(bool)), TQT_SLOT(changed()));

  g_enMIdCB=new TQCheckBox(i18n("&Generate message-id"), ggb);
  connect(g_enMIdCB, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(slotGenMIdCBToggled(bool)));
  ggbL->addMultiCellWidget(g_enMIdCB, 4,4, 0,1);
  h_ost=new KLineEdit(ggb);
  h_ost->setEnabled(false);
  h_ostL=new TQLabel(h_ost, i18n("Ho&st name:"), ggb);
  h_ostL->setEnabled(false);
  ggbL->addWidget(h_ostL, 5,0);
  ggbL->addWidget(h_ost, 5,1);
  ggbL->setColStretch(1,1);
  connect(h_ost, TQT_SIGNAL(textChanged(const TQString&)), TQT_SLOT(changed()));

  // ==== X-Headers =============================================================

  TQGroupBox *xgb=new TQGroupBox(i18n("X-Headers"), this);
  topL->addWidget(xgb, 1);
  TQGridLayout *xgbL=new TQGridLayout(xgb, 7,2, 8,5);

  xgbL->addRowSpacing(0, fontMetrics().lineSpacing()-4);

  l_box=new KNDialogListBox(false, xgb);
  connect(l_box, TQT_SIGNAL(selected(int)), TQT_SLOT(slotItemSelected(int)));
  connect(l_box, TQT_SIGNAL(selectionChanged()), TQT_SLOT(slotSelectionChanged()));
  xgbL->addMultiCellWidget(l_box, 1,4, 0,0);

  a_ddBtn=new TQPushButton(i18n("&Add..."), xgb);
  connect(a_ddBtn, TQT_SIGNAL(clicked()), TQT_SLOT(slotAddBtnClicked()));
  xgbL->addWidget(a_ddBtn, 1,1);

  d_elBtn=new TQPushButton(i18n("Dele&te"), xgb);
  connect(d_elBtn, TQT_SIGNAL(clicked()), TQT_SLOT(slotDelBtnClicked()));
  xgbL->addWidget(d_elBtn, 2,1);

  e_ditBtn=new TQPushButton(i18n("modify something","&Edit..."), xgb);
  connect(e_ditBtn, TQT_SIGNAL(clicked()), TQT_SLOT(slotEditBtnClicked()));
  xgbL->addWidget(e_ditBtn, 3,1);

  TQLabel *placeHolders = new TQLabel(i18n("<qt>Placeholders for replies: <b>%NAME</b>=sender's name, <b>%EMAIL</b>=sender's address</qt>"), xgb);
  xgbL->addMultiCellWidget(placeHolders, 5, 5, 0, 1);

  i_ncUaCB=new TQCheckBox(i18n("Do not add the \"&User-Agent\" identification header"), xgb);
  xgbL->addMultiCellWidget(i_ncUaCB, 6,6, 0,1);
  connect(i_ncUaCB, TQT_SIGNAL(toggled(bool)), TQT_SLOT(changed()));

  xgbL->setRowStretch(4,1);
  xgbL->setColStretch(0,1);

  load();

  slotSelectionChanged();
}


KNConfig::PostNewsTechnicalWidget::~PostNewsTechnicalWidget()
{
}


void KNConfig::PostNewsTechnicalWidget::load()
{
  c_harset->setCurrentItem(d_ata->indexForCharset(d_ata->charset()));
  e_ncoding->setCurrentItem(d_ata->a_llow8BitBody? 0:1);
  u_seOwnCSCB->setChecked(d_ata->u_seOwnCharset);
  g_enMIdCB->setChecked(d_ata->g_enerateMID);
  h_ost->setText(d_ata->h_ostname);
  i_ncUaCB->setChecked(d_ata->d_ontIncludeUA);

  l_box->clear();
  for(XHeaders::Iterator it=d_ata->x_headers.begin(); it!=d_ata->x_headers.end(); ++it)
    l_box->insertItem((*it).header());
}

void KNConfig::PostNewsTechnicalWidget::save()
{
  d_ata->c_harset=c_harset->currentText().latin1();
  d_ata->a_llow8BitBody=(e_ncoding->currentItem()==0);
  d_ata->u_seOwnCharset=u_seOwnCSCB->isChecked();
  d_ata->g_enerateMID=g_enMIdCB->isChecked();
  d_ata->h_ostname=h_ost->text().latin1();
  d_ata->d_ontIncludeUA=i_ncUaCB->isChecked();
  d_ata->x_headers.clear();
  for(unsigned int idx=0; idx<l_box->count(); idx++)
    d_ata->x_headers.append( XHeader(l_box->text(idx)) );

  d_ata->setDirty(true);
}



void KNConfig::PostNewsTechnicalWidget::slotGenMIdCBToggled(bool b)
{
  h_ost->setEnabled(b);
  h_ostL->setEnabled(b);
  emit changed(true);
}



void KNConfig::PostNewsTechnicalWidget::slotSelectionChanged()
{
  d_elBtn->setEnabled(l_box->currentItem()!=-1);
  e_ditBtn->setEnabled(l_box->currentItem()!=-1);
}



void KNConfig::PostNewsTechnicalWidget::slotItemSelected(int)
{
  slotEditBtnClicked();
}



void KNConfig::PostNewsTechnicalWidget::slotAddBtnClicked()
{
  XHeaderConfDialog *dlg=new XHeaderConfDialog(TQString::null, this);
  if (dlg->exec())
    l_box->insertItem(dlg->result());

  delete dlg;

  slotSelectionChanged();
  emit changed(true);
}



void KNConfig::PostNewsTechnicalWidget::slotDelBtnClicked()
{
  int c=l_box->currentItem();
  if (c == -1)
    return;

  l_box->removeItem(c);
  slotSelectionChanged();
  emit changed(true);
}



void KNConfig::PostNewsTechnicalWidget::slotEditBtnClicked()
{
  int c=l_box->currentItem();
  if (c == -1)
    return;

  XHeaderConfDialog *dlg=new XHeaderConfDialog(l_box->text(c), this);
  if (dlg->exec())
    l_box->changeItem(dlg->result(),c);

  delete dlg;

  slotSelectionChanged();
  emit changed(true);
}


//===================================================================================================


KNConfig::XHeaderConfDialog::XHeaderConfDialog(const TQString &h, TQWidget *p, const char *n)
  : KDialogBase(Plain, i18n("X-Headers"),Ok|Cancel, Ok, p, n)
{
  TQFrame* page=plainPage();
  TQHBoxLayout *topL=new TQHBoxLayout(page, 5,8);
  topL->setAutoAdd(true);

  new TQLabel("X-", page);
  n_ame=new KLineEdit(page);
  new TQLabel(":", page);
  v_alue=new KLineEdit(page);

  int pos=h.find(": ", 2);
  if(pos!=-1) {
    n_ame->setText(h.mid(2, pos-2));
    pos+=2;
    v_alue->setText(h.mid(pos, h.length()-pos));
  }

  setFixedHeight(sizeHint().height());
  KNHelper::restoreWindowSize("XHeaderDlg", this, sizeHint());

  n_ame->setFocus();
}



KNConfig::XHeaderConfDialog::~XHeaderConfDialog()
{
  KNHelper::saveWindowSize("XHeaderDlg", size());
}



TQString KNConfig::XHeaderConfDialog::result()
{
  TQString value = v_alue->text();
  // just in case someone pastes a newline
  value.replace( '\n', ' ' );
  return TQString( "X-%1: %2" ).arg( n_ame->text() ).arg( value );
}


//===================================================================================================


KNConfig::PostNewsComposerWidget::PostNewsComposerWidget( PostNewsComposer *d, TQWidget *p, const char *n ) :
  KCModule( p, n ),
  d_ata( d )
{
  TQVBoxLayout *topL=new TQVBoxLayout(this, 5);

  // === general ===========================================================

  TQGroupBox *generalB=new TQGroupBox(i18n("General"), this);
  topL->addWidget(generalB);
  TQGridLayout *generalL=new TQGridLayout(generalB, 3,3, 8,5);

  generalL->addRowSpacing(0, fontMetrics().lineSpacing()-4);

  w_ordWrapCB=new TQCheckBox(i18n("Word &wrap at column:"), generalB);
  generalL->addWidget(w_ordWrapCB,1,0);
  m_axLen=new KIntSpinBox(20, 200, 1, 20, 10, generalB);
  generalL->addWidget(m_axLen,1,2);
  connect(w_ordWrapCB, TQT_SIGNAL(toggled(bool)), m_axLen, TQT_SLOT(setEnabled(bool)));
  connect(w_ordWrapCB, TQT_SIGNAL(toggled(bool)), TQT_SLOT(changed()));
  connect(m_axLen, TQT_SIGNAL(valueChanged(int)), TQT_SLOT(changed()));

  o_wnSigCB=new TQCheckBox(i18n("Appe&nd signature automatically"), generalB);
  generalL->addMultiCellWidget(o_wnSigCB,2,2,0,1);
  connect(o_wnSigCB, TQT_SIGNAL(toggled(bool)), TQT_SLOT(changed()));

  generalL->setColStretch(1,1);

  // === reply =============================================================

  TQGroupBox *replyB=new TQGroupBox(i18n("Reply"), this);
  topL->addWidget(replyB);
  TQGridLayout *replyL=new TQGridLayout(replyB, 7,2, 8,5);

  replyL->addRowSpacing(0, fontMetrics().lineSpacing()-4);

  i_ntro=new KLineEdit(replyB);
  replyL->addMultiCellWidget(new TQLabel(i_ntro,i18n("&Introduction phrase:"), replyB),1,1,0,1);
  replyL->addMultiCellWidget(i_ntro, 2,2,0,1);
  replyL->addMultiCellWidget(new TQLabel(i18n("<qt>Placeholders: <b>%NAME</b>=sender's name, <b>%EMAIL</b>=sender's address,<br><b>%DATE</b>=date, <b>%MSID</b>=message-id, <b>%GROUP</b>=group name, <b>%L</b>=line break</qt>"), replyB),3,3,0,1);
  connect(i_ntro, TQT_SIGNAL(textChanged(const TQString&)), TQT_SLOT(changed()));

  r_ewrapCB=new TQCheckBox(i18n("Rewrap quoted te&xt automatically"), replyB);
  replyL->addMultiCellWidget(r_ewrapCB, 4,4,0,1);
  connect(r_ewrapCB, TQT_SIGNAL(toggled(bool)), TQT_SLOT(changed()));

  a_uthSigCB=new TQCheckBox(i18n("Include the a&uthor's signature"), replyB);
  replyL->addMultiCellWidget(a_uthSigCB, 5,5,0,1);
  connect(a_uthSigCB, TQT_SIGNAL(toggled(bool)), TQT_SLOT(changed()));

  c_ursorOnTopCB=new TQCheckBox(i18n("Put the cursor &below the introduction phrase"), replyB);
  replyL->addMultiCellWidget(c_ursorOnTopCB, 6,6,0,1);
  connect(c_ursorOnTopCB, TQT_SIGNAL(toggled(bool)), TQT_SLOT(changed()));

  replyL->setColStretch(1,1);

  // === external editor ========================================================

  TQGroupBox *editorB=new TQGroupBox(i18n("External Editor"), this);
  topL->addWidget(editorB);
  TQGridLayout *editorL=new TQGridLayout(editorB, 6,3, 8,5);

  editorL->addRowSpacing(0, fontMetrics().lineSpacing()-4);

  e_ditor=new KLineEdit(editorB);
  editorL->addWidget(new TQLabel(e_ditor, i18n("Specify edi&tor:"), editorB),1,0);
  editorL->addWidget(e_ditor,1,1);
  TQPushButton *btn = new TQPushButton(i18n("Choo&se..."),editorB);
  connect(btn, TQT_SIGNAL(clicked()), TQT_SLOT(slotChooseEditor()));
  connect(e_ditor, TQT_SIGNAL(textChanged(const TQString&)), TQT_SLOT(changed()));
  editorL->addWidget(btn,1,2);

  editorL->addMultiCellWidget(new TQLabel(i18n("%f will be replaced with the filename to edit."), editorB),2,2,0,2);

  e_xternCB=new TQCheckBox(i18n("Start exte&rnal editor automatically"), editorB);
  editorL->addMultiCellWidget(e_xternCB, 3,3,0,2);
  connect(e_xternCB, TQT_SIGNAL(clicked()), TQT_SLOT(changed()));

  editorL->setColStretch(1,1);

  topL->addStretch(1);

  load();
}


KNConfig::PostNewsComposerWidget::~PostNewsComposerWidget()
{
}


void KNConfig::PostNewsComposerWidget::load()
{
  w_ordWrapCB->setChecked(d_ata->w_ordWrap);
  m_axLen->setEnabled(d_ata->w_ordWrap);
  a_uthSigCB->setChecked(d_ata->i_ncSig);
  c_ursorOnTopCB->setChecked(d_ata->c_ursorOnTop);
  e_xternCB->setChecked(d_ata->u_seExtEditor);
  o_wnSigCB->setChecked(d_ata->a_ppSig);
  r_ewrapCB->setChecked(d_ata->r_ewrap);
  m_axLen->setValue(d_ata->m_axLen);
  i_ntro->setText(d_ata->i_ntro);
  e_ditor->setText(d_ata->e_xternalEditor);
}


void KNConfig::PostNewsComposerWidget::save()
{
  d_ata->w_ordWrap=w_ordWrapCB->isChecked();
  d_ata->m_axLen=m_axLen->value();
  d_ata->r_ewrap=r_ewrapCB->isChecked();
  d_ata->a_ppSig=o_wnSigCB->isChecked();
  d_ata->i_ntro=i_ntro->text();
  d_ata->i_ncSig=a_uthSigCB->isChecked();
  d_ata->c_ursorOnTop=c_ursorOnTopCB->isChecked();
  d_ata->e_xternalEditor=e_ditor->text();
  d_ata->u_seExtEditor=e_xternCB->isChecked();

  d_ata->setDirty(true);
}


void KNConfig::PostNewsComposerWidget::slotChooseEditor()
{
  TQString path=e_ditor->text().simplifyWhiteSpace();
  if (path.right(3) == " %f")
    path.truncate(path.length()-3);

  path=KFileDialog::getOpenFileName(path, TQString::null, this, i18n("Choose Editor"));

  if (!path.isEmpty())
    e_ditor->setText(path+" %f");
}


//===================================================================================================


KNConfig::PostNewsSpellingWidget::PostNewsSpellingWidget( TQWidget *p, const char *n ) :
  KCModule( p, n )
{
  TQVBoxLayout *topL=new TQVBoxLayout(this, 5);

  c_onf = new KSpellConfig( this, "spell", 0, false );
  topL->addWidget(c_onf);
  connect(c_onf, TQT_SIGNAL(configChanged()), TQT_SLOT(changed()));

  topL->addStretch(1);
}


KNConfig::PostNewsSpellingWidget::~PostNewsSpellingWidget()
{
}


void KNConfig::PostNewsSpellingWidget::save()
{
  c_onf->writeGlobalSettings();
}


//==============================================================================================================

KNConfig::PrivacyWidget::PrivacyWidget(TQWidget *p, const char *n) :
  KCModule( p, n )
{
  TQBoxLayout *topLayout = new TQVBoxLayout(this, 5);
  c_onf = new Kpgp::Config(this,"knode pgp config",false);
  topLayout->addWidget(c_onf);
  connect(c_onf, TQT_SIGNAL(changed()), TQT_SLOT(changed()));

  topLayout->addStretch(1);

  load();
}


KNConfig::PrivacyWidget::~PrivacyWidget()
{
}


void KNConfig::PrivacyWidget::save()
{
  c_onf->applySettings();
}


//==============================================================================================================


//BEGIN: Cleanup configuration widgets ---------------------------------------


KNConfig::GroupCleanupWidget::GroupCleanupWidget( Cleanup *data, TQWidget *parent, const char *name )
  : TQWidget( parent, name ), mData( data )
{
  TQVBoxLayout *top = new TQVBoxLayout( this );

  if (!mData->isGlobal()) {
    mDefault = new TQCheckBox( i18n("&Use global cleanup configuration"), this );
    connect( mDefault, TQT_SIGNAL(toggled(bool)), TQT_SLOT(slotDefaultToggled(bool)) );
    top->addWidget( mDefault );
  }

  mExpGroup = new TQGroupBox( i18n("Newsgroup Cleanup Settings"), this );
  mExpGroup->setColumnLayout(0, Qt::Vertical );
  mExpGroup->layout()->setSpacing( KDialog::spacingHint() );
  mExpGroup->layout()->setMargin( KDialog::marginHint() );
  top->addWidget( mExpGroup );
  TQGridLayout *grid = new TQGridLayout( mExpGroup->layout(), 7, 2 );

  grid->setRowSpacing( 0, KDialog::spacingHint() );

  mExpEnabled = new TQCheckBox( i18n("&Expire old articles automatically"), mExpGroup );
  grid->addMultiCellWidget( mExpEnabled, 1, 1, 0, 1 );
  connect( mExpEnabled, TQT_SIGNAL(toggled(bool)), TQT_SIGNAL(changed()) );

  mExpDays = new KIntSpinBox( 0, 99999, 1, 0, 10, mExpGroup );
  TQLabel *label = new TQLabel( mExpDays, i18n("&Purge groups every:"), mExpGroup );
  grid->addWidget( label, 2, 0 );
  grid->addWidget( mExpDays, 2, 1, Qt::AlignRight );
  connect( mExpDays, TQT_SIGNAL(valueChanged(int)), TQT_SIGNAL(changed()) );
  connect( mExpDays, TQT_SIGNAL(valueChanged(int)), TQT_SLOT(expDaysChanged(int)) );
  connect( mExpEnabled, TQT_SIGNAL(toggled(bool)), label, TQT_SLOT(setEnabled(bool)) );
  connect( mExpEnabled, TQT_SIGNAL(toggled(bool)), mExpDays, TQT_SLOT(setEnabled(bool)) );

  mExpReadDays = new KIntSpinBox( 0, 99999, 1, 0, 10, mExpGroup );
  label = new TQLabel( mExpReadDays, i18n("&Keep read articles:"), mExpGroup );
  grid->addWidget( label, 3, 0 );
  grid->addWidget( mExpReadDays, 3, 1, Qt::AlignRight );
  connect( mExpReadDays, TQT_SIGNAL(valueChanged(int)), TQT_SIGNAL(changed()) );
  connect( mExpReadDays, TQT_SIGNAL(valueChanged(int)), TQT_SLOT(expReadDaysChanged(int)) );

  mExpUnreadDays = new KIntSpinBox( 0, 99999, 1, 0, 10, mExpGroup );
  label = new TQLabel( mExpUnreadDays, i18n("Keep u&nread articles:"), mExpGroup );
  grid->addWidget( label, 4, 0 );
  grid->addWidget( mExpUnreadDays, 4, 1, Qt::AlignRight );
  connect( mExpUnreadDays, TQT_SIGNAL(valueChanged(int)), TQT_SIGNAL(changed()) );
  connect( mExpUnreadDays, TQT_SIGNAL(valueChanged(int)), TQT_SLOT(expUnreadDaysChanged(int)) );

  mExpUnavailable = new TQCheckBox( i18n("&Remove articles that are not available on the server"), mExpGroup );
  grid->addMultiCellWidget( mExpUnavailable, 5, 5, 0, 1 );
  connect( mExpUnavailable, TQT_SIGNAL(toggled(bool)), TQT_SIGNAL(changed()) );

  mPreserveThreads = new TQCheckBox( i18n("Preser&ve threads"), mExpGroup );
  grid->addMultiCellWidget( mPreserveThreads, 6, 6, 0, 1 );
  connect( mPreserveThreads, TQT_SIGNAL(toggled(bool)), TQT_SIGNAL(changed()) );

  grid->setColStretch(1,1);
}

void KNConfig::GroupCleanupWidget::expDaysChanged(int value)
{
  mExpDays->setSuffix( i18n(" day", " days", value) );
}

void KNConfig::GroupCleanupWidget::expReadDaysChanged(int value)
{
  mExpReadDays->setSuffix( i18n(" day", " days", value) );
}

void KNConfig::GroupCleanupWidget::expUnreadDaysChanged(int value)
{
  mExpUnreadDays->setSuffix( i18n(" day", " days", value) );
}

void KNConfig::GroupCleanupWidget::load()
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


void KNConfig::GroupCleanupWidget::save()
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


void KNConfig::GroupCleanupWidget::slotDefaultToggled( bool state )
{
    mExpGroup->setEnabled( !state );
}


KNConfig::CleanupWidget::CleanupWidget( TQWidget *p, const char *n ) :
  KCModule( p, n ),
  d_ata( knGlobals.configManager()->cleanup() )
{
  TQVBoxLayout *topL=new TQVBoxLayout(this, 5);

  mGroupCleanup = new GroupCleanupWidget( d_ata, this );
  topL->addWidget( mGroupCleanup );
  connect( mGroupCleanup, TQT_SIGNAL(changed()), TQT_SLOT(changed()) );

  // === folders =========================================================

  TQGroupBox *foldersB=new TQGroupBox(i18n("Folders"), this);
  foldersB->setColumnLayout(0, Qt::Vertical );
  foldersB->layout()->setSpacing( KDialog::spacingHint() );
  foldersB->layout()->setMargin( KDialog::marginHint() );

  topL->addWidget(foldersB);
  TQGridLayout *foldersL=new TQGridLayout(foldersB->layout(), 3,2);

  foldersL->setRowSpacing( 0, KDialog::spacingHint() );

  f_olderCB=new TQCheckBox(i18n("Co&mpact folders automatically"), foldersB);
  connect(f_olderCB, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(slotFolderCBtoggled(bool)));
  foldersL->addMultiCellWidget(f_olderCB,1,1,0,1);

  f_olderDays=new KIntSpinBox(0, 99999, 1, 0, 10, foldersB);
  f_olderDaysL=new TQLabel(f_olderDays,i18n("P&urge folders every:"), foldersB);
  foldersL->addWidget(f_olderDaysL,2,0);
  foldersL->addWidget(f_olderDays,2,1,Qt::AlignRight);
  connect(f_olderDays, TQT_SIGNAL(valueChanged(int)), TQT_SLOT(changed()));
  connect(f_olderDays, TQT_SIGNAL(valueChanged(int)), TQT_SLOT(slotFolderDaysChanged(int)));

  foldersL->setColStretch(1,1);

  topL->addStretch(1);

  load();
}


KNConfig::CleanupWidget::~CleanupWidget()
{
}


void KNConfig::CleanupWidget::load()
{
  f_olderCB->setChecked(d_ata->d_oCompact);
  slotFolderCBtoggled(d_ata->d_oCompact);
  f_olderDays->setValue(d_ata->c_ompactInterval);
  mGroupCleanup->load();
}


void KNConfig::CleanupWidget::save()
{
  d_ata->d_oCompact=f_olderCB->isChecked();
  d_ata->c_ompactInterval=f_olderDays->value();

  mGroupCleanup->save();

  d_ata->setDirty(true);
}


void KNConfig::CleanupWidget::slotFolderCBtoggled(bool b)
{
  f_olderDaysL->setEnabled(b);
  f_olderDays->setEnabled(b);
  emit changed(true);
}

void KNConfig::CleanupWidget::slotFolderDaysChanged(int value)
{
  f_olderDays->setSuffix(i18n(" day", " days", value));
}

//END: Cleanup configuration widgets -----------------------------------------

//==============================================================================================================

/*
KNConfig::CacheWidget::CacheWidget(Cache *d, TQWidget *p, const char *n)
  : KCModule p, n), d_ata(d)
{
  TQVBoxLayout *topL=new TQVBoxLayout(this, 5);

  // memory
  TQGroupBox *memGB=new TQGroupBox(i18n("Memory Cache"), this);
  topL->addWidget(memGB);
  TQGridLayout *memL=new TQGridLayout(memGB, 3,2, 8,5);
  memL->addRowSpacing(0, fontMetrics().lineSpacing()-4);

  memL->addWidget(new TQLabel(i18n("Max articles to keep:"), memGB), 1,0);
  m_emMaxArt=new KIntSpinBox(0, 99999, 1, 1, 10, memGB);
  memL->addWidget(m_emMaxArt, 1,1);

  memL->addWidget(new TQLabel(i18n("Max memory usage:"), memGB), 2,0);
  m_emMaxKB=new KIntSpinBox(0, 99999, 1, 1, 10, memGB);
  m_emMaxKB->setSuffix(" KB");
  memL->addWidget(m_emMaxKB, 2,1);

  memL->setColStretch(0,1);


  // disk
  TQGroupBox *diskGB=new TQGroupBox(i18n("Disk Cache"), this);
  topL->addWidget(diskGB);
  TQGridLayout *diskL=new TQGridLayout(diskGB, 3,2, 8,5);
  diskL->addRowSpacing(0, fontMetrics().lineSpacing()-4);

  d_iskMaxArtL=new TQLabel(i18n("Max articles to keep:"), diskGB);
  diskL->addWidget(d_iskMaxArtL, 2,0);
  d_iskMaxArt=new KIntSpinBox(0, 99999, 1, 1, 10, diskGB);
  diskL->addWidget(d_iskMaxArt, 2,1);

  d_iskMaxKBL=new TQLabel(i18n("Max disk usage:"), diskGB);
  diskL->addWidget(d_iskMaxKBL, 3,0);
  d_iskMaxKB=new KIntSpinBox(0, 99999, 1, 1, 10, diskGB);
  d_iskMaxKB->setSuffix(" KB");
  diskL->addWidget(d_iskMaxKB, 3,1);

  diskL->setColStretch(0,1);
  7

  topL->addStretch(1);


  // init
  m_emMaxArt->setValue(d->memoryMaxArticles());
  m_emMaxKB->setValue(d->memoryMaxKBytes());
  d_iskMaxArt->setValue(d->diskMaxArticles());
  d_iskMaxKB->setValue(d->diskMaxKBytes());
}


KNConfig::CacheWidget::~CacheWidget()
{
}


void KNConfig::CacheWidget::apply()
{
  d_ata->m_emMaxArt=m_emMaxArt->value();
  d_ata->m_emMaxKB=m_emMaxKB->value();

  d_ata->d_iskMaxArt=d_iskMaxArt->value();
  d_ata->d_iskMaxKB=d_iskMaxKB->value();

  d_ata->setDirty(true);
}
*/


//------------------------
#include "knconfigwidgets.moc"
