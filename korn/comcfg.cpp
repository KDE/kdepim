/*
 * (C) 2000 Rik Hemsley <rik@kde.org>
 */

#include <qlineedit.h>
#include <qlayout.h>
#include <qlabel.h>

#include <kdialog.h>
#include <klocale.h>

#include "comcfg.h"
#include "maildrop.h"

  QWidget *
KCommandsCfg::makeWidget(QWidget * parent)
{
  QWidget * w = new QWidget(parent);

  int marginHint = KDialog::marginHint();
  int spacingHint = KDialog::spacingHint();

  QVBoxLayout * topLayout       = new QVBoxLayout(w, marginHint, spacingHint);
  QGridLayout * commandsLayout  = new QGridLayout(topLayout, 2, 2);

  topLayout->addStretch(1);

  _onReceipt  = new QLineEdit(w);
  _onClick    = new QLineEdit(w);
  
  QLabel * l = new QLabel(i18n("&New mail:"), w);
  QLabel * l2 = new QLabel(i18n("C&lick:"), w);

  l->setBuddy(_onReceipt);
  l2->setBuddy(_onClick);

  commandsLayout->addWidget(l,  0, 0);
  commandsLayout->addWidget(l2,    1, 0);

  commandsLayout->addWidget(_onReceipt, 0, 1);
  commandsLayout->addWidget(_onClick,   1, 1);

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
}

void KCommandsCfg::updateConfig()
{
  drop()->setClickCmd    (_onClick->text());
  drop()->setNewMailCmd  (_onReceipt->text());
}
#include "comcfg.moc"
