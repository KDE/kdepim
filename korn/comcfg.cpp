/*
 * (C) 2000 Rik Hemsley <rik@kde.org>
 */

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
  QGridLayout * commandsLayout  = new QGridLayout(topLayout, 2, 3);

  topLayout->addStretch(1);

  _onReceipt  = new QLineEdit(w);
  _onClick    = new QLineEdit(w);
  _soundFile  = new KURLRequester(w);
  
  QLabel * l = new QLabel(i18n("&New mail:"), w);
  QLabel * l2 = new QLabel(i18n("C&lick:"), w);
  QLabel * l3 = new QLabel(i18n("Play &sound:"), w);

  l->setBuddy(_onReceipt);
  l2->setBuddy(_onClick);
  l3->setBuddy(_soundFile);
  
  _soundFile->setMode(KFile::File | KFile::ExistingOnly | KFile::LocalOnly);
  _soundFile->setFilter("*.wav");
  
  commandsLayout->addWidget(l,  0, 0);
  commandsLayout->addWidget(l2,    1, 0);
  commandsLayout->addWidget(l3,    2, 0);

  commandsLayout->addWidget(_onReceipt, 0, 1);
  commandsLayout->addWidget(_onClick,   1, 1);
  commandsLayout->addWidget(_soundFile, 2, 1);

  readConfig();

  return w;
}

QString KCommandsCfg::name() const
{
  return i18n("&Commands");
}

void KCommandsCfg::readConfig()
{
  _onClick->setText(drop()->clickCmd());
  _onReceipt->setText(drop()->newMailCmd());
  _soundFile->setURL(drop()->soundFile());
}

void KCommandsCfg::updateConfig()
{
  drop()->setClickCmd    (_onClick->text());
  drop()->setNewMailCmd  (_onReceipt->text());
  drop()->setSoundFile   (_soundFile->url());
}
#include "comcfg.moc"
