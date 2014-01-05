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

#ifndef SELECTRELATIONALMATCHTYPE_H
#define SELECTRELATIONALMATCHTYPE_H

#include <QWidget>
class KComboBox;
namespace KSieveUi {
class SelectRelationalMatchType : public QWidget
{
    Q_OBJECT
public:
    explicit SelectRelationalMatchType(QWidget *parent = 0);
    ~SelectRelationalMatchType();

    QString code() const;
    void setCode(const QString &type, const QString &comparatorStr, const QString &name, QString &error);

private:
    void initialize();
    KComboBox *mType;
    KComboBox *mMatch;
};
}


#endif // SELECTRELATIONALMATCHTYPE_H
