/*
    knconfigwidgets.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2004 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/


#include <qvbox.h>
#include <qpainter.h>
#include <qwhatsthis.h>
#include <qlabel.h>

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
#include "kngroupmanager.h"
#include "knglobals.h"
#include "knnntpaccount.h"
#include "utilities.h"
#include "knarticlewidget.h"
#include "knfiltermanager.h"
#include "knarticlefilter.h"
#include "knscoring.h"
#include "knconfigmanager.h"
#include <kpgp.h>


KNConfig::IdentityWidget::IdentityWidget(Identity *d, QWidget *p, const char *n) : BaseWidget(p, n), d_ata(d)
{
  QString msg;

  QGridLayout *topL=new QGridLayout(this,  11, 3, 5,5);

  n_ame=new KLineEdit(this);
  QLabel *l=new QLabel(n_ame, i18n("&Name:"), this);
  topL->addWidget(l, 0,0);
  topL->addMultiCellWidget(n_ame, 0,0, 1,2);
  msg = i18n("<qt><p>Your name as it will appear to others reading your articles.</p>"
	"<p>Ex: <b>John Stuart Masterson III</b>.</p></qt>");
  QWhatsThis::add( n_ame, msg );
  QWhatsThis::add( l, msg );
  connect( n_ame, SIGNAL(textChanged(const QString&)), SLOT(slotEmitChanged()) );

  o_rga=new KLineEdit(this);
  l=new QLabel(o_rga, i18n("Organi&zation:"), this);
  topL->addWidget(l, 1,0);
  topL->addMultiCellWidget(o_rga, 1,1, 1,2);
  msg = i18n( "<qt><p>The name of the organization you work for.</p>"
	"<p>Ex: <b>KNode, Inc</b>.</p></qt>" );
  QWhatsThis::add( o_rga, msg );
  QWhatsThis::add( l, msg );
  connect( o_rga, SIGNAL(textChanged(const QString&)), SLOT(slotEmitChanged()) );

  e_mail=new KLineEdit(this);
  l=new QLabel(e_mail, i18n("Email a&ddress:"), this);
  topL->addWidget(l, 2,0);
  topL->addMultiCellWidget(e_mail, 2,2, 1,2);
  msg = i18n( "<qt><p>Your email address as it will appear to others "
	"reading your articles</p><p>Ex: <b>nospam@please.com</b>.</qt>" );
  QWhatsThis::add( l, msg );
  QWhatsThis::add( e_mail, msg );
  connect( e_mail, SIGNAL(textChanged(const QString&)), SLOT(slotEmitChanged()) );

  r_eplyTo=new KLineEdit(this);
  l=new QLabel(r_eplyTo, i18n("&Reply-to address:"), this);
  topL->addWidget(l, 3,0);
  topL->addMultiCellWidget(r_eplyTo, 3,3, 1,2);
  msg = i18n( "<qt><p>When someone reply to your article by email, this is the address the message "
	      "will be sent. If you fill in this field, please do it with a real "
	      "email address.</p><p>Ex: <b>john@doe.com</b>.</p></qt>" );
  QWhatsThis::add( l, msg );
  QWhatsThis::add( r_eplyTo, msg );
  connect( r_eplyTo, SIGNAL(textChanged(const QString&)), SLOT(slotEmitChanged()) );

  m_ailCopiesTo=new KLineEdit(this);
  l=new QLabel(m_ailCopiesTo, i18n("&Mail-copies-to:"), this);
  topL->addWidget(l, 4,0);
  topL->addMultiCellWidget(m_ailCopiesTo, 4,4, 1,2);
  connect( m_ailCopiesTo, SIGNAL(textChanged(const QString&)), SLOT(slotEmitChanged()) );

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
  QWhatsThis::add( l, msg );
  QWhatsThis::add( s_igningKey, msg );
  connect( s_igningKey, SIGNAL(changed()), SLOT(slotEmitChanged()) );

  b_uttonGroup = new QButtonGroup(this);
  connect( b_uttonGroup, SIGNAL(clicked(int)),
           this, SLOT(slotSignatureType(int)) );
  b_uttonGroup->setExclusive(true);
  b_uttonGroup->hide();

  s_igFile = new QRadioButton( i18n("&Use a signature from file"), this );
  b_uttonGroup->insert(s_igFile, 0);
  topL->addMultiCellWidget(s_igFile, 6, 6, 0, 2);
  QWhatsThis::add( s_igFile,
		  i18n( "<qt><p>Mark this to let KNode read the signature from a file.</p></qt>" ) );
  s_ig = new KLineEdit(this);

  f_ileName = new QLabel(s_ig, i18n("Signature &file:"), this);
  topL->addWidget(f_ileName, 7, 0 );
  topL->addWidget(s_ig, 7, 1 );
  c_ompletion = new KURLCompletion();
  s_ig->setCompletionObject(c_ompletion);
  msg = i18n( "<qt><p>The file from which the signature will be read.</p>"
	"<p>Ex: <b>/home/robt/.sig</b>.</p></qt>" );
  QWhatsThis::add( f_ileName, msg );
  QWhatsThis::add( s_ig, msg );

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
  QWhatsThis::add( s_igGenerator, msg );
  connect( s_igGenerator, SIGNAL(toggled(bool)), SLOT(slotEmitChanged()) );

  s_igEdit = new QRadioButton( i18n("Specify signature &below"), this);
  b_uttonGroup->insert(s_igEdit, 1);
  topL->addMultiCellWidget(s_igEdit, 9, 9, 0, 2);

  s_igEditor = new QTextEdit(this);
  s_igEditor->setTextFormat(Qt::PlainText);
  topL->addMultiCellWidget(s_igEditor, 10, 10, 0, 2);
  connect( s_igEditor, SIGNAL(textChanged()), SLOT(slotEmitChanged()) );

  topL->setColStretch(1,1);
  topL->setRowStretch(7,1);
  topL->setResizeMode(QLayout::Minimum);
  connect(s_ig,SIGNAL(textChanged ( const QString & )),
          this,SLOT(textFileNameChanged(const QString &)));

  load();
}


KNConfig::IdentityWidget::~IdentityWidget()
{
  delete c_ompletion;
}

void KNConfig::IdentityWidget::textFileNameChanged(const QString &text)
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
  if(!d_irty)
    return;

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
  QString tmp=KFileDialog::getOpenFileName(c_ompletion->replacedPath(s_ig->text()),QString::null,this,i18n("Choose Signature"));
  if(!tmp.isEmpty()) s_ig->setText(tmp);
  emit changed( true );
}


void KNConfig::IdentityWidget::slotSignatureEdit()
{
  QString fileName = c_ompletion->replacedPath(s_ig->text()).stripWhiteSpace();

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
  KURL u(fileName);

  if (offer)
    KRun::run(*offer, u);
  else
    KRun::displayOpenWithDialog(u);
  emit changed( true );
}



//==========================================================================================

// BEGIN: NNTP account configuration widgets ----------------------------------

KNConfig::NntpAccountListWidget::NntpAccountListWidget(QWidget *p, const char *n)
  : BaseWidget(p, n), a_ccManager(knGlobals.accountManager())
{
  p_ixmap = SmallIcon("server");

  QGridLayout *topL=new QGridLayout(this, 6,2, 5,5);

  // account listbox
  l_box=new KNDialogListBox(false, this);
  connect(l_box, SIGNAL(selected(int)), this, SLOT(slotItemSelected(int)));
  connect(l_box, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()));
  topL->addMultiCellWidget(l_box, 0,4, 0,0);

  // info box
  QGroupBox *gb = new QGroupBox(2,Qt::Vertical,QString::null,this);
  topL->addWidget(gb,5,0);

  s_erverInfo = new QLabel(gb);
  p_ortInfo = new QLabel(gb);

  // buttons
  a_ddBtn=new QPushButton(i18n("&Add..."), this);
  connect(a_ddBtn, SIGNAL(clicked()), this, SLOT(slotAddBtnClicked()));
  topL->addWidget(a_ddBtn, 0,1);

  e_ditBtn=new QPushButton(i18n("modify something","&Edit..."), this);
  connect(e_ditBtn, SIGNAL(clicked()), this, SLOT(slotEditBtnClicked()));
  topL->addWidget(e_ditBtn, 1,1);

  d_elBtn=new QPushButton(i18n("&Delete"), this);
  connect(d_elBtn, SIGNAL(clicked()), this, SLOT(slotDelBtnClicked()));
  topL->addWidget(d_elBtn, 2,1);

  s_ubBtn=new QPushButton(i18n("&Subscribe..."), this);
  connect(s_ubBtn, SIGNAL(clicked()), this, SLOT(slotSubBtnClicked()));
  topL->addWidget(s_ubBtn, 3,1);

  topL->setRowStretch(4,1);   // stretch the server listbox

  load();

  // the settings dialog is non-modal, so we have to react to changes
  // made outside of the dialog
  connect(a_ccManager, SIGNAL(accountAdded(KNNntpAccount*)), this, SLOT(slotAddItem(KNNntpAccount*)));
  connect(a_ccManager, SIGNAL(accountRemoved(KNNntpAccount*)), this, SLOT(slotRemoveItem(KNNntpAccount*)));
  connect(a_ccManager, SIGNAL(accountModified(KNNntpAccount*)), this, SLOT(slotUpdateItem(KNNntpAccount*)));

  slotSelectionChanged();     // disable Delete & Edit initially
}


KNConfig::NntpAccountListWidget::~NntpAccountListWidget()
{
}


void KNConfig::NntpAccountListWidget::load()
{
  l_box->clear();
  for(KNNntpAccount *a=a_ccManager->first(); a; a=a_ccManager->next())
    slotAddItem(a);
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
    a_ccManager->newAccount(acc);
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


KNConfig::NntpAccountConfDialog::NntpAccountConfDialog(KNNntpAccount *a, QWidget *p, const char *n)
  : KDialogBase(Tabbed, (a->id()!=-1)? i18n("Properties of %1").arg(a->name()):i18n("New Account"),
                Ok|Cancel|Help, Ok, p, n),
    a_ccount(a)
{
  QFrame* page=addPage(i18n("Ser&ver"));
  QGridLayout *topL=new QGridLayout(page, 11, 3, 5);

  n_ame=new KLineEdit(page);
  QLabel *l=new QLabel(n_ame,i18n("&Name:"),page);
  topL->addWidget(l, 0,0);
  n_ame->setText(a->name());
  topL->addMultiCellWidget(n_ame, 0, 0, 1, 2);

  s_erver=new KLineEdit(page);
  l=new QLabel(s_erver,i18n("&Server:"), page);
  s_erver->setText(a->server());
  topL->addWidget(l, 1,0);
  topL->addMultiCellWidget(s_erver, 1, 1, 1, 2);

  p_ort=new KLineEdit(page);
  l=new QLabel(p_ort, i18n("&Port:"), page);
  p_ort->setValidator(new KIntValidator(0,65536,this));
  p_ort->setText(QString::number(a->port()));
  topL->addWidget(l, 2,0);
  topL->addWidget(p_ort, 2,1);

  h_old = new KIntSpinBox(5,1800,5,5,10,page);
  l = new QLabel(h_old,i18n("Hol&d connection for:"), page);
  h_old->setSuffix(i18n(" sec"));
  h_old->setValue(a->hold());
  topL->addWidget(l,3,0);
  topL->addWidget(h_old,3,1);

  t_imeout = new KIntSpinBox(15,600,5,15,10,page);
  l = new QLabel(t_imeout, i18n("&Timeout:"), page);
  t_imeout->setValue(a->timeout());
  t_imeout->setSuffix(i18n(" sec"));
  topL->addWidget(l,4,0);
  topL->addWidget(t_imeout,4,1);

  f_etchDes=new QCheckBox(i18n("&Fetch group descriptions"), page);
  f_etchDes->setChecked(a->fetchDescriptions());
  topL->addMultiCellWidget(f_etchDes, 5,5, 0,3);

  /*u_seDiskCache=new QCheckBox(i18n("&Cache articles on disk"), page);
  u_seDiskCache->setChecked(a->useDiskCache());
  topL->addMultiCellWidget(u_seDiskCache, 6,6, 0,3);*/

  a_uth=new QCheckBox(i18n("Server requires &authentication"), page);
  connect(a_uth, SIGNAL(toggled(bool)), this, SLOT(slotAuthChecked(bool)));
  topL->addMultiCellWidget(a_uth, 6,6, 0,3);

  u_ser=new KLineEdit(page);
  u_serLabel=new QLabel(u_ser,i18n("&User:"), page);
  u_ser->setText(a->user());
  topL->addWidget(u_serLabel, 7,0);
  topL->addMultiCellWidget(u_ser, 7,7, 1,2);

  p_ass=new KLineEdit(page);
  p_assLabel=new QLabel(p_ass, i18n("Pass&word:"), page);
  p_ass->setEchoMode(KLineEdit::Password);
  p_ass->setText(a->pass());
  topL->addWidget(p_assLabel, 8,0);
  topL->addMultiCellWidget(p_ass, 8,8, 1,2);

  i_nterval=new QCheckBox(i18n("Enable &interval news checking"), page);
  connect(i_nterval, SIGNAL(toggled(bool)), this, SLOT(slotIntervalChecked(bool)));
  topL->addMultiCellWidget(i_nterval, 9,9, 0,3);

  c_heckInterval=new KIntSpinBox(1,10000,1,1,10,page);
  c_heckIntervalLabel=new QLabel(c_heckInterval, i18n("Check inter&val:"), page);
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
  QFrame* cleanupPage = addPage( i18n("&Cleanup") );
  QVBoxLayout *cleanupLayout = new QVBoxLayout( cleanupPage, KDialog::spacingHint() );
  mCleanupWidget = new GroupCleanupWidget( a->cleanupConfig(), cleanupPage );
  mCleanupWidget->load();
  cleanupLayout->addWidget( mCleanupWidget );
  cleanupLayout->addStretch( 1 );


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

