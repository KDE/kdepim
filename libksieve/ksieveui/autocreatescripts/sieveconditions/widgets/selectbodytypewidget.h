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

#ifndef SELECTBODYTYPEWIDGET_H
#define SELECTBODYTYPEWIDGET_H

#include <QWidget>

class KComboBox;
class QLineEdit;

namespace KSieveUi
{
class SelectBodyTypeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SelectBodyTypeWidget(QWidget *parent = Q_NULLPTR);
    ~SelectBodyTypeWidget();

    QString code() const;
    void setCode(const QString &type, const QString &content, const QString &name, QString &error);

Q_SIGNALS:
    void valueChanged();

private Q_SLOTS:
    void slotBodyTypeChanged(int);

private:
    void initialize();
    KComboBox *mBodyCombobox;
    QLineEdit *mBodyLineEdit;
};
}

#endif // SELECTBODYTYPEWIDGET_H
