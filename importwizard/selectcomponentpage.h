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
#ifndef SELECTCOMPONENTPAGE_H
#define SELECTCOMPONENTPAGE_H

#include <QWidget>
#include "abstractimporter.h"

namespace Ui {
class SelectComponentPage;
}

class SelectComponentPage : public QWidget
{
    Q_OBJECT

public:
    explicit SelectComponentPage(QWidget *parent = 0);
    ~SelectComponentPage();

    void setEnabledComponent(AbstractImporter::TypeSupportedOptions options);
    AbstractImporter::TypeSupportedOptions selectedComponents() const;

private Q_SLOTS:
    void slotEverythingClicked( bool clicked );
    void slotComponentClicked();

Q_SIGNALS:
    void atLeastOneComponentSelected(bool componentSelected);

private:
    AbstractImporter::TypeSupportedOptions mOptions;
    Ui::SelectComponentPage *ui;
};

#endif // SELECTCOMPONENTPAGE_H