// END: NNTP account configuration widgets ------------------------------------

//=============================================================================================


KNConfig::SmtpAccountWidget::SmtpAccountWidget(QWidget *p, const char *n) : BaseWidget(p, n)
{
  QGridLayout *topL=new QGridLayout(this, 6, 3, 5);

  u_seExternalMailer = new QCheckBox(i18n("&Use external mail program"), this);
  connect(u_seExternalMailer, SIGNAL(toggled(bool)), SLOT(useExternalMailerToggled(bool)));
  topL->addMultiCellWidget(u_seExternalMailer, 0, 0, 0, 2);

  s_erver=new KLineEdit(this);
  s_erverLabel=new QLabel(s_erver, i18n("&Server:"), this);
  topL->addWidget(s_erverLabel, 1,0);
  topL->addMultiCellWidget(s_erver, 1, 1, 1, 2);
  connect(s_erver, SIGNAL(textChanged(const QString&)), SLOT(slotEmitChanged()));

  p_ort=new KLineEdit(this);
  p_ortLabel=new QLabel(p_ort, i18n("&Port:"), this);
  topL->addWidget(p_ortLabel, 2,0);
  p_ort->setValidator(new KIntValidator(0,65536,this));
  topL->addWidget(p_ort, 2,1);
  connect(p_ort, SIGNAL(textChanged(const QString&)), SLOT(slotEmitChanged()));

  h_old = new KIntSpinBox(0,300,5,0,10,this);
  h_old->setSuffix(i18n(" sec"));
  h_oldLabel = new QLabel(h_old, i18n("Hol&d connection for:"), this);
  topL->addWidget(h_oldLabel,3,0);
  topL->addWidget(h_old,3,1);
  connect(h_old, SIGNAL(valueChanged(int)), SLOT(slotEmitChanged()));

  t_imeout = new KIntSpinBox(15,300,5,15,10,this);
  t_imeout->setSuffix(i18n(" sec"));
  t_imeoutLabel = new QLabel(t_imeout, i18n("&Timeout:"), this);
  topL->addWidget(t_imeoutLabel,4,0);
  topL->addWidget(t_imeout,4,1);
  connect(t_imeout, SIGNAL(valueChanged(int)), SLOT(slotEmitChanged()));

  topL->setColStretch(1,1);
  topL->setColStretch(2,1);

  s_erverInfo=knGlobals.accountManager()->smtp();

  load();
}



