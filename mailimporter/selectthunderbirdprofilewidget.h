/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>
  
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

#ifndef SELECTTHUNDERBIRDPROFILEWIDGET_H
#define SELECTTHUNDERBIRDPROFILEWIDGET_H

#include "mailimporter_export.h"
#include <QDialog>

#include <QMap>
#include <KConfigGroup>

namespace Ui {
class SelectThunderbirdProfileWidget;
}

class MAILIMPORTER_EXPORT SelectThunderbirdProfileWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SelectThunderbirdProfileWidget(QWidget *parent = 0);
    ~SelectThunderbirdProfileWidget();

    void fillProfile(const QMap<QString,QString>& map, const QString&defaultProfile);
    QString selectedProfile() const;

private:
    Ui::SelectThunderbirdProfileWidget *ui;
};


class SelectThunderbirdProfileDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SelectThunderbirdProfileDialog(QWidget *parent = 0);
    ~SelectThunderbirdProfileDialog();
    QString selectedProfile() const;
    void fillProfile(const QMap<QString,QString>& map, const QString &defaultProfile);

private:
    SelectThunderbirdProfileWidget *mSelectProfile;
};



#endif // SELECTTHUNDERBIRDPROFILEWIDGET_H
