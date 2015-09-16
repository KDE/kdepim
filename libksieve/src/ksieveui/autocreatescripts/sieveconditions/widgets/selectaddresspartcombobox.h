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

#ifndef SELECTADDRESSPARTCOMBOBOX_H
#define SELECTADDRESSPARTCOMBOBOX_H

#include <QComboBox>

namespace KSieveUi
{
class SelectAddressPartComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit SelectAddressPartComboBox(QWidget *parent = Q_NULLPTR);
    ~SelectAddressPartComboBox();

    QString code() const;
    void setCode(const QString &code, const QString &name, QString &error);

    QString extraRequire() const;

Q_SIGNALS:
    void valueChanged();

private:
    void initialize();
    bool mHasSubaddressCapability;
};
}

#endif // SELECTADDRESSPARTCOMBOBOX_H
