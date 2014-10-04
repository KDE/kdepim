/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#ifndef CUSTOMHEADERSETTINGDIALOG_H
#define CUSTOMHEADERSETTINGDIALOG_H

#include <QDialog>

namespace MessageViewer
{
class CustomHeaderSettingWidget;
class CustomHeaderSettingDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CustomHeaderSettingDialog(QWidget *parent = 0);
    ~CustomHeaderSettingDialog();

    void writeSettings();

private:
    void readConfig();
    void writeConfig();

    CustomHeaderSettingWidget *mCustomHeaderWidget;
};
}

#endif // CUSTOMHEADERSETTINGDIALOG_H
