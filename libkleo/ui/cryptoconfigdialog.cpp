/*
    cryptoconfigdialog.h

    This file is part of kgpgcertmanager
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2, as published by the Free Software Foundation.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "cryptoconfigdialog.h"
#include "cryptoconfigmodule.h"
#include <KLocalizedString>
#include <kacceleratormanager.h>
#include <QDialogButtonBox>
#include <QPushButton>
#include <KGuiItem>
#include <QVBoxLayout>

Kleo::CryptoConfigDialog::CryptoConfigDialog( Kleo::CryptoConfig* config, QWidget *parent )
  : QDialog( parent)
{
  setWindowTitle( i18n( "Configure GnuPG Backend" ) );
  mButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::RestoreDefaults|QDialogButtonBox::Apply);
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  QPushButton *okButton = mButtonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  QPushButton *user1Button = new QPushButton;
  mButtonBox->addButton(user1Button, QDialogButtonBox::ActionRole);
  connect(mButtonBox, &QDialogButtonBox::accepted, this, &CryptoConfigDialog::accept);
  connect(mButtonBox, &QDialogButtonBox::rejected, this, &CryptoConfigDialog::reject);
  okButton->setDefault(true);
  setModal( true );
  KGuiItem::assign(user1Button, KGuiItem(i18n("&Reset")));
//add style :  Qt::WStyle_Customize | Qt::WStyle_DialogBorder | Qt::WStyle_Maximize | Qt::WStyle_Title | Qt::WStyle_SysMenu

  mMainWidget = new CryptoConfigModule( config, this );
  mainLayout->addWidget(mMainWidget);
  mainLayout->addWidget(mButtonBox);

  connect(mMainWidget, &CryptoConfigModule::changed, this, &CryptoConfigDialog::slotChanged);
  mButtonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
  if ( mMainWidget->hasError() ) {
      mButtonBox->button(QDialogButtonBox::RestoreDefaults)->setVisible(false);
      user1Button->setVisible(false);
      mButtonBox->button(QDialogButtonBox::Apply)->setVisible(false);
      okButton->setVisible(false);
  }

  // Automatically assign accelerators
  KAcceleratorManager::manage( this );
  connect(user1Button, &QPushButton::clicked, this, &CryptoConfigDialog::slotUser1);
  connect(mButtonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &CryptoConfigDialog::slotCancel);
  connect(okButton, &QPushButton::clicked, this, &CryptoConfigDialog::slotOk);
  connect(mButtonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, this, &CryptoConfigDialog::slotDefault);
  connect(mButtonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &CryptoConfigDialog::slotApply);
}

void Kleo::CryptoConfigDialog::slotOk()
{
  slotApply();
  accept();
}

void Kleo::CryptoConfigDialog::slotCancel()
{
  mMainWidget->cancel();
  reject();
}

void Kleo::CryptoConfigDialog::slotDefault()
{
  mMainWidget->defaults();
  slotChanged();
}

void Kleo::CryptoConfigDialog::slotApply()
{
  mMainWidget->save();
  mButtonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
}

void Kleo::CryptoConfigDialog::slotUser1() // reset
{
  mMainWidget->reset();
  mButtonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
}

void Kleo::CryptoConfigDialog::slotChanged()
{
  mButtonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

