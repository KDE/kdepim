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
#include <KLineEdit>

#include <QGridLayout>
#include <QLabel>

using namespace PimCommon;

LoginDialog::LoginDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Authorize" ) );
    setButtons( Ok | Cancel );
    setDefaultButton(Ok);

    QWidget *w = new QWidget;
    QGridLayout *grid = new QGridLayout;
    w->setLayout(grid);

    mLabUsername = new QLabel(i18n("Username:"));
    grid->addWidget(mLabUsername, 0, 0);

    mUsername = new KLineEdit;
    grid->addWidget(mUsername, 0, 1);

    QLabel *lab = new QLabel(i18n("Password:"));
    grid->addWidget(lab, 1, 0);
    mPassword = new KLineEdit;
    grid->addWidget(mPassword, 1, 1);
    mPassword->setEchoMode(KLineEdit::Password);

    setMainWidget(w);
    connect(mUsername, SIGNAL(textChanged(QString)), this, SLOT(slotUserNameChanged(QString)));
    enableButtonOk(false);
    resize(300,100);
    mUsername->setFocus();
}

LoginDialog::~LoginDialog()
{

}

void LoginDialog::setUsernameLabel(const QString &labelName)
{
    mLabUsername->setText(labelName);
}

QString LoginDialog::password() const
{
    return mPassword->text();
}

QString LoginDialog::username() const
{
    return mUsername->text();
}

void LoginDialog::slotUserNameChanged(const QString &name)
{
    enableButtonOk(!name.isEmpty());
}
