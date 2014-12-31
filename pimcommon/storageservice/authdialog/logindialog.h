/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

class QLineEdit;
class QLabel;
class QPushButton;
namespace PimCommon
{
class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = Q_NULLPTR);
    ~LoginDialog();

    void setUsernameLabel(const QString &labelName);

    QString password() const;
    QString username() const;

    void setPassword(const QString &pass);
    void setUserName(const QString &name);
private Q_SLOTS:
    void slotUserNameChanged(const QString &passwd);

private:
    QLineEdit *mUsername;
    QLineEdit *mPassword;
    QLabel *mLabUsername;
    QPushButton *mOkButton;
};

}
#endif // LOGINDIALOG_H
