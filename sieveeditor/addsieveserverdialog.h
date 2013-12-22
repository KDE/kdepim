/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#ifndef ADDSIEVESERVERDIALOG_H
#define ADDSIEVESERVERDIALOG_H

#include <KDialog>
class ServerSieveSettings;
class AddSieveServerDialog : public KDialog
{
    Q_OBJECT
public:
    explicit AddSieveServerDialog(QWidget *parent=0);
    ~AddSieveServerDialog();

    QString serverName() const;
    void setServerName(const QString &name);

    int port() const;
    void setPort(int value);

    QString userName() const;
    void setUserName(const QString &name);

    QString password() const;
    void setPassword(const QString &pass);

private:
    ServerSieveSettings *mServerSieveSettings;
};

#endif // ADDSIEVESERVERDIALOG_H