KNConfig::SmtpAccountWidget::~SmtpAccountWidget()
{
}


void KNConfig::SmtpAccountWidget::load()
{
  u_seExternalMailer->setChecked(knGlobals.configManager()->postNewsTechnical()->useExternalMailer());
  useExternalMailerToggled(knGlobals.configManager()->postNewsTechnical()->useExternalMailer());
  s_erver->setText(s_erverInfo->server());
  p_ort->setText(QString::number(s_erverInfo->port()));
  h_old->setValue(s_erverInfo->hold());
  t_imeout->setValue(s_erverInfo->timeout());
}

void KNConfig::SmtpAccountWidget::save()
{
  if(!d_irty)
    return;

  knGlobals.configManager()->postNewsTechnical()->u_seExternalMailer = u_seExternalMailer->isChecked();
  knGlobals.configManager()->postNewsTechnical()->setDirty(true);

  s_erverInfo->setServer(s_erver->text());
  s_erverInfo->setPort(p_ort->text().toInt());
  s_erverInfo->setHold(h_old->value());
  s_erverInfo->setTimeout(t_imeout->value());

  KConfig *conf=knGlobals.config();
  conf->setGroup("MAILSERVER");
  s_erverInfo->saveConf(conf);
}


void KNConfig::SmtpAccountWidget::useExternalMailerToggled(bool b)
{
  s_erver->setEnabled(!b);
  p_ort->setEnabled(!b);
  h_old->setEnabled(!b);
  t_imeout->setEnabled(!b);
  s_erverLabel->setEnabled(!b);
  p_ortLabel->setEnabled(!b);
  h_oldLabel->setEnabled(!b);
  t_imeoutLabel->setEnabled(!b);
  emit changed(true);
}


//=============================================================================================


//===================================================================================
// code taken from KMail, Copyright (C) 2000 Espen Sand, espen@kde.org

KNConfig::AppearanceWidget::ColorListItem::ColorListItem( const QString &text, const QColor &color )
  : QListBoxText(text), mColor( color )
{
}


KNConfig::AppearanceWidget::ColorListItem::~ColorListItem()
{
}


void KNConfig::AppearanceWidget::ColorListItem::paint( QPainter *p )
{
  QFontMetrics fm = p->fontMetrics();
  int h = fm.height();

  p->drawText( 30+3*2, fm.ascent() + fm.leading()/2, text() );

  p->setPen( Qt::black );
  p->drawRect( 3, 1, 30, h-1 );
  p->fillRect( 4, 2, 28, h-3, mColor );
}


int KNConfig::AppearanceWidget::ColorListItem::height(const QListBox *lb ) const
{
  return( lb->fontMetrics().lineSpacing()+1 );
}


int KNConfig::AppearanceWidget::ColorListItem::width(const QListBox *lb ) const
{
  return( 30 + lb->fontMetrics().width( text() ) + 6 );
}


//===================================================================================


KNConfig::AppearanceWidget::FontListItem::FontListItem( const QString &name, const QFont &font )
  : QListBoxText(name), f_ont(font)
{
  fontInfo = QString("[%1 %2]").arg(f_ont.family()).arg(f_ont.pointSize());
}


KNConfig::AppearanceWidget::FontListItem::~FontListItem()
{
}


void KNConfig::AppearanceWidget::FontListItem::setFont(const QFont &font)
{
  f_ont = font;
  fontInfo = QString("[%1 %2]").arg(f_ont.family()).arg(f_ont.pointSize());
}


void KNConfig::AppearanceWidget::FontListItem::paint( QPainter *p )
{
  QFont fnt = p->font();
  fnt.setWeight(QFont::Bold);
  p->setFont(fnt);
  int fontInfoWidth = p->fontMetrics().width(fontInfo);
  int h = p->fontMetrics().ascent() + p->fontMetrics().leading()/2;
  p->drawText(2, h, fontInfo );
  fnt.setWeight(QFont::Normal);
  p->setFont(fnt);
  p->drawText(5 + fontInfoWidth, h, text() );
}


int KNConfig::AppearanceWidget::FontListItem::width(const QListBox *lb ) const
{
  return( lb->fontMetrics().width(fontInfo) + lb->fontMetrics().width(text()) + 20 );
}


//===================================================================================


