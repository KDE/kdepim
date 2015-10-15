/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#ifndef BALOODEBUGDIALOG_H
#define BALOODEBUGDIALOG_H

#include <QDialog>
#include "pimcommon_export.h"
#include "akonadisearchdebugsearchpathcombobox.h"
#include <AkonadiCore/Item>

namespace PimCommon
{
class AkonadiSearchDebugDialogPrivate;
class PIMCOMMON_EXPORT AkonadiSearchDebugDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AkonadiSearchDebugDialog(QWidget *parent = Q_NULLPTR);
    ~AkonadiSearchDebugDialog();

    void setAkonadiId(Akonadi::Item::Id akonadiId);
    void setSearchType(AkonadiSearchDebugSearchPathComboBox::SearchType type);
    void doSearch();
private Q_SLOTS:
    void slotSaveAs();
private:
    void readConfig();
    void writeConfig();
    void saveTextAs(const QString &text, const QString &filter);
    AkonadiSearchDebugDialogPrivate *const d;
    bool saveToFile(const QString &filename, const QString &text);
};
}

#endif // BALOODEBUGDIALOG_H

