/*
 * (C) 2000 Rik Hemsley <rik@kde.org>
 */

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qlabel.h>

#include <kdialog.h>
#include <klocale.h>
#include <kurlrequester.h>

#include "comcfg.h"
#include "maildrop.h"

  QWidget *
KCommandsCfg::makeWidget(QWidget * parent)
{
  QWidget * w = new QWidget(parent);

  int marginHint = KDialog::marginHint();
  int spacingHint = KDialog::spacingHint();

  QVBoxLayout * topLayout       = new QVBoxLayout(w, marginHint, spacingHint);
  
  topLayout->addStretch(1);
  
  //Groupbox for commands.
  QGroupBox *commands = new QGroupBox( 2, Horizontal, i18n( "Commands" ), w, "Commands" );
  topLayout->addWidget( commands );
  //Fill commands groupbox
  QLabel * l = new QLabel(i18n("&New mail:"), commands);
  _onReceipt  = new QLineEdit(commands);
  QLabel * l2 = new QLabel(i18n("C&lick:"), commands);
  _onClick    = new QLineEdit(commands);
  
  //Groupbox for passive popup
  QGroupBox *popup = new QGroupBox( 1, Horizontal, i18n( "Passive Popups (KIO only)" ), w, "Popup" );
  topLayout->addWidget( popup );
  //Fill popup groupbox
  _passivePopup = new QCheckBox( i18n( "&Passive popup on new mail" ), popup );
  _passiveDate  = new QCheckBox( i18n( "P&opup contains date of mail" ), popup );
  
  //Groupbox for misc. options.
  QGroupBox *misc = new QGroupBox( 2, Horizontal, i18n( "Miscellaneous options" ), w, "Misc" );
  topLayout->addWidget( misc );
  //Fill misc. options.
  QLabel * l3 = new QLabel(i18n("Play &sound:"), misc );
  _soundFile  = new KURLRequester(misc);
  new QLabel( misc ); //no caption for Reset counter checkbox
  _resetCounter = new QCheckBox(i18n("&Reset to 0 on click"), misc);
  
  topLayout->addStretch(20);
  
  l->setBuddy(_onReceipt);
  l2->setBuddy(_onClick);
  l3->setBuddy(_soundFile);
  
  _soundFile->setMode(KFile::File | KFile::ExistingOnly | KFile::LocalOnly);
  _soundFile->setFilter("*.wav");
  
  connect(_passivePopup, SIGNAL(toggled(bool)), _passiveDate, SLOT(setEnabled(bool)));

  readConfig();

  return w;
}

QString KCommandsCfg::name() const
{
  return i18n("&Actions"); //It does now more then only Commands.
  //The name of this file and class seems to be a little outdated, but I will change that at some day.
}

void KCommandsCfg::readConfig()
{
  _onClick->setText(drop()->clickCmd());
  _onReceipt->setText(drop()->newMailCmd());
  _passivePopup->setChecked(drop()->passivePopup());
  _passiveDate ->setEnabled(drop()->passivePopup());
  _passiveDate->setChecked(drop()->passiveDate());
  _soundFile->setURL(drop()->soundFile());
  _resetCounter->setChecked(drop()->resetCounter()>=0);
}

void KCommandsCfg::updateConfig()
{
  drop()->setClickCmd    (_onClick->text());
  drop()->setNewMailCmd  (_onReceipt->text());
  drop()->setPassivePopup(_passivePopup->isChecked());
  drop()->setPassiveDate (_passiveDate->isChecked());
  drop()->setSoundFile   (_soundFile->url());
  if(!_resetCounter->isChecked())
  	drop()->setResetCounter(-1); //Disable reset counter option
  else if(drop()->resetCounter()<0)  //Check if is is already enabled.
  	drop()->setResetCounter(0);  //Enable reset counter option
}
#include "comcfg.moc"