KNConfig::AppearanceWidget::AppearanceWidget(QWidget *p, const char *n)
  : BaseWidget(p, n), d_ata(knGlobals.configManager()->appearance())
{
  QGridLayout *topL=new QGridLayout(this, 8,2, 5,5);

  //color-list
  c_List = new KNDialogListBox(false, this);
  topL->addMultiCellWidget(c_List,1,3,0,0);
  connect(c_List, SIGNAL(selected(QListBoxItem*)),SLOT(slotColItemSelected(QListBoxItem*)));
  connect(c_List, SIGNAL(selectionChanged()), SLOT(slotColSelectionChanged()));

  c_olorCB = new QCheckBox(i18n("&Use custom colors"),this);
  topL->addWidget(c_olorCB,0,0);
  connect(c_olorCB, SIGNAL(toggled(bool)), this, SLOT(slotColCheckBoxToggled(bool)));

  c_olChngBtn=new QPushButton(i18n("Cha&nge..."), this);
  connect(c_olChngBtn, SIGNAL(clicked()), this, SLOT(slotColChangeBtnClicked()));
  topL->addWidget(c_olChngBtn,1,1);

  //font-list
  f_List = new KNDialogListBox(false, this);
  topL->addMultiCellWidget(f_List,5,7,0,0);
  connect(f_List, SIGNAL(selected(QListBoxItem*)),SLOT(slotFontItemSelected(QListBoxItem*)));
  connect(f_List, SIGNAL(selectionChanged()),SLOT(slotFontSelectionChanged()));

  f_ontCB = new QCheckBox(i18n("Use custom &fonts"),this);
  topL->addWidget(f_ontCB,4,0);
  connect(f_ontCB, SIGNAL(toggled(bool)), this, SLOT(slotFontCheckBoxToggled(bool)));

  f_ntChngBtn=new QPushButton(i18n("Chang&e..."), this);
  connect(f_ntChngBtn, SIGNAL(clicked()), this, SLOT(slotFontChangeBtnClicked()));
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
  if(!d_irty)
    return;

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
void KNConfig::AppearanceWidget::slotColItemSelected(QListBoxItem *it)
{
  if (it) {
    ColorListItem *colorItem = static_cast<ColorListItem*>(it);
    QColor col = colorItem->color();
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
void KNConfig::AppearanceWidget::slotFontItemSelected(QListBoxItem *it)
{
  if (it) {
    FontListItem *fontItem = static_cast<FontListItem*>(it);
    QFont font = fontItem->font();
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


KNConfig::ReadNewsGeneralWidget::ReadNewsGeneralWidget(ReadNewsGeneral *d, QWidget *p, const char *n)
  : BaseWidget(p, n), d_ata(d)
{
  QGroupBox *hgb=new QGroupBox(i18n("Article Handling"), this);
  QGroupBox *lgb=new QGroupBox(i18n("Article List"), this);
  QGroupBox *cgb=new QGroupBox(i18n("Memory Consumption"), this);
  QLabel *l1, *l2, *l3;

  a_utoCB=new QCheckBox(i18n("Check for new articles a&utomatically"), hgb);
  m_axFetch=new KIntSpinBox(0, 100000, 1, 0, 10, hgb);
  l1=new QLabel(m_axFetch, i18n("&Maximum number of articles to fetch:"), hgb);
  m_arkCB=new QCheckBox(i18n("Mar&k article as read after:"), hgb);
  m_arkSecs=new KIntSpinBox(0, 9999, 1, 0, 10, hgb);
  connect(m_arkCB, SIGNAL(toggled(bool)), m_arkSecs, SLOT(setEnabled(bool)));
  m_arkSecs->setSuffix(i18n(" sec"));
  m_arkCrossCB=new QCheckBox(i18n("Mark c&rossposted articles as read"), hgb);

  s_martScrollingCB=new QCheckBox(i18n("Smart scrolli&ng"), lgb);
  e_xpThrCB=new QCheckBox(i18n("Show &whole thread on expanding"), lgb);
  d_efaultExpandCB=new QCheckBox(i18n("Default to e&xpanded threads"), lgb);
  s_coreCB=new QCheckBox(i18n("Show article &score"), lgb);
  l_inesCB=new QCheckBox(i18n("Show &line count"), lgb);
  u_nreadCB=new QCheckBox(i18n("Show unread count in &thread"), lgb);

  c_ollCacheSize=new KIntSpinBox(0, 99999, 1, 1, 10, cgb);
  c_ollCacheSize->setSuffix(" KB");
  l2=new QLabel(c_ollCacheSize, i18n("Cach&e size for headers:"), cgb);
  a_rtCacheSize=new KIntSpinBox(0, 99999, 1, 1, 10, cgb);
  a_rtCacheSize->setSuffix(" KB");
  l3=new QLabel(a_rtCacheSize, i18n("Cache si&ze for articles:"), cgb);

  QVBoxLayout *topL=new QVBoxLayout(this, 5);
  QGridLayout *hgbL=new QGridLayout(hgb, 5,2, 8,5);
  QVBoxLayout *lgbL=new QVBoxLayout(lgb, 8, 5);
  QGridLayout *cgbL=new QGridLayout(cgb, 3,2, 8,5);

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

  topL->setResizeMode(QLayout::Minimum);

  connect(a_utoCB,           SIGNAL(toggled(bool)),     SLOT(slotEmitChanged()));
  connect(m_axFetch,         SIGNAL(valueChanged(int)), SLOT(slotEmitChanged()));
  connect(m_arkCB,           SIGNAL(toggled(bool)),     SLOT(slotEmitChanged()));
  connect(m_arkSecs,         SIGNAL(valueChanged(int)), SLOT(slotEmitChanged()));
  connect(m_arkCrossCB,      SIGNAL(toggled(bool)),     SLOT(slotEmitChanged()));
  connect(s_martScrollingCB, SIGNAL(toggled(bool)),     SLOT(slotEmitChanged()));
  connect(e_xpThrCB,         SIGNAL(toggled(bool)),     SLOT(slotEmitChanged()));
  connect(d_efaultExpandCB,  SIGNAL(toggled(bool)),     SLOT(slotEmitChanged()));
  connect(l_inesCB,          SIGNAL(toggled(bool)),     SLOT(slotEmitChanged()));
  connect(s_coreCB,          SIGNAL(toggled(bool)),     SLOT(slotEmitChanged()));
  connect(u_nreadCB,         SIGNAL(toggled(bool)),     SLOT(slotEmitChanged()));
  connect(c_ollCacheSize,    SIGNAL(valueChanged(int)), SLOT(slotEmitChanged()));
  connect(a_rtCacheSize,     SIGNAL(valueChanged(int)), SLOT(slotEmitChanged()));

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
  if(!d_irty)
    return;

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


KNConfig::ReadNewsNavigationWidget::ReadNewsNavigationWidget(ReadNewsNavigation *d, QWidget *p, const char *n)
  : BaseWidget(p, n), d_ata(d)
{
  QVBoxLayout *topL=new QVBoxLayout(this, 5);

  // ==== Mark All as Read ====================================================

  QGroupBox *gb=new QGroupBox(i18n("\"Mark All as Read\" Triggers Following Actions"), this);
  QVBoxLayout *gbL=new QVBoxLayout(gb, 8, 5);
  topL->addWidget(gb);

  gbL->addSpacing(fontMetrics().lineSpacing()-4);
  m_arkAllReadGoNextCB=new QCheckBox(i18n("&Switch to the next group"), gb);
  gbL->addWidget(m_arkAllReadGoNextCB);

  connect(m_arkAllReadGoNextCB, SIGNAL(toggled(bool)), SLOT(slotEmitChanged()));

  // ==== Mark Thread as Read =================================================

  gb=new QGroupBox(i18n("\"Mark Thread as Read\" Triggers Following Actions"), this);
  gbL=new QVBoxLayout(gb, 8, 5);
  topL->addWidget(gb);

  gbL->addSpacing(fontMetrics().lineSpacing()-4);
  m_arkThreadReadCloseThreadCB=new QCheckBox(i18n("Clos&e the current thread"), gb);
  gbL->addWidget(m_arkThreadReadCloseThreadCB);
  m_arkThreadReadGoNextCB=new QCheckBox(i18n("Go &to the next unread thread"), gb);
  gbL->addWidget(m_arkThreadReadGoNextCB);

  connect(m_arkThreadReadCloseThreadCB, SIGNAL(toggled(bool)), SLOT(slotEmitChanged()));
  connect(m_arkThreadReadGoNextCB, SIGNAL(toggled(bool)), SLOT(slotEmitChanged()));

  // ==== Ignore Thread =======================================================

  gb=new QGroupBox(i18n("\"Ignore Thread\" Triggers Following Actions"), this);
  gbL=new QVBoxLayout(gb, 8, 5);
  topL->addWidget(gb);

  gbL->addSpacing(fontMetrics().lineSpacing()-4);
  i_gnoreThreadCloseThreadCB=new QCheckBox(i18n("Close the cu&rrent thread"), gb);
  gbL->addWidget(i_gnoreThreadCloseThreadCB);
  i_gnoreThreadGoNextCB=new QCheckBox(i18n("Go to the next &unread thread"), gb);
  gbL->addWidget(i_gnoreThreadGoNextCB);

  connect(i_gnoreThreadCloseThreadCB, SIGNAL(toggled(bool)), SLOT(slotEmitChanged()));
  connect(i_gnoreThreadGoNextCB, SIGNAL(toggled(bool)), SLOT(slotEmitChanged()));

  topL->addStretch(1);
  topL->setResizeMode(QLayout::Minimum);

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
  if(!d_irty)
    return;

  d_ata->m_arkAllReadGoNext = m_arkAllReadGoNextCB->isChecked();
  d_ata->m_arkThreadReadGoNext = m_arkThreadReadGoNextCB->isChecked();
  d_ata->m_arkThreadReadCloseThread = m_arkThreadReadCloseThreadCB->isChecked();
  d_ata->i_gnoreThreadGoNext = i_gnoreThreadGoNextCB->isChecked();
  d_ata->i_gnoreThreadCloseThread = i_gnoreThreadCloseThreadCB->isChecked();

  d_ata->setDirty(true);
}


//=============================================================================================


KNConfig::ReadNewsViewerWidget::ReadNewsViewerWidget(ReadNewsViewer *d, QWidget *p, const char *n)
  : BaseWidget(p, n), d_ata(d)
{
  QGroupBox *appgb=new QGroupBox(i18n("Appearance"), this);
  QGroupBox *agb=new QGroupBox(i18n("Attachments"), this);
  QGroupBox *bgb=new QGroupBox(i18n("Browser"), this);
  QLabel *l1;

  d_ecoCB=new QCheckBox(i18n("Show fancy header deco&rations"), appgb);
  r_ewrapCB=new QCheckBox(i18n("Re&wrap text when necessary"), appgb);
  r_emoveTrailingCB=new QCheckBox(i18n("Re&move trailing empty lines"), appgb);
  s_igCB=new QCheckBox(i18n("Show sig&nature"), appgb);
  f_ormatCB=new QCheckBox(i18n("Interpret te&xt format tags"), appgb);
  q_uoteCharacters=new KLineEdit(appgb);
  QLabel *quoteCharL = new QLabel(q_uoteCharacters, i18n("Recognized q&uote characters:"), appgb);

  i_nlineCB=new QCheckBox(i18n("Show attachments &inline if possible"), agb);
  o_penAttCB=new QCheckBox(i18n("Open a&ttachments on click"), agb);
  a_ltAttCB=new QCheckBox(i18n("Show alternati&ve contents as attachments"), agb);

  b_rowser=new QComboBox(bgb);
  b_rowser->insertItem(i18n("Default Browser"));
  b_rowser->insertItem("Konqueror");
  b_rowser->insertItem("Netscape");
  b_rowser->insertItem("Mozilla");
  b_rowser->insertItem("Opera");
  b_rowser->insertItem(i18n("Other Browser"));
  connect(b_rowser, SIGNAL(activated(int)), SLOT(slotBrowserTypeChanged(int)));
  l1=new QLabel(b_rowser, i18n("Open &links with:"), bgb);
  b_rowserCommand = new KLineEdit(bgb);
  c_hooseBrowser= new QPushButton(i18n("Choo&se..."),bgb);
  connect(c_hooseBrowser, SIGNAL(clicked()), SLOT(slotChooseBrowser()));

  QVBoxLayout *topL=new QVBoxLayout(this, 5);
  QGridLayout *appgbL=new QGridLayout(appgb, 7,2, 8,5);
  QVBoxLayout *agbL=new QVBoxLayout(agb, 8, 5);
  QGridLayout *bgbL=new QGridLayout(bgb, 3,3, 8,5);

  topL->addWidget(appgb);
  topL->addWidget(agb);
  topL->addWidget(bgb);
  topL->addStretch(1);

  appgbL->addRowSpacing(0, fontMetrics().lineSpacing()-4);
  appgbL->addMultiCellWidget(d_ecoCB, 1,1, 0,1);
  appgbL->addMultiCellWidget(r_ewrapCB, 2,2, 0,1);
  appgbL->addMultiCellWidget(r_emoveTrailingCB, 3,3, 0,1);
  appgbL->addMultiCellWidget(s_igCB, 4,4, 0,1);
  appgbL->addMultiCellWidget(f_ormatCB, 5,5, 0,1);
  appgbL->addWidget(quoteCharL, 6,0);
  appgbL->addWidget(q_uoteCharacters, 6,1);

  agbL->addSpacing(fontMetrics().lineSpacing()-4);
  agbL->addWidget(i_nlineCB);
  agbL->addWidget(o_penAttCB);
  agbL->addWidget(a_ltAttCB);

  bgbL->addRowSpacing(0, fontMetrics().lineSpacing()-4);
  bgbL->addWidget(l1, 1,0);
  bgbL->addMultiCellWidget(b_rowser,1,1,1,2);
  bgbL->addMultiCellWidget(b_rowserCommand,2,2,0,1);
  bgbL->addWidget(c_hooseBrowser,2,2);
  bgbL->setColStretch(1,1);

  topL->setResizeMode(QLayout::Minimum);

  connect(d_ecoCB, SIGNAL(toggled(bool)), SLOT(slotEmitChanged()));
  connect(r_ewrapCB, SIGNAL(toggled(bool)), SLOT(slotEmitChanged()));
  connect(r_emoveTrailingCB, SIGNAL(toggled(bool)), SLOT(slotEmitChanged()));
  connect(s_igCB, SIGNAL(toggled(bool)), SLOT(slotEmitChanged()));
  connect(f_ormatCB, SIGNAL(toggled(bool)), SLOT(slotEmitChanged()));
  connect(q_uoteCharacters, SIGNAL(textChanged(const QString&)), SLOT(slotEmitChanged()));
  connect(i_nlineCB, SIGNAL(toggled(bool)), SLOT(slotEmitChanged()));
  connect(o_penAttCB, SIGNAL(toggled(bool)), SLOT(slotEmitChanged()));
  connect(a_ltAttCB, SIGNAL(toggled(bool)), SLOT(slotEmitChanged()));
  connect(b_rowserCommand, SIGNAL(textChanged(const QString&)), SLOT(slotEmitChanged()));

  load();
}


KNConfig::ReadNewsViewerWidget::~ReadNewsViewerWidget()
{
}


void KNConfig::ReadNewsViewerWidget::load()
{
  d_ecoCB->setChecked(d_ata->s_howHeaderDeco);
  r_ewrapCB->setChecked(d_ata->r_ewrapBody);
  r_emoveTrailingCB->setChecked(d_ata->r_emoveTrailingNewlines);
  s_igCB->setChecked(d_ata->s_howSig);
  f_ormatCB->setChecked(d_ata->i_nterpretFormatTags);
  q_uoteCharacters->setText(d_ata->q_uoteCharacters);
  i_nlineCB->setChecked(d_ata->i_nlineAtt);
  o_penAttCB->setChecked(d_ata->o_penAtt);
  a_ltAttCB->setChecked(d_ata->s_howAlts);
  b_rowser->setCurrentItem((int)(d_ata->b_rowser));
  b_rowserCommand->setText(d_ata->b_rowserCommand);
  b_rowserCommand->setEnabled(d_ata->b_rowser==ReadNewsViewer::BTother);
  c_hooseBrowser->setEnabled(d_ata->b_rowser==ReadNewsViewer::BTother);
}


void KNConfig::ReadNewsViewerWidget::save()
{
  if(!d_irty)
    return;

  d_ata->s_howHeaderDeco=d_ecoCB->isChecked();
  d_ata->r_ewrapBody=r_ewrapCB->isChecked();
  d_ata->r_emoveTrailingNewlines=r_emoveTrailingCB->isChecked();
  d_ata->s_howSig=s_igCB->isChecked();
  d_ata->i_nterpretFormatTags=f_ormatCB->isChecked();
  d_ata->q_uoteCharacters=q_uoteCharacters->text();
  d_ata->i_nlineAtt=i_nlineCB->isChecked();
  d_ata->o_penAtt=o_penAttCB->isChecked();
  d_ata->s_howAlts=a_ltAttCB->isChecked();
  d_ata->b_rowser=(ReadNewsViewer::browserType)(b_rowser->currentItem());
  d_ata->b_rowserCommand=b_rowserCommand->text();

  d_ata->setDirty(true);
}


void KNConfig::ReadNewsViewerWidget::slotBrowserTypeChanged(int i)
{
  bool enabled=((ReadNewsViewer::browserType)(i)==ReadNewsViewer::BTother);
  b_rowserCommand->setEnabled(enabled);
  c_hooseBrowser->setEnabled(enabled);
  emit changed(true);
}


void KNConfig::ReadNewsViewerWidget::slotChooseBrowser()
{
  QString path=b_rowserCommand->text().simplifyWhiteSpace();
  if (path.right(3) == " %u")
    path.truncate(path.length()-3);

  path=KFileDialog::getOpenFileName(path, QString::null, this, i18n("Choose Browser"));

  if (!path.isEmpty())
    b_rowserCommand->setText(path+" %u");
}


//=============================================================================================


KNConfig::DisplayedHeadersWidget::DisplayedHeadersWidget(DisplayedHeaders *d, QWidget *p, const char *n)
  : BaseWidget(p, n), s_ave(false), d_ata(d)
{
  QGridLayout *topL=new QGridLayout(this, 7,2, 5,5);

  //listbox
  l_box=new KNDialogListBox(false, this);
  connect(l_box, SIGNAL(selected(int)), this, SLOT(slotItemSelected(int)));
  connect(l_box, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()));
  topL->addMultiCellWidget(l_box, 0,6, 0,0);

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



KNConfig::DisplayedHeadersWidget::~DisplayedHeadersWidget()
{
}


void KNConfig::DisplayedHeadersWidget::load()
{
  l_box->clear();
  for(KNDisplayedHeader *h = d_ata->h_drList.first(); h; h = d_ata->h_drList.next())
    l_box->insertItem(generateItem(h));
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


KNConfig::DisplayedHeaderConfDialog::DisplayedHeaderConfDialog(KNDisplayedHeader *h, QWidget *p, char *n)
  : KDialogBase(Plain, i18n("Header Properties"),Ok|Cancel|Help, Ok, p, n),
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
  QVBoxLayout *ngbL = new QVBoxLayout(ngb, 8, 5);
  ngbL->setAutoAdd(true);
  ngbL->addSpacing(fontMetrics().lineSpacing()-4);
  n_ameCB[0]=new QCheckBox(i18n("&Large"), ngb);
  n_ameCB[1]=new QCheckBox(i18n("&Bold"), ngb);
  n_ameCB[2]=new QCheckBox(i18n("&Italic"), ngb);
  n_ameCB[3]=new QCheckBox(i18n("&Underlined"), ngb);
  topL->addWidget(ngb,1,0);

  QGroupBox *vgb=new QGroupBox(i18n("Value"), page);
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
  h_drC->insertStrList(KNDisplayedHeader::predefs());
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
void KNConfig::DisplayedHeaderConfDialog::slotNameChanged(const QString& str)
{
  for(int i=0; i<4; i++)
      n_ameCB[i]->setEnabled(!str.isEmpty());
}

//=============================================================================================


KNConfig::ScoringWidget::ScoringWidget(Scoring *d, QWidget *p, const char *n)
  : BaseWidget(p,n), d_ata(d)
{
  QGridLayout *topL = new QGridLayout(this,4,2, 5,5);
  ksc = new KScoringEditorWidget(knGlobals.scoringManager(), this);
  topL->addMultiCellWidget(ksc, 0,0, 0,1);

  topL->addRowSpacing(1, 10);

  i_gnored=new KIntSpinBox(-100000, 100000, 1, 0, 10, this);
  QLabel *l=new QLabel(i_gnored, i18n("Default score for &ignored threads:"), this);
  topL->addWidget(l, 2, 0);
  topL->addWidget(i_gnored, 2, 1);
  connect(i_gnored, SIGNAL(valueChanged(int)), SLOT(slotEmitChanged()));

  w_atched=new KIntSpinBox(-100000, 100000, 1, 0, 10, this);
  l=new QLabel(w_atched, i18n("Default score for &watched threads:"), this);
  topL->addWidget(l, 3, 0);
  topL->addWidget(w_atched, 3, 1);
  connect(w_atched, SIGNAL(valueChanged(int)), SLOT(slotEmitChanged()));

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
  if(!d_irty)
    return;

  d_ata->i_gnoredThreshold = i_gnored->value();
  d_ata->w_atchedThreshold = w_atched->value();

  d_ata->setDirty(true);
}


//=============================================================================================


KNConfig::FilterListWidget::FilterListWidget(QWidget *p, const char *n)
 : BaseWidget(p,n), f_ilManager(knGlobals.filterManager())
{
  QGridLayout *topL=new QGridLayout(this, 6,2, 5,5);

  // == Filters =================================================

  f_lb=new KNDialogListBox(false, this);
  topL->addWidget(new QLabel(f_lb, i18n("&Filters:"),this),0,0);

  connect(f_lb, SIGNAL(selectionChanged()), SLOT(slotSelectionChangedFilter()));
  connect(f_lb, SIGNAL(selected(int)), SLOT(slotItemSelectedFilter(int)));
  topL->addMultiCellWidget(f_lb,1,5,0,0);

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

  m_lb=new KNDialogListBox(false, this);
  topL->addWidget(new QLabel(m_lb, i18n("&Menu:"),this),6,0);

  connect(m_lb, SIGNAL(selectionChanged()), SLOT(slotSelectionChangedMenu()));
  topL->addMultiCellWidget(m_lb,7,11,0,0);

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
  if(d_irty)
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


QValueList<int> KNConfig::FilterListWidget::menuOrder()
{
  KNArticleFilter *f;
  QValueList<int> lst;

  for(uint i=0; i<m_lb->count(); i++) {
    f= (static_cast<LBoxItem*>(m_lb->item(i)))->filter;
    if(f)
      lst << f->id();
    else
      lst << -1;
  }
 return lst;
}


int KNConfig::FilterListWidget::findItem(QListBox *l, KNArticleFilter *f)
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


KNConfig::PostNewsTechnicalWidget::PostNewsTechnicalWidget(PostNewsTechnical *d, QWidget *p, const char *n)
  : BaseWidget(p, n), d_ata(d)
{
  QVBoxLayout *topL=new QVBoxLayout(this, 5);

  // ==== General =============================================================

  QGroupBox *ggb=new QGroupBox(i18n("General"), this);
  QGridLayout *ggbL=new QGridLayout(ggb, 6,2, 8,5);
  topL->addWidget(ggb);

  ggbL->addRowSpacing(0, fontMetrics().lineSpacing()-4);
  c_harset=new QComboBox(ggb);
  c_harset->insertStringList(d->composerCharsets());
  ggbL->addWidget(new QLabel(c_harset, i18n("Cha&rset:"), ggb), 1,0);
  ggbL->addWidget(c_harset, 1,1);
  connect(c_harset, SIGNAL(activated(int)), SLOT(slotEmitChanged()));

  e_ncoding=new QComboBox(ggb);
  e_ncoding->insertItem(i18n("Allow 8-bit"));
  e_ncoding->insertItem(i18n("7-bit (Quoted-Printable)"));
  ggbL->addWidget(new QLabel(e_ncoding, i18n("Enco&ding:"), ggb), 2,0);
  ggbL->addWidget(e_ncoding, 2,1);
  connect(e_ncoding, SIGNAL(activated(int)), SLOT(slotEmitChanged()));

  u_seOwnCSCB=new QCheckBox(i18n("Use o&wn default charset when replying"), ggb);
  ggbL->addMultiCellWidget(u_seOwnCSCB, 3,3, 0,1);
  connect(u_seOwnCSCB, SIGNAL(toggled(bool)), SLOT(slotEmitChanged()));

  g_enMIdCB=new QCheckBox(i18n("&Generate message-id"), ggb);
  connect(g_enMIdCB, SIGNAL(toggled(bool)), this, SLOT(slotGenMIdCBToggled(bool)));
  ggbL->addMultiCellWidget(g_enMIdCB, 4,4, 0,1);
  h_ost=new KLineEdit(ggb);
  h_ost->setEnabled(false);
  h_ostL=new QLabel(h_ost, i18n("Ho&st name:"), ggb);
  h_ostL->setEnabled(false);
  ggbL->addWidget(h_ostL, 5,0);
  ggbL->addWidget(h_ost, 5,1);
  ggbL->setColStretch(1,1);
  connect(h_ost, SIGNAL(textChanged(const QString&)), SLOT(slotEmitChanged()));

  // ==== X-Headers =============================================================

  QGroupBox *xgb=new QGroupBox(i18n("X-Headers"), this);
  topL->addWidget(xgb, 1);
  QGridLayout *xgbL=new QGridLayout(xgb, 7,2, 8,5);

  xgbL->addRowSpacing(0, fontMetrics().lineSpacing()-4);

  l_box=new KNDialogListBox(false, xgb);
  connect(l_box, SIGNAL(selected(int)), SLOT(slotItemSelected(int)));
  connect(l_box, SIGNAL(selectionChanged()), SLOT(slotSelectionChanged()));
  xgbL->addMultiCellWidget(l_box, 1,4, 0,0);

  a_ddBtn=new QPushButton(i18n("&Add..."), xgb);
  connect(a_ddBtn, SIGNAL(clicked()), SLOT(slotAddBtnClicked()));
  xgbL->addWidget(a_ddBtn, 1,1);

  d_elBtn=new QPushButton(i18n("Dele&te"), xgb);
  connect(d_elBtn, SIGNAL(clicked()), SLOT(slotDelBtnClicked()));
  xgbL->addWidget(d_elBtn, 2,1);

  e_ditBtn=new QPushButton(i18n("modify something","&Edit..."), xgb);
  connect(e_ditBtn, SIGNAL(clicked()), SLOT(slotEditBtnClicked()));
  xgbL->addWidget(e_ditBtn, 3,1);

  QLabel *placeHolders = new QLabel("<qt>Placeholders: %NAME=name, %EMAIL=email address</qt>", xgb);
  xgbL->addMultiCellWidget(placeHolders, 5, 5, 0, 1);

  i_ncUaCB=new QCheckBox(i18n("Do not add the \"&User-Agent\" identification header"), xgb);
  xgbL->addMultiCellWidget(i_ncUaCB, 6,6, 0,1);
  connect(i_ncUaCB, SIGNAL(toggled(bool)), SLOT(slotEmitChanged()));

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
  if(!d_irty)
    return;

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
  XHeaderConfDialog *dlg=new XHeaderConfDialog(QString::null, this);
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


KNConfig::XHeaderConfDialog::XHeaderConfDialog(const QString &h, QWidget *p, const char *n)
  : KDialogBase(Plain, i18n("X-Headers"),Ok|Cancel, Ok, p, n)
{
  QFrame* page=plainPage();
  QHBoxLayout *topL=new QHBoxLayout(page, 5,8);
  topL->setAutoAdd(true);

  new QLabel("X-", page);
  n_ame=new KLineEdit(page);
  new QLabel(":", page);
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



QString KNConfig::XHeaderConfDialog::result()
{
  return QString("X-%1: %2").arg(n_ame->text()).arg(v_alue->text());
}


//===================================================================================================


KNConfig::PostNewsComposerWidget::PostNewsComposerWidget(PostNewsComposer *d, QWidget *p, const char *n)
  : BaseWidget(p, n), d_ata(d)
{
  QVBoxLayout *topL=new QVBoxLayout(this, 5);

  // === general ===========================================================

  QGroupBox *generalB=new QGroupBox(i18n("General"), this);
  topL->addWidget(generalB);
  QGridLayout *generalL=new QGridLayout(generalB, 3,3, 8,5);

  generalL->addRowSpacing(0, fontMetrics().lineSpacing()-4);

  w_ordWrapCB=new QCheckBox(i18n("Word &wrap at column:"), generalB);
  generalL->addWidget(w_ordWrapCB,1,0);
  m_axLen=new KIntSpinBox(20, 200, 1, 20, 10, generalB);
  generalL->addWidget(m_axLen,1,2);
  connect(w_ordWrapCB, SIGNAL(toggled(bool)), m_axLen, SLOT(setEnabled(bool)));
  connect(w_ordWrapCB, SIGNAL(toggled(bool)), SLOT(slotEmitChanged()));
  connect(m_axLen, SIGNAL(valueChanged(int)), SLOT(slotEmitChanged()));

  o_wnSigCB=new QCheckBox(i18n("Appe&nd signature automatically"), generalB);
  generalL->addMultiCellWidget(o_wnSigCB,2,2,0,1);
  connect(o_wnSigCB, SIGNAL(toggled(bool)), SLOT(slotEmitChanged()));

  generalL->setColStretch(1,1);

  // === reply =============================================================

  QGroupBox *replyB=new QGroupBox(i18n("Reply"), this);
  topL->addWidget(replyB);
  QGridLayout *replyL=new QGridLayout(replyB, 7,2, 8,5);

  replyL->addRowSpacing(0, fontMetrics().lineSpacing()-4);

  i_ntro=new KLineEdit(replyB);
  replyL->addMultiCellWidget(new QLabel(i_ntro,i18n("&Introduction phrase:"), replyB),1,1,0,1);
  replyL->addMultiCellWidget(i_ntro, 2,2,0,1);
  replyL->addMultiCellWidget(new QLabel(i18n("Placeholders: %NAME=name, %EMAIL=email address,\n%DATE=date, %MSID=message-id, %GROUP=group name, %L=line break"), replyB),3,3,0,1);
  connect(i_ntro, SIGNAL(textChanged(const QString&)), SLOT(slotEmitChanged()));

  r_ewrapCB=new QCheckBox(i18n("Rewrap quoted te&xt automatically"), replyB);
  replyL->addMultiCellWidget(r_ewrapCB, 4,4,0,1);
  connect(r_ewrapCB, SIGNAL(toggled(bool)), SLOT(slotEmitChanged()));

  a_uthSigCB=new QCheckBox(i18n("Include the a&uthor's signature"), replyB);
  replyL->addMultiCellWidget(a_uthSigCB, 5,5,0,1);
  connect(a_uthSigCB, SIGNAL(toggled(bool)), SLOT(slotEmitChanged()));

  c_ursorOnTopCB=new QCheckBox(i18n("Put the cursor &below the introduction phrase"), replyB);
  replyL->addMultiCellWidget(c_ursorOnTopCB, 6,6,0,1);
  connect(c_ursorOnTopCB, SIGNAL(toggled(bool)), SLOT(slotEmitChanged()));

  replyL->setColStretch(1,1);

  // === external editor ========================================================

  QGroupBox *editorB=new QGroupBox(i18n("External Editor"), this);
  topL->addWidget(editorB);
  QGridLayout *editorL=new QGridLayout(editorB, 6,3, 8,5);

  editorL->addRowSpacing(0, fontMetrics().lineSpacing()-4);

  e_ditor=new KLineEdit(editorB);
  editorL->addWidget(new QLabel(e_ditor, i18n("Specify edi&tor:"), editorB),1,0);
  editorL->addWidget(e_ditor,1,1);
  QPushButton *btn = new QPushButton(i18n("Choo&se..."),editorB);
  connect(btn, SIGNAL(clicked()), SLOT(slotChooseEditor()));
  connect(e_ditor, SIGNAL(textChanged(const QString&)), SLOT(slotEmitChanged()));
  editorL->addWidget(btn,1,2);

  editorL->addMultiCellWidget(new QLabel(i18n("%f will be replaced with the filename to edit."), editorB),2,2,0,2);

  e_xternCB=new QCheckBox(i18n("Start exte&rnal editor automatically"), editorB);
  editorL->addMultiCellWidget(e_xternCB, 3,3,0,2);
  connect(e_xternCB, SIGNAL(clicked()), SLOT(slotEmitChanged()));

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
  if(!d_irty)
    return;

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
  QString path=e_ditor->text().simplifyWhiteSpace();
  if (path.right(3) == " %f")
    path.truncate(path.length()-3);

  path=KFileDialog::getOpenFileName(path, QString::null, this, i18n("Choose Editor"));

  if (!path.isEmpty())
    e_ditor->setText(path+" %f");
}


//===================================================================================================


KNConfig::PostNewsSpellingWidget::PostNewsSpellingWidget(QWidget *p, const char *n)
  : BaseWidget(p, n)
{
  QVBoxLayout *topL=new QVBoxLayout(this, 5);

  c_onf = new KSpellConfig( this, "spell", 0, false );
  topL->addWidget(c_onf);
  connect(c_onf, SIGNAL(configChanged()), SLOT(slotEmitChanged()));

  topL->addStretch(1);
}


KNConfig::PostNewsSpellingWidget::~PostNewsSpellingWidget()
{
}


void KNConfig::PostNewsSpellingWidget::save()
{
  if(d_irty)
     c_onf->writeGlobalSettings();
}


//==============================================================================================================

KNConfig::PrivacyWidget::PrivacyWidget(QWidget *p, const char *n)
  : BaseWidget(p,n)
{
  QBoxLayout *topLayout = new QVBoxLayout(this, 5);
  c_onf = new Kpgp::Config(this,"knode pgp config",false);
  topLayout->addWidget(c_onf);
  connect(c_onf, SIGNAL(changed()), SLOT(slotEmitChanged()));

  QGroupBox *optBox = new QGroupBox(i18n("KNode Specific Options"), this);
  topLayout->addWidget(optBox);
  QBoxLayout *groupL = new QVBoxLayout(optBox, KDialog::spacingHint());
  groupL->addSpacing(fontMetrics().lineSpacing());
  a_utoCheckSigCB = new QCheckBox(i18n("Ch&eck signatures automatically"),optBox);
  groupL->addWidget(a_utoCheckSigCB);
  connect(a_utoCheckSigCB, SIGNAL(toggled(bool)), SLOT(slotEmitChanged()));
  topLayout->addStretch(1);

  load();
}


KNConfig::PrivacyWidget::~PrivacyWidget()
{
}


void KNConfig::PrivacyWidget::load()
{
  a_utoCheckSigCB->setChecked(knGlobals.configManager()->readNewsGeneral()->autoCheckPgpSigs());
}


void KNConfig::PrivacyWidget::save()
{
  if(!d_irty)
    return;

  c_onf->applySettings();
  knGlobals.configManager()->readNewsGeneral()->setAutoCheckPgpSigs(a_utoCheckSigCB->isChecked());
  knGlobals.configManager()->readNewsGeneral()->setDirty(true);
}


//==============================================================================================================


// BEGIN: Cleanup configuration widgets ---------------------------------------


KNConfig::GroupCleanupWidget::GroupCleanupWidget( Cleanup *data, QWidget *parent, const char *name )
  : QWidget( parent, name ), mData( data )
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

  mExpDays = new KIntSpinBox( 0, 99999, 1, 0, 10, mExpGroup );
  mExpDays->setSuffix( i18n(" days") );
  QLabel *label = new QLabel( mExpDays, i18n("&Purge groups every:"), mExpGroup );
  grid->addWidget( label, 2, 0 );
  grid->addWidget( mExpDays, 2, 1, Qt::AlignRight );
  connect( mExpDays, SIGNAL(valueChanged(int)), SIGNAL(changed()) );
  connect( mExpEnabled, SIGNAL(toggled(bool)), label, SLOT(setEnabled(bool)) );
  connect( mExpEnabled, SIGNAL(toggled(bool)), mExpDays, SLOT(setEnabled(bool)) );

  mExpReadDays = new KIntSpinBox( 0, 99999, 1, 0, 10, mExpGroup );
  mExpReadDays->setSuffix( i18n(" days") );
  label = new QLabel( mExpReadDays, i18n("&Keep read articles:"), mExpGroup );
  grid->addWidget( label, 3, 0 );
  grid->addWidget( mExpReadDays, 3, 1, Qt::AlignRight );
  connect( mExpReadDays, SIGNAL(valueChanged(int)), SIGNAL(changed()) );

  mExpUnreadDays = new KIntSpinBox( 0, 99999, 1, 0, 10, mExpGroup );
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


KNConfig::CleanupWidget::CleanupWidget(QWidget *p, const char *n)
  : BaseWidget(p, n), d_ata(knGlobals.configManager()->cleanup())
{
  QVBoxLayout *topL=new QVBoxLayout(this, 5);

  mGroupCleanup = new GroupCleanupWidget( d_ata, this );
  topL->addWidget( mGroupCleanup );
  connect( mGroupCleanup, SIGNAL(changed()), SLOT(slotEmitChanged()) );

  // === folders =========================================================

  QGroupBox *foldersB=new QGroupBox(i18n("Folders"), this);
  topL->addWidget(foldersB);
  QGridLayout *foldersL=new QGridLayout(foldersB, 3,2, 8,5);

  foldersL->setRowSpacing( 0, KDialog::spacingHint() );

  f_olderCB=new QCheckBox(i18n("Co&mpact folders automatically"), foldersB);
  connect(f_olderCB, SIGNAL(toggled(bool)), this, SLOT(slotFolderCBtoggled(bool)));
  foldersL->addMultiCellWidget(f_olderCB,1,1,0,1);

  f_olderDays=new KIntSpinBox(0, 99999, 1, 0, 10, foldersB);
  f_olderDays->setSuffix(i18n(" days"));
  f_olderDaysL=new QLabel(f_olderDays,i18n("P&urge folders every:"), foldersB);
  foldersL->addWidget(f_olderDaysL,2,0);
  foldersL->addWidget(f_olderDays,2,1,Qt::AlignRight);
  connect(f_olderDays, SIGNAL(valueChanged(int)), SLOT(slotEmitChanged()));

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
  if(!d_irty)
    return;

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


// END: Cleanup configuration widgets -----------------------------------------

//==============================================================================================================

/*
KNConfig::CacheWidget::CacheWidget(Cache *d, QWidget *p, const char *n)
  : BaseWidget(p, n), d_ata(d)
{
  QVBoxLayout *topL=new QVBoxLayout(this, 5);

  // memory
  QGroupBox *memGB=new QGroupBox(i18n("Memory Cache"), this);
  topL->addWidget(memGB);
  QGridLayout *memL=new QGridLayout(memGB, 3,2, 8,5);
  memL->addRowSpacing(0, fontMetrics().lineSpacing()-4);

  memL->addWidget(new QLabel(i18n("max articles to keep"), memGB), 1,0);
  m_emMaxArt=new KIntSpinBox(0, 99999, 1, 1, 10, memGB);
  memL->addWidget(m_emMaxArt, 1,1);

  memL->addWidget(new QLabel(i18n("max memory usage"), memGB), 2,0);
  m_emMaxKB=new KIntSpinBox(0, 99999, 1, 1, 10, memGB);
  m_emMaxKB->setSuffix(" KB");
  memL->addWidget(m_emMaxKB, 2,1);

  memL->setColStretch(0,1);


  // disk
  QGroupBox *diskGB=new QGroupBox(i18n("Disk Cache"), this);
  topL->addWidget(diskGB);
  QGridLayout *diskL=new QGridLayout(diskGB, 3,2, 8,5);
  diskL->addRowSpacing(0, fontMetrics().lineSpacing()-4);

  d_iskMaxArtL=new QLabel(i18n("max articles to keep"), diskGB);
  diskL->addWidget(d_iskMaxArtL, 2,0);
  d_iskMaxArt=new KIntSpinBox(0, 99999, 1, 1, 10, diskGB);
  diskL->addWidget(d_iskMaxArt, 2,1);

  d_iskMaxKBL=new QLabel(i18n("max disk usage"), diskGB);
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
#include "knconfig.moc"
