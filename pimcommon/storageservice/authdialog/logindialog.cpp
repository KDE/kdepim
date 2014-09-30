/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "logindialog.h"

#include <KLocalizedString>
#include <QLineEdit>

#include <QGridLayout>
#include <QLabel>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

using namespace PimCommon;

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Authorize"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    mOkButton->setDefault(true);
    mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &LoginDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &LoginDialog::reject);
    mOkButton->setDefault(true);

    QWidget *w = new QWidget;
    mainLayout->addWidget(w);
    mainLayout->addWidget(buttonBox);

    QGridLayout *grid = new QGridLayout;
    w->setLayout(grid);

    mLabUsername = new QLabel(i18n("Username:"));
    grid->addWidget(mLabUsername, 0, 0);

    mUsername = new QLineEdit;
    mUsername->setClearButtonEnabled(true);
    grid->addWidget(mUsername, 0, 1);

    QLabel *lab = new QLabel(i18n("Password:"));
    grid->addWidget(lab, 1, 0);
    mPassword = new QLineEdit;
    grid->addWidget(mPassword, 1, 1);
    mPassword->setEchoMode(QLineEdit::Password);

    connect(mUsername, &QLineEdit::textChanged, this, &LoginDialog::slotUserNameChanged);
    mOkButton->setEnabled(false);
    resize(300, 100);
    mLabUsername->setFocus();
}

LoginDialog::~LoginDialog()
{

}

void LoginDialog::setUsernameLabel(const QString &labelName)
{
    mLabUsername->setText(labelName);
}

void LoginDialog::setPassword(const QString &pass)
{
    mPassword->setText(pass);
}

QString LoginDialog::password() const
{
    return mPassword->text();
}

void LoginDialog::setUserName(const QString &name)
{
    mUsername->setText(name);
}

QString LoginDialog::username() const
{
    return mUsername->text();
}

void LoginDialog::slotUserNameChanged(const QString &name)
{
    mOkButton->setEnabled(!name.isEmpty());
}
