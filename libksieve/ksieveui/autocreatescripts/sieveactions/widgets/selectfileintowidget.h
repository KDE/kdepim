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

#ifndef SELECTFILEINTOWIDGET_H
#define SELECTFILEINTOWIDGET_H

#include <QDialog>

class QLineEdit;

namespace KSieveUi
{

class SelectFileIntoDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SelectFileIntoDialog(QWidget *parent = 0);
    ~SelectFileIntoDialog();

    QString selectedFolder() const;
};

class SelectFileIntoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SelectFileIntoWidget(QWidget *parent = 0);
    ~SelectFileIntoWidget();

    QString selectedFolder() const;

Q_SIGNALS:
    void valueChanged();

private Q_SLOTS:
    void slotSelectFolder();

private:
    QLineEdit *mLineEdit;
};
}

#endif // SELECTFILEINTOWIDGET_H
