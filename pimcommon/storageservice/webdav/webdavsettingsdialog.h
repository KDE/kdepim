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

#ifndef WEBDAVSETTINGSDIALOG_H
#define WEBDAVSETTINGSDIALOG_H

#include <QDialog>

class QLineEdit;
class QPushButton;
namespace PimCommon
{
class WebDavSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit WebDavSettingsDialog(QWidget *parent = 0);
    ~WebDavSettingsDialog();

    QString serviceLocation() const;

    QString publicLocation() const;

private Q_SLOTS:
    void slotServiceLocationChanged(const QString &text);

private:
    QLineEdit *mServiceLocation;
    QLineEdit *mPublicLocation;
    QPushButton *mOkButton;
};
}

#endif // WEBDAVSETTINGSDIALOG_H
