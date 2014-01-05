/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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

#ifndef ADBLOCKCREATEFILTERDIALOG_H
#define ADBLOCKCREATEFILTERDIALOG_H

#include "adblockblockableitemswidget.h"

#include <KDialog>
namespace Ui {
class AdBlockCreateFilterWidget;
}
namespace MessageViewer {
class AdBlockCreateFilterDialog : public KDialog
{
    Q_OBJECT
public:
    explicit AdBlockCreateFilterDialog(QWidget *parent=0);
    ~AdBlockCreateFilterDialog();

    void setPattern(AdBlockBlockableItemsWidget::TypeElement type, const QString &pattern);

    QString filter() const;

private Q_SLOTS:
    void slotUpdateFilter();

private:
    enum ElementType {
        ElementValue = Qt::UserRole + 1
    };

    void readConfig();
    void writeConfig();
    void initialize();
    QString mPattern;
    AdBlockBlockableItemsWidget::TypeElement mCurrentType;
    Ui::AdBlockCreateFilterWidget *mUi;
};
}

#endif // ADBLOCKCREATEFILTERDIALOG_H
