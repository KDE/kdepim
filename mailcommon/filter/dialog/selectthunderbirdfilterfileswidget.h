/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>
  
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

#ifndef SELECTTHUNDERBIRDFILTERFILESWIDGET_H
#define SELECTTHUNDERBIRDFILTERFILESWIDGET_H

#include <QWidget>
class KUrl;
class QAbstractButton;

namespace Ui {
class SelectThunderbirdFilterFilesWidget;
}

class SelectThunderbirdFilterFilesWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit SelectThunderbirdFilterFilesWidget(QWidget *parent = 0);
    ~SelectThunderbirdFilterFilesWidget();
    QStringList selectedFiles() const;

    void setStartDir(const KUrl&);

Q_SIGNALS:
    void enableOkButton(bool);

private Q_SLOTS:
    void slotButtonClicked(QAbstractButton*button);
    void slotProfileChanged(int);

    void slotUrlChanged(const QString &path);
    void slotItemSelectionChanged();
private:
    Ui::SelectThunderbirdFilterFilesWidget *ui;
};

#endif // SELECTTHUNDERBIRDFILTERFILESWIDGET_H
